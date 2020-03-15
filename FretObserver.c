char sendByte = 3; // byte to send over I2C
char receivedByte = 0; // byte received over I2C
short clearReg = 0; // used to read a register for purposes of clearing it

// sets the lower 4 bits of the I2C slave device address to contain the
// status of the frets
short set_state(int i) {
  short state = (log(i) / log(2)) + 1;
  SSPADD &= 0b11100000;
  SSPADD |= state << 1;
}

void I2C_Slave_Init(short address) {

//  WPUB |= 0x12; // set pull up resistors

  SSPSTAT = 0x80;
  SSPADD = address;   // setting address
  SSP1CON1 = 0x36;    // 7-bit slave device mode
  SSP1CON2 = 0x01;    // clock stretching is enanbled
  
  TRISB.RB1 = 1;    //setting SCL and SDL to input
  TRISB.RB4 = 1;
  
  INTCON.GIE = 1;   //Global interrupt enable
  INTCON.PEIE = 1;  //Peripheral interrupt enable
  PIR1.SSP1IF = 0;  //Clear interrupt flag
  PIE1.SSP1IE = 1;  //Synchronous serial port interrupt enable
}

void interrupt() {
  
  // I2C interrupt
  if(PIR1.SSP1IF == 1) {
    SSP1CON1.CKP = 0; // set clock line low
    
    // if SSPBUF buffer overflow or collision
    if ((SSP1CON1.SSPOV) || (SSP1CON1.WCOL)) {
      clearReg = SSPBUF; // Read the previous value to clear the buffer
      SSP1CON1.SSPOV = 0; // Clear the overflow flag
      SSP1CON1.WCOL = 0; // Clear the collision bit
      SSP1CON1.CKP = 1; // release the clock by setting it high
    }
  
    // If last byte received was address in write mode
    if (!SSPSTAT.D_NOT_A && !SSPSTAT.R_NOT_W) {
      while(!SSPSTAT.BF); // while Receive not complete
      receivedByte = SSPBUF; // receive byte
      SSP1CON1.CKP = 1; // release the clock by setting it high
    }
  
    // if last byte receieved was address in read mode
    else if (!SSPSTAT.D_NOT_A && SSPSTAT.R_NOT_W) {
      clearReg = SSPBUF; // read the buffer to clear it
      SSPSTAT.BF = 0; // state that buffer is being written to
      SSPBUF = sendByte; // write the byte to send to the buffer
      SSP1CON1.CKP = 1; // release the clock
      while(SSPSTAT.BF); // wait for buffer to be read
    }

    PIR1.SSP1IF = 0; // clear interrupt flag
  }
}

// capacitive touch input detection
char touch_switch_pin (int pin) {
  int const dischargeDelay = 250;
  int const chargedDelay = 2;

  TRISB = 0x12;              // output (apart from RB1 and RB4 for I2C clock and data lines)
  TRISA = 0x00;              // output

  PORTA.RA1 = 0;
  PORTA.RA4 = 0;
  
  PORTB &= 0b00010010;       // discharge pins (apart from RB1 and RB4)

  delay_us(dischargeDelay);  // Wait
    
  if (pin == 2 || pin == 16) {
    PORTA |= pin;         // Turn on the Constant current source
    TRISA |= pin;         // input

    delay_Us(chargedDelay); // Wait

    if ((PORTA & pin) == pin)
      return (0);       // No touch detected
    else
      return (1);       // Touch detected
  }
    
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
  
  // supported 7-bit addresses 0b1100000, 0b1000000, 0b0100000, 0b1110000
  short address = 0b1000000 << 1;
  I2C_Slave_Init(address);

  PORTA = 0x00;
  TRISA = 0x00;
  ANSELA = 0x00;

  TRISB = 0x12;
  ANSELB = 0x00;
  
  while(1) {
    inner:
    for (i = 128; i > 0; i/=2) {
      if(touch_switch_pin(i) == 0) {
        PORTA.RA2 = 1; // debug LED
        set_state(i); // set fret status
        goto inner;
      }
    }
    PORTA.RA2 = 0; // debug LED
    SSPADD &= 0b11100000; // set fret status (no fret pressed)
  }
}