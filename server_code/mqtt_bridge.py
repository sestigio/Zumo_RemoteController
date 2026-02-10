import paho.mqtt.client as mqtt

class MQTTBridge:
    def __init__(self, broker="localhost", port=1883):
        self.client = mqtt.Client()
        self.broker = broker
        self.port = port
        self.topic_control = "zumo/comandos"
        self.topic_telemetria = "zumo/telemetria"

    def connect(self):
        try:
            self.client.on_connect = self.on_connect
            self.client.on_message = self.on_message
            self.client.connect(self.broker, self.port, 60)
            self.client.loop_start()
            print(f"✅ MQTT: Conectado a {self.broker}")
        except Exception as e:
            print(f"❌ MQTT: Error de conexión: {e}")

    def on_connect(self, client, userdata, flags, rc):
        print(f"📡 MQTT: Suscrito a {self.topic_telemetria}")
        self.client.subscribe(self.topic_telemetria)

    def on_message(self, client, userdata, msg):
        # Aquí procesaremos lo que el Zumo nos envíe (sensores, batería, etc.)
        print(f"🤖 Zumo dice: {msg.payload.decode()}")

    def enviar_comando(self, comando):
        self.client.publish(self.topic_control, comando)
        print(f"📤 Comando enviado: {comando}")

# Instancia global para ser importada
bridge = MQTTBridge()