cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain-arm.cmake CACHE PATH "ARM cross toolchain")
endif()

project(v3s_hello C ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -marm -ffreestanding -nostdlib -Wall -Wextra -Os")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-a7 -marm -x assembler-with-cpp")

add_executable(hello.elf
    src/startup.S
    src/main.c
    src/uart.c
    src/delay.c
    src/ephy.c
    src/lradc.c
)

target_include_directories(hello.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_options(hello.elf PRIVATE
    -T ${CMAKE_CURRENT_SOURCE_DIR}/link.ld
    -nostdlib
    -Wl,--build-id=none
)

set(BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/hello.bin)
add_custom_command(
    OUTPUT ${BIN_FILE}
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:hello.elf> ${BIN_FILE}
    DEPENDS hello.elf
    COMMENT "Генерирую hello.bin"
)
add_custom_target(bin ALL DEPENDS ${BIN_FILE})

find_program(SUNXI_FEL NAMES sunxi-fel)

add_custom_target(flash
    COMMAND ${SUNXI_FEL} write 0x2000 ${BIN_FILE}
    COMMAND ${SUNXI_FEL} execute 0x2000
    DEPENDS ${BIN_FILE}
    COMMENT "Загружаю hello.bin в SRAM (0x2000) через FEL"
)

add_custom_target(reset
    COMMAND ${SUNXI_FEL} wdreset
    COMMENT "Сброс V3s (возврат в FEL)"
)