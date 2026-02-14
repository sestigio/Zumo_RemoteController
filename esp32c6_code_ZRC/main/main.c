#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h" //uso de tasks, delays y temporizadores
#include "freertos/task.h" 
#include "freertos/timers.h" 
#include "esp_log.h" // transmisión de mensajes al serial mejorada y detección de errores de ESP-IDF
// #include "driver/ledc.h" // uso de pwm
// #include "driver/gpio.h" // control de pines
// #include "driver/adc.h" // uso de adc de la placa
#include "driver/i2c.h" // para conexión con mpu6050
#include "esp_timer.h" // para timer estilo millis()

#include <wifi_component.h> // como tal no son componentes de ESP-IDF, sino ejemplos de implementación de dichos componentes u otros extra
#include <mqtt_component.h>

static const char* TAG = "main_tag";

void app_main(void)
{
    //============= WIFI SETUP ===============//
    iniciar_wifi();

    //============= MQTT SETUP ===============//
    iniciar_mqtt();

    // VARIABLES DE INTERVALO TEMPORAL PARA ENVÍO MQTT
    int64_t last_send_time = esp_timer_get_time(); // Inicializa el tiempo de envío al inicio
    const int64_t send_interval = 1000000; // 1 segundo en microsegundos (1000ms)

    while (1)
    {        
        int64_t now = esp_timer_get_time();
        if ((now - last_send_time) >= send_interval) {
            enviar_telemetria_zumo(45); // Ejemplo de velocidad real
            last_send_time = now;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Evita un bucle de CPU al 100%
    }

}
