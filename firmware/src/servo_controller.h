#pragma once

#include <Arduino.h>

// Ajusta estos parámetros durante la instalación.
// Los pines 8 y 9 son valores genéricos; a cambiar.
#ifndef SERVO1_PIN
#define SERVO1_PIN 8
#endif

#ifndef SERVO2_PIN
#define SERVO2_PIN 9
#endif

#ifndef SERVO1_MAX_ANGLE
#define SERVO1_MAX_ANGLE 120
#endif

#ifndef SERVO2_MAX_ANGLE
#define SERVO2_MAX_ANGLE 90
#endif

#ifndef SERVO1_HOME_ANGLE
#define SERVO1_HOME_ANGLE 0
#endif

#ifndef SERVO2_HOME_ANGLE
#define SERVO2_HOME_ANGLE 0
#endif

#ifndef SERVO_MIN_PULSE_US
#define SERVO_MIN_PULSE_US 500
#endif

#ifndef SERVO_MAX_PULSE_US
#define SERVO_MAX_PULSE_US 2500
#endif

#ifndef SERVO_PWM_FREQUENCY_HZ
#define SERVO_PWM_FREQUENCY_HZ 50
#endif

// Inicialización de ambos servos.
void servosInit();

// Movimiento individual.
bool servoSetAngle(uint8_t servo, int angle);

// Movimiento a posición inicial.
void servosHome();
void servoHome(uint8_t servo);

// Secuencia automática.
bool servosStartSequence(uint32_t waitMs);

// Cancelación y actualización.
void servosCancelSequence();
void servosUpdate();

// Consulta del estado.
bool servosSequenceRunning();
int servoCurrentAngle(uint8_t servo);
