cmake_minimum_required(VERSION 3.16)
project(Master LANGUAGES C ASM)

# Yêu cầu file toolchain
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
    message(FATAL_ERROR "Vui lòng sử dụng toolchain file: cmake -DCMAKE_TOOLCHAIN_FILE=arm-none-eabi.cmake")
endif()

# Tạo file compile_commands.json để fix lỗi IntelliSense (gạch đỏ) trong VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Các cờ kiến trúc cho STM32H750VBTX (Cortex-M7, FPU)
set(CPU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPU_FLAGS} -x assembler-with-cpp")

# Cấu hình file linker
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32H750VBTX_FLASH.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CPU_FLAGS} -T \"${LINKER_SCRIPT}\" -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs")

# Thu thập tất cả các file mã nguồn
file(GLOB_RECURSE PROJECT_SOURCES
    "Core/Src/*.c"
    "Drivers/STM32H7xx_HAL_Driver/Src/*.c"
    "Core/Startup/*.s"
)

# Target executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Các thư mục chứa file header
target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc
    Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32H7xx/Include
    Drivers/CMSIS/Include
)

# Các macro định nghĩa (dựa theo cấu hình CubeMX)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_PWR_LDO_SUPPLY
    USE_HAL_DRIVER
    STM32H750xx
)
