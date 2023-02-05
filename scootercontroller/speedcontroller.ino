static volatile uint8_t leftmotor_dat[256] = {0};
static volatile uint8_t leftmotor_pos = 0;

static volatile uint8_t rightmotor_dat[256] = {0};
static volatile uint8_t rightmotor_pos = 0;

static volatile const uint32_t RX_PACKET_SIZE = 14;

void readLeftMotorSerial() {
  #if 1
  while (Serial.available()) {
    // get the new byte:
    uint8_t inByte = Serial.read();
    // add it to the inputString:
    leftmotor_dat[leftmotor_pos] = inByte;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:

    if (inByte == 0x0E && leftmotor_pos > 0 && leftmotor_dat[leftmotor_pos - 1] == 0x02) {
      // Start of a new command
      leftmotor_pos = 2;
      leftmotor_dat[0] = 0x02;
      leftmotor_dat[1] = 0x0E;
    } else if (leftmotor_pos == RX_PACKET_SIZE - 1) {
      leftmotor_pos = 0;
      uint16_t raw_speed = (((uint16_t)leftmotor_dat[8]) << 8) | ((uint16_t)leftmotor_dat[9]);
      // Serial.println(raw_speed, HEX);
      leftmotor_speed = motor_speed_to_rpm(raw_speed);
    } else {
      leftmotor_pos++;
    }
  }
  #endif
}

void readRightMotorSerial() {
  #if 1
  while (Serial2.available()) {
    // get the new byte:
    uint8_t inByte = Serial2.read();
    // add it to the inputString:
    rightmotor_dat[rightmotor_pos] = inByte;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:

    if (inByte == 0x0E && rightmotor_pos > 0 && rightmotor_dat[rightmotor_pos - 1] == 0x02) {
      // Start of a new command
      rightmotor_pos = 2;
      rightmotor_dat[0] = 0x02;
      rightmotor_dat[1] = 0x0E;
    } else if (rightmotor_pos == RX_PACKET_SIZE - 1) {
      rightmotor_pos = 0;
      uint16_t raw_speed = (((uint16_t)rightmotor_dat[8]) << 8) | ((uint16_t)rightmotor_dat[9]);
      //Serial.println(raw_speed, HEX);
      rightmotor_speed = motor_speed_to_rpm(raw_speed);
    } else {
      rightmotor_pos++;
    }
  }
  #endif
}

float motor_speed_to_rpm(uint16_t speed) {
  if (speed == 0x1770) {
    return 0.0;
  } else {
    return 60000.0 / ((float)speed + 1.0);
  }
}