from flask import Flask, jsonify
from flask_cors import CORS
from mqtt_bridge import bridge

app = Flask(__name__)
CORS(app)

# Inicializamos la conexión MQTT al arrancar el servidor
bridge.connect()

@app.route('/')
def home():
    return {"status": "online", "robot": "Zumo Pololu"}

@app.route('/mover/<direccion>')
def mover(direccion):
    # Usamos el puente para enviar el comando
    bridge.enviar_comando(direccion)
    return jsonify({
        "status": "comando_procesado",
        "destino": "ESP32-C6",
        "comando": direccion
    })

if __name__ == '__main__':
    # host='0.0.0.0' para que el móvil pueda acceder vía IP
    app.run(host='0.0.0.0', port=5000, debug=True, use_reloader=False)