/*
O arquivo ssd1306.c implementa todas as funções necessárias para controlar o display OLED SSD1306 via I2C.
Este arquivo é responsável por enviar comandos e dados para o display OLED SSD1306, permitindo exibir gráficos, textos e desenhar formas geométricas na tela.
*/


#include "ssd1306.h"
#include "font.h"

ssd1306_t ssd;
PIO pio = pio0;
uint offset = 0;
uint sm = 0;
uint32_t RED   = 0x00FF00;
uint32_t GREEN = 0xFF0000;
uint32_t BLUE  = 0x0000FF;
uint32_t WHITE  = 0xFFFFFF;

void ssd1306_init(ssd1306_t *ssd, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c) {
  ssd->width = width;
  ssd->height = height;
  ssd->pages = height / 8U;
  ssd->address = address;
  ssd->i2c_port = i2c;
  ssd->external_vcc = external_vcc;
  ssd->bufsize = ssd->pages * ssd->width + 1;
  ssd->ram_buffer = calloc(ssd->bufsize, sizeof(uint8_t));
  ssd->ram_buffer[0] = 0x40;
  ssd->port_buffer[0] = 0x80;
}

void ssd1306_config(ssd1306_t *ssd) {
  ssd1306_command(ssd, SET_DISP | 0x00);
  ssd1306_command(ssd, SET_MEM_ADDR);
  ssd1306_command(ssd, 0x01);
  ssd1306_command(ssd, SET_DISP_START_LINE | 0x00);
  ssd1306_command(ssd, SET_SEG_REMAP | 0x01);
  ssd1306_command(ssd, SET_MUX_RATIO);
  ssd1306_command(ssd, HEIGHT - 1);
  ssd1306_command(ssd, SET_COM_OUT_DIR | 0x08);
  ssd1306_command(ssd, SET_DISP_OFFSET);
  ssd1306_command(ssd, 0x00);
  ssd1306_command(ssd, SET_COM_PIN_CFG);
  ssd1306_command(ssd, 0x12);
  ssd1306_command(ssd, SET_DISP_CLK_DIV);
  ssd1306_command(ssd, 0x80);
  ssd1306_command(ssd, SET_PRECHARGE);
  ssd1306_command(ssd, 0xF1);
  ssd1306_command(ssd, SET_VCOM_DESEL);
  ssd1306_command(ssd, 0x30);
  ssd1306_command(ssd, SET_CONTRAST);
  ssd1306_command(ssd, 0xFF);
  ssd1306_command(ssd, SET_ENTIRE_ON);
  ssd1306_command(ssd, SET_NORM_INV);
  ssd1306_command(ssd, SET_CHARGE_PUMP);
  ssd1306_command(ssd, 0x14);
  ssd1306_command(ssd, SET_DISP | 0x01);
}

// Inicializa a Matriz WS2812
void init_matrix() {
  offset = pio_add_program(pio, &ws2812_program);
  sm = pio_claim_unused_sm(pio, true);
  ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000, false);
}

// Define a cor da matriz
void set_matrix_color(uint32_t color) {
    float brilho = 0.2f; // Valor máxido de 1

    // Extrai os componentes GRB
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t r = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    // Aplica o fator de brilho
    g = (uint8_t)(g * brilho);
    r = (uint8_t)(r * brilho);
    b = (uint8_t)(b * brilho);

    // Recombina no formato GRB
    uint32_t cor = (g << 16) | (r << 8) | b;

    // Envia para todos os LEDs da matriz
    for (int i = 0; i < NUM_LEDS; i++) {
        ws2812_put_pixel(cor);
    }
}

void ws2812_put_pixel(uint32_t pixel_grb) {
  // Aguarda o FIFO estar disponível
  while (pio_sm_is_tx_fifo_full(pio, sm));
  // Envia os bits do pixel no formato GRB
  pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

void ssd1306_command(ssd1306_t *ssd, uint8_t command) {
  ssd->port_buffer[1] = command;
  i2c_write_blocking(
    ssd->i2c_port,
    ssd->address,
    ssd->port_buffer,
    2,
    false
  );
}

void ssd1306_send_data(ssd1306_t *ssd) {
  ssd1306_command(ssd, SET_COL_ADDR);
  ssd1306_command(ssd, 0);
  ssd1306_command(ssd, ssd->width - 1);
  ssd1306_command(ssd, SET_PAGE_ADDR);
  ssd1306_command(ssd, 0);
  ssd1306_command(ssd, ssd->pages - 1);
  i2c_write_blocking(
    ssd->i2c_port,
    ssd->address,
    ssd->ram_buffer,
    ssd->bufsize,
    false
  );
}

void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool value) {
  uint16_t index = (y >> 3) + (x << 3) + 1;
  uint8_t pixel = (y & 0b111);
  if (value)
    ssd->ram_buffer[index] |= (1 << pixel);
  else
    ssd->ram_buffer[index] &= ~(1 << pixel);
}

void ssd1306_fill(ssd1306_t *ssd, bool value) {
    // Itera por todas as posições do display
    for (uint8_t y = 0; y < ssd->height; ++y) {
        for (uint8_t x = 0; x < ssd->width; ++x) {
            ssd1306_pixel(ssd, x, y, value);
        }
    }
}

void ssd1306_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill) {
  for (uint8_t x = left; x < left + width; ++x) {
    ssd1306_pixel(ssd, x, top, value);
    ssd1306_pixel(ssd, x, top + height - 1, value);
  }
  for (uint8_t y = top; y < top + height; ++y) {
    ssd1306_pixel(ssd, left, y, value);
    ssd1306_pixel(ssd, left + width - 1, y, value);
  }

  if (fill) {
    for (uint8_t x = left + 1; x < left + width - 1; ++x) {
      for (uint8_t y = top + 1; y < top + height - 1; ++y) {
        ssd1306_pixel(ssd, x, y, value);
      }
    }
  }
}

void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        ssd1306_pixel(ssd, x0, y0, value); // Desenha o pixel atual

        if (x0 == x1 && y0 == y1) break; // Termina quando alcança o ponto final

        int e2 = err * 2;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value) {
  for (uint8_t x = x0; x <= x1; ++x)
    ssd1306_pixel(ssd, x, y, value);
}

void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value) {
  for (uint8_t y = y0; y <= y1; ++y)
    ssd1306_pixel(ssd, x, y, value);
}

// Função para desenhar um caractere
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y) {
  uint16_t index = 0;
  bool is_special = false;

  if (c >= '0' && c <= '9') {
      index = (c - '0' + 1) * 8;  // Números (0-9)
  } 
  else if (c >= 'A' && c <= 'Z') {
      index = (c - 'A' + 11) * 8; // Letras maiúsculas (A-Z)
  } 
  else if (c >= 'a' && c <= 'z') {
      index = (c - 'a' + 37) * 8; // Letras minúsculas (a-z)
  } 
  else if (c == ':') {
      index = 63 * 8;  // Índice correto do ':'
      is_special = true;
  } 
  else if (c == '/') {
      index = 64 * 8;  // Índice correto do '/'
  } 
  else if (c == '(') {
      index = 65 * 8;  // Índice correto do '('
  }
  else if (c == ')') {
      index = 66 * 8;  // Índice correto do ')'
  }
  else {
      return;  // Se o caractere não for suportado, não desenha nada
  }

  // 🔹 Para caracteres normais, mantém a exibição correta
  if (!is_special) {
      for (uint8_t col = 0; col < 8; ++col) {  
          uint8_t line = font[index + col];  
          for (uint8_t row = 0; row < 8; ++row) {  
              if (line & (1 << row)) {  
                  ssd1306_pixel(ssd, x + col, y + row, true);
              } else {
                  ssd1306_pixel(ssd, x + col, y + row, false);
              }
          }
      }
  } 
  // 🔹 Para `:` apenas, usa a inversão para exibição correta
  else {
      for (uint8_t col = 0; col < 8; ++col) {  
          uint8_t line = font[index + col];  
          for (uint8_t row = 0; row < 8; ++row) {  
              if (line & (1 << row)) {  
                  ssd1306_pixel(ssd, x + row, y + col, true);  // 🔹 Corrigido apenas para `:`
              } else {
                  ssd1306_pixel(ssd, x + row, y + col, false);
              }
          }
      }
  }
}

// Função para desenhar uma string
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y)
{
  while (*str)
  {
    ssd1306_draw_char(ssd, *str++, x, y);
    x += 8;
    if (x + 8 >= ssd->width)
    {
      x = 0;
      y += 8;
    }
    if (y + 8 >= ssd->height)
    {
      break;
    }
  }
}

void ssd1306_draw_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
  int err = dx + dy, e2;
  
  while (1) {
      ssd1306_pixel(ssd, x0, y0, color);  // Desenha um pixel na posição
      
      if (x0 == x1 && y0 == y1) break;
      e2 = 2 * err;
      
      if (e2 >= dy) { err += dy; x0 += sx; }
      if (e2 <= dx) { err += dx; y0 += sy; }
  }
}
