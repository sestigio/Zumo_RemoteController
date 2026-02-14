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

# RUTA PARA PEDALES (Acelerar / Frenar)
@app.route('/pedals', methods=['POST'])
def control_pedals():
    data = request.json
    comando = data.get('cmd') # 'accel', 'brake', 'stop'...
    
    # Enviamos al Zumo con un prefijo para que sepa qué es
    # Ejemplo: "P:accel"
    bridge.enviar_comando(f"P:{comando}")
    
    return jsonify({"status": "ok", "type": "pedal", "val": comando})

# RUTA PARA DIRECCIÓN (Giro del volante)
@app.route('/steer', methods=['POST'])
def control_steer():
    data = request.json
    giro = data.get('s', 0) # Valor entre -100 y 100
    
    # Enviamos al Zumo con prefijo de Steering
    # Ejemplo: "S:45"
    bridge.enviar_comando(f"S:{giro}")
    
    return jsonify({"status": "ok", "type": "steer", "val": giro})

# RUTA PARA RECUPERAR VELOCIDAD (Telemetría)
@app.route('/get_telemetry', methods=['GET'])
def get_telemetry():
    return jsonify({
        "velocidad_real": bridge.velocidad_real
    })

if __name__ == '__main__':
    # host='0.0.0.0' para que el móvil pueda acceder vía IP
    app.run(host='0.0.0.0', port=5000, debug=True, use_reloader=False)