 /*
 Example 41.1 - Microchip MCP23017 with Arduino
 http://tronixstuff.com/tutorials > chapter 41
 John Boxall | CC by-sa-nc
*/
// pins 15~17 to GND, I2C bus address is 0x20
#include <Wire.h>
#include <Joystick.h>


const uint8_t IODIRA = 0x00;
const uint8_t IODIRB = 0x01;
const uint8_t IPOLA  = 0x02;
const uint8_t IPOLB  = 0x03;

const uint8_t GPIOA = 0x12;
const uint8_t GPIOB = 0x13;

const uint8_t GPPUA = 0x0c;
const uint8_t GPPUB = 0x0d;

const uint8_t ExpanderOne = 0x20;
const uint8_t ExpanderTwo = 0x21;

// Bit values
const uint8_t BIT0 = 0x01;
const uint8_t BIT1 = 0x02;
const uint8_t BIT2 = 0x04;
const uint8_t BIT3 = 0x08;
const uint8_t BIT4 = 0x10;
const uint8_t BIT5 = 0x20;
const uint8_t BIT6 = 0x40;
const uint8_t BIT7 = 0x80;

uint8_t IIC_error;
byte inputs = 0;

// report errors by blinking onboard LED
void blink_LED(int count) {
  
  // blink the error on the built in LED
  if (count != 0) {
    while (true) {
      for (int i = 0; i < count; i++ ) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
      }
      delay( 2000 );
    }
  }
}

void setup()
{
    // Initialize Joystick Library
  Joystick.begin();
  Wire.begin(); // wake up I2C bus
  
  // set Port B I/O pins to outputs - LEDS
  Wire.beginTransmission(ExpanderOne);

  Wire.write(IODIRB); // IODIRB register
  Wire.write(~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7)); // set all of port B to outputs
 
  Wire.endTransmission();

  // set Port A I/O pins to inputs - switches on expander 1
  Wire.beginTransmission(ExpanderOne);
  Wire.write(IODIRA); // IODIRB register
  Wire.write(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7); // set all of port A to inputs
  Wire.endTransmission();

  // set Port A I/O pins to inputs - switches on expander 2
  Wire.beginTransmission(ExpanderTwo);
  Wire.write(IODIRA); // IODIRB register
  Wire.write(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7); // set all of port A to inputs
  Wire.endTransmission();    // set Port A I/O pins to inputs - switches on expander 2

  // set Port B I/O pins to inputs - switches on expander 2
  Wire.beginTransmission(ExpanderTwo);
  Wire.write(IODIRB); // IODIRB register
  Wire.write(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7); // set all of port A to inputs
  blink_LED( Wire.endTransmission());
  
  // set up expander 1 inputs - jut port A - pull ups, and polarity
  Wire.beginTransmission(ExpanderOne);
  Wire.write(GPPUA);
  Wire.write(0xff);   // pullups on all inputs 
  Wire.endTransmission();
  Wire.beginTransmission(ExpanderOne);
  Wire.write(IPOLA);
  Wire.write(0xff);   // invert all inputs - normally goes low on button press
  Wire.endTransmission();

  // set up expander 2 inputs - Port A and Port B - pull ups, and polarity
  Wire.beginTransmission(ExpanderTwo);
  Wire.write(GPPUA);
  Wire.write(0xff);   // pullups on all inputs 
  Wire.endTransmission();
  Wire.beginTransmission(ExpanderTwo);
  Wire.write(GPPUB);
  Wire.write(0xff);   // pullups on all inputs 
  Wire.endTransmission();
  Wire.beginTransmission(ExpanderTwo);
  Wire.write(IPOLA);
  Wire.write(0xff);   // invert all inputs - normally goes low on button press
  Wire.endTransmission();
  Wire.beginTransmission(ExpanderTwo);
  Wire.write(IPOLB);
  Wire.write(0xff);   // invert all inputs - normally goes low on button press
  Wire.endTransmission();
  


  binaryDisplay(0);
  
}

// return the bit number of the first set bit found (starting at 0)
int whatBit( uint8_t num)
{
  for (int a = 0; a<8; a++)
  {
    if ( bitRead( num, a ) == 1 )
      return a;
  }
  return -1;
}

// What Button? - report what button is pushed  - only one button can be active at a time
// buttons 0-5 - hatch buttons
// buttons 6-15 - hatch buttons
// buttons 16-23 - extra TBD buttons

int curButton = -1; // The current button that is active.  -1 means nothing active
                    // Previously set button - needed to know if button changes


// returns curButton or -1 if no button is pressed
int whatButton()
{

  int thisButton = -1;    // what button do we see right now
  
  // check button values
  uint8_t portData1a = GetPort( ExpanderOne, GPIOA); // Hatch buttons 0-6
  uint8_t portData2a = GetPort( ExpanderTwo, GPIOA); // Cargo 0-7
  uint8_t portData2b = GetPort( ExpanderTwo, GPIOB); // Cargo 8-9
  
//  Serial.println( portData1a, BIN);
//  Serial.println( portData2a, BIN);
//  Serial.println( portData2b, BIN);
//  Serial.println("--");
//  delay(200);

  if ( portData1a != 0x0 ) { // buttons 0-7
    thisButton = whatBit( portData1a );  
    if ( thisButton > 5 )
      thisButton = -1; 
  } else if ( portData2a != 0x0 ) { 
    thisButton = whatBit( portData2a ) + 6;   // buttons 7 - 13
  }  else if ( portData2b != 0x0 ) { 
    thisButton = whatBit( portData2b ) + 14;   // buttons 7 - 13
    if ( thisButton > 15 )
      thisButton = -1;
  }


  return thisButton;

  
}

void binaryDisplay(uint8_t num)
{
  // write to port B
  Wire.beginTransmission(ExpanderOne);
  Wire.write(GPIOB); // GPIOB
  Wire.write(num); // port B
  Wire.endTransmission();
  delay( 50 );
} 

uint8_t GetPort( uint8_t dev, uint8_t port )
{
  uint8_t data;
  Wire.beginTransmission( dev );
  Wire.write( port );
  Wire.endTransmission();
  Wire.requestFrom(dev, 1);

  data = Wire.read();

  return data;
}
 

int buttonCount = 0;

bool buttonState = false;

void loop()
{

  int button =  whatButton();

  if (button < 0) {
    if ( curButton >= 0 ) {
       Joystick.setButton(curButton, LOW);
       curButton = -1;
       return;
    }
  } else {
    if (button == curButton)    // do nothing button has not changed
      return;
  }

  curButton = button;   // remember the button pressed

  if (curButton >= 0) {
    Joystick.setButton(curButton, HIGH);
    binaryDisplay( curButton+1 );
  } else {
    binaryDisplay( 0 );
  }
 
}
