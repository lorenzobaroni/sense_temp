cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

# Define o nome do projeto como Teste_ADC_Display, suportando C, C++ e Assembly.
project(sense_temp C CXX ASM) 
pico_sdk_init()


add_executable(${PROJECT_NAME}  
        sense_temp.c # Código principal em C
        
        )

target_link_libraries(${PROJECT_NAME} 
        pico_stdlib 

        )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})