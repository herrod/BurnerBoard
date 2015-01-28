
const INPUT = 0;
const OUTPUT = 1;

const LOW = 0;
const HIGH = 1;

function digitalWrite(pin, value) {
}

function digitalRead(pin) {

  return 0;
}

function pinMode(pin, value) {
}

function analogRead(pin) {
  if (pin == 1)
    return 1000;    
  return 0;
}

function button_mode_up() {
  encoder_pos++;
  clear_screen();
}

function button_mode_down() {
  encoder_pos--;
  if (encoder_pos < 0)
    encoder_pos = 0;
  clear_screen();
}

audio.play();
