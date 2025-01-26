#include "pico/stdlib.h"
#include <string.h>       
#include <stdio.h>
#include <ctype.h>
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

#define RED_LED_PIN 13    // LED vermelho
#define GREEN_LED_PIN 11  // LED verde
#define YELLOW_LED_PIN 12 // LED amarelo
#define PEDESTRIAN_BTN 5  // Botão para pedestre

int btn_state = 0;  // Variável para verificar o status do botão

// Função para atualizar o display OLED
void update_display(uint8_t *ssd_buf, struct render_area *disp_area, const char *lines[], uint count) {
    memset(ssd_buf, 0, ssd1306_buffer_length);  // Limpa o buffer
    int ypos = 0;
    for (uint idx = 0; idx < count; idx++) {
        ssd1306_draw_string(ssd_buf, 5, ypos, (char *)lines[idx]);  // Exibe o texto
        ypos += 8;
    }
    render_on_display(ssd_buf, disp_area);  // Atualiza no display
}

// Função para o sinal vermelho
void RedSignal(uint8_t *ssd_buf, struct render_area *disp_area) {
    gpio_put(RED_LED_PIN, 1);
    gpio_put(GREEN_LED_PIN, 0);
    gpio_put(YELLOW_LED_PIN, 0);

    const char *msg[] = {
        "SINAL FECHADO",
        "POR FAVOR, AGUARDE"
    };
    update_display(ssd_buf, disp_area, msg, 2);
}

// Função para o sinal amarelo
void YellowSignal(uint8_t *ssd_buf, struct render_area *disp_area) {
    gpio_put(RED_LED_PIN, 1);
    gpio_put(GREEN_LED_PIN, 1);
    gpio_put(YELLOW_LED_PIN, 0);

    const char *msg[] = {
        "SINAL DE ALERTA",
        "PREPARE-SE"
    };
    update_display(ssd_buf, disp_area, msg, 2);
}

// Função para o sinal verde
void GreenSignal(uint8_t *ssd_buf, struct render_area *disp_area) {
    gpio_put(RED_LED_PIN, 0);
    gpio_put(GREEN_LED_PIN, 1);
    gpio_put(YELLOW_LED_PIN, 0);

    const char *msg[] = {
        "SINAL ABERTO",
        "ATRAVESSE COM",
        "CUIDADO"
    };
    update_display(ssd_buf, disp_area, msg, 3);
}

// Função para monitorar o botão
int ButtonPressWait(int timeout_ms) {
    for (int elapsed = 0; elapsed < timeout_ms; elapsed += 100) {
        btn_state = !gpio_get(PEDESTRIAN_BTN);
        if (btn_state) {
            return 1;  // Botão detectado
        }
        sleep_ms(100);
    }
    return 0;  // Tempo esgotado sem pressionar
}

int main() {
    stdio_init_all();  

    // Configuração do I2C para o OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    // Configuração do display OLED
    ssd1306_init();

    struct render_area disp_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&disp_area);

    uint8_t ssd_buf[ssd1306_buffer_length];
    memset(ssd_buf, 0, sizeof(ssd_buf));
    render_on_display(ssd_buf, &disp_area);

    // Configuração dos LEDs
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_init(YELLOW_LED_PIN);
    gpio_set_dir(YELLOW_LED_PIN, GPIO_OUT);

    // Configuração do botão
    gpio_init(PEDESTRIAN_BTN);
    gpio_set_dir(PEDESTRIAN_BTN, GPIO_IN);
    gpio_pull_up(PEDESTRIAN_BTN);

    while (true) {
        // Estado inicial: Sinal vermelho
        RedSignal(ssd_buf, &disp_area);
        btn_state = ButtonPressWait(8000);

        if (btn_state) {
            YellowSignal(ssd_buf, &disp_area);
            sleep_ms(2000);
            GreenSignal(ssd_buf, &disp_area);
            sleep_ms(8000);
        } else {
            YellowSignal(ssd_buf, &disp_area);
            sleep_ms(2000);
            GreenSignal(ssd_buf, &disp_area);
            sleep_ms(8000);
        }
    }

    return 0;
}
