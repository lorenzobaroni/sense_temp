# 🧠 Projeto: Monitoramento de Temperatura com Raspberry Pi Pico W

Este projeto simula a leitura de temperatura utilizando o joystick da placa BitDogLab (via ADC) e exibe os dados em um display OLED SSD1306. Além disso, conta com um menu interativo que permite alternar entre a visualização da temperatura e o controle de um quadrado que se movimenta na tela com o joystick. O sistema também aciona LEDs RGB e um buzzer conforme a faixa da temperatura.

---

## 🔧 Funcionalidades

- 📊 **Leitura simulada de temperatura** usando o joystick (via ADC).
- 🖥️ **Exibição no display OLED SSD1306 (128x64)** via I2C.
- 🔁 **Dois menus interativos**, alternáveis com botões físicos:
  - **Menu Temperatura**: Exibe a temperatura simulada e aciona alertas visuais/sonoros.
  - **Menu Quadrado**: Um quadrado na tela é movimentado conforme o joystick.
- 💡 **Controle de cor da matriz WS2812** conforme a faixa de temperatura:
  - Azul: temperatura baixa
  - Verde: temperatura normal
  - Vermelho + buzzer: temperatura alta
- 🧠 **Interruptores com debounce** nos botões A (GPIO 5) e B (GPIO 6)

---

## 🧪 Simulação da Temperatura

- A leitura do eixo Y do joystick é convertida para uma temperatura entre `0°C` e `50°C`.
- Faixas:
  - `< 15°C`: LED azul aceso
  - `15°C – 35°C`: LED verde aceso
  - `> 35°C`: LED vermelho aceso + buzzer

---

## 🎮 Menu Quadrado (Joystick)

- O joystick controla um quadrado de 8x8 pixels na tela OLED.
- O eixo X e Y do joystick controlam as posições horizontais e verticais.
- A movimentação é contínua e proporcional à posição do joystick.

---

## 📌 Mapeamento de Pinos

| Componente     | GPIO         |
|----------------|--------------|
| Joystick Y     | ADC0 (GPIO 26) |
| Joystick X     | ADC1 (GPIO 27) |
| Botão A        | GPIO 5       |
| Botão B        | GPIO 6       |
| Buzzer         | GPIO 21      |
| LED RGB RED    | GPIO 13      |
| LED RGB GREEN  | GPIO 11      |
| LED RGB BLUE   | GPIO 12      |
| WS2812 Matrix  | GPIO 7       |
| Display OLED   | SDA: GPIO 14 / SCL: GPIO 15 |

---

## 🛠️ Compilação

### 1️⃣ Configurar o ambiente
Antes de compilar e rodar o código, certifique-se de ter:
- **SDK do Raspberry Pi Pico** corretamente instalado.
- **Compilador GCC ARM** para processadores Cortex M0+.
- **CMake e Ninja** para build do projeto.

### 2️⃣ Clonar o repositório
```sh
git clone https://github.com/lorenzobaroni/sense_temp
```

### 3️⃣ Compilar o projeto

### 4️⃣ Transferir o firmware para a Pico W
- Conecte a **Pico W** ao PC em **modo BOOTSEL**.
- Copie o arquivo `biossensor.uf2` gerado para a unidade da Pico.

### 📡 Monitoramento via Serial
Para visualizar os dados enviados pela Raspberry Pi Pico W abra um Monitor Serial e acompanhe as informações.

## 📝 Licença
Este programa foi desenvolvido como um exemplo educacional e pode ser usado livremente para fins de estudo e aprendizado.

## 📌 Autor
LORENZO GIUSEPPE OLIVEIRA BARONI
