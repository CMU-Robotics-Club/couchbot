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

void loop() {
  //readLeftMotorSerial();
  //readRightMotorSerial();

  mapJoystickToVandW(v, w);

  if (!cruise_control) {
    if (v > v_lag + 5) {
      v_lag += 5;
    } else if (v < v_lag - 15) {
      v_lag -= 15;
    } else {
      v_lag = v;
    }
    v_lag = max(v_lag, 0.0f);
  }

  if (w > w_lag + 15) {
    w_lag += 15;
  } else if (w < w_lag - 15) {
    w_lag -= 15;
  } else {
    w_lag = w;
  }

  if (v_lag == 0 && w_lag == 0) {
    i_term_left = 0;
    i_term_right = 0;
  }

  float vl = (v_lag + w_lag);
  float vr = (v_lag - w_lag);

  // Ensure that turning with no throttle
  // won't just throw away half the power
  //float discarded_turn = min(0.0f, min(vl, vr));
  //vl -= discarded_turn;
  //vr -= discarded_turn;

  //vl = max(vl, 0.0f);
  //vr = max(vr, 0.0f);
  if (vl > 1000 || vr > 1000) {
    float divisor = max(vl, vr);
    vl = vl * 1000 / divisor;
    vr = vr * 1000 / divisor;
  }

  if (reverse) {
    vl *= -1;
    vr *= -1;
  }

  // P controller
  float vl_err = vl - leftmotor_speed;
  float vr_err = vr - rightmotor_speed;

  i_term_left = constrain(i_term_left + vl_err, -i_max, i_max);
  i_term_right = constrain(i_term_right + vr_err, -i_max, i_max);

  // Note that the mapping from RPM to power is almost 1:1 (0-750 to 0-1000), so
  // we don't need to do any extra math here.
  //int32_t power_l = (int32_t)(vl + vl_err * kp + i_term_left * ki);
  //int32_t power_r = (int32_t)(vr + vr_err * kp + i_term_right * ki);

  int32_t power_l = (int32_t)(vl);
  int32_t power_r = (int32_t)(vr);

  power_l = constrain(power_l, -1000, 1000);
  power_r = constrain(power_r, -1000, 1000);

  power_l = (power_l * 7.5);
  power_r = (power_r * 7.5);

  vesc.setRPM(power_l);
  vesc.setRPM(power_r, CAN_ID_R);

  //commandMotor(LeftMotor, power_l);
  //commandMotor(RightMotor, power_r);

  delay(25);
}

void mapJoystickToVandW(float &v, float &w) {
  v = joystickY > 0 ? map(joystickY, 0, 127, 0, 750) : 0;
  v = v * v / 750;
  w = map(joystickX, -128, 127, -100.0, 100.0);

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
