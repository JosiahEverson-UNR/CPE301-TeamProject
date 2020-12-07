// CPE301 Group Project. Group 17: Al Francis De Vera, Fate Jacobson, Josiah Everson

// Libraries
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>
#include <dht11.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <RTClib.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Terminology Used
/*
[Switch]
  ENABLED Mode (ON) = When the system is currently simulating CODE

  DISABLED Mode (OFF) = When the system is not monitoring temperature and water level.
                  Also, the fan motor is OFF.

[Conditions]
  RUNNING State = The water_level is above the w_threshold AND the temperature is
                  ABOVE the t_threshold so the fan is turned ON.
                  {It's hot enough so one turns on the fan to cool themself}

  ERROR State = The water_level is LOW so the fan is turned OFF. The temperature doesn't
                matter. {If the water is seen as power and it runs low, the motor won't be
                able to function}

  IDLE State = This is when water level is above the threshold while temperature
                     is lower than the t_threshold so the fan motor is turned off.
                     {It's cool enough that one doesn't need the fan anymore.}

*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//REGISTERS

//PORT B DECLARATION: Used as Output for the LEDs
volatile unsigned char* myPORT_B = (unsigned char*) 0x25;
volatile unsigned char* myDDR_B  = (unsigned char*) 0x24;
volatile unsigned char* myPIN_B  = (unsigned char*) 0x23;

//PORT C DECLARATION: Used as input for Temperature and Humdity sensor
volatile unsigned char* myPORT_C = (unsigned char*) 0x28;
volatile unsigned char* myDDR_C  = (unsigned char*) 0x27;
volatile unsigned char* myPIN_C  = (unsigned char*) 0x26;

//PORT H DECLARATION: Used as Input for push button
volatile unsigned char* myPORT_H = (unsigned char*) 0x102;
volatile unsigned char* myDDR_H  = (unsigned char*) 0x101;
volatile unsigned char* myPIN_H  = (unsigned char*) 0x100;

//PORT F DECLARATION: Used as Input for Water Level Detection
volatile unsigned char* myPORT_F = (unsigned char*) 0x31;
volatile unsigned char* myDDR_F  = (unsigned char*) 0x30;
volatile unsigned char* myPIN_F  = (unsigned char*) 0x2F;

//PORT E DECLARATION: Used for Fan Motor
volatile unsigned char* myPORT_E = (unsigned char*) 0x2E;
volatile unsigned char* myDDR_E  = (unsigned char*) 0x2D;
volatile unsigned char* myPIN_E  = (unsigned char*) 0x2C;

//PORT K DECLARATION: Used for Button and Transistor
volatile unsigned char* myPORT_K = (unsigned char*) 0x108;
volatile unsigned char* myDDR_K  = (unsigned char*) 0x107;
volatile unsigned char* myPIN_K  = (unsigned char*) 0x106;

//ADC Registers
volatile unsigned char* my_ADMUX     = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRA    = (unsigned char*) 0x7A;
volatile unsigned char* my_ADCSRB    = (unsigned char*) 0x7B;
volatile unsigned int*  my_ADC_DATA  = (unsigned int*)  0x78;

//TIMER Registers
volatile unsigned char *myTCCR1A = (unsigned char*) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char*) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char*) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char*) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned int*)  0x84;
volatile unsigned char *myTIFR1  = (unsigned char*) 0x36;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initializations, Declarations, and Definitions

// Water level threshold
#define w_threshold 150
// Temperature threshold
#define t_threshold 22

// DHT
#define DHT11PIN 36
dht11 DHT11;

// Global Variables
// Temperature and Humidity variables
float temperature = 0;
float humidity = 0;

//Machine starts in DISABLED mode
unsigned int state_counter = 0;

// Initialize LCD
LiquidCrystal lcd(8,7,6,5,4,3);

//Function initializations
void idle_state(int water_level, float temperature_C);

void error_state(int water_level, float temperature_C);

void running_state(int water_level, float temperature_C);

void disabled_mode();

float lcd_display (float temperature_C, float humidity);

void adc_init();

unsigned int adc_read(unsigned char adc_channel_num);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//SETUP
void setup()
{
  adc_init();

  Serial.begin(9600);

  //PORT B
  *myDDR_B |= 0xFF;         // Turn all pins to output
  
  //PORT H
  *myDDR_K &= 0x00;         // Turn all pins to inputs
  
  //PORT K
  *myPORT_K |= 0xFF;        // Enable internal pull-up resistor for all pins

  //PORT F 
  *myDDR_F &= 0xFF;         //Turn all pins to output
  *myPORT_F |= 0x80;        //Enable internal pull-up resistor

  //PORT E: Motor PIN (PE4)
  *myDDR_E &= 0x10;         //Set pin to output
  *myPORT_E |= 0x10;        //Enable internal pull-up resistor

  // LCD size
  lcd.begin(16,2);
  // Set so that LCD display begins on upper-left corner
  lcd.setCursor(0,0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//LOOP
void loop()
{
  // Checks whether the button is pushed; checks bit 6 (0100 0000)
  if (!(*myPIN_K & 0x40))
  {
    // A loop that does nothing to make sure noise is not included
    // (which occurs at a micro second)
    for (volatile unsigned int i = 0; i < 1000; i++);
    
    // Checks again if the button is pressed
    if (!(*myPIN_K & 0x40))
    {
      // If currently at 0, then becomes 1
      // If currently at 1, then becomes 2
      state_counter++;
      
      // 1 % 2 = 1; which means that it'll now in ENABLED Mode
      // 2 % 2 = 0; which means that it'll now be in DISABLE Mode
      state_counter %= 2;

      // Makes sure that counter increments by one per pressed button
      while (!(*myPIN_H & 0x40));
    }
  }

  // If the system should currently be DISABLED 
  if(state_counter == 0)
  {
    // Function puts the system into DISABLED mode
    disabled_mode();
  }
  // Else the system should be ENABLED
  else
  {
    // Water Level Reading
    unsigned int water_level = adc_read(0);
    
    // Temperature & Humidity Sensor Reading
    int chk = DHT11.read(DHT11PIN);
    temperature = (float)DHT11.temperature;
    humidity = (float)DHT11.humidity;
    
    // LCD Display
    lcd.clear();
    lcd.print("Temp: ");        // Abbreviated to display temperature on one line
    lcd.print(temperature);     // Displays Temperature Value from DHT function
    lcd.print((char)223);       // Prints degree symbol
    lcd.print("C");             // prints "C" for Celsius
    lcd.setCursor(0,1);         // Adds new line
    lcd.print("Humidity: ");
    lcd.print(humidity);        // Displays Humidity Value from DHT function
    lcd.print("%");


    // Serial Monitor Display
    // Print Water Level, Air Temperature, Humidity, and Machine State to the Serial Monitor
    Serial.print("Water Level: ");
    Serial.print(water_level);
    Serial.print("      Temperature: ");
    Serial.print(temperature);
    Serial.print("C      Humidity: ");
    Serial.print(humidity);
    Serial.print("%");

    // Functions are called to determine the current states
    // They use the water level and air temperature to decide
    idle_state(water_level, temperature);
    error_state(water_level, temperature);
    running_state(water_level, temperature);
    
    Serial.println("");        // New line helps organize the serial monitor
  }
}

//***********************\\FUNCTIONS//********************************

void idle_state (int water_level, float temperature)
{
  // If the water level is above the w_threshold and Temperature is under the t_threshold
  if(water_level > w_threshold && temperature < t_threshold)
  {
    Serial.print("      Machine Status: Idle");
    
    // Where time stamp would go
    
    // LEDs
    *myPORT_B &=  0x00;               // Turn all LEDs off
    *myPORT_B |=  0x80;               // Turn on GREEN LED

    // Turn Motor off
    *myPORT_B |= 0x08;        
    *myPORT_B &= 0xFD;        
  }
}

void error_state (int water_level, float temperature)
{
  // If water level is below the threshold 
  if(water_level <= w_threshold)
  {
     // LEDs
     *myPORT_B &=  0x00;               // Turn all LEDs off
     *myPORT_B |=  0x20;               // Turn on RED LED

    // Error Message
    Serial.print(" **Error (Water level too LOW)**");

    // Turn Motor off
    *myPORT_B |= 0x08;        
    *myPORT_B &= 0xFD;       
  }
}

// If the water level is above the w_threshold
// Temperature is above the t_threshold
void running_state (int water_level, float temperature)
{
  // If the water level is above the w_threshold and Temperature is above the t_threshold
  if(water_level > w_threshold && temperature > t_threshold)
  {
    Serial.print("      Machine Status: Running");
    
    // Where the Time Stamp would go
    
    // LEDs
    *myPORT_B &=  0x00;               // Turn all LEDs off
    *myPORT_B |=  0x40;               // Turn on BLUE LED

    // Turn Motor ON
    *myPORT_B |= 0x08;        
    *myPORT_B |= 0x02;        
  }
}


void disabled_mode ()
{
  // ---DISABLED Mode---
  //not monitoring any temperature or water level

  Serial.println("      Machine Status: Disabled");

  // Clear the LCD
  lcd.clear();

  // LEDs
  *myPORT_B &=  0x00;               // Turn all LEDs off
  *myPORT_B |=  0x10;               // Turn on YELLOW LED

  // Turn Motor Off
  *myPORT_B |= 0x08;        
  *myPORT_B &= 0xFD;        
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LCD Display Function

float lcd_display (float temperature_C, float humidity)
{
  // CLears Display
  lcd.clear();
  // Abbreviated to display temperature on one line
  lcd.print("Temp: ");
  // Displays Temperature Value from DHT function
  lcd.print(temperature_C);
  // Prints degree symbol
  lcd.print((char)223);
  // prints "C" for Celsius
  lcd.print("C");
  // Adds new line
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  // Displays Humidity Value from DHT function
  lcd.print(humidity);
  lcd.print("%");
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
}
