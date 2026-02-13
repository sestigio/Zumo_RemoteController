from flask import Flask, render_template, request, jsonify
from flask_cors import CORS
from mqtt_bridge import bridge

app = Flask(__name__)
CORS(app)

# Inicializamos la conexión MQTT al arrancar el servidor
bridge.connect()

@app.route('/')
def index():
    # Esta función busca el archivo index.html dentro de la carpeta /templates/
    return render_template('index.html')

@app.route('/update', methods=['POST'])
def update_robot():
    # Recibimos un JSON con {v: velocidad, s: steering}
    data = request.json
    v = data.get('v', 0)
    s = data.get('s', 0)
    
    # Enviamos un mensaje compacto al Zumo: "V:50,S:-10"
    mensaje = f"V:{v},S:{s}"
    bridge.enviar_comando(mensaje)
    
    return jsonify({"status": "sent", "data": mensaje})

if __name__ == '__main__':
    # host='0.0.0.0' para que el móvil pueda acceder vía IP
    app.run(host='0.0.0.0', port=5000, debug=True, use_reloader=False)