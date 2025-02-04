# I target a recent cmake, it shouldn't be a problem on a dev machine
cmake_minimum_required(VERSION 3.28)

set(CMAKE_SYSTEM_NAME Generic)

# Project name
project("Default" VERSION 0.1)

# Product filename
set(PRODUCT_NAME result)

# Directory for includes
set(INC_DIR inc)

# Directory for object files
set(OBJ_DIR obj)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OBJ_DIR})

# Directory for debug files
set(DEBUG_DIR debug)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## AVR Chip Configuration
# 16Mhz, this should match the crystal on your board
set(F_CPU 16000000UL)

# CPU, you can find the list here:
# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
set(MCU atmega328p)

# Baudrate for UART, read avr include/util/setbaud.h for usage
set(BAUD 115200 CACHE STRING "Baud rate for UART")  # Default value: 115200 baud
message(STATUS "Selected UART Baud rate is ${BAUD}")

# Default serial device for UART
set(SERIAL_PORT /dev/ttyUSB0 CACHE STRING "Serial port for UART communication" )  # Default value: /dev/ttyUSB0
message(STATUS "Selected port for UART communication is ${SERIAL_PORT}")

# The programmer to use, read avrdude manual for list
set(PROG_TYPE usbasp)

# Programmer speed
set(PROG_SPEED 1500kHz CACHE STRING "Programmer frequency for communication with target")
message(STATUS "Selected frequency for communication with target is ${PROG_SPEED}")

# AVR Fuses, must be in concordance with your hardware and F_CPU
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p
set(E_FUSE 0x05)
set(H_FUSE 0xD6)
set(L_FUSE 0xFF)
set(LOCK_BIT 0xff)

# Use AVR GCC toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# Pass defines to compiler
add_definitions(
    -DF_CPU=${F_CPU}
    -DBAUD=${BAUD}
)

add_compile_options(
    -mmcu=${MCU} # MCU
    -std=gnu99 # C99 standard
    -Os # optimize for size
    -Wall # enable warnings
    -Wno-main
    -Wundef
    -pedantic
    -Wstrict-prototypes
    # -Werror
    # -Wfatal-errors
    # -Wno-array-bounds
    # -Wl,--relax,--gc-sections
    # -g
    # -gdwarf-2
    # -funsigned-char # a few optimizations
    # -funsigned-bitfields
    # -fpack-struct
    # -fshort-enums
    # -ffunction-sections
    # -fdata-sections
    # -fno-split-wide-types
    # -fno-tree-scev-cprop
)

add_link_options(
    -mmcu=${MCU} # MCU
    -Wl,-u,vfprintf -lprintf_flt -lm
)

file(GLOB SRC_FILES "src/*.c") # Load all files in src folder

# Include header files directories
include_directories(${INC_DIR})

# Create one target
add_executable(${PRODUCT_NAME} ${SRC_FILES})

# Rename the output to .elf as we will create multiple files
set_target_properties(${PRODUCT_NAME} PROPERTIES OUTPUT_NAME ${PRODUCT_NAME}.elf)

# Strip binary for upload
add_custom_target(strip ALL avr-strip ${OBJ_DIR}/${PRODUCT_NAME}.elf DEPENDS ${OBJ_DIR}/${PRODUCT_NAME})

# Transform binary into hex file, we ignore the eeprom segments in the step
add_custom_target(hex ALL avr-objcopy -R .eeprom -O ihex ${OBJ_DIR}/${PRODUCT_NAME}.elf ${OBJ_DIR}/${PRODUCT_NAME}.hex DEPENDS strip)

# Create dump from ELF file
add_custom_target(dump ALL avr-objdump -D ${OBJ_DIR}/${PRODUCT_NAME}.elf > ${DEBUG_DIR}/${PRODUCT_NAME}.dump DEPENDS ${OBJ_DIR}/${PRODUCT_NAME}.hex)

# Transform binary into hex file, this is the eeprom part (empty if you don't
# use eeprom static variables)
add_custom_target(eeprom avr-objcopy -j .eeprom  --set-section-flags=.eeprom="alloc,load"  --change-section-lma .eeprom=0 -O ihex ${PRODUCT_NAME}.elf ${PRODUCT_NAME}.eep DEPENDS strip)

# Upload the firmware with avrdude
# add_custom_target(upload sudo avrdude -c${PROG_TYPE} -p${MCU} -B${PROG_SPEED} -v -U flash:w:${OBJ_DIR}/${PRODUCT_NAME}.hex DEPENDS hex)
add_custom_target(upload sudo avrdude -c${PROG_TYPE} -p${MCU} -B${PROG_SPEED} -v -U flash:w:${OBJ_DIR}/${PRODUCT_NAME}.hex DEPENDS hex)

# Upload the eeprom with avrdude
add_custom_target(upload_eeprom sudo avrdude -c${PROG_TYPE} -p${MCU} -B${PROG_SPEED} -v -U eeprom:w:${PRODUCT_NAME}.eep DEPENDS eeprom)

# Burn fuses
add_custom_target(fuses sudo avrdude -c${PROG_TYPE} -p${MCU} -B${PROG_SPEED} -v -U lfuse:w:${L_FUSE}:m -U hfuse:w:${H_FUSE}:m -U efuse:w:${E_FUSE}:m -U lock:w:${LOCK_BIT}:m )

# Start serial communication with screen 
add_custom_target(serial sudo screen ${SERIAL_PORT} ${BAUD})

# Upload the firmware and start serial communication 
add_custom_target(serialu sudo screen ${SERIAL_PORT} ${BAUD} DEPENDS upload)

# Clean extra files
file(GLOB OBJ_FILES "obj/*") # Load all files in obj folder
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${OBJ_FILES}")
