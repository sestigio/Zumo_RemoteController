#include "uart_component.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "UART_APP";

// Cambiamos pines a unos compatibles con C6 (verifica tu placa)
#define TXD_PIN (GPIO_NUM_6) 
#define RXD_PIN (GPIO_NUM_7)
// El C6 NO tiene UART_NUM_2, usamos el 1
#define UART_PORT (UART_NUM_1) 
#define BUF_SIZE (1024)

void init_uart() {
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        // En C6 es mejor ser explícito con el reloj
        .source_clk = UART_SCLK_DEFAULT, 
    };
    
    // Instalamos el driver
    // Tip: Si vas a recibir muchos datos, el buffer de TX (tercer parámetro) 
    // no debería ser 0. Ponle al menos BUF_SIZE.
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, BUF_SIZE, 0, NULL, 0));
    
    // Configuramos los parámetros
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    
    // Asignamos pines
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    ESP_LOGI(TAG, "UART inicializada correctamente en el ESP32-C6");
}
