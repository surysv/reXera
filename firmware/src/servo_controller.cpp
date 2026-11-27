#include "servo_controller.h"

#include <driver/mcpwm.h>
#include <driver/gpio.h>

namespace {

enum SequenceState : uint8_t {
  SEQ_IDLE = 0,
  SEQ_WAIT_AFTER_SERVO2,
  SEQ_WAIT_AFTER_SERVO1
};

SequenceState sequenceState = SEQ_IDLE;

uint32_t sequenceWaitMs = 1000;
uint32_t sequenceDeadline = 0;

int currentServo1Angle = SERVO1_HOME_ANGLE;
int currentServo2Angle = SERVO2_HOME_ANGLE;


// ======================================================
// CONVERSIÓN DE ÁNGULO A PULSO
// ======================================================

bool angleToPulse(
  int angle,
  int maxAngle,
  uint32_t &pulseUs
) {

  if (maxAngle <= 0 || angle < 0 || angle > maxAngle) {
    return false;
  }

  pulseUs = SERVO_MIN_PULSE_US +
      (uint32_t)(
        (uint64_t)(SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)
        * angle / maxAngle
      );

  return true;
}


// ======================================================
// ESCRIBIR POSICIÓN DE UN SERVO
// ======================================================

void writeServo(
  uint8_t servo,
  int angle
) {

  uint32_t pulseUs = 0;

  if (
    servo == 1 &&
    angleToPulse(
      angle,
      SERVO1_MAX_ANGLE,
      pulseUs
    )
  ) {

    mcpwm_set_duty_in_us(
      MCPWM_UNIT_0,
      MCPWM_TIMER_0,
      MCPWM_OPR_A,
      pulseUs
    );

    currentServo1Angle = angle;

  }

  else if (
    servo == 2 &&
    angleToPulse(
      angle,
      SERVO2_MAX_ANGLE,
      pulseUs
    )
  ) {

    mcpwm_set_duty_in_us(
      MCPWM_UNIT_0,
      MCPWM_TIMER_1,
      MCPWM_OPR_A,
      pulseUs
    );

    currentServo2Angle = angle;
  }
}

}  // namespace


// ======================================================
// INICIALIZACIÓN
// ======================================================

void servosInit() {

  // Servo 1 -> MCPWM TIMER 0, operador A.
  mcpwm_gpio_init(
    MCPWM_UNIT_0,
    MCPWM0A,
    (gpio_num_t)SERVO1_PIN
  );

  // Servo 2 -> MCPWM TIMER 1, operador A.
  mcpwm_gpio_init(
    MCPWM_UNIT_0,
    MCPWM1A,
    (gpio_num_t)SERVO2_PIN
  );

  mcpwm_config_t config = {};

  config.frequency = SERVO_PWM_FREQUENCY_HZ;
  config.cmpr_a = 0.0;
  config.cmpr_b = 0.0;
  config.counter_mode = MCPWM_UP_COUNTER;
  config.duty_mode = MCPWM_DUTY_MODE_0;

  mcpwm_init(
    MCPWM_UNIT_0,
    MCPWM_TIMER_0,
    &config
  );

  mcpwm_init(
    MCPWM_UNIT_0,
    MCPWM_TIMER_1,
    &config
  );

  // Posiciones iniciales.
  writeServo(1, SERVO1_HOME_ANGLE);
  writeServo(2, SERVO2_HOME_ANGLE);

  sequenceState = SEQ_IDLE;
}


// ======================================================
// CONTROL INDIVIDUAL
// ======================================================

bool servoSetAngle(
  uint8_t servo,
  int angle
) {

  if (servo == 1) {

    if (angle < 0 || angle > SERVO1_MAX_ANGLE) {
      return false;
    }

    writeServo(1, angle);
    return true;
  }

  if (servo == 2) {

    if (angle < 0 || angle > SERVO2_MAX_ANGLE) {
      return false;
    }

    writeServo(2, angle);
    return true;
  }

  return false;
}


// ======================================================
// VOLVER A POSICIÓN INICIAL
// ======================================================

void servosHome() {

  servosCancelSequence();

  writeServo(
    2,
    SERVO2_HOME_ANGLE
  );

  writeServo(
    1,
    SERVO1_HOME_ANGLE
  );
}


void servoHome(uint8_t servo) {

  servosCancelSequence();

  if (servo == 1) {

    writeServo(
      1,
      SERVO1_HOME_ANGLE
    );
  }

  else if (servo == 2) {

    writeServo(
      2,
      SERVO2_HOME_ANGLE
    );
  }
}


// ======================================================
// INICIAR SECUENCIA AUTOMÁTICA
// ======================================================

bool servosStartSequence(
  uint32_t waitMs
) {

  if (waitMs > 60000) {
    return false;
  }

  servosCancelSequence();

  // PASO 1:
  // Servo 2 a su posición máxima.
  writeServo(
    2,
    SERVO2_MAX_ANGLE
  );

  sequenceWaitMs = waitMs;

  sequenceDeadline = millis() + sequenceWaitMs;

  sequenceState = SEQ_WAIT_AFTER_SERVO2;

  return true;
}


// ======================================================
// CANCELAR SECUENCIA
// ======================================================

void servosCancelSequence() {

  if (sequenceState != SEQ_IDLE) {

    sequenceState = SEQ_IDLE;

    Serial.println(
      "SERVO_SEQUENCE_CANCELLED"
    );
  }
}


// ======================================================
// ACTUALIZAR SECUENCIA
// ======================================================

void servosUpdate() {

  if (sequenceState == SEQ_IDLE) {
    return;
  }

  if (
    (int32_t)(millis() - sequenceDeadline) < 0
  ) {
    return;
  }

  // ----------------------------------
  // ESPERA 1 TERMINADA
  // ----------------------------------

  if (
    sequenceState == SEQ_WAIT_AFTER_SERVO2
  ) {

    // PASO 2:
    // Servo 1 a su posición máxima.
    writeServo(
      1,
      SERVO1_MAX_ANGLE
    );

    sequenceDeadline = millis() + sequenceWaitMs;

    sequenceState = SEQ_WAIT_AFTER_SERVO1;

    Serial.println(
      "SERVO_SEQUENCE_STEP_2"
    );

    return;
  }

  // ----------------------------------
  // ESPERA 2 TERMINADA
  // ----------------------------------

  if (
    sequenceState == SEQ_WAIT_AFTER_SERVO1
  ) {

    // PASO 3:
    // Servo 2 vuelve a origen.
    writeServo(
      2,
      SERVO2_HOME_ANGLE
    );

    // PASO 4:
    // Servo 1 vuelve a origen.
    writeServo(
      1,
      SERVO1_HOME_ANGLE
    );

    sequenceState = SEQ_IDLE;

    Serial.println(
      "SERVO_SEQUENCE_DONE"
    );
  }
}


// ======================================================
// CONSULTAS
// ======================================================

bool servosSequenceRunning() {

  return sequenceState != SEQ_IDLE;
}


int servoCurrentAngle(
  uint8_t servo
) {

  if (servo == 1) {
    return currentServo1Angle;
  }

  if (servo == 2) {
    return currentServo2Angle;
  }

  return -1;
}
