#include <Ps3Controller.h>

uint32_t secret_speed_mode_timer = millis();

void dualshockSetup() {
  Ps3.attach(dualshockNotify);
  Ps3.begin();
}

void dualshockNotify() {
  /* Joystick */
  if (abs(Ps3.event.analog_changed.stick.lx)) {
    joystickX = Ps3.data.analog.stick.lx;
  }

  if (abs(Ps3.event.analog_changed.button.l2)) {
    joystickY = Ps3.data.analog.button.l2 / 2;
    if (!abs(Ps3.data.analog.button.right)) {
      cruise_control = false;
    }
  }

  if (abs(Ps3.event.analog_changed.stick.rx)) {
    joystickX = Ps3.data.analog.stick.rx;
  }

  if (abs(Ps3.event.analog_changed.button.r2)) {
    joystickY = Ps3.data.analog.button.r2 / 2;
    if (!abs(Ps3.data.analog.button.right)) {
      cruise_control = false;
    }
  }


  /* Cruise Control */
  if (!cruise_control) {
    if (Ps3.event.button_down.right) {
      cruise_control = true;
    }
  } else {
    if (Ps3.event.button_up.up) {
      v_lag += 0.25;
      v_lag = constrain(v_lag, 0, MAX_SPEED);
    }
    if (Ps3.event.button_up.down) {
      v_lag -= 0.25;
      v_lag = constrain(v_lag, 0, MAX_SPEED);
    }
  }


  /* Secret SPEEEEEED mode */
  if (current_speed < 0.1) {
    // Only change speed mode when not moving
    if (abs(Ps3.event.analog_changed.button.triangle)) {
      int32_t speed_index = Ps3.data.analog.button.triangle / 64;
      Ps3.setPlayer(speed_index + 1);
      if (speed_index != 2) {
        secret_speed_mode_timer = millis();
      } else {
        if (millis() > secret_speed_mode_timer + 3000) {
          current_mode = 2;
          Ps3.setRumble(100.0);
          delay(250);
          Ps3.setRumble(0.0);
          secret_speed_mode_timer = millis();
        }
      }
    } else if (!Ps3.data.analog.button.triangle) {
      if (baby_mode) {
        Ps3.setPlayer(10);
      } else {
        Ps3.setPlayer(current_mode + 1);
      }
    }
    if (Ps3.event.button_up.circle) {
      current_mode = 0;
      Ps3.setRumble(100.0);
      delay(250);
      Ps3.setRumble(0.0);
    }
    if (Ps3.event.button_down.triangle) {
      current_mode = 1;
      Ps3.setRumble(100.0);
      delay(250);
      Ps3.setRumble(0.0);
    }

    if (Ps3.event.button_up.square) {
      baby_mode = !baby_mode;
      Ps3.setRumble(100.0);
      delay(250);
      Ps3.setRumble(0.0);
    }
  }

  if (Ps3.event.button_down.cross) {
    reverse = true;
  }
  if (Ps3.event.button_up.cross) {
    reverse = false;
    reverseGracePeriod = 5;
  }

  if (Ps3.event.button_down.r1) {
    braking = true;
  }
  if (Ps3.event.button_up.r1) {
    braking = false;
  }

  if (Ps3.event.button_down.l1) {
    braking = true;
  }
  if (Ps3.event.button_up.l1) {
    braking = false;
  }
}