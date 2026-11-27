/*

This is a very early testing sketch to get the arm working.
Do not use in prod.

*/


#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
// Depending on your servo make/model, these values may need tweaking 
// to reach the full 0-180 degree sweep
#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)

#define SERVOCUANTITY 6 // number of servos used

// Arm dimensions (mm)
#define WAIST_HEIGHT
#define WAIST_FRONT_DISPLACEMENT
#define ARM_LENGTH
#define SHOULDER_UP_DISPLACEMENT
#define SHOULDER_LENGTH
#define WRIST_LENGTH

#define DEBUG_MODE 1

#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.println(x)
  #define DEBUG_PRINTVAL(x, y) { Serial.print(x); Serial.println(y); }
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTVAL(x, y)
#endif

// Called this if you want to use the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


// Our servo channel # (0 to 15)
uint8_t servonum = 0;
uint8_t degree = 90;

struct ArmStruct {
  int waist;
  int shoulder;
  int elbow;
  int forearmd;
  int wristpitch;
  int wristroll;
};

ArmStruct armState;


int sweepdir = 1;
int sweeppos = 50;

// ============================================
//    SETUP
// ============================================

void setup() {
  Serial.begin(115200);
  DEBUG_PRINT("INITIALIZING..........................");

  // Initialize I2C pins explicitly for ESP32 D1 Mini (SDA = 21, SCL = 22)
  Wire.begin(21, 22);

  pwm.begin();
  
  // In theory the PCA9685 has a 50Hz clock, but in practice it can vary.
  // Setting it slightly lower or higher can help calibrate servos.
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  goToDefaultPosition();
  updateServos();
  delay(1000);
}

// ============================================
//    MAIN LOOP
// ============================================

void loop() {
  DEBUG_PRINT("Loop started");
  updateServos();
  
  moveServo(1, sweeppos);
  moveServo(2, 0);
  DEBUG_PRINT(sweeppos);
  sweeppos += sweepdir;

  if (sweeppos >= 70 || sweeppos <= 40) {
    sweepdir = -sweepdir;
  }
  
  DEBUG_PRINTVAL("Sweep to: ", sweeppos);

  delay(50);
}

/* Transforms a realworld position to degrees and writes it to armState */
void runIK() {
  
}

/* Transforms the given relative degree to absolute degree and writes it to armState, with constraints */
int moveServo(uint8_t servo, uint8_t degree) {
  DEBUG_PRINTVAL("[moveServo] Servo: ", servo);
  DEBUG_PRINTVAL("[moveServo] Input Degree: ", degree);
  
  int constrainedDeg = servoConstraints(servo, degree);
  
  DEBUG_PRINTVAL("[moveServo] Constrained Deg: ", constrainedDeg);
  
  switch (servo) {
    case 0: armState.waist = constrainedDeg; break;
    case 1: {
      armState.shoulder = constrainedDeg; 
      moveServo(2, armState.elbow); break;
    } 
    case 2: armState.elbow = constrainedDeg; break;
    case 3: armState.forearmd = constrainedDeg; break;
    case 4: armState.wristpitch = constrainedDeg; break;
    case 5: armState.wristroll = constrainedDeg; break;
  }
  DEBUG_PRINT("[moveServo] finished operation.");
  return 0;
}

/* Returns given degree after constraints for given servo */
int servoConstraints(uint8_t servo, uint8_t degree) {
  DEBUG_PRINTVAL("[Constraints] Evaluating servo ", servo);
  DEBUG_PRINTVAL("[Constraints] Degree: ", degree);

  switch (servo) {
    case 0: return constrain(degree, 0, 180);
    case 1: return constrain(degree, 10, 140); 
    case 2: {
        int relDeg = 170 + armState.shoulder + constrain(degree, -50, 40);
        return constrain(relDeg, 20, 180); 
    }
    case 3: return constrain(degree, 0, 180);
    case 4: return constrain(degree, 0, 180);
    case 5: return constrain(degree, 0, 180);
  }
  return -1;
}

/* Directly writes the default position into armState */
void goToDefaultPosition() {
    DEBUG_PRINT("[Init] Setting default position...");
    armState.waist       = 0;
    armState.shoulder   = 30;
    armState.elbow      = 120;
    armState.forearmd = 90;
    armState.wristpitch   = 180;
    armState.wristroll  = 90; 
}

/* Writes the values given in armState to the servos */
void updateServos() {
    pwm.setPWM(0, 0, degreeToPulse(armState.waist));
    pwm.setPWM(1, 0, degreeToPulse(armState.shoulder));
    pwm.setPWM(2, 0, degreeToPulse(armState.elbow));
    pwm.setPWM(3, 0, degreeToPulse(armState.forearmd));
    pwm.setPWM(4, 0, degreeToPulse(armState.wristpitch));
    pwm.setPWM(6, 0, degreeToPulse(armState.wristroll));
}

/* You know what this does */
int degreeToPulse(int degree) {
  return map(degree, 0, 180, SERVOMIN, SERVOMAX);
} 