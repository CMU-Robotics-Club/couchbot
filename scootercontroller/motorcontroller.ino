static const uint8_t start_cmd_bytes[] = { 0x01, 0x14, 0x01, 0x02, 0x05, 0x80, 0x1E, 0x00, 0x42, 0x01, 0x05, 0x00, 0x64, 0x0C, 0x01, 0x40, 0x00, 0x00, 0x16, 0x14, 0x01, 0x02, 0x05, 0x80, 0x1E, 0x00, 0x42, 0x01, 0x05, 0x00, 0x64, 0x0C, 0x01, 0x40, 0x00, 0x00, 0x16, 0xF4 };
static const uint8_t cmd_bytes[] = { 0x01, 0x14, 0x01, 0x02, 0x0F, 0x80, 0x1E, 0x00, 0x42, 0x01, 0x05, 0x00, 0x64, 0x0C, 0x01, 0x40, 0x00, 0x00, 0x16, 0x00 };
static const size_t START_CMD_BYTES_LEN = 38;
static const size_t CMD_BYTES_LEN = 20;

static const uint8_t MODES[] = {0x05, 0x0A, 0x0F};


void motorControllerSetup() {
  //LeftMotor.begin(9600, SERIAL_8N1);
  //RightMotor.begin(9600, SERIAL_8N1, 16, 17);  // ESP32 Hardware Serial2
  // Serial1 is used for flash programming, so using it causes a kernel panic

  //sendInitCmd(LeftMotor);
  //sendInitCmd(RightMotor);
}

void sendInitCmd(Stream &ser) {
  //ser.write(start_cmd_bytes, START_CMD_BYTES_LEN);
}

void commandMotor(Stream &ser, uint16_t val) {
  // Val ranges from 0 to 1000
  /*uint8_t cmd[CMD_BYTES_LEN];
  memcpy(cmd, cmd_bytes, CMD_BYTES_LEN);

  cmd[4] = MODES[current_mode];
  cmd[16] = (val >> 8) & 0xFF;
  cmd[17] = val & 0xFF;
  calc_checksum(cmd);

  ser.write(cmd, CMD_BYTES_LEN);*/
}

void calc_checksum(uint8_t *cmd) {
  uint8_t checksum = cmd[0];
  for (int i = 1; i < CMD_BYTES_LEN - 1; i++) {
    checksum ^= cmd[i];
  }
  cmd[CMD_BYTES_LEN - 1] = checksum;
}