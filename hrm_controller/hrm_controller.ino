/* Example for the ANT+ Library @ https://github.com/brodykenrick/ANTPlus_Arduino
Copyright 2013 Brody Kenrick.
Developed for http://retrorunnerreadout.blogspot.com
Interfacing of Garmin ANT+ device (via a cheap Nordic nRF24AP UART module) to an Arduino.
Opens an ANT+ channel listening for HRM. Prints out computed heart rate.
Hardware
An Arduino Pro Mini 3v3 connected to this nRF24AP2 module : http://www.goodluckbuy.com/nrf24ap2-networking-module-zigbee-module-with-ant-transceiver-.html
The connector on nRF24AP2 board is (looking from the front, pin 1 is marked []):
[]GND(=VSS) | VDD(=3.3 volts)
UART_TX   | UART_RX
!(SUSP)   | SLEEP
RTS       | !(RESET)
Wiring to the Arduino Pro Mini 3v3 can be seen in 'antplus' below.
*/

#include <Arduino.h>
#include "Adafruit_WS2801.h"

#include "FastLED.h"

#define LED_DATA_PIN 12
#define COLOR_ORDER GRB
#define NUM_LEDS 10
#define LED_TYPE WS2801

//#define NDEBUG
#define __ASSERT_USE_STDERR
#include <assert.h>

//#define ANTPLUS_ON_HW_UART //!< H/w UART (i.e. Serial) instead of software serial. NOTE: There seems to be issues in not getting as many broadcast packets when using hardware serial.........
//#define ANTPLUS_ON_HW_UART

#if !defined(ANTPLUS_ON_HW_UART)
#include <SoftwareSerial.h>
#endif

#include <ANTPlus.h>


#define USE_SERIAL_CONSOLE //!<Use the hardware serial as the console. This needs to be off if using hardware serial for driving the ANT+ module.
#if defined(NDEBUG) || defined(ANTPLUS_ON_HW_UART)
#undef USE_SERIAL_CONSOLE
#endif

// Initializing LEDs
// Choose which 2 pins you will use for output.
// Can be any valid output pins.
// The colors of the wires may be totally different so
// BE SURE TO CHECK YOUR PIXELS TO SEE WHICH WIRES TO USE!
uint8_t dataPin  = 12;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 13;    // Green wire on Adafruit Pixels

// Don't forget to connect the ground wire to Arduino ground,
// and the +5V wire to a +5V supply

// Set the first variable to the NUMBER of pixels. 25 = 25 pixels in a row
#define NUMBER_OF_LEDS 10
Adafruit_WS2801 strip = Adafruit_WS2801(NUMBER_OF_LEDS, dataPin, clockPin);


//Logging macros
//********************************************************************
#define SERIAL_DEBUG
#if !defined(USE_SERIAL_CONSOLE)
//Disable logging under these circumstances
#undef SERIAL_DEBUG
#endif

//F() stores static strings that come into existence here in flash (makes things a bit more stable)
#ifdef SERIAL_DEBUG

#define SERIAL_DEBUG_PRINT(x)            (Serial.print(x))
#define SERIAL_DEBUG_PRINTLN(x)         (Serial.println(x))
#define SERIAL_DEBUG_PRINT_F(x)   (Serial.print(F(x)))
#define SERIAL_DEBUG_PRINTLN_F(x) (Serial.println(F(x)))
#define SERIAL_DEBUG_PRINT2(x,y)    (Serial.print(x,y))
#define SERIAL_DEBUG_PRINTLN2(x,y)  (Serial.println(x,y))

#else

#define SERIAL_DEBUG_PRINT(x)
#define SERIAL_DEBUG_PRINTLN(x)
#define SERIAL_DEBUG_PRINT_F(x)
#define SERIAL_DEBUG_PRINTLN_F(x)
#define SERIAL_DEBUG_PRINT2(x,y)
#define SERIAL_DEBUG_PRINTLN2(x,y)

#endif

//********************************************************************

#define ANTPLUS_BAUD_RATE (9600) //!< The module I am using is hardcoded to this baud rate.


//The ANT+ network keys are not allowed to be published so they are stripped from here.
//They are available in the ANT+ docs at thisisant.com
//#define ANT_SENSOR_NETWORK_KEY {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
//#define ANT_GPS_NETWORK_KEY    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#define ANT_SENSOR_NETWORK_KEY {0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45}
#define ANT_GPS_NETWORK_KEY    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#if !defined( ANT_SENSOR_NETWORK_KEY ) || !defined(ANT_GPS_NETWORK_KEY)
#error "The Network Keys are missing. Better go find them by signing up at thisisant.com"
#endif

// ****************************************************************************
// ******************************  GLOBALS  ***********************************
// ****************************************************************************

//Arduino Pro Mini pins to the nrf24AP2 modules pinouts
static const int RTS_PIN      = 2; //!< RTS on the nRF24AP2 module
static const int RTS_PIN_INT  = 0; //!< The interrupt equivalent of the RTS_PIN

byte hr = 255; 
byte hr2 = 255; 

#if !defined(ANTPLUS_ON_HW_UART)
static const int TX_PIN       = 8; //Using software serial for the UART
static const int RX_PIN       = 9; //Ditto
static SoftwareSerial ant_serial(TX_PIN, RX_PIN); // RXArd, TXArd -- Arduino is opposite to nRF24AP2 module
#else
//Using Hardware Serial (0,1) instead
#endif

static ANTPlus        antplus   = ANTPlus(RTS_PIN, 3/*SUSPEND*/, 4/*SLEEP*/, 5/*RESET*/ );

//ANT Channel config for HRM
static ANT_Channel hrm_channel =
{
  0, //Channel Number
  PUBLIC_NETWORK,
  DEVCE_TIMEOUT,
  DEVCE_TYPE_HRM,
  DEVCE_SENSOR_FREQ,
  DEVCE_HRM_LOWEST_RATE,
  ANT_SENSOR_NETWORK_KEY,
  ANT_CHANNEL_ESTABLISH_PROGRESSING,
  FALSE,
  0, //state_counter
};

volatile int rts_ant_received = 0; //!< ANT RTS interrupt flag see isr_rts_ant()

// **************************************************************************************************
// *********************************  ISRs  *********************************************************
// **************************************************************************************************

//! Interrupt service routine to get RTS from ANT messages
void isr_rts_ant()
{
  rts_ant_received = 1;
}

// **************************************************************************************************
// ***********************************  ANT+  *******************************************************
// **************************************************************************************************

byte process_packet( ANT_Packet * packet )
{
#if defined(USE_SERIAL_CONSOLE) && defined(ANTPLUS_DEBUG)
  //This function internally uses Serial.println
  //Only use it if the console is available and if the ANTPLUS library is in debug mode
  antplus.printPacket( packet, false );
#endif //defined(USE_SERIAL_CONSOLE) && defined(ANTPLUS_DEBUG)
   
  switch ( packet->msg_id )
  {
    case MESG_BROADCAST_DATA_ID:
    {
      const ANT_Broadcast * broadcast = (const ANT_Broadcast *) packet->data;
      SERIAL_DEBUG_PRINT_F( "CHAN " );
      SERIAL_DEBUG_PRINT( broadcast->channel_number );
      SERIAL_DEBUG_PRINT_F( " " );
      const ANT_DataPage * dp = (const ANT_DataPage *) broadcast->data;
      
      //Update received data
      if( broadcast->channel_number == hrm_channel.channel_number )
      {
        hrm_channel.data_rx = true;
        //To determine the device type -- and the data pages -- check channel setups
        if(hrm_channel.device_type == DEVCE_TYPE_HRM)
        {
            switch(dp->data_page_number)
            {
              case DATA_PAGE_HEART_RATE_0:
              case DATA_PAGE_HEART_RATE_0ALT:
              case DATA_PAGE_HEART_RATE_1:
              case DATA_PAGE_HEART_RATE_1ALT:
              case DATA_PAGE_HEART_RATE_2:
              case DATA_PAGE_HEART_RATE_2ALT:
              case DATA_PAGE_HEART_RATE_3:
              case DATA_PAGE_HEART_RATE_3ALT:
              case DATA_PAGE_HEART_RATE_4:
              case DATA_PAGE_HEART_RATE_4ALT:
              {
                //As we only care about the computed heart rate
                // we use a same struct for all HRM pages
                const ANT_HRMDataPage * hrm_dp = (const ANT_HRMDataPage *) dp;
                SERIAL_DEBUG_PRINT_F( "HR[any_page] : BPM = ");
                SERIAL_DEBUG_PRINTLN( hrm_dp->computed_heart_rate );
                return hrm_dp->computed_heart_rate;
              }
              break;
  
              default:
                  SERIAL_DEBUG_PRINT_F(" HRM DP# ");
                  SERIAL_DEBUG_PRINTLN( dp->data_page_number );
                break;
            }
        }
    }
    break;
    }
    
    default:
      SERIAL_DEBUG_PRINTLN_F("Non-broadcast data received.");
      break;
  }
}




// **************************************************************************************************
// ************************************  Setup  *****************************************************
// **************************************************************************************************
void setup()
{
// Setup leds
  strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();
  
#if defined(USE_SERIAL_CONSOLE)
  //Serial.begin(115200); 
  Serial.begin(115200); 
#endif //defined(USE_SERIAL_CONSOLE)

  SERIAL_DEBUG_PRINTLN("ANTPlus HRM Test!");
  SERIAL_DEBUG_PRINTLN_F("Setup.");

  SERIAL_DEBUG_PRINTLN_F("ANT+ Config.");

  //We setup an interrupt to detect when the RTS is received from the ANT chip.
  //This is a 50 usec HIGH signal at the end of each valid ANT message received from the host at the chip
  attachInterrupt(RTS_PIN_INT, isr_rts_ant, RISING);


#if defined(ANTPLUS_ON_HW_UART)
  //Using hardware UART
  Serial.begin(ANTPLUS_BAUD_RATE); 
  antplus.begin( Serial );
#else
  //Using soft serial
  ant_serial.begin( ANTPLUS_BAUD_RATE ); 
  antplus.begin( ant_serial );
#endif

  SERIAL_DEBUG_PRINTLN_F("ANT+ Config Finished.");
  SERIAL_DEBUG_PRINTLN_F("Setup Finished.");
  
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(6, OUTPUT);
}

// **************************************************************************************************
// ************************************  Loop *******************************************************
// **************************************************************************************************

void loop()
{
  byte packet_buffer[ANT_MAX_PACKET_LEN];
  ANT_Packet * packet = (ANT_Packet *) packet_buffer;
  MESSAGE_READ ret_val = MESSAGE_READ_NONE;
  
  if(rts_ant_received == 1)
  {
    SERIAL_DEBUG_PRINTLN_F("Received RTS Interrupt. ");
    antplus.rTSHighAssertion();
    //Clear the ISR flag
    rts_ant_received = 0;
  }

  //Read messages until we get a none
  while( (ret_val = antplus.readPacket(packet, ANT_MAX_PACKET_LEN, 0 )) != MESSAGE_READ_NONE )
  {
    if((ret_val == MESSAGE_READ_EXPECTED) || (ret_val == MESSAGE_READ_OTHER))
    {
      SERIAL_DEBUG_PRINT_F( "ReadPacket success = " );
      if( (ret_val == MESSAGE_READ_EXPECTED) )
      {
        SERIAL_DEBUG_PRINTLN_F( "Expected packet" );
      }
      else
      if( (ret_val == MESSAGE_READ_OTHER) )
      {
        SERIAL_DEBUG_PRINTLN_F( "Other packet" );
      }
      hr = process_packet(packet);
      /*if (hr>50){
        hr2=hr-50;
        else
      }*/
      analogWrite(6, hr*1.5);
      colorWipe(Color(100, 100, 100), 50);
      //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      //delay(500/hr2);                       // wait for a second
      //digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      //delay(500/hr2);                       // wait for a second
      if (hr<60){
        colorWipe(Color(0, 0, 255), (uint8_t) 1000*60/hr);
      }else if (hr<75){
        colorWipe(Color(255, 0, 0), (uint8_t) 1000*60/hr);
      } else if (hr<90){
        colorWipe(Color(0, 255, 0), (uint8_t) 1000*60/hr);
      } else {
          rainbow(20); 
      }
    }
    else
    {
      SERIAL_DEBUG_PRINT_F( "ReadPacket Error = " );
      SERIAL_DEBUG_PRINTLN( ret_val );
      if(ret_val == MESSAGE_READ_ERROR_MISSING_SYNC)
      {
        //Nothing -- allow a re-read to get back in sync
      }
      else
      if(ret_val == MESSAGE_READ_ERROR_BAD_CHECKSUM)
      {
        //Nothing -- fully formed package just bit errors
      }
      else
      {
        break;
      }
    }
  }


  if(hrm_channel.channel_establish != ANT_CHANNEL_ESTABLISH_COMPLETE)
  {
    antplus.progress_setup_channel( &hrm_channel );
    if(hrm_channel.channel_establish == ANT_CHANNEL_ESTABLISH_COMPLETE)
    {
      SERIAL_DEBUG_PRINT( hrm_channel.channel_number );
      SERIAL_DEBUG_PRINTLN_F( " - Established." );
    }
    else
    if(hrm_channel.channel_establish == ANT_CHANNEL_ESTABLISH_PROGRESSING)
    {
      SERIAL_DEBUG_PRINT( hrm_channel.channel_number );
      SERIAL_DEBUG_PRINTLN_F( " - Progressing." );
    }
    else
    {
      SERIAL_DEBUG_PRINT( hrm_channel.channel_number );
      SERIAL_DEBUG_PRINTLN_F( " - ERROR!" );
    }
  }
  
  //Serial.println(hr);
  /*digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500/hr);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(500/hr);                       // wait for a second*/
/*  for (int i=0; i <= 255; i++){
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500/hr);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(500/hr);                       // wait for a second
  }*/
  
}






// LED FUNCTIONS

void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 256; j++) {     // 3 cycles of all 256 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 255));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 5; j++) {     // 5 cycles of all 25 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

