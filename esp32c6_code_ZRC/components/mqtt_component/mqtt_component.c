#include "mqtt_component.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "MQTT_APP";
static esp_mqtt_client_handle_t global_client; // Referencia global para publicar

static mqtt_data_callback_t master_callback = NULL;

// Variables globales para que tu lógica de control las consulte
//int velocidad_objetivo = 0; // Se derivará de los pedales
int angulo_giro = 0;        // -100 a 100

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado al Broker");
            // Aquí puedes suscribirte a los comandos de la Raspberry Pi
            esp_mqtt_client_subscribe(global_client, "zumo/comandos", 0);
            break;
        case MQTT_EVENT_DATA:
            if (master_callback) {
                // Le pasamos el marrón a main.c, que es quien tiene la lógica de control. Solo le damos el topic, data y longitud.
                master_callback(event->topic, event->data, event->data_len);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT Desconectado");
            break;

        // ESTA ES LA CLAVE:
        default:
            ESP_LOGD(TAG, "Evento MQTT no manejado ID: %" PRIi32, event_id);
            break;
    }
}

// Función que llamarás desde el main
void iniciar_mqtt(mqtt_data_callback_t cb) {
    master_callback = cb;
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
    global_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(global_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(global_client);
}   

// En mqtt_component.c 
//Función para envío genérico de mensajes
void mqtt_publicar_mensaje(const char* topic, const char* data) {
    if (global_client != NULL) {
        esp_mqtt_client_publish(global_client, topic, data, 0, 1, 0);
    }
}

// Función para enviar la velocidad real al móvil
void enviar_telemetria_zumo(int velocidad_actual) {
    char data_str[16];
    // Creamos el mensaje con el prefijo "VR:" que espera el bridge.py
    sprintf(data_str, "VR:%d", velocidad_actual);
    
    if (global_client != NULL) {
        esp_mqtt_client_publish(global_client, "zumo/telemetria", data_str, 0, 1, 0);
    }
}

// NUEVA FUNCIÓN: Para publicar desde cualquier sitio
// void mqtt_publicar(const char *topic, const char *data) {
//     if (global_client != NULL) {
//         esp_mqtt_client_publish(global_client, topic, data, 0, 1, 0);
//     }
// }
