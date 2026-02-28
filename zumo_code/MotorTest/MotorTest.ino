/* This example drives Zumo forward and backward and allows steering.  The yellow user LED is on when Zumo is accelerating and the red
when it is decelerating.
If a motor on your Zumo has been flipped, you can correct its
direction by uncommenting the call to flipLeftMotor() or
flipRightMotor() in the setup() function. */

#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4Motors motors;
Zumo32U4ButtonC buttonC;

const int MAX_SPEED = 400; // El Zumo 32U4 acepta valores de -400 a 400
const float ALFA_VELOCIDAD = 0.03;
const float ALFA_GIRO = 0.1; // Para suavizar cambios bruscos en el giro
const float COEF_GIRO = 2.0;

const unsigned long INTERVALO_TELEMETRIA = 300; // 100ms = 10 Hz (suficiente para gráficas)

// Definimos el "Estado" del robot
struct RobotState {
    float giro_objetivo;          
    float giro_actual;             // Giro actual suavizado
    float velocidad_objetivo;     
    float velocidad_actual;         // Velocidad actual suavizada
    bool pedalAcel;      // Estado del acelerador
    bool pedalFreno;     // Estado del freno
    unsigned long ultimoEnvioTelemetria; // Para controlar el tiempo
    unsigned long ultimoComandoRecibido; // Para seguridad (timeout)
  } g_zumo = {0.0, 0.0, false, false, 0, 0}; // Instancia única global

void setup()
{
  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);
  Serial.begin(9600);
  Serial1.begin(9600);

  // Wait for the user to press button C.
  buttonC.waitForButton();

  // Delay so that the robot does not move away while the user is
  // still touching it.
  delay(2000);
}

void procesar_comando(String comando) {
  g_zumo.ultimoComandoRecibido = millis(); // Actualizamos el tiempo del último comando recibido
  if (comando.startsWith("S:")) {
      g_zumo.giro_objetivo = comando.substring(2).toInt();
  } 
  else if (comando.startsWith("P:")) {
      String accion = comando.substring(2);

      if (accion == "ACC_ON")   {
        g_zumo.pedalAcel = true; 
        Serial.println("Aceleración ON");
        g_zumo.velocidad_objetivo = 250;
        ledYellow(1);
      }
      // if (accion == "ACC_OFF")  {
      //   g_zumo.pedalAcel = false; 
      //   Serial.println("Aceleración OFF");
      // }
      else if (accion == "BRAKE_ON") {
        g_zumo.pedalFreno = true; 
        g_zumo.velocidad_objetivo = -250;
        ledYellow(1);
        //Serial.println("Freno ON");
      }
      // if (accion == "BRAKE_OFF") {
      //   g_zumo.pedalFreno = false; 
      //   //Serial.println("Freno OFF");
      // }
      else if (accion == "STOP") {
        g_zumo.pedalAcel = false; 
        g_zumo.pedalFreno = false; 
        g_zumo.velocidad_objetivo = 0;
        //alfa_velocidad = ALFA_NEUTRO;
        ledYellow(0);
        ledRed(0);
        //Serial.println("Pedales OFF");
      }
      
  }
}

void actualizar_motores() {
  // Seguridad: Si no hay noticias de la ESP32 hace más de 500ms, PARADA TOTAL
    if (millis() - g_zumo.ultimoComandoRecibido > 2000) {
        g_zumo.velocidad_objetivo = 0;
        g_zumo.giro_objetivo = 0;
        motors.setSpeeds(0, 0);
        ledRed(1); // Aviso visual de pérdida de enlace
        return; 
    }

  // Filtro de paso bajo para el giro
  g_zumo.giro_actual = (ALFA_GIRO * g_zumo.giro_objetivo) + (1.0 - ALFA_GIRO) * g_zumo.giro_actual;

  // 2. Suavizar velocidad (Filtro)
  g_zumo.velocidad_actual = (ALFA_VELOCIDAD * g_zumo.velocidad_objetivo) + (1.0 - ALFA_VELOCIDAD) * g_zumo.velocidad_actual;

  // 3. Mezclar Velocidad + Giro (Diferencial)
  // El giro suma a un lado y resta al otro
  float diferencia = g_zumo.giro_actual * COEF_GIRO;
  int vel_izq = g_zumo.velocidad_actual + diferencia;
  int vel_der = g_zumo.velocidad_actual - diferencia;

  // 4. Limitar y aplicar
  vel_izq = constrain(vel_izq, -MAX_SPEED, MAX_SPEED);
  vel_der = constrain(vel_der, -MAX_SPEED, MAX_SPEED);

  motors.setSpeeds(vel_izq, vel_der);
}

void enviar_telemetria() {
    unsigned long tiempoActual = millis();

    // Comprobamos si ha pasado el intervalo (ej. 100ms)
    if (tiempoActual - g_zumo.ultimoEnvioTelemetria >= INTERVALO_TELEMETRIA) {
        // Calculamos una velocidad media aproximada para enviar
        // (En el Zumo 32U4 podrías usar los encoders para velocidad real)
        int velocidad_a_enviar = (int)g_zumo.velocidad_actual;

        Serial1.print("VR:"); // Usamos VR para que tu Python lo reconozca
        Serial1.println(velocidad_a_enviar);

        Serial.print("VR:"); // Usamos VR para que tu Python lo reconozca
        Serial.println(velocidad_a_enviar);

        Serial.print("Estado accel: ");
        Serial.println(g_zumo.pedalAcel);

        Serial.print("Estado freno: ");
        Serial.println(g_zumo.pedalFreno);

        Serial.print("Giro: ");
        Serial.println(g_zumo.giro_actual);

        // Actualizamos el cronómetro
        g_zumo.ultimoEnvioTelemetria = tiempoActual;
    }
}

void loop()
{
  // 1. Comprobar si hay comando de la ESP32 y procesarlo
    if (Serial1.available() > 0) {
        String comando = Serial1.readStringUntil('\n');
        comando.trim();
        procesar_comando(comando);
        //actualizar_motores();
    }

  // 2. Aplicar la lógica de control para actualizar motores
  actualizar_motores();

  // 3. Enviar datos de vuelta (Solo cuando toque, ej. 10 Hz)
  enviar_telemetria();

  delay(10); // Mantiene el loop a ~100Hz
}
