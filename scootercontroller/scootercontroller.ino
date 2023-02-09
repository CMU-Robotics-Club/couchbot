#include <VescUart.h>

#define DEBUG true

// Constants
static const uint32_t CAN_ID_R = 61;

static const float RPM_PER_MPH = 603.0;
static const float WHEEL_BASE_MILES = (30.375 / (12.0 * 5280.0));

static const float MAX_SPEED = 12.0;        // Miles per hour
static const float MAX_STEER = 60.0;        // Degrees per second
static const uint32_t LOOP_PERIOD_MS = 25;  // Milliseconds

static const float ACCEL_RAMP = (5.0 * (LOOP_PERIOD_MS / 1000.0));
static const float DECEL_RAMP = (10.0 * (LOOP_PERIOD_MS / 1000.0));
static const float DECEL_BRAKE_CURRENT = 1.0;
static const float DECEL_BRAKE_STEER_MODULATION = 30.0;
static const float EBRAKE_CURRENT = 100.0;
static const float STEER_RAMP = (360.0 * (LOOP_PERIOD_MS / 1000.0));

// Variables
static int16_t joystickX = 0;
static int16_t joystickY = 0;
static bool cruise_control = false;
static size_t current_mode = 0;
static bool baby_mode = true;

static volatile float leftmotor_speed = 0;
static volatile float rightmotor_speed = 0;

static bool reverse = false;
static bool braking = false;

static int reverseGracePeriod = 0;

static float current_speed = 0;

VescUart vesc;

void setup() {
  dualshockSetup();
  delay(1000);

#if DEBUG
  Serial.begin(115200);
#endif

  Serial2.begin(115200);
  vesc.setSerialPort(&Serial2);
}

static float v_lag = 0;
static float w_lag = 0;

void loop() {
  float v = 0;
  float w = 0;
  mapJoystickToVandW(v, w);

  if (braking) {
    cruise_control = false;
    v = 0.0;
    v_lag = 0.0;
  } else {
    if (reverse) {
      v *= -1.0;
    }

    if (!cruise_control) {
      if (reverse) {
        if (v < v_lag - ACCEL_RAMP) {
          v_lag -= ACCEL_RAMP;
        } else if (v > v_lag + DECEL_RAMP) {
          v_lag += DECEL_RAMP;
        } else {
          v_lag = v;
        }
      } else {
        if (v > v_lag + ACCEL_RAMP) {
          v_lag += ACCEL_RAMP;
        } else if (v < v_lag - DECEL_RAMP) {
          v_lag -= DECEL_RAMP;
        } else {
          v_lag = v;
        }
      }
    }
  }

  if (w > w_lag + STEER_RAMP) {
    w_lag += STEER_RAMP;
  } else if (w < w_lag - STEER_RAMP) {
    w_lag -= STEER_RAMP;
  } else {
    w_lag = w;
  }

  if (reverseGracePeriod > 0) {
    --reverseGracePeriod;
  }

  current_speed = current_speed * 0.75 + read_speed() * 0.25;

#if DEBUG
  Serial.printf("%f %f\n", current_speed, v_lag);
#endif

  // If we need to slow down, set the brake current instead of speed target
  if (braking) {
    vesc.setBrakeCurrent(EBRAKE_CURRENT);
    vesc.setBrakeCurrent(EBRAKE_CURRENT, CAN_ID_R);
  } else {
    if (v_lag > 0.5 && abs(current_speed) > abs(v_lag) + 0.05) {
      float brake_l = DECEL_BRAKE_CURRENT - (w_lag / MAX_STEER) * DECEL_BRAKE_STEER_MODULATION;
      float brake_r = DECEL_BRAKE_CURRENT + (w_lag / MAX_STEER) * DECEL_BRAKE_STEER_MODULATION;
      vesc.setBrakeCurrent(max(0.0f, brake_l));
      vesc.setBrakeCurrent(max(0.0f, brake_r), CAN_ID_R);
    } else {
      setSpeed(v_lag, w_lag);
    }
  }
  delay(LOOP_PERIOD_MS);
}

/* 
  v: miles per hour
  w: degrees per second
*/
void setSpeed(float v, float w) {
  float w_rate = w * M_PI / 180;  // radians per second
  w_rate *= 3600.0;               // radians per hour
  float turn_rate_mph = w_rate * WHEEL_BASE_MILES / 2.0;

  float mph_l = (v + turn_rate_mph);
  float mph_r = (v - turn_rate_mph);

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
}

void mapJoystickToVandW(float &v, float &w) {
  v = joystickY > 0 ? map(joystickY, 0, 127, 0.0, MAX_SPEED) : 0;
  v = v * v / MAX_SPEED;

  w = map(joystickX, -128, 127, -MAX_STEER, MAX_STEER);

  // Map v to lower range depending on current mode
  if (reverseGracePeriod > 0) {
    v = 0;
  } else if (baby_mode || reverse) {
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

float read_speed() {
  vesc.getVescValues();
  float leftRPM = vesc.data.rpm;
  vesc.getVescValues(CAN_ID_R);
  float rightRPM = vesc.data.rpm;

  float avgRPM = (leftRPM + rightRPM) / 2.0;
  float speed = avgRPM / RPM_PER_MPH;

  return speed;
}
