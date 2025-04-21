#include <stdio.h>
#include "pico/stdlib.h"
#include "lib/ssd1306.h"
#include "hardware/adc.h"
#include "lib/ws2812.pio.h"
#include "hardware/pwm.h"

// Habilita ou desabilita o modo de depuração
#define DEBUG 1

#if DEBUG
    #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
#endif

// Configuração dos pinos do biossensor e botões
#define SENSE 26 
#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26
       
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

// Variáveis Globais
uint border_size = 2;
volatile uint32_t ultimo_tempo_A = 0;
volatile uint32_t ultimo_tempo_B = 0;
const uint32_t debounce = 200;
uint16_t adc_x, adc_y;
bool escolha_feita = false;
bool menu_quadrado = false;

void texto_temperatura(int temperatura_simulada){
    char buffer[32];
    ssd1306_draw_string(&ssd, "Temperatura: ", 10, 10);
    if(temperatura_simulada > 1){
        sprintf(buffer, "%d Graus", temperatura_simulada);
    } else {
        sprintf(buffer, "%d Grau", temperatura_simulada);
    }
    ssd1306_draw_string(&ssd, buffer, 10, 20);
}

void desenha_borda(){
    for (int i = 0; i < border_size; i++) {
        ssd1306_rect(&ssd, i, i, WIDTH - (2 * i), HEIGHT - (2 * i), true, false);
    }
}

// Função para gerar tons no buzzer
void tone(uint buzzer, uint frequencia, uint duracao) {
    uint32_t periodo = 1000000 / frequencia; 
    uint32_t meio_periodo = periodo / 2;    
    uint32_t ciclos = frequencia * duracao / 1000;

    for (uint32_t i = 0; i < ciclos; i++) {
        gpio_put(buzzer, 1); 
        sleep_us(meio_periodo);
        gpio_put(buzzer, 0); 
        sleep_us(meio_periodo);
    }
}

// Função de callback para os botões A e B
void button_callback(uint gpio, uint32_t events){
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
    if(gpio == BOTAO_A && (tempo_atual - ultimo_tempo_A > debounce)){
        ultimo_tempo_A = tempo_atual;
        menu_quadrado = true;
    }
    if(gpio == BOTAO_B && (tempo_atual - ultimo_tempo_B > debounce)){
        ultimo_tempo_B = tempo_atual;
        menu_quadrado = false;
    }
}

int main()
{
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    adc_init();  
    adc_gpio_init(SENSE); 
    adc_select_input(0);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_put(BUZZER, 0);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    init_matrix();

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, button_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, button_callback);

    while (true) {
        uint16_t valor_adc = adc_read();
        float tensao = (valor_adc * 3.3) / 4095;
        float temperatura_simulada = (valor_adc / 4095.0f) * 50.0f;

        // Exibe mensagens de depuração no terminal serial 
        DEBUG_PRINT("ADC: %d | Tensão: %.2fV | Temperatura Simulada: %.2f\n", valor_adc, tensao, temperatura_simulada);

        if (temperatura_simulada < 15.0) {
            set_matrix_color(BLUE); // Acende a matriz de led na cor azul
            gpio_put(LED_BLUE, 1);
            gpio_put(LED_GREEN, 0);
            gpio_put(LED_RED, 0);
        } else if (temperatura_simulada > 35.0) {
            set_matrix_color(RED); // Acende a matriz de led na cor vermelha
            gpio_put(LED_RED, 1);
            gpio_put(LED_BLUE, 0);
            gpio_put(LED_GREEN, 0);
            tone(BUZZER, 500, 350);
        } else {
            set_matrix_color(GREEN); // Acende a matriz de led na cor verde
            gpio_put(LED_GREEN, 1);
            gpio_put(LED_BLUE, 0);
            gpio_put(LED_RED, 0);
        }
        // Limpar a tela
        ssd1306_fill(&ssd, false);

        // Mostra na tela a informações da temperatura
        texto_temperatura(temperatura_simulada);

        desenha_borda();
        ssd1306_send_data(&ssd);

        while(menu_quadrado){
            // Definição dos Leds em branco
            gpio_put(LED_BLUE, 1);
            gpio_put(LED_GREEN, 1);
            gpio_put(LED_RED, 1);
            set_matrix_color(WHITE);

            // Leitura do eixo X do joystick
            adc_select_input(1);
            uint16_t adc_x = adc_read();

            // Leitura do eixo Y do joystick
            adc_select_input(0);
            uint16_t adc_y = adc_read();

            // Exibe mensagens de depuração no terminal serial 
            DEBUG_PRINT("ADC X: %d | ADC Y: %d\n", adc_x, adc_y);

            // Limpar a tela
            ssd1306_fill(&ssd, false);

            // Converte os valores do joystick para coordenadas do display OLED
            uint8_t pos_x = (adc_x * (WIDTH - 8)) / 4095;
            uint8_t pos_y = ((4095 - adc_y) * (HEIGHT - 8)) / 4095;

            // Desenha um quadrado na posição do joystick
            ssd1306_rect(&ssd, pos_y, pos_x, 8, 8, true, true);
            
            desenha_borda();
            ssd1306_send_data(&ssd);
        }
        sleep_ms(50);
    }
}
