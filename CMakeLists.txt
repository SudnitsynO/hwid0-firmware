cmake_minimum_required(VERSION 3.16)

# Possible values: openocd, pyocd, stlink, blackmagic. stlink is default
#set(CMX_DEBUGGER "openocd")
#set(OPENOCD_CFG "${CMAKE_CURRENT_SOURCE_DIR}/openocd.cfg")
#set(OPENOCD_SVD_FILE "STM32F103.svd")
#set(OPENOCD_LAUNCH_TYPE "launch")
project(new_tairg)

include(cubemx.cmake/cubemx.cmake)

add_executable(new_tairg 
    LIB/src/FlashDrive.cpp 
    LIB/src/crc.cpp 
    LIB/src/Buffer.cpp 
    LIB/src/random.cpp 
    LIB/src/fonts.cpp 
    LIB/src/mutex_freertos.cpp 
    LIB/src/memory_freertos.cpp 
    LIB/src/SPI_base.cpp 
    LIB/src/delay_hal_F1.cpp 
    LIB/src/delay_freertos.cpp 
    LIB/src/DateTime.cpp 
    LIB/src/Format.cpp 
    LIB/src/RTC_F1.cpp 
    LIB/src/font5x8.cpp 
    LIB/src/utf8_converter.cpp
)

cubemx_target(
    TARGET new_tairg
    IOC "${CMAKE_CURRENT_LIST_DIR}/new_tairg.ioc"
)

target_include_directories(new_tairg PRIVATE
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3
    Program/Inc
    LIB/inc 
)

file(GLOB_RECURSE MIDDLEWARE_SRC
    "Middlewares/*.c"
    "Program/*.cpp"
    "STM32CubeIDE/Application/*.c"
)

target_sources(new_tairg PRIVATE ${MIDDLEWARE_SRC})

target_compile_options(new_tairg PRIVATE -O3 -Wall -g -gdwarf-2  )

target_link_options(new_tairg PRIVATE -specs=nosys.specs)

set(CMAKE_CXX_FLAGS "-std=c++17 -fno-rtti")

# Depending on the project setup, sometimes one of these symbols must be omitted. (Cannot be reliably determined from the .ioc file)
target_compile_definitions(new_tairg PRIVATE USE_HAL_DRIVER STM32F103xE LCD_ILI9325 )
