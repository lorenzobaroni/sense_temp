#include <stdio.h>
#include "pico/stdlib.h"
#include "lib/ssd1306.h"
#include "hardware/adc.h"

// Configuração dos pinos do biossensor e botões
#define SENSE 26 
#define JOYSTICK_PB 22
       
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12
#define BUZZER 21
#define BOTAO_A 5
#define BOTAO_B 6

// Configuração do I2C para o display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDR 0x3C 

// Configuração da matriz de LEDs WS2812
#define MATRIX_PIN 7
#define NUM_LEDS 25

int main()
{
    stdio_init_all();

    while (true) {
        sleep_ms(5);
    }
}
