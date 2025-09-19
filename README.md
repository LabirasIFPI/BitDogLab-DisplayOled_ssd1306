# Módulo de Display OLED com Driver SSD1306

Esta documentação detalha o funcionamento e a aplicação de um módulo de software para controlar displays OLED baseados no driver SSD1306 com o Raspberry Pi Pico via comunicação I2C.

> **Datasheet de referência:** [https://www.mouser.com/datasheet/2/737/SSD1306-1159828.pdf](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
>

## 1. Como Utilizar o Módulo do Display

Esta seção é um guia direto para integrar a biblioteca do display SSD1306 em um novo projeto.

### 1.1. Conexão de Hardware

| Componente | Pino no Pico | Observação |
| --- | --- | --- |
| Display VCC | 3V3 (OUT) | Tensão de alimentação para o display. |
| Display GND | GND | Essencial. O terra (GND) deve ser comum entre o Pico e o display. |
| Display SDA | Qualquer pino I2C | Pino de dados. O padrão no seu módulo é o **GPIO14**. |
| Display SCL | Qualquer pino I2C | Pino de clock. O padrão no seu módulo é o **GPIO15**. |

>O display OLED SSD1306 utiliza o protocolo I2C. Certifique-se de usar pinos do Raspberry Pi Pico que sejam compatíveis com a função I2C e que o barramento (`i2c0` ou `i2c1`) seja inicializado corretamente no software.

### 1.2. Adicionando a Biblioteca (Git Submodule)

Para que o projeto funcione, ele depende da biblioteca `pico-ssd1306`. A maneira recomendada de adicioná-la é como um **submódulo do Git**, o que garante que você possa facilmente obter atualizações da biblioteca original.

Na raiz do seu projeto, execute o seguinte comando no terminal:

```bash
git submodule add https://github.com/daschr/pico-ssd1306 lib/pico-ssd1306
```

Este comando irá clonar a biblioteca para a pasta `lib/pico-ssd1306` e criar um arquivo `.gitmodules` que rastreia a versão exata da biblioteca utilizada no seu projeto.

### 1.3. Integração dos Arquivos

Com a biblioteca adicionada, a estrutura de arquivos do seu projeto deve conter:

1. **Biblioteca Principal (via Submódulo):**
    - `lib/pico-ssd1306/ssd1306.h`
    - `lib/pico-ssd1306/ssd1306.c`
2. **Módulo de Abstração (Opcional, mas recomendado):**
    - `display.h` no seu diretório de cabeçalhos (`inc/`).
    - `display.c` no seu diretório de código-fonte (`src/`).

### 1.4. Configuração do Build (CMakeLists.txt)

Para que o SDK do Pico compile seu projeto, adicione os fontes da biblioteca e do seu módulo de display, além da dependência `hardware_i2c` ao arquivo `CMakeLists.txt`.

**Adicionar os arquivos fonte:**

```c
add_executable(meu_projeto
    main.c
    src/display.c
    lib/pico-ssd1306/ssd1306.c // ADICIONE ESTÁ LINHA AOS EXECUTAVÉS
    # ... outros arquivos .c
)
```

**Adicionar a biblioteca de hardware:**

```c
target_link_libraries(meu_projeto
    pico_stdlib
    hardware_i2c
)
```

Adicione o diretório da biblioteca como destino de inclusão:

```c
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/lib/pico-ssd1306 // ADICIONE ESTÁ LINHA
)
```

### 2. Análise Técnica da Biblioteca `pico-ssd1306`

### 2.1. Princípios de Comunicação com o SSD1306 (I2C)

O display SSD1306 é controlado via protocolo I2C. Ele funciona como um dispositivo "escravo" que responde às solicitações do "mestre" (o Raspberry Pi Pico).

- **Endereçamento:** O display possui um endereço I2C (no seu caso, `0x3C`) que o Pico usa para se comunicar.
- **Comandos vs. Dados:** A comunicação I2C com o SSD1306 envia um byte de controle antes de cada pacote de dados. Esse byte informa ao display se a informação a seguir é um **comando** (para configurar o display, como ajustar contraste ou ligá-lo) ou **dados** (os pixels que serão exibidos na tela).
- **Buffer de Memória (GDDRAM):** O SSD1306 possui uma memória interna para armazenar o estado de cada pixel. A biblioteca `pico-ssd1306` cria um *buffer* na memória do Pico com o mesmo tamanho. As funções de desenho (`ssd1306_draw_pixel`, `ssd1306_draw_string`, etc.) modificam este buffer local. A função `ssd1306_show()` é responsável por transmitir o conteúdo desse buffer para a memória do display, atualizando a imagem na tela.

### 2.2. Arquivo de Cabeçalho `ssd1306.h`

Este arquivo é a interface pública da biblioteca. Ele define:

- **`ssd1306_command_t`**: Uma enumeração com todos os códigos de comando I2C que o display entende, como `SET_CONTRAST` (0x81) e `SET_DISP` (0xAE, para ligar/desligar).
- **`ssd1306_t`**: Uma estrutura (`struct`) que armazena todas as informações de configuração e estado de um display, incluindo:
  - `width` e `height`: As dimensões do display.
  - `address`: O endereço I2C do dispositivo.
  - `i2c_i`: A instância do barramento I2C (`i2c0` ou `i2c1`).
  - `buffer`: O ponteiro para o buffer de memória no Pico que armazena a imagem a ser desenhada.
- **Protótipos das Funções:**
  - `bool ssd1306_init(...)`: Inicializa o display, aloca o buffer e envia a sequência de comandos de configuração I2C.
  - `void ssd1306_clear(...)`: Preenche o buffer de memória com zeros, efetivamente "limpando" a imagem.
  - `void ssd1306_draw_pixel(...)`: Modifica um único bit no buffer para representar um pixel aceso.
  - `void ssd1306_draw_string(...)`: Desenha uma sequência de caracteres no buffer usando uma fonte pré-definida.
  - `void ssd1306_show(...)`: Envia todo o conteúdo do buffer local para a memória interna do display via I2C, atualizando a tela.

### 2.3. Implementação `ssd1306.c`

- **`ssd1306_init()`**: Esta função é crucial e realiza várias etapas:
    1. Aloca memória para o buffer do display com base na largura e altura.
    2. Envia uma longa sequência de comandos I2C para o display. Esses comandos definem a taxa de multiplexação, o contraste, a orientação da tela e ativam a bomba de carga interna para gerar a tensão necessária para os pixels do OLED.
    3. Finalmente, limpa o display e o liga.
- **Funções de Desenho**: Funções como `ssd1306_draw_pixel`, `ssd1306_draw_line` e `ssd1306_draw_string` não se comunicam diretamente com o display. Elas apenas realizam cálculos para determinar quais bytes e bits no buffer de memória do Pico precisam ser alterados. Isso torna as operações de desenho muito rápidas.
- **`ssd1306_show()`**: Esta é a única função (além da `init`) que realiza uma comunicação I2C intensiva. Ela envia comandos para definir os endereços de página e coluna e, em seguida, transmite todo o buffer de pixels para o display. É por isso que ela deve ser chamada sempre que você quiser que as alterações feitas pelas funções de desenho se tornem visíveis.

### 3. Apêndice: Projeto de Demonstração

Esta seção descreve o projeto de exemplo completo que demonstra o uso do módulo de display OLED para fornecer feedback visual.

### 3.1. Visão Geral do Exemplo

O projeto de exemplo inicializa o display OLED e, em seguida, executa uma contagem de 0 a 20. O texto é exibido no display com um efeito de "rolagem", onde a tela é limpa a cada 8 linhas, dando a impressão de que o texto está subindo. Ao final, uma mensagem de conclusão é exibida.

### 3.2. Estrutura de Arquivos do Exemplo

O projeto é organizado de forma modular para separar as responsabilidades e facilitar a manutenção:

```bash
Oled
 ├── inc
 |    ├── display.h # Interface pública para o módulo do display (camada de abstração)
 |    └── init.h # Módulo para inicializações do sistema
 ├── src
 |    ├── display.c # Implementação das funções para interagir com o display
 |    ├── init.c # Centraliza as inicializações de hardware (I2C, etc.)
 |    └── main.c # Lógica principal da aplicação
 ├── lib/pico-ssd1306 # Biblioteca de baixo nível para o SSD1306 (submódulo Git)
 └──... 
```

### 3.3. Análise do Módulo de Display (`display.h` e `display.c`)

Para simplificar a interação com a biblioteca `pico-ssd1306`, foi criada uma camada de abstração com os arquivos `display.h` e `display.c`. Isso permite que o `main.c` chame funções mais simples e diretas, sem precisar lidar com os detalhes da estrutura `ssd1306_t` a todo momento.

### **`display.h` (A Interface)**

Este arquivo de cabeçalho define a interface pública do nosso módulo de display.

```c
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "hardware/i2c.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C // Endereço I2C do display
#define I2C_SDA_DISPLAY 14          // Pino SDA
#define I2C_SCL_DISPLAY 15          // Pino SCL

int initializeDisplay();
void showText(const char *texto, uint32_t x, uint32_t y, uint32_t scale);
void updateTextLine(const char* text, uint32_t x, uint32_t y, uint32_t scale, uint32_t clear_width);
void clearScreen();

extern ssd1306_t display;

#endif  // DISPLAY_H
```

- **Constantes:** `SCREEN_WIDTH`, `SCREEN_HEIGHT`, `SCREEN_ADDRESS`, `I2C_SDA_DISPLAY`, e `I2C_SCL_DISPLAY` centralizam todas as configurações de hardware em um único local, facilitando futuras modificações.
- **Protótipos das Funções:**
  - `int initializeDisplay()`: Encapsula a chamada à função `ssd1306_init` da biblioteca, tornando a inicialização mais limpa.
  - `void showText(...)`: Uma função de alto nível que desenha um texto e **imediatamente atualiza a tela**. Isso simplifica o uso comum, combinando `ssd1306_draw_string` e `ssd1306_show` em uma única chamada.
  - `void updateTextLine(...)`: Uma função especializada que primeiro limpa uma área retangular do display e depois desenha um novo texto no lugar. Ideal para atualizar informações que mudam com frequência, como leituras de sensores ou contadores.
  - `void clearScreen()`: Combina `ssd1306_clear` e `ssd1306_show` para limpar a tela de forma imediata.
- **`extern ssd1306_t display;`**: Declara que uma variável global do tipo `ssd1306_t` chamada `display` existe em algum outro arquivo (`display.c`). Isso permite que o `main.c` acesse diretamente a instância do display se necessário (por exemplo, para chamar `ssd1306_show`).

### **`display.c` (A Implementação)**

Este arquivo contém o código que implementa as funções prometidas em `display.h`.

```c
#include "display.h"

ssd1306_t display; // Declara uma instância do display

// Inicializa o display
int initializeDisplay() {
    if (!ssd1306_init(&display, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS, i2c1)) { 
        printf("Falha ao inicializar o display SSD1306\n"); 
        return 1;
    }

    ssd1306_poweron(&display); // Liga o display
    printf("Display SSD1306 inicializado\n");
    return 0;
}

void showText(const char *texto, uint32_t x, uint32_t y, uint32_t scale){
    ssd1306_draw_string(&display, x, y, scale, texto); // Desenha o texto
    ssd1306_show(&display); // Atualiza a tela
}

void clearScreen(){
    ssd1306_clear(&display); // Limpa a tela
    ssd1306_show(&display); // Atualiza a tela 
}

void updateTextLine(const char* text, uint32_t x, uint32_t y, uint32_t scale, uint32_t clear_width) {
    // A altura da fonte padrão é 8 pixels
    uint32_t char_height = 8;

    // Limpa a área retangular onde o texto ficará
    ssd1306_clear_square(&display, x, y, clear_width, char_height * scale);

    // Desenha a nova string
    ssd1306_draw_string(&display, x, y, scale, text);
}
```

- **`initializeDisplay()`**: Esta função chama `ssd1306_init` com os parâmetros definidos em `display.h`, configurando o display para uso no barramento `i2c1`. Também liga o display com `ssd1306_poweron`.
- **`showText(...)`**: Simplesmente chama `ssd1306_draw_string` para desenhar o texto no buffer e, em seguida, chama `ssd1306_show` para garantir que o texto apareça na tela.
- **`clearScreen()`**: Da mesma forma, chama `ssd1306_clear` e `ssd1306_show`.
- **`updateTextLine(...)`**: Esta função é mais específica. Ela usa `ssd1306_clear_square` para apagar a área onde o novo texto será escrito antes de usar `ssd1306_draw_string`. **Importante**: esta função *não* chama `ssd1306_show`, permitindo que o código principal (`main.c`) faça várias atualizações no buffer antes de enviar tudo de uma vez para a tela, o que é mais eficiente e evita piscadas (flickering).

### 3.4. Lógica do `main.c` do Exemplo

O laço principal implementa a lógica de contagem e atualização do display usando a camada de abstração.

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "init.h" 

int main() {
    // Esta função única inicializa tudo!
    initializeSystem(); 
    sleep_ms(1000);

    // Buffer para formatar o texto para o display
    char buffer[40];
    
    // Mensagem de boas-vindas criativa
    showText("Iniciando...", 25, 28, 1);
    sleep_ms(2000);
    clearScreen();

    // Loop principal para contar de 0 a 50
    for (int i = 0; i <= 20; i++) {
        // Formata a string que será exibida
        sprintf(buffer, "Linha de teste N: %d", i);

        // A cada 8 linhas, limpa a tela para criar um efeito de rolagem
        if (i > 0 && i % 8 == 0) {
            clearScreen();
        }

        // Calcula a posição Y para que o texto "suba" na tela.
        // (i % 8) resulta em valores de 0 a 7, que são multiplicados pela altura da fonte (8 pixels).
        uint32_t y_pos = (i % 8) * 8;

        // A função updateTextLine não atualiza a tela sozinha,
        // então chamamos ssd1306_show() manualmente depois.
        updateTextLine(buffer, 0, y_pos, 1, SCREEN_WIDTH);
        ssd1306_show(&display);

        // Uma pequena pausa para que a contagem seja visível
        sleep_ms(150);
    }

    // Mensagem de finalização
    sleep_ms(1000);
    clearScreen();
    showText("Concluido!", 10, 20, 2);

    while (1) {
        // Loop infinito para manter a mensagem final no display
        tight_loop_contents();
    }
    return 0; 
}
```

Essa estrutura mostra claramente a vantagem da abstração: o `main.c` se preocupa com a lógica da aplicação (o que exibir e quando), enquanto os detalhes de como limpar, desenhar e atualizar o hardware do display são tratados pelo módulo `display`.
