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