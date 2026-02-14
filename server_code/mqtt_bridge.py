import paho.mqtt.client as mqtt

class MQTTBridge:
    def __init__(self, broker="localhost", port=1883):
        self.client = mqtt.Client()
        self.broker = broker
        self.port = port
        self.topic_command = "zumo/comandos"
        self.topic_telemetria = "zumo/telemetria"

        # Variable para almacenar la velocidad que viene del Zumo
        self.velocidad_real = 0

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
        payload = msg.payload.decode()
        # Esperamos que el Zumo envíe algo como "VR:45" (Velocidad Real)
        if payload.startswith("VR:"):
            try:
                self.velocidad_real = int(payload.split(":")[1])
                print(f"📈 Telemetría recibida: {self.velocidad_real} km/h")
            except ValueError:
                pass

    def enviar_comando(self, comando):
        self.client.publish(self.topic_command, comando)
        print(f"📤 Comando enviado: {comando}")

# Instancia global para ser importada
bridge = MQTTBridge()