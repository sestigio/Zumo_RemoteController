#ifndef MQTT_COMPONENT_H
#define MQTT_COMPONENT_H

void iniciar_mqtt(void);
//void mqtt_publicar(const char *topic, const char *data);
void enviar_telemetria_zumo(int velocidad_actual);

#endif