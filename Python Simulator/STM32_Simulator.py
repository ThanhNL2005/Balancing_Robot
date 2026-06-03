import serial
import serial.tools.list_ports
import time
import struct
import random
import threading
import sys

# Cấu hình Baudrate mặc định
BAUD_RATE = 115200
CYCLE_MS = 0.05  # 50ms

# Hàm tính Modbus CRC16
def modbus_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for pos in data:
        crc ^= pos
        for _ in range(8):
            if (crc & 1) != 0:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc

# Hàm tạo gói tin 18 bytes
def create_frame(x_angle, z_angle, x_vel, z_vel, x_coord, l_torque, r_torque):
    # Pack 15 bytes đầu: Start (1) + 7 thông số int16 (14)
    # <B: little endian unsigned char (1 byte)
    # h: little endian short (2 bytes)
    payload = struct.pack('<Bhhhhhhh', 
                          0xAA, 
                          int(x_angle), int(z_angle), 
                          int(x_vel), int(z_vel), 
                          int(x_coord), 
                          int(l_torque), int(r_torque))
    
    # Tính CRC16 cho 15 bytes này
    crc = modbus_crc16(payload)
    
    # Pack CRC16 (2 bytes, little endian) và End byte (1 byte)
    tail = struct.pack('<H B', crc, 0xED)
    
    return payload + tail

# Cấu hình các chế độ mô phỏng
modes = {
    "1": {"name": "Gửi dữ liệu cho ESP32 Slave (giả lập quanh điểm cân bằng)", 
          "base": [0, 0, 0, 0, 0, 0, 0], "noise": 30}
}

current_mode = "1"

def tx_thread(ser):
    global current_mode
    while ser.is_open:
        base_vals = modes[current_mode]["base"]
        noise_level = modes[current_mode]["noise"]
        
        # Tạo nhiễu giả lập cảm biến
        noisy_vals = [
            b + random.randint(-noise_level, noise_level) for b in base_vals
        ]
        
        # Đóng gói frame
        frame = create_frame(*noisy_vals)
        
        # Gửi xuống cổng COM
        ser.write(frame)
        time.sleep(CYCLE_MS)

def rx_thread(ser):
    buffer = bytearray()
    while ser.is_open:
        try:
            if ser.in_waiting > 0:
                new_data = ser.read(ser.in_waiting)
                buffer.extend(new_data)
                
            while len(buffer) >= 9:
                if buffer[0] != 0xBB:
                    print(f"\n[LỖI FRAME] Sai Start Byte: {hex(buffer[0])}. Đang loại bỏ nhiễu...")
                    buffer.pop(0)
                    continue
                    
                # Chiều dài phụ thuộc byte 2 (index 2)
                frame_len = 9 if buffer[2] == 0x00 else 12
                
                if len(buffer) < frame_len:
                    break
                    
                if buffer[frame_len - 1] != 0xED:
                    print(f"\n[LỖI FRAME] Sai End Byte: {hex(buffer[frame_len - 1])}. Khung truyền bị hỏng!")
                    buffer.pop(0)
                    continue
                    
                # Tính CRC
                crc_calc = modbus_crc16(buffer[0:frame_len - 3])
                crc_rx = buffer[frame_len - 3] | (buffer[frame_len - 2] << 8)
                
                if crc_calc == crc_rx:
                    parse_rx_frame(buffer[0:frame_len])
                    buffer = buffer[frame_len:]
                else:
                    print(f"\n[LỖI FRAME] Sai CRC! Tính được: {hex(crc_calc)}, Gói tin chứa: {hex(crc_rx)}")
                    buffer.pop(0)
        except Exception as e:
            print(f"\n[RX ERROR] Ngoại lệ luồng nhận: {e}")
        time.sleep(0.01)

def parse_rx_frame(frame):
    data_group = frame[1]
    is_long = (frame[2] == 0x01)
    
    if data_group == 0x00:
        cmd = frame[3]
        if cmd == 0x01:
            print(f"\n[>> NHẬN LỆNH CỦA MASTER] -> CHẠY XE (Control = 1)")
            print("Nhập số để chọn chế độ: ", end="", flush=True)
        elif cmd == 0x00:
            print(f"\n[>> NHẬN LỆNH CỦA MASTER] -> DỪNG XE (Control = 0)")
            print("Nhập số để chọn chế độ: ", end="", flush=True)
            
    elif not is_long:
        index = frame[3]
        raw_val = struct.unpack('<h', bytes(frame[4:6]))[0]
        real_val = raw_val / 100.0
        
        group_name = "SMC" + str(data_group) if 0x01 <= data_group <= 0x03 else "Unknown"
        param_name = ["c", "ETA", "k"][index] if index <= 2 else "Unknown"
        
        print(f"\n[>> NHẬN CẬP NHẬT 1 THAM SỐ] -> {group_name} | {param_name} = {real_val:.2f}")
        print("Nhập số để chọn chế độ: ", end="", flush=True)
        
    elif is_long:
        val_a = struct.unpack('<h', bytes(frame[3:5]))[0] / 100.0
        val_beta = struct.unpack('<h', bytes(frame[5:7]))[0] / 100.0
        val_k = struct.unpack('<h', bytes(frame[7:9]))[0] / 100.0
        
        group_name = "SMC" + str(data_group) if 0x01 <= data_group <= 0x03 else "Unknown"
        print(f"\n[>> NHẬN CẬP NHẬT 3 THAM SỐ] -> {group_name} | c = {val_a:.2f}, ETA = {val_beta:.2f}, k = {val_k:.2f}")
        print("Nhập số để chọn chế độ: ", end="", flush=True)

def main():
    global current_mode
    print("="*55)
    print(" PHẦN MỀM GIẢ LẬP STM32 - ROBOT CÂN BẰNG 2 BÁNH ")
    print("="*55)
    
    # 1. Quét các cổng COM hiện có
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("[!] Không tìm thấy cổng COM nào trên máy tính!")
        com_port = input("Nhập thủ công cổng COM (VD: COM5): ").strip().upper()
    else:
        print("\nDanh sách cổng COM đang kết nối:")
        for i, port in enumerate(ports):
            print(f"[{i + 1}] {port.device} - {port.description}")
        
        choice = input(f"\nChọn cổng COM (1-{len(ports)}) hoặc nhập tên cổng (VD: COM5): ").strip().upper()
        
        # Nếu nhập số
        if choice.isdigit() and 1 <= int(choice) <= len(ports):
            com_port = ports[int(choice) - 1].device
        # Nếu nhập thẳng COMx
        elif choice.startswith("COM"):
            com_port = choice
        else:
            print("[!] Lựa chọn không hợp lệ, thoát chương trình.")
            return

    # 2. Khởi tạo kết nối COM
    try:
        ser = serial.Serial(com_port, BAUD_RATE, timeout=0.1)
        print(f"\n[*] Mở thành công cổng {com_port} với Baudrate {BAUD_RATE}")
    except Exception as e:
        print(f"\n[!] LỖI: Không thể mở cổng {com_port}.")
        print("    -> Vui lòng kiểm tra cáp và TẮT phần mềm Arduino Serial Monitor trước!")
        return

    # Chạy luồng gửi và nhận dữ liệu
    t_tx = threading.Thread(target=tx_thread, args=(ser,), daemon=True)
    t_tx.start()
    
    t_rx = threading.Thread(target=rx_thread, args=(ser,), daemon=True)
    t_rx.start()

    while True:
        print("\n--- CHỌN CHẾ ĐỘ CHƯƠNG TRÌNH ---")
        for key, val in modes.items():
            print(f"[{key}] {val['name']}")
        print("[0] Thoát chương trình")
        
        choice = input("Nhập số để chọn chế độ: ")
        
        if choice == "0":
            print("Đang đóng cổng COM...")
            ser.close()
            sys.exit(0)
        elif choice in modes:
            current_mode = choice
            print(f">>> ĐÃ ÁP DỤNG CHẾ ĐỘ: {modes[choice]['name']}")
        else:
            print("[!] Lựa chọn không hợp lệ!")

if __name__ == "__main__":
    main()
