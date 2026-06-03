using System;
using System.CodeDom;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Design;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using ZedGraph;

namespace BalancingRobotUI
{
    public partial class RobotSystemConfig : Form
    {
        // Khởi tạo form giao diên chính để liên kết với form hiện tại này
        private MainDashboard mainForm;

        // Control lenh dieu khien
        private Button btnRobotRun = new Button();
        private Button btnRobotStop = new Button();
        private Button btnSendChangedParams = new Button();
        private Button btnSendAllVisibleParams = new Button();
        private Button btnSendSMC1 = new Button();
        private Button btnSendSMC2 = new Button();
        private Button btnSendSMC3 = new Button();

        // Panel dieu khien
        private Panel pnSMC1Values = new Panel();
        private Panel pnSMC2Values = new Panel();
        private Panel pnSMC3Values = new Panel();
        private Panel pnDownloadControls = new Panel();

        // Dữ liệu loại 1: SMC1
        private System.Windows.Forms.Label lblSMC1 = new System.Windows.Forms.Label();
        private List<TextBox> txtDataType1 = new List<TextBox>();
        private List<System.Windows.Forms.Label> lblDataType1 = new List<System.Windows.Forms.Label>();

        // Dữ liệu loại 2: SMC2
        private System.Windows.Forms.Label lblSMC2 = new System.Windows.Forms.Label();
        private List<TextBox> txtDataType2 = new List<TextBox>();
        private List<System.Windows.Forms.Label> lblDataType2 = new List<System.Windows.Forms.Label>();

        // Dữ liệu loại 3: SMC3
        private System.Windows.Forms.Label lblSMC3 = new System.Windows.Forms.Label();
        private List<TextBox> txtDataType3 = new List<TextBox>();
        private List<System.Windows.Forms.Label> lblDataType3 = new List<System.Windows.Forms.Label>();

        private List<TextBox>[] textBoxGroup = new List<TextBox>[3];
        private List<System.Windows.Forms.Label>[] labelGroup = new List<System.Windows.Forms.Label>[3];

        // List lưu trữ giá trị các ô thông số bộ điều khiển
        private List<string> lstControlParamsLastUpdated = new List<string>();
        private List<string> lstControlParamsTextBox = new List<string>();

        // Lưu giá trị cũ của từng TextBox (trong RAM, chỉ lưu trong phiên làm việc)
        private Dictionary<TextBox, string> lastTextValues = new Dictionary<TextBox, string>();

        // Định nghĩa frame lệnh gửi cho xe
        byte startByte = 0xBB; // byte bắt đầu
        byte endByte = 0xED; // byte kết thúc
        public RobotSystemConfig(MainDashboard mainForm)
        {
            InitializeComponent();
            this.ClientSize = new Size(800, 390);
            this.mainForm = mainForm;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.AutoScaleMode = AutoScaleMode.Dpi;
            this.Icon = Properties.Resources.BalanceCarIcon;
            // Khởi tạo và định dạng chung cho các controls
            CreatePanels();
            AdjustPanelsLayout();
            CreateControlObjects();
            CreateValueTextBoxes();
            FormatAllControls(this);
            // Định dạng một số controls riêng
            lblSMC1.Font = new Font("Segoe UI", 11, FontStyle.Bold | FontStyle.Italic); lblSMC1.ForeColor = Color.Green; lblSMC1.BackColor = Color.WhiteSmoke;
            lblSMC2.Font = new Font("Segoe UI", 11, FontStyle.Bold | FontStyle.Italic); lblSMC2.ForeColor = Color.Green; lblSMC2.BackColor = Color.WhiteSmoke;
            lblSMC3.Font = new Font("Segoe UI", 11, FontStyle.Bold | FontStyle.Italic); lblSMC3.ForeColor = Color.Green; lblSMC3.BackColor = Color.WhiteSmoke;
            //lblHSMC.Font = new Font("Segoe UI", 11, FontStyle.Bold | FontStyle.Italic); lblHSMC.ForeColor = Color.Green; lblHSMC.BackColor = Color.WhiteSmoke;
            // Cập nhật dữ liệu tham số các bộ điều khiển và lưu lại cho lần mở giao diện kế tiếp
            ControlParametersChanged();
            ControlParametersUpdated();
            // Lưu giá trị ban đầu của mỗi TextBox trong RAM
            foreach (var group in textBoxGroup)
            {
                foreach (var tb in group)
                {
                    lastTextValues[tb] = tb.Text;
                }
            }
        }
        private void RobotSystemConfig_Load(object sender, EventArgs e)
        {
            //AdjustPanelsLayout();
        }
        private void FormatAllControls(Control parent)
        {
            foreach (Control ctrl in parent.Controls)
            {
                if (ctrl is System.Windows.Forms.Label lbl)
                {
                    lbl.AutoSize = true;
                    lbl.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    lbl.ForeColor = Color.DarkBlue;
                }
                else if (ctrl is TextBox txt)
                {
                    //txt.AutoSize = true;
                    txt.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    txt.ForeColor = Color.Black;
                }
                else if (ctrl is ComboBox cmb)
                {
                    //cmb.AutoSize = true;
                    cmb.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    cmb.ForeColor = Color.Black;
                }
                else if (ctrl is RadioButton rad)
                {
                    //rad.AutoSize = true;
                    rad.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    rad.ForeColor = Color.Purple;
                }
                else if (ctrl is GroupBox grp)
                {
                    //grp.AutoSize = true;
                    grp.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    grp.ForeColor = Color.BlueViolet;
                }
                else if (ctrl is Button btn)
                {
                    btn.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    btn.ForeColor = Color.Brown;
                }
                if (ctrl.HasChildren)
                {
                    FormatAllControls(ctrl);
                }
            }
        }
        private void AdjustPanelsLayout()
        {
            // Phương thức thay đổi kích cỡ các panel
            pnSMC1Values.Width = (int)(this.ClientSize.Width * 0.27);
            pnSMC1Values.Height = (int)(this.ClientSize.Height * 1);
            pnSMC2Values.Width = (int)(this.ClientSize.Width * 0.27);
            pnSMC2Values.Height = (int)(this.ClientSize.Height * 1);
            pnSMC3Values.Width = (int)(this.ClientSize.Width * 0.27);
            pnSMC3Values.Height = (int)(this.ClientSize.Height * 1);
            pnDownloadControls.Width = (int)(this.ClientSize.Width * 0.19);
            pnDownloadControls.Height = (int)(this.ClientSize.Height * 1);

            pnSMC1Values.Location = new Point(0, 0);
            pnSMC2Values.Location = new Point(pnSMC1Values.Width, 0);
            pnSMC3Values.Location = new Point(pnSMC1Values.Width + pnSMC2Values.Width, 0);
            pnDownloadControls.Location = new Point(pnSMC1Values.Width + pnSMC2Values.Width + pnSMC3Values.Width, 0);
        }
        private void CreatePanels()
        {
            // Tạo ra các panel phân chia giao diện chính
            this.Controls.Add(pnSMC1Values);
            this.Controls.Add(pnSMC2Values);
            this.Controls.Add(pnSMC3Values);
            this.Controls.Add(pnDownloadControls);

            pnSMC1Values.BackColor = Color.PapayaWhip;
            pnSMC2Values.BackColor = Color.PapayaWhip;
            pnSMC3Values.BackColor = Color.PapayaWhip;
            pnDownloadControls.BackColor = Color.OldLace;

            pnSMC1Values.BorderStyle = BorderStyle.FixedSingle;
            pnSMC2Values.BorderStyle = BorderStyle.FixedSingle;
            pnSMC3Values.BorderStyle = BorderStyle.FixedSingle;
            pnDownloadControls.BorderStyle = BorderStyle.FixedSingle;
        }
        private void CreateControlObjects()
        {
            // Các nút lệnh gửi riêng các thông số các bộ điều khiển SMC
            btnSendSMC1.Size = new Size(80, 50); btnSendSMC1.Location = new Point((pnSMC1Values.Width - btnSendSMC1.Width) / 2, 250);
            btnSendSMC2.Size = new Size(80, 50); btnSendSMC2.Location = new Point((pnSMC2Values.Width - btnSendSMC2.Width) / 2, 250);
            btnSendSMC3.Size = new Size(80, 50); btnSendSMC3.Location = new Point((pnSMC3Values.Width - btnSendSMC3.Width) / 2, 250);
            btnSendSMC1.Text = "Gửi SMC 1"; btnSendSMC1.BackColor = Color.WhiteSmoke;
            btnSendSMC2.Text = "Gửi SMC 2"; btnSendSMC2.BackColor = Color.WhiteSmoke;
            btnSendSMC3.Text = "Gửi SMC 3"; btnSendSMC3.BackColor = Color.WhiteSmoke;
            pnSMC1Values.Controls.Add(btnSendSMC1);
            pnSMC2Values.Controls.Add(btnSendSMC2);
            pnSMC3Values.Controls.Add(btnSendSMC3);

            btnSendSMC1.Click += BtnSendPID1_Click;
            btnSendSMC2.Click += BtnSendPID2_Click;
            btnSendSMC3.Click += BtnSendPID3_Click;


            // Các nút lệnh RUN, nút lệnh STOP và nút lệnh gửi frame lưu thông số các bộ điều khiển
            btnRobotRun.Location = new Point(30, 60); btnRobotRun.Size = new Size(90, 50);
            btnRobotStop.Location = new Point(30, 130); btnRobotStop.Size = new Size(90, 50);
            btnSendChangedParams.Location = new Point(20, 200); btnSendChangedParams.Size = new Size(110, 50);
            btnSendAllVisibleParams.Location = new Point(20, 270); btnSendAllVisibleParams.Size = new Size(110, 50);
            btnRobotRun.Text = "Chạy xe"; btnRobotRun.BackColor = Color.MistyRose;
            btnRobotStop.Text = "Dừng xe"; btnRobotStop.BackColor = Color.MistyRose;
            btnSendChangedParams.Text = "Gửi thông số đã thay đổi"; btnSendChangedParams.BackColor = Color.WhiteSmoke;
            btnSendAllVisibleParams.Text = "Gửi lại tất cả thông số"; btnSendAllVisibleParams.BackColor = Color.WhiteSmoke;
            pnDownloadControls.Controls.Add(btnRobotRun);
            pnDownloadControls.Controls.Add(btnRobotStop);
            pnDownloadControls.Controls.Add(btnSendChangedParams);
            pnDownloadControls.Controls.Add(btnSendAllVisibleParams);

            btnRobotRun.Click += btnRobotRun_Click;
            btnRobotStop.Click += btnRobotStop_Click;
            btnSendChangedParams.Click += btnSendChangedParams_Click;
            btnSendAllVisibleParams.Click += BtnSendAllVisibleParams_Click;
        }
        private void ControlParametersUpdated()
        {
            // Lấy dữ liệu từ trong Properties đã lưu trước đó và hiển thị lại trên các ô TextBox
            // SMC 1
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKpPID1))    txtDataType1[0].Text = Properties.Settings.Default.LastKpPID1;
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKiPID1))    txtDataType1[1].Text = Properties.Settings.Default.LastKiPID1;
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKdPID1))    txtDataType1[2].Text = Properties.Settings.Default.LastKdPID1;
            // SMC 2
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKpPID2))    txtDataType2[0].Text = Properties.Settings.Default.LastKpPID2;
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKiPID2))    txtDataType2[1].Text = Properties.Settings.Default.LastKiPID2;
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKdPID2))    txtDataType2[2].Text = Properties.Settings.Default.LastKdPID2;
            // SMC 3
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKpPID3))    txtDataType3[0].Text = Properties.Settings.Default.LastKpPID3;
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKiPID3))    txtDataType3[1].Text = Properties.Settings.Default.LastKiPID3;
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastKdPID3))    txtDataType3[2].Text = Properties.Settings.Default.LastKdPID3;
        }
        private void ControlParametersChanged()
        {
            for (int i = 0; i < textBoxGroup.Length; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    textBoxGroup[i][j].TextChanged += TxtControlParameters_Changed;
                }
            }
        }
        private void TxtControlParameters_Changed(object sender, EventArgs e)
        {
            if (sender is TextBox tb)
            {
                // Kiểm tra textbox + Cập nhật dữ liệu vào Properties đối với bộ điều khiển PID 1
                if (tb == txtDataType1[0])          Properties.Settings.Default.LastKpPID1 = txtDataType1[0].Text;
                else if (tb == txtDataType1[1])     Properties.Settings.Default.LastKiPID1 = txtDataType1[1].Text;
                else if (tb == txtDataType1[2])     Properties.Settings.Default.LastKdPID1 = txtDataType1[2].Text;
                // Kiểm tra textbox + Cập nhật dữ liệu vào Properties đối với bộ điều khiển PID 2
                else if (tb == txtDataType2[0])     Properties.Settings.Default.LastKpPID2 = txtDataType2[0].Text;
                else if (tb == txtDataType2[1])     Properties.Settings.Default.LastKiPID2 = txtDataType2[1].Text;
                else if (tb == txtDataType2[2])     Properties.Settings.Default.LastKdPID2 = txtDataType2[2].Text;
                // Kiểm tra textbox + Cập nhật dữ liệu vào Properties đối với bộ điều khiển PID 3
                else if (tb == txtDataType3[0])     Properties.Settings.Default.LastKpPID3 = txtDataType3[0].Text;
                else if (tb == txtDataType3[1])     Properties.Settings.Default.LastKiPID3 = txtDataType3[1].Text;
                else if (tb == txtDataType3[2])     Properties.Settings.Default.LastKdPID3 = txtDataType3[2].Text;
            }
            // Lưu dữ liệu trong Properties lại để phục vụ cho lần kế tiếp mở giao diện
            Properties.Settings.Default.Save();
        }
        private void CreateValueTextBoxes()
        {
            textBoxGroup[0] = txtDataType1; labelGroup[0] = lblDataType1;
            textBoxGroup[1] = txtDataType2; labelGroup[1] = lblDataType2;
            textBoxGroup[2] = txtDataType3; labelGroup[2] = lblDataType3;

            // Tạo TextBox và Label
            int count = 3; // Số lượng TextBox mỗi loại
            int startX = 30, startY = 120, spacingX = 65, spacingY = 40;
            for (int j = 1; j <= 3; j++) 
            {
                switch (j)
                {
                    case 1:
                        pnSMC1Values.Controls.Add(lblSMC1);
                        lblSMC1.Text = "SMC1";
                        lblSMC1.AutoSize = true;
                        lblSMC1.Location = new Point((pnSMC1Values.Width - lblSMC1.Width) / 2 - 5, 70);
                        break;
                    case 2:
                        pnSMC2Values.Controls.Add(lblSMC2);
                        lblSMC2.Text = "SMC2";
                        lblSMC2.AutoSize = true;
                        lblSMC2.Location = new Point((pnSMC2Values.Width - lblSMC2.Width) / 2 - 5, 70);
                        break;
                    case 3:
                        pnSMC3Values.Controls.Add(lblSMC3);
                        lblSMC3.Text = "SMC3";
                        lblSMC3.AutoSize = true;
                        lblSMC3.Location = new Point((pnSMC3Values.Width - lblSMC3.Width) / 2 - 5, 70);
                        break;
                }
                for (int i = 0; i < count; i++)
                {
                    // Tạo Label và Textbox
                    System.Windows.Forms.Label lbl = new System.Windows.Forms.Label();
                    TextBox txt = new TextBox();
                    switch (j)
                    {
                        case 1:
                            pnSMC1Values.Controls.Add(lbl); pnSMC1Values.Controls.Add(txt);
                            switch (i)
                            {
                                case 0: lbl.Text = "a1"; break;
                                case 1: lbl.Text = "beta1"; break;
                                case 2: lbl.Text = "k1"; break;
                            }
                            break;
                        case 2:
                            pnSMC2Values.Controls.Add(lbl); pnSMC2Values.Controls.Add(txt);
                            switch (i)
                            {
                                case 0: lbl.Text = "a2"; break;
                                case 1: lbl.Text = "beta2"; break;
                                case 2: lbl.Text = "k2"; break;
                            }
                            break;
                        case 3:
                            pnSMC3Values.Controls.Add(lbl); pnSMC3Values.Controls.Add(txt);
                            switch (i)
                            {
                                case 0: lbl.Text = "a3"; break;
                                case 1: lbl.Text = "beta3"; break;
                                case 2: lbl.Text = "k3"; break;
                            }
                            break;
                    }
                    if (j>=1 && j<=3)
                    {
                        lbl.Location = new Point(startX, startY + i * spacingY);
                        lbl.AutoSize = true;
                        labelGroup[j - 1].Add(lbl);
                        txt.Location = new Point(startX + spacingX, startY + i * spacingY);
                        txt.Size = new Size(80, 20);
                        textBoxGroup[j - 1].Add(txt);
                    }
                    else
                    {
                        lbl.Location = new Point(startX + (j - 4) * pnSMC1Values.Width, startY + i * spacingY);
                        lbl.AutoSize = true;
                        labelGroup[j - 1].Add(lbl);
                        txt.Location = new Point(startX + (j - 4) * pnSMC1Values.Width + spacingX, startY + i * spacingY);
                        txt.Size = new Size(80, 20);
                        textBoxGroup[j - 1].Add(txt);
                    }
                }
            }
        }
        private int ChangeCount(List<TextBox> txtDataType)
        {
            int change = 0;
            foreach (var tb in txtDataType)
            {
                if (tb.Text != lastTextValues[tb])
                {
                    change++;
                }
            }
            return change;
        }
        private List<int> ChangeIndex(List<TextBox> txtDataType)
        {
            List<int> changedIndex = new List<int>();
            for (int i = 0; i < txtDataType.Count; i++)
            {
                var tb = txtDataType[i];
                if (tb.Text != lastTextValues[tb])
                {
                    changedIndex.Add(i);
                }
            }
            return changedIndex;
        }
        private int BlankCount(List<TextBox> txtDataType)
        {
            // Trả về số ô trống trong mảng textbox đã chọn
            int blank = 0;
            foreach (var tb in txtDataType)
            {
                if (string.IsNullOrEmpty(tb.Text))
                {
                    blank++;
                }
            }
            return blank;
        }
        private ushort Modbus_CRC16(byte[] data, ushort frameLength, ushort pos_start, ushort pos_end)
        {
            ushort crc = 0xFFFF;
            for (ushort pos = pos_start; pos <= pos_end; pos++)
            {
                crc ^= (ushort)data[pos];
                for (byte i = 0; i < 8; i++)
                {
                    if ((crc & 0x0001) == 0x0001)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else
                    {
                        crc >>= 1;
                    }
                }
            }
            return crc;
        }
        private void SendShortFrame(byte startByte, byte endByte, int DataType, int Index, ushort dataSend)
        {
            // Frame 9 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3: thứ tự trong loại/ 4-5: 2 byte dữ liệu/ 6-7: checksum/ 8: end}
            byte[] transBuff = new byte[9];
            transBuff[0] = startByte; transBuff[8] = endByte;
            transBuff[1] = (byte)DataType; transBuff[2] = 0x00;
            transBuff[3] = (byte)Index;
            transBuff[4] = (byte)(dataSend); transBuff[5] = (byte)(dataSend >> 8);
            ushort crc16 = Modbus_CRC16(transBuff, 9, 0, 5);
            transBuff[6] = (byte)(crc16 & 0xFF);
            transBuff[7] = (byte)((crc16 >> 8) & 0xFF);
            mainForm.serCOM.Write(transBuff, 0, 9);
        }
        private void SendLongFrame(byte startByte, byte endByte, int DataType, List<TextBox> txtDataType)
        {
            // Frame 12 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3-4: 2 byte data_a/ 5-6: 2 byte data_beta/ 7-8: 2 byte data_k/ 9-10: checksum/ 11: end}
            byte[] transBuff = new byte[12];
            transBuff[0] = startByte; transBuff[11] = endByte;
            transBuff[1] = (byte)DataType; transBuff[2] = 0x01;
            for (int i = 0; i < txtDataType.Count; i++)     // i là thứ tự của giá trị trong loại dữ liệu đó
            {
                string text = txtDataType[i].Text;
                ushort DataTypejSend = (ushort)(Convert.ToDouble(txtDataType[i].Text) * 100.0);
                transBuff[3 + i * 2] = (byte)(DataTypejSend);
                transBuff[4 + i * 2] = (byte)(DataTypejSend >> 8);
            }
            ushort crc16 = Modbus_CRC16(transBuff, 12, 0, 8);
            transBuff[9] = (byte)(crc16 & 0xFF);
            transBuff[10] = (byte)((crc16 >> 8) & 0xFF);
            mainForm.serCOM.Write(transBuff, 0, 12);
        }
        private async void btnRobotRun_Click(object sender, EventArgs e)
        {
            if (mainForm.serCOM.IsOpen)
            {
                // Frame 8 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3: thứ tự trong loại/ 4-5: 2 byte dữ liệu/ 6: checksum/ 7: end}
                // Với lệnh RUN: byte [1] dữ liệu loại 0; byte [3] thứ tự là 1; byte [4] và byte [5] là 0x00 và 0x00
                
                SendShortFrame(startByte, endByte, 0, 1, 0);
                await Task.Delay(50);
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
        private async void btnRobotStop_Click(object sender, EventArgs e) 
        {
            if (mainForm.serCOM.IsOpen)
            {
                // Frame 8 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3: thứ tự trong loại/ 4-5: 2 byte dữ liệu/ 6: checksum/ 7: end}
                // Với lệnh STOP: byte [1] dữ liệu loại 0; byte [3] thứ tự là 0; byte [4] và byte [5] là 0x00 và 0x00

                SendShortFrame(startByte, endByte, 0, 0, 0);
                await Task.Delay(50);
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
        private async void btnSendChangedParams_Click(object sender, EventArgs e)
        {
            if (mainForm.serCOM.IsOpen)
            {
                for (int j = 0; j < 3; j++)   // j là loại dữ liệu trừ 1
                {
                    List<TextBox> txtDataType = textBoxGroup[j];
                    if (ChangeCount(txtDataType) == 1)
                    {
                        // chỉ có 1 textbox thay đổi nên ChangeIndex chỉ chứa 1 giá trị
                        int changedTextbox = ChangeIndex(txtDataType)[0];    // vị trí của textbox dc thay đổi đó
                        if (!string.IsNullOrEmpty(txtDataType[changedTextbox].Text))
                        {
                            ushort DataSend = (ushort)(Convert.ToDouble(txtDataType[changedTextbox].Text) * 100.0);
                            SendShortFrame(startByte, endByte, j + 1, changedTextbox, DataSend);
                            await Task.Delay(50);
                        }
                        await Task.Delay(50);
                    }
                    else if (ChangeCount(txtDataType) == 2 || ChangeCount(txtDataType) == 3)
                    {
                        if (BlankCount(txtDataType) == 0)
                        {
                            SendLongFrame(startByte, endByte, j + 1, txtDataType);
                            await Task.Delay(50);
                        }
                        else if (BlankCount(txtDataType) == 1 || BlankCount(txtDataType) == 2)
                        {
                            for (int i = 0; i < ChangeIndex(txtDataType).Count; i++)     // có 2-3 textbox thay đổi nên sẽ duyệt tìm vị trí của chúng
                            {
                                // ChangeIndex chứa 2-3 giá trị
                                int changedTextbox = ChangeIndex(txtDataType)[i];    // vị trí của textbox dc thay đổi
                                if (!string.IsNullOrEmpty(txtDataType[changedTextbox].Text))
                                {
                                    ushort DataSend = (ushort)(Convert.ToDouble(txtDataType[changedTextbox].Text) * 100.0);
                                    SendShortFrame(startByte, endByte, j + 1, changedTextbox, DataSend);
                                    await Task.Delay(50);
                                }
                            }
                        }
                    }
                }
                // Cập nhật giá trị textbox mới vào lastTextValues
                foreach (var group in textBoxGroup)
                {
                    foreach (var tb in group)
                    {
                        if (tb.Text != lastTextValues[tb])
                        {
                            lastTextValues[tb] = tb.Text;
                        }
                    }
                }
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
        private async void BtnSendAllVisibleParams_Click(object sender, EventArgs e)
        {
            if (mainForm.serCOM.IsOpen)
            {
                for (int j = 0; j < 3; j++)
                {
                    List<TextBox> txtDataType = textBoxGroup[j];
                    if (BlankCount(txtDataType) == 0)
                    {
                        SendLongFrame(startByte, endByte, j + 1, txtDataType);
                        await Task.Delay(50);
                    }
                    else if (BlankCount(txtDataType) == 1 || BlankCount(txtDataType) == 2)
                    {
                        for (int i = 0; i < txtDataType.Count; i++)
                        {
                            if (!string.IsNullOrEmpty(txtDataType[i].Text))
                            {
                                ushort dataSend = (ushort)(Convert.ToDouble(txtDataType[i].Text) * 100.0);
                                SendShortFrame(startByte, endByte, j + 1, i, dataSend);
                                await Task.Delay(50);
                            }
                        }
                    }
                }
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
        private async void BtnSendPID1_Click(object sender, EventArgs e)
        {
            if (mainForm.serCOM.IsOpen)
            {
                List<TextBox> txtDataType = textBoxGroup[0];
                if (BlankCount(txtDataType) == 0)
                {
                    SendLongFrame(startByte, endByte, 1, txtDataType);
                    
                    await Task.Delay(50);
                }
                else if (BlankCount(txtDataType) == 1 || BlankCount(txtDataType) == 2)
                {
                    for (int i = 0; i < txtDataType.Count; i++)
                    {
                        if (!string.IsNullOrEmpty(txtDataType[i].Text))
                        {
                            ushort dataSend = (ushort)(Convert.ToDouble(txtDataType[i].Text) * 100.0);
                            SendShortFrame(startByte, endByte, 1, i, dataSend);
                            await Task.Delay(50);
                        }
                    }
                }
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
        private async void BtnSendPID2_Click(object sender, EventArgs e)
        {
            if (mainForm.serCOM.IsOpen)
            {
                List<TextBox> txtDataType = textBoxGroup[1];
                if (BlankCount(txtDataType) == 0)
                {
                    SendLongFrame(startByte, endByte, 2, txtDataType);
                    await Task.Delay(50);
                }
                else if (BlankCount(txtDataType) == 1 || BlankCount(txtDataType) == 2)
                {
                    for (int i = 0; i < txtDataType.Count; i++)
                    {
                        if (!string.IsNullOrEmpty(txtDataType[i].Text))
                        {
                            ushort dataSend = (ushort)(Convert.ToDouble(txtDataType[i].Text) * 100.0);
                            SendShortFrame(startByte, endByte, 2, i, dataSend);
                            await Task.Delay(50);
                        }
                    }
                }
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
        private async void BtnSendPID3_Click(object sender, EventArgs e)
        {
            if (mainForm.serCOM.IsOpen)
            {
                List<TextBox> txtDataType = textBoxGroup[2];
                if (BlankCount(txtDataType) == 0)
                {
                    SendLongFrame(startByte, endByte, 3, txtDataType);
                    await Task.Delay(50);
                }
                else if (BlankCount(txtDataType) == 1 || BlankCount(txtDataType) == 2)
                {
                    for (int i = 0; i < txtDataType.Count; i++)
                    {
                        if (!string.IsNullOrEmpty(txtDataType[i].Text))
                        {
                            ushort dataSend = (ushort)(Convert.ToDouble(txtDataType[i].Text) * 100.0);
                            SendShortFrame(startByte, endByte, 3, i, dataSend);
                            await Task.Delay(50);
                        }
                    }
                }
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
    }
}

