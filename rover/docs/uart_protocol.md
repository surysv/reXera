# Protocolo UART

> Es el protocolo del firmware XERA (`firmware/src/main.cpp`, repo `4space-uniovi/xera`).
> Si el firmware cambia, solo hay que tocar `serial_controller.py` (ver "Cómo cambiar el formato").

## Protocolo `csv` (el actual)

Una línea ASCII por orden, terminada en `\n`:

```
throttle,steering\n
```

- **throttle** (avance): entero de -100 (marcha atrás) a 100 (adelante).
- **steering** (giro): entero de -100 (izquierda) a 100 (derecha). Con throttle = 0 el rover gira sobre sí mismo.

| Frase | Trama |
|-------|-------|
| avanzar recto | `50,0` |
| marcha atrás | `-50,0` |
| avanzar girando a la derecha | `50,30` |
| girar sobre sí mismo a la derecha | `0,100` |
| parar | `0,0` |

## Servos

Las órdenes de servos comparten el puerto serie con el movimiento. No cambian la codificación de las tramas de motores.

| Orden | Significado |
|---|---|
| `SERVO,1,60` | Llevar servo 1 a 60° (rango 0–120°) |
| `SERVO,2,45` | Llevar servo 2 a 45° (rango 0–90°) |
| `SERVO_SEQ,1000` | Ejecutar secuencia automática con X = 1000 ms |
| `SERVO_HOME` | Llevar ambos servos a las posiciones HOME del firmware |
| `SERVO_HOME,1` | Llevar servo 1 a su posición HOME |
| `SERVO_HOME,2` | Llevar servo 2 a su posición HOME |
| `SERVO_CANCEL` | Cancelar la secuencia y mantener las posiciones actuales |
| `STOP` | Parar motores y cancelar la secuencia |

La secuencia `SERVO_SEQ,X` es ejecutada localmente por el ESP32:

1. Servo 2 a su ángulo máximo.
2. Esperar X milisegundos.
3. Servo 1 a su ángulo máximo.
4. Esperar X milisegundos.
5. Servo 2 a HOME y servo 1 a HOME.

Se implementa mediante una máquina de estados basada en `millis()`, sin `delay()`.

## Respuestas del ESP32

El firmware emite mensajes de diagnóstico y confirmación, entre ellos:

- `RECEIVED -> Throttle: ... Steering: ...`
- `SERVO_ACCEPTED,...`
- `SERVO_SEQUENCE_STARTED,...`
- `SERVO_SEQUENCE_STEP_2`
- `SERVO_SEQUENCE_DONE`
- `SERVO_SEQUENCE_CANCELLED`
- `ERROR,...`
- `WATCHDOG -> no command, stopped`

El backend muestra la última línea recibida en el estado serie. La escritura en USB confirma que los bytes se enviaron al puerto, no que se haya verificado físicamente el movimiento del servo.


> ⚠️ **El orden es throttle primero.** En la web y en el código del backend se usa (x, y) con
> x = steering e y = throttle; el protocolo `csv` los intercambia al enviar. Valores fuera de
> -100..100 son ignorados por el firmware.

Parámetros del puerto: 115200 baudios, 8N1. El firmware responde `RECEIVED -> Throttle: … Steering: …`
por cada orden aceptada (se ve en la pestaña Estado).

## Cómo cambiar el formato

Todo el formato vive en `backend/controllers/serial_controller.py`. Crea una clase y regístrala;
el resto del backend no cambia:

```python
@register_protocol
class MiProtocolo(UartProtocol):
    name = "mio"

    def encode(self, x: int, y: int) -> bytes:       # x = steering, y = throttle
        return f"M {y} {x}\r\n".encode("ascii")

    def decode(self, frame: bytes) -> tuple[int, int]:   # lo usa el simulador
        _, y, x = frame.decode().split()
        return int(x), int(y)
```

Luego elige `mio` en Ajustes → Protocolo (o `serial.protocol: mio` en el YAML).

## Watchdog del firmware (obligatorio y AÚN NO IMPLEMENTADO)

El firmware actual **conserva la última orden indefinidamente**: si se desconecta el USB o muere el
proceso Linux mientras el rover avanza, los motores siguen. Hay que añadir en `firmware/src/main.cpp`
un temporizador que ponga `throttle = steering = 0` si no llega ninguna trama válida durante
~300–500 ms. El backend envía tramas a ~20 Hz mientras hay alguien conduciendo y `0,0` al parar.
