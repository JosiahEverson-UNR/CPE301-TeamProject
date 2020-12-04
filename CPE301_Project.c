
// Libraries
#include <Arduino.h>
#include <Wire.h>
//Elegoo
#include <dht_nonblocking.h>
#include <LiquidCrystal.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//REGISTERS

//PORT B DECLARATION: Used as an input for the LEDs
volatile unsigned char* myPORT_B = (unsigned char*) 0x25;
volatile unsigned char* myDDR_B  = (unsigned char*) 0x24;
volatile unsigned char* myPIN_B  = (unsigned char*) 0x23;

//PORT H DECLARATION: Used as an input for push button
volatile unsigned char* myPORT_H = (unsigned char*) 0x102;
volatile unsigned char* myDDR_H  = (unsigned char*) 0x101;
volatile unsigned char* myPIN_H  = (unsigned char*) 0x100;

//PORT F DECLARATION:
volatile unsigned char* myPORT_F = (unsigned char*) 0x31;
volatile unsigned char* myDDR_F  = (unsigned char*) 0x30;
volatile unsigned char* myPIN_F  = (unsigned char*) 0x2F;

//PORT K DECLARATION: Analog
volatile unsigned char* myPORT_K = (unsigned char*) 0x108;
volatile unsigned char* myDDR_K  = (unsigned char*) 0x107;
volatile unsigned char* myPIN_K  = (unsigned char*) 0x106;

//ADC Registers
volatile unsigned char* my_ADMUX     = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRA    = (unsigned char*) 0x7A;
volatile unsigned char* my_ADCSRB    = (unsigned char*) 0x7B;
volatile unsigned int*  my_ADC_DATA  = (unsigned int*)  0x78;

//TIMER Registers
volatile unsigned char *myTCCR1A = (unsigned char*) 0x80;      //for timer, can decide when to start/stop counting
volatile unsigned char *myTCCR1B = (unsigned char*) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char*) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char*) 0x6F;      //reset TOV bit
volatile unsigned int  *myTCNT1  = (unsigned int*)  0x84;      //Notes: TCNT increments every clk tick (62.5 nsec), when it hits max, it flips TOV bit then resets to 0.
volatile unsigned char *myTIFR1  = (unsigned char*) 0x36;      //Contains TOV (locate which bit)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Initializations OR Declarations

// Global Variables
// Water level threshold
#define w_threshold 200
// Temperature threshold
#define t_threshold 25

// float?
unsigned int humidity = 0;

// State Check Variables
// Whenever the counter is:
// 0, the system is at DISABLED state
// 1, the system is OPERATING state (working)

// Starts at DISABLED mode
unsigned int state_counter = 0;

// Initialize LCD
// RS: Pin 8, PH5
// Enabler (E): 7, PH4
// D4: 6, PH3
// D5: 5, PH
// D6: 4,
// D7: 3,
LiquidCrystal lcd(8,7,6,5,4,3);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  adc_init();

  Serial.begin(9600);

  // Set up the ddr, port registers for input and output ports

  //PORT B
  // & 0000 1111 (0x0F) makes pin 7 as an input
  *myDDR_B &= 0x0F;
  // | 1111 0000 enables internal pull-up resistor
  *myPORT_B |= 0xF0;

  //PORT H
  // & 1011 1111 (0xBF) makes pin 7 as an input
  *myDDR_H &= 0xBF;
  // | 0100 0000 (0x40) enables internal pull-up resistor
  *myPORT_H |= 0x40;

  //PORT F *****
  // & 1111 1111 (0xFF) makes all bits to output
  *myDDR_F &= 0xFF;
  // | 1000 0000 (0x80) enables internal pull-up resistor
  *myPORT_F |= 0x80;

  // LCD size
  lcd.begin(16,2);
  // Set so that LCD display start on upper-left corner
  lcd.setCursor(0,0);
}

/*
WORK BUCKET:
    //***** NEEDS CODE

1. Whether to separate every state into each function
   so that lcd can be displayed to each IDLE STATE.

   lcd_display (temperature, humidity);

2. Integrate motor into code.

3. Temperature reading

4. Humidity reading

5. Vent angle

6. Add Timer to record time whenever the system changes state.

7. Physically build the circuit

8. Project Report




*/

void loop()
{
  // Water Sensor: PK0

  // water level = adc_reading, channel 0
  unsigned int water_level = adc_read(0);
  // prints adc
  // print_int(adc_reading);
  Serial.println(water_level);

  // Thermometer/Temperature Sensor
  //*****
  //unsigned int temperature = 0;

  // If the system is disabled or OFF ******
  if(state_counter == 0)
  {
    // Checks whether the button is pushed; checks bit 6 (0100 0000)
    if (!(*myPIN_H & 0x40))
    {
      // A loop that does nothing to make sure noise is not included
      // (which occurs at a micro second)
      for (volatile unsigned int i = 0; i < 1000; i++);

      // Checks again if the button is pressed
      if (!(*myPIN_H & 0x40))
      {
        // It's currently at 0 then becomes 1
        state_counter++;
        // 1 % 2 = 1; which means that it's now in OPERATING mode
        state_counter %= 2;

        // LCD display
        lcd_display (temperature, humidity);
        // Function is called to determine the current states
        // Uses water_level and temperature variables as parameters
        state_checker0(water_level, temperature);

        // Makes sure that counter increments by one per pressed button
        while (!(*myPIN_H & 0x40));
      }
    }
  }


  //if the system is ON
  if(state_counter == 1)
  {
    //checks whether the button is pushed; checks bit 7
    if (!(*myPIN_B & 0x80))
    {
      //a loop that does nothing to make sure noise is not included (which occurs at a micro second)
      for (volatile unsigned int i = 0; i < 1000; i++);

      //checks again if the button is pressed
      if (!(*myPIN_B & 0x80))
      {
        // It's currently at 1 then becomes 0
        state_counter++;
        // 0 % 2 = 0; which means that it's now in DISABLED mode
        state_counter %= 2;

        // Function makes the system DISABLED mode
        state_checker1();

        //makes sure that counter increments by one per pressed button
        while (!(*myPIN_B & 0x80));

      }
    }
  }

}



//***********************\\FUNCTIONS//********************************

// STATE FUNCTION
// Filled with IF statements to output each states and their respective functions

unsigned int state_checker0 (int water_level, int temperature)
{

  // LEDs Location:
  // PB7 - GREEN (IDLE) LED,
  // PB6 - YELLOW (DISABLED) LED,
  // PB5 - RED (ERROR) LED,
  // PB4 - BLUE (RUNNING) LED
  // PH6 - Push Button


  // ===IDLE State===

  // GREEN LED ON (1000 0000)
  *myPORT_B &=  0x00;               //to turn them all off
  *myPORT_B |=  0x80;               //to turn on GREEN LED
  // Time stamps
  //*****

  // Monitor water level
  // If statement if the water level is under the threshold (low)

  if(water_level < w_threshold)
  {
    // ===ERROR State===

    // RED LED ON (0010 0000)
    *myPORT_B &=  0x00;               //to turn them all off
    *myPORT_B |=  0x20;               //to turn on RED LED

    // Error Message
    //Serial.println("Water level is too LOW");

    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // Continuously check the water level
    while (water_level < w_threshold)
    {
      if(water_level < w_threshold)
      {
        // ===ERROR State===

        // RED LED ON (0010 0000)
        *myPORT_B &=  0x00;               //to turn them all off
        *myPORT_B |=  0x20;               //to turn on RED LED

      }

      if(water_level > w_threshold)
      {

      }
    }
  }


  // If the water level is above the threshold (high)

  if(water_level > w_threshold)
  {

    // ===IDLE State===

    // Time stamps
    // Monitor water level

    // GREEN LED ON (1000 0000)
    *myPORT_B &=  0x00;               //to turn them all off
    *myPORT_B |=  0x80;               //to turn on GREEN LED


    // ===RUNNING State===

    // BLUE LED ON (0001 0000)
    *myPORT_B &=  0x00;               //to turn them all off
    *myPORT_B |=  0x10;               //to turn on BLUE LED

    // motor is on *****
    // [INSERT CODE]

  }
}


void state_checker1 ()
{
  // ===DISABLED State===
  //not monitoring any temperature or water level

  //Yellow LED on
  *myPORT_B &=  0x00;               //to turn them all off
  *myPORT_B |=  0x02;               //to turn on Yellow LED
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LCD Display Function

unsigned int lcd_display (int temperature, int humidity)
{
  // CLears Display
  lcd.clear();
  // Abbreviated to display temperature on one line
  lcd.print("Temp: ");
  // Displays Temperature Value from DHT function
  //lcd.print("DHT.temperature");
  // Prints degree symbol
  lcd.print((char)223);
  // prints "C" for Celsius
  lcd.print("C");
  // Adds new line
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  // Displays Humidity Value from DHT function
  //lcd.print("DHT.humidity");
  lcd.print("%");

  /* Displays

  Temp: [temperature]oC
  Humidity: [humidity]%

  */
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ADC Functions

void adc_init()
{
  //setup the A register

  //set bit 7 to 1 to enable the ADC
  *my_ADCSRA |= 0x80;
  // 0000 0000 | 1000 0000 = 1000 0000

  //clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0xDF;
  // 1000 0000 & 1101 1111 = 1000 0000

  //clear bit 3 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0xF7;
  // 1000 0000 & 1111 0111 = 1000 0000

  //clear bits 2-0 to 0 to set the prescaler selection to slow reading
  *my_ADCSRA &= 0xFB;
  // 1000 0000 & 1111 1011

  //~~~~~~~~~~~~~~

  //setup the B register

  //clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0xF7;
  // 0000 0000 & 1111 0111 = 0000 0000

  //clear bit 2 to 0 to set the prescaler selection to slow reading
  *my_ADCSRB &= 0xFB;
  // 0000 0000 & 1111 1011 = 0000 0000

  //~~~~~~~~~~~~~~~

  //setup the MUX Register

  //set bit 7 to 0 for AVCC analog reference
  *my_ADMUX &= 0x00;
  // 0000 0000 & 0000 0000 = 0000 0000

  //set bit 6 to 1 for AVCC analog reference
  *my_ADMUX |= 0x40;
  // 0000 0000 | 0100 0000 = 0100 0000

  //clear bit 5 to 0 for right adjust result
  *my_ADMUX &= 0xDF;
  // 0100 0000 & 1101 1111 = 0100 0000

  //clear bit 4-0 to 0 to reset the channel and gain bits
  *my_ADMUX &= 0xE0;
  // 0100 0000 & 1110 0000 = 0100 0000
}

  //~~~~~~~~~~~~~~~

  unsigned int adc_read(unsigned char adc_channel_num)
{
  //clear the channel selection bits (MUX 4:0) flip a certain bit
  //on which register?
  *my_ADMUX &= 0xE0;
  // 0000 0000 & 1110 0000 = 0000 0000

  //clear the channel selection bits (MUX 5)
  //on which register?
  *my_ADCSRB &= 0xF7;
  // 0000 0000 & 1111 0111 = 0000 0000

  //set the channel number (which is the input ranging from 0-15)
  //the last 3 bits in ADMUX determines which pins are going to be used
  // 000 = channel 0, ..., 111 = channel 7
  //to change channel, set MUX5 from ADCSRB

  //adc_channel_num = 0;

  if (adc_channel_num > 7)
  {
    //set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;

    //set the MUX bit 5
    //on which register?
    //if MUX 5 is set to 0, it accesses A0-A7. If it's set to 1 A8-A15.

    *my_ADCSRB &= 0xF7;
  // 0000 0000 & 1111 0111 = 0000 0000
  }

  //set the channel selection bits
  *my_ADMUX |= adc_channel_num;

  //set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // 0000 0000 | 0100 000 = 0100 0000

  //wait for the conversion to complete
  while ((*my_ADCSRA & 0x40) != 0);

  //return the result in the ADC data register
  return *my_ADC_DATA;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Temperature with LCD Sample
/*
#include <LiquidCrystal.h>
int tempPin = 0;
//                BS  E  D4 D5  D6 D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
void setup()
{
  lcd.begin(16, 2);
}
void loop()
{
  int tempReading = analogRead(tempPin);
  // This is OK
  double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius
  float tempF = (tempC * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit


*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Temperature & Humidity Sample code

/*
#include <dht.h>


#define dht_apin A0 // Analog Pin sensor is connected to

dht DHT;

void setup(){

  Serial.begin(9600);
  delay(500);//Delay to let system boot
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  delay(1000);//Wait before accessing Sensor

}//end "setup()"

void loop(){
  //Start of Program

    DHT.read11(dht_apin);

    Serial.print("Current humidity = ");
    Serial.print(DHT.humidity);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(DHT.temperature);
    Serial.println("C  ");

    delay(5000);//Wait 5 seconds before accessing sensor again.

  //Fastest should be once every two seconds.

}// end loop(
*/


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*

//~~~~~~~~~~~~~~~~setup timer register FUNCTION~~~~~~~~~~~~~~~~~~~
void setup_timer_regs()
{
  // Setup the Timer Control Registers
  // 0000 0000 & 0000 0000 = 0000 000{0}
  *myTCCR1A &= 0x00;
  *myTCCR1B &= 0x00;
  *myTCCR1C &= 0x00;

  // Reset the TOV Flag:       0000 0000 | 0000 0001 = 0000 000{1}
  *myTIFR1 |= 0x01;
  // Enable the TOV Interrupt: 0000 0000 | 0000 0001 = 0000 000{1}
  *myTIMSK1 |= 0x01;
}


//~~~~~~~~~~~~~~~~~~TIMER OVERFLOW ISR~~~~~~~~~~~~~~~~~~~
// Gets called once the Flag is Set
ISR(TIMER1_OVF_vect)
{
   // Stop Timer:      0000 0000 & 1111 1110 = 0000 000{0}
   *myTCCR1B &= 0xFE;
   // Load the Count (TCNT Register)
   *myTCNT1 |= 65536 - char_tick;
   // Start the Timer: 0000 0000 | 0000 0001 = 0000 000{1}
   *myTCCR1B |= 0x01;

  // if it's not the STOP amount
  if (char_tick != 65535)
  {
    //XOR to toggle PB6:
    // 0000 0000 ^ 0100 0000 = 0{1}00 0000
    // 0100 0000 ^ 0100 0000 = 0{0}00 0000
    *port_B ^= 0x40;
  }
}

*/

/*
Ignore these:
      Serial.println("Increment by 1.");
      //increments the counter by 1
      counter++;
      //1 % 16 = 1, 2 % 16 = 2, ... , 16 % 16 = 0
      //to reset the counter to zero after the value becomes 16 (16 % 16 = 0)
      counter %= 16;

      //prints counter to the serial monitor
      Serial.println(counter);
*/
