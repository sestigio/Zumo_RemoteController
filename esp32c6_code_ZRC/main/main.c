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

void mqtt_enviar_telemetria(void) {

    char json_buffer[512]; // Tamaño suficiente para el string

    // Creamos el JSON manualmente
    snprintf(json_buffer, sizeof(json_buffer),
        "{\"curr_st\":%d,\"prior_st\":%d,\"servo_st\":%d,\"gyro_st\":%d,\"mean\":%d,\"bearing\":%d,\"x_accel_glob\":%.2f,\"y_accel_glob\":%.2f,\"z_accel_glob\":%.2f,\"x1_accel\":%.2f,\"y1_accel\":%.2f,\"z1_accel\":%.2f,\"x2_accel\":%.2f,\"y2_accel\":%.2f,\"z2_accel\":%.2f,\"x3_accel\":%.2f,\"y3_accel\":%.2f,\"z3_accel\":%.2f,\"s1_v_sh\":%.2f,\"s1_curr\":%.2f,\"s1_v_bus\":%.2f,\"s2_v_sh\":%.2f,\"s2_curr\":%.2f,\"s2_v_bus\":%.2f}",
        current_state, prior_state, servo_state, gyro_state, mobile_mean, servo_bearing, x_accel_glob, y_accel_glob, z_accel_glob, x1_accel, y1_accel, z1_accel, x2_accel, y2_accel, z2_accel, x3_accel, y3_accel, z3_accel,servo1_shunt_voltage, servo1_current, servo1_bus_voltage, servo2_shunt_voltage, servo2_current, servo2_bus_voltage);

    // Reutilizamos tu función de publicar
    mqtt_publicar("ZRC_project/telemetry", json_buffer);
}

void app_main(void)
{
    //============= WIFI SETUP ===============//
    iniciar_wifi();

    //============= MQTT SETUP ===============//
    iniciar_mqtt();

    // VARIABLES DE INTERVALO TEMPORAL PARA ENVÍO MQTT
    int64_t last_send_time = 0;
    const int64_t send_interval = 1000000; // 1 segundo en microsegundos (1000ms)

    while (1)
    {        
        int64_t now = esp_timer_get_time();
        if ((now - last_send_time) >= send_interval) {
            mqtt_enviar_telemetria();
            last_send_time = now;
        }
    }

}
