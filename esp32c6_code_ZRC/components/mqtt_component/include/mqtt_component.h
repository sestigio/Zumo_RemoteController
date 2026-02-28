#ifndef MQTT_COMPONENT_H
#define MQTT_COMPONENT_H

typedef void (*mqtt_data_callback_t)(char* topic, char* data, int len);
// Prototipos de tus funciones
void iniciar_mqtt(mqtt_data_callback_t cb);
void mqtt_publicar_mensaje(const char* topic, const char* data);

#endif