
// capacitive touch input detection
char touch_switch_pin (int pin) {
  int const dischargeDelay = 250;
  int const chargedDelay = 2;

  TRISB = 0x00;              // output
  PORTB &= 0xF0;             // discharge pins RB0-RB3

  delay_us(dischargeDelay);  // Wait

  PORTB |= pin;         // Turn on the Constant current source
  TRISB |= pin;         // input

  delay_Us(chargedDelay); // Wait

  if ((PORTB & pin) == pin)
    return (0);       // No touch detected
  else
    return (1);       // Touch detected
}

void main() {
  int i;

  PORTA = 0x00;
  TRISA = 0x00;
  ANSELA = 0x00;

  TRISB = 0x00;
  ANSELB = 0x00;

  while(1) {
    for (i = 1; i < 9; i*=2) {
      if(touch_switch_pin(i) == 0) {
        // if touch detected, set appropriate pin connected to adafruit high
        switch(i) {
          case 1:
            PORTB.RB4 = 1;
            break;
          case 2:
            PORTB.RB5 = 1;
            break;
          case 4:
            PORTB.RB6 = 1;
            break;
          case 8:
            PORTB.RB7 = 1;
            break;
        }
      } else {
        // if touch not detected, set appropriate pin connected to adafruit low
        switch(i) {
          case 1:
            PORTB.RB4 = 0;
            break;
          case 2:
            PORTB.RB5 = 0;
            break;
          case 4:
            PORTB.RB6 = 0;
            break;
          case 8:
            PORTB.RB7 = 0;
            break;
        }
      }
    }
  }
}