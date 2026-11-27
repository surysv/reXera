
# Servos y mando USB en XERA

## Configuración de los servos

Los parámetros se editan en `firmware/src/servo_controller.h`:

- `SERVO1_PIN`, `SERVO2_PIN`: GPIO de señal.
- `SERVO1_MAX_ANGLE` = 120 y `SERVO2_MAX_ANGLE` = 90.
- `SERVO1_HOME_ANGLE`, `SERVO2_HOME_ANGLE`: posición de origen.
- `SERVO_MIN_PULSE_US`, `SERVO_MAX_PULSE_US`: límites de pulso.
- `SERVO_PWM_FREQUENCY_HZ`: frecuencia de señal, 50 Hz por defecto.

El código usa el periférico MCPWM de ESP-IDF para no consumir los ocho canales LEDC que ya utiliza el control de los cuatro motores.

## Interfaz web

La pestaña Control incorpora:

- Dos deslizadores para las posiciones de servo.
- Botones de origen individual y conjunto.
- Tiempo X en milisegundos para la secuencia automática.
- Botón de ejecutar y cancelar secuencia.
- Selector de fuente: joystick web o mando USB.

Los mensajes WebSocket nuevos son `servo_set`, `servo_sequence`, `servo_home`, `servo_home_one` y `servo_cancel`. Todos requieren ser propietario del control. `stop` sigue disponible para cualquier cliente y cancela la secuencia.

## Mando PlayStation / USB

El mando se conecta al ordenador que ejecuta el navegador, no al ESP32. `frontend/gamepad.js` consulta la Gamepad API del navegador; `app.js` transforma las entradas y reutiliza `/ws/control`.

Mapeo estándar inicial:

| Entrada | Función |
|---|---|
| Stick izquierdo | Avance y giro del rover |
| Stick derecho | Ajuste incremental de servo 1 y servo 2 |
| Cruz (X) | Pedir o liberar control |
| Círculo | STOP |
| Cuadrado | Ambos servos a HOME |
| Triángulo | Ejecutar secuencia automática |

Se utiliza una zona muerta del 12 %. El mando debe ser reconocido por el sistema operativo y el navegador. Si el navegador informa un mapeo no estándar, habrá que adaptar los índices de ejes y botones de `gamepad.js`.

## LEDs: limitación de la versión recibida

La versión original del repositorio solo define `NUM_LEDS 1` en `firmware/src/main.cpp`, que corresponde al NeoPixel RGB de estado integrado en el ESP32-S3. No contiene controlador ni configuración de una cinta LED externa. No se ha reducido ese valor a cero porque apagaría el único indicador RGB del ESP32 y no modificaría una cinta que no está implementada en este código.
