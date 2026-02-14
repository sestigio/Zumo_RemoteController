/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "MQTT_APP";
static esp_mqtt_client_handle_t global_client; // Referencia global para publicar

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
            // 1. Creamos un buffer temporal para el comando y lo terminamos en NULL
            char buf[32]; 
            int len = (event->data_len < sizeof(buf) - 1) ? event->data_len : sizeof(buf) - 1;
            memcpy(buf, event->data, len);
            buf[len] = '\0';

            ESP_LOGI(TAG, "Mensaje recibido: %s", buf);

            // 2. PARSER DE PREFIJOS (El corazón de la comunicación)
            
            // Caso A: DIRECCIÓN (S:valor)
            if (buf[0] == 'S' && buf[1] == ':') {
                angulo_giro = atoi(&buf[2]);
                ESP_LOGI(TAG, "Giro actualizado: %d", angulo_giro);
            } 
            // Caso B: PEDALES (P:comando)
            else if (buf[0] == 'P' && buf[1] == ':') {
                char *comando = &buf[2];
                
                if (strcmp(comando, "ACC_ON") == 0) {
                    //velocidad_objetivo = 100; // O la lógica que decidas
                    ESP_LOGI(TAG, "Acelerador presionado");
                } else if (strcmp(comando, "ACC_OFF") == 0) {
                    //velocidad_objetivo = 0;
                } else if (strcmp(comando, "BRAKE_ON") == 0) {
                    //velocidad_objetivo = -50; // Marcha atrás o freno
                    ESP_LOGI(TAG, "Freno presionado");
                }
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
void iniciar_mqtt(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
    global_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(global_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(global_client);
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
