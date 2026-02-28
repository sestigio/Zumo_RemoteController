#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h" //uso de tasks, delays y temporizadores
#include "freertos/task.h" 
#include "freertos/timers.h" 
#include "freertos/queue.h"
#include "esp_log.h" // transmisión de mensajes al serial mejorada y detección de errores de ESP-IDF
// #include "driver/ledc.h" // uso de pwm
// #include "driver/gpio.h" // control de pines
// #include "driver/adc.h" // uso de adc de la placa
#include "driver/i2c.h" // para conexión con mpu6050
#include "esp_timer.h" // para timer estilo millis()

#include <wifi_component.h> // como tal no son componentes de ESP-IDF, sino ejemplos de implementación de dichos componentes u otros extra
#include <mqtt_component.h>
#include <uart_component.h>

// Estructura para los mensajes
typedef struct {
    char sensor_data[20];
    uint64_t entrada_sistema_us;
} zumo_data_t;

// Variable global para el manejador de la cola
QueueHandle_t zumo_queue;

static const char* TAG = "main_tag";

void logica_mqtt_recibido(char *topic, char *data, int len) {

    // 1. Limitar el tamaño para no desbordar el buffer del Zumo
    // La mayoría de comandos (S:100, P:ACC_ON) son cortos.
    char buf[32]; 
    int final_len = (len < sizeof(buf) - 2) ? len : sizeof(buf) - 2;
    memcpy(buf, data, final_len);

    // 2. AÑADIR UN CARÁCTER DE TERMINACIÓN
    // El Zumo leerá hasta encontrar un '\n' (salto de línea).
    buf[final_len] = '\n'; 
    final_len++; 

    // 3. ENVIAR POR UART
    // Usamos el puerto que definiste en tu componente (ej. UART_NUM_1)
    int bytes_enviados = uart_write_bytes(UART_PORT, buf, final_len);

    if (bytes_enviados > 0) {
        // Opcional: Log para depurar que el ESP32 está reenviando
        ESP_LOGI("MQTT_TO_UART", "Enviado al Zumo: %.*s", final_len - 1, buf);
    }
    else {
        ESP_LOGE("MQTT_TO_UART", "Error al enviar por UART");
    }
}

void uart_rx_task(void *arg) {
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE);
    zumo_data_t mensaje_para_cola; // La estructura que definimos para la Queue

    while (1) {
        // Leemos del puerto UART
        int len = uart_read_bytes(UART_PORT, data, BUF_SIZE - 1, pdMS_TO_TICKS(10));
        
        if (len > 0) {
            data[len] = '\0';
            char* msg = (char*)data;

            // 1. Detección de VELOCIDAD (VR:45)
            if (msg[0] == 'V') {
                int velocidad = atoi(&msg[3]);
                //ESP_LOGI(TAG, "Velocidad Zumo: %d", velocidad);
                
                // Preparamos para enviar a la nube vía Queue
                snprintf(mensaje_para_cola.sensor_data, sizeof(mensaje_para_cola.sensor_data), "VR:%d", velocidad);
                mensaje_para_cola.entrada_sistema_us = esp_timer_get_time();
                xQueueSend(zumo_queue, &mensaje_para_cola, 0);
            }
            
            // 2. Detección de OBJETOS (OI:1 o OD:0)
            else if (msg[0] == 'O' && (msg[1] == 'I' || msg[1] == 'D')) {
                char lado = msg[1]; // 'I' o 'D'
                int detectado = atoi(&msg[3]); // El valor después del ':'

                if (detectado == 1) {
                    ESP_LOGW(TAG, "¡Objeto detectado a la %s!", (lado == 'I' ? "IZQUIERDA" : "DERECHA"));
                }
                
                // Enviamos el estado del objeto a la nube para telemetría
                strncpy(mensaje_para_cola.sensor_data, msg, sizeof(mensaje_para_cola.sensor_data));
                xQueueSend(zumo_queue, &mensaje_para_cola, 0);
            }
        }
    }
    free(data); // Nunca se llegará aquí en este loop, pero es buena práctica
    vTaskDelete(NULL);
}

void mqtt_tx_task(void *arg) {
    zumo_data_t datos_recibidos;
    while (1) {
        // Se queda bloqueado aquí hasta que llegue algo a la cola
        if (xQueueReceive(zumo_queue, &datos_recibidos, portMAX_DELAY)) {
            
            uint64_t ahora = esp_timer_get_time();
            
            // Publicamos el contenido del sensor_data en el tópico de telemetría
            mqtt_publicar_mensaje("zumo/telemetria", datos_recibidos.sensor_data);

            // CÁLCULO DE LATENCIA
            //uint64_t latencia = ahora - datos_recibidos.entrada_sistema_us;

            // MOSTRAR POR CONSOLA
            //ESP_LOGI("PERF", "Mensaje: %s | Latencia cola uart-mqtt: %llu us", 
            //         datos_recibidos.sensor_data, latencia);
        }
    }
}

void app_main(void)
{
    // Creamos una cola para 10 elementos de tipo zumo_data_t
    zumo_queue = xQueueCreate(10, sizeof(zumo_data_t));

    // 2. Configuración de Hardware y Red
    iniciar_wifi();
    init_uart(); // ¡No olvides inicializar el driver UART!
    iniciar_mqtt(logica_mqtt_recibido); // Pasamos el callback

    // 3. Lanzar las Tareas
    // Tarea 1: Lee de UART -> Escribe en Cola
    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 10, NULL);
    
    // Tarea 2: Lee de Cola -> Envía a MQTT (¡LA QUE TE FALTABA!)
    xTaskCreate(mqtt_tx_task, "mqtt_tx_task", 4096, NULL, 5, NULL);

    // 4. El app_main puede morir o quedarse vigilando el sistema
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}
