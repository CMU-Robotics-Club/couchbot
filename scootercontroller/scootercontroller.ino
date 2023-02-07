#define LeftMotor Serial
#define RightMotor Serial2

static int16_t joystickX = 0;
static int16_t joystickY = 0;
static bool cruise_control = false;
static size_t current_mode = 0;
static bool baby_mode = true;

static volatile float leftmotor_speed = 0;
static volatile float rightmotor_speed = 0;

const static float kp = 0.5;
const static float ki = 0.05;
const static float i_max = 500;
static float i_term_left = 0.0;
static float i_term_right = 0.0;

static bool reverse = false;
static bool braking = false;

#include <VescUart.h>

VescUart vesc;

#define CAN_ID_L 60
#define CAN_ID_R 61

void setup() {
  motorControllerSetup();
  dualshockSetup();
  delay(1000);
  
  Serial2.begin(115200);

  vesc.setSerialPort(&Serial2);
}

float v_lag = 0;
float v = 0;
float w_lag = 0;
float w = 0;

#define RPM_PER_MPH 603.0

#define MAX_SPEED 10.0

#define LOOP_PERIOD_MS 25

#define ACCEL_RAMP (5.0 * (LOOP_PERIOD_MS / 1000.0))
#define DECEL_RAMP (5.0 * (LOOP_PERIOD_MS / 1000.0))
#define BRAKING_RAMP (30.0 * (LOOP_PERIOD_MS / 1000.0))

#define STEER_RAMP (360.0 * (LOOP_PERIOD_MS / 1000.0))

// Degrees per second
#define MAX_STEER 60.0

#define WHEEL_BASE_MILES (30.375 / 63360.0)

void loop() {
  //readLeftMotorSerial();
  //readRightMotorSerial();

  mapJoystickToVandW(v, w);
  
  if (braking) {
    cruise_control = false;

    v = 0;

    if (v < v_lag - BRAKING_RAMP) {
      v_lag -= BRAKING_RAMP;
    } else {
      v_lag = 0.0;
    }
  } else {
    if (!cruise_control) {
      if (v > v_lag + ACCEL_RAMP) {
        v_lag += ACCEL_RAMP;
      } else if (v < v_lag - DECEL_RAMP) {
        v_lag -= DECEL_RAMP;
      } else {
        v_lag = v;
      }
      v_lag = max(v_lag, 0.0f);
    }
  }

  float v_mph = v_lag;
  if (reverse) {
    v_mph *= -1.0;
  }

  if (w > w_lag + STEER_RAMP) {
    w_lag += STEER_RAMP;
  } else if (w < w_lag - STEER_RAMP) {
    w_lag -= STEER_RAMP;
  } else {
    w_lag = w;
  }

  // degrees per second
  float w_rate = w_lag * 2 * M_PI / 360;
  // degrees per hour
  w_rate *= 3600.0;
  float turn_rate_mph = w_rate * WHEEL_BASE_MILES / 2.0;

  float mph_l = (v_mph + turn_rate_mph);
  float mph_r = (v_mph - turn_rate_mph);

  if (abs(mph_l) > MAX_SPEED || abs(mph_r) > MAX_SPEED) {
    float divisor = max(abs(mph_l), abs(mph_r)) / MAX_SPEED;
    mph_l = mph_l / divisor;
    mph_r = mph_r / divisor;
  }

  mph_l = constrain(mph_l, -MAX_SPEED, MAX_SPEED);
  mph_r = constrain(mph_r, -MAX_SPEED, MAX_SPEED);

  float erpm_l = mph_l * RPM_PER_MPH;
  float erpm_r = mph_r * RPM_PER_MPH; 

  vesc.setRPM(erpm_l);
  vesc.setRPM(erpm_r, CAN_ID_R);

  delay(LOOP_PERIOD_MS);
}

void mapJoystickToVandW(float &v, float &w) {
  v = joystickY > 0 ? map(joystickY, 0, 127, 0.0, MAX_SPEED) : 0;
  v = v * v / MAX_SPEED;

  w = map(joystickX, -128, 127, -MAX_STEER, MAX_STEER);

  // Map v to lower range depending on current mode
  if (baby_mode) {
    v /= 5;
  } else {
    switch (current_mode) {
      case 0:
        {
          v /= 3;
          break;
        }
      case 1:
        {
          v = (v * 2) / 3;
        }
      case 2:
      default:
        {
          // No scale-down
          break;
        }
    }
  }
}
