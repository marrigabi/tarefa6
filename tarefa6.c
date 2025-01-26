#include "pico/stdlib.h"
#include <string.h>       // Para o uso de memset
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

#define LED_R_PIN 13  // LED vermelho
#define LED_G_PIN 11  // LED verde
#define LED_B_PIN 12  // LED amarelo
#define BTN_A_PIN 5   // Botão para pedestre

int A_state = 0;  // Variável para verificar se o botão foi pressionado

// Função para exibir mensagens no OLED
void display_message(uint8_t *ssd, struct render_area *frame_area, const char *text[], uint line_count) {
    memset(ssd, 0, ssd1306_buffer_length);  // Limpa o buffer do display
    int y = 0;
    for (uint i = 0; i < line_count; i++) {
        ssd1306_draw_string(ssd, 5, y, (char *)text[i]);  // Exibe a linha
        y += 8;
    }
    render_on_display(ssd, frame_area);  // Atualiza o display
}

// Função para o estado vermelho (sinal fechado)
void SinalFechado(uint8_t *ssd, struct render_area *frame_area) {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);

    const char *message[] = {
        "SINAL FECHADO",
        "AGUARDE"
    };
    display_message(ssd, frame_area, message, 2);
}

// Função para o estado amarelo (sinal de atenção)
void SinalAtencao(uint8_t *ssd, struct render_area *frame_area) {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    const char *message[] = {
        "SINAL DE ATENCAO",
        "PREPARE-SE"
    };
    display_message(ssd, frame_area, message, 2);
}

// Função para o estado verde (sinal aberto)
void SinalAberto(uint8_t *ssd, struct render_area *frame_area) {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    const char *message[] = {
        "SINAL ABERTO",
        "ATRAVESSAR COM",
        "CUIDADO"
    };
    display_message(ssd, frame_area, message, 3);
}

// Função para aguardar o botão ser pressionado
int WaitWithRead(int timeMS) {
    for (int i = 0; i < timeMS; i += 100) {
        A_state = !gpio_get(BTN_A_PIN);
        if (A_state == 1) {
            return 1;  // Botão pressionado
        }
        sleep_ms(100);
    }
    return 0;  // Tempo esgotado sem pressionamento
}

int main() {
    stdio_init_all();  // Inicializa os tipos stdio

    // Inicialização do I2C para o OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    // Inicialização do OLED
    ssd1306_init();

    // Preparar área de renderização para o display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Zerar o display
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Inicialização dos LEDs
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Inicialização do botão
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    while (true) {
        // Estado inicial: Vermelho
        SinalFechado(ssd, &frame_area);
        A_state = WaitWithRead(8000);  // Espera até 8 segundos ou até o botão ser pressionado

        if (A_state) {  // Botão pressionado
            // Estado de atenção (amarelo) por 2 segundos
            SinalAtencao(ssd, &frame_area);
            sleep_ms(2000);

            // Estado aberto (verde) por 8 segundos
            SinalAberto(ssd, &frame_area);
            sleep_ms(8000);
        } else {  // Botão não pressionado
            // Estado de atenção (amarelo) por 2 segundos
            SinalAtencao(ssd, &frame_area);
            sleep_ms(2000);

            // Estado aberto (verde) por 8 segundos
            SinalAberto(ssd, &frame_area);
            sleep_ms(8000);
        }

        // Retorna ao estado inicial (vermelho)
    }

    return 0;
}