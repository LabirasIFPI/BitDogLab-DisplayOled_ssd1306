// init.c
#include "init.h"

// Inicializa o I2C1 para o display
void initI2C_Display() {
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA_DISPLAY, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISPLAY, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISPLAY);
    gpio_pull_up(I2C_SCL_DISPLAY);
}

// Função principal de inicialização do sistema
void initializeSystem(void) {
    stdio_init_all();
    sleep_ms(1000);

    // Configura os dois barramentos I2C
    printf("Configurando I2C do OLED (I2C1)...\n");
    initI2C_Display();


    // Inicializa o display
    printf("Iniciando SSD1306...\n");
    if (initializeDisplay() != 0) {
        printf("Erro ao inicializar o SSD1306\n");
    }
    
    printf("Tela limpa.\n");
    clearScreen();
}