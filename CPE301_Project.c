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

//water level threshold
#define w_threshold 200
//temperature threshold
#define t_threshold 25


void setup()
{
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
  *myPORT_B |= 0x80;
}

void loop()
{
  // LEDs Location:
  // PB7 - GREEN (IDLE) LED,
  // PB6 - YELLOW (DISABLED) LED,
  // PB5 - RED (ERROR) LED,
  // PB4 - BLUE (RUNNING) LED
  // PH6 - Push Button

  // if the system is disabled or OFF
  if()
  {
    //checks whether the button is pushed; checks bit 6 (0100 0000)
    if (!(*myPIN_H & 0x40))
    {
      //a loop that does nothing to make sure noise is not included (which occurs at a micro second)
      for (volatile unsigned int i = 0; i < 1000; i++);

      //checks again if the button is pressed
      if (!(*myPIN_H & 0x40))
      {
        //IDLE State

        //time stamps
        //monitor water level

        //if statement if the water level is under the threshold (low)

/*      if(water_level < w_threshold)
        {
          //ERROR state

          //error message
          //Serial.println("Water level is too LOW");

          //RED LED ON (0010 0000)
          //*myPORT_B &=  0x00;               //to turn them all off
          //*myPORT_B |=  0x20;               //to turn on RED LED

        }

*/

        //if the water level is above the threshold (high)

/*      if(water_level > w_threshold)
        {

        // IDLE State

        // Time stamps
        // Monitor water level

        // GREEN LED ON (1000 0000)
        //*myPORT_B &=  0x00;               //to turn them all off
        //*myPORT_B |=  0x80;               //to turn on GREEN LED


        // RUNNING State

        // motor is on *****
        // [INSERT CODE]
        // BLUE LED ON (0001 0000)
        //*myPORT_B &=  0x00;               //to turn them all off
        //*myPORT_B |=  0x10;               //to turn on BLUE LED

        // Error Message if water level < threshold

          if(water_level < w_threshold)
          {
            //ERROR state

            //error message
            //Serial.println("Water level is too LOW");

            //RED LED ON (0010 0000)
            //*myPORT_B &=  0x00;               //to turn them all off
            //*myPORT_B |=  0x20;               //to turn on RED LED

          }
        }
*/
      //makes sure that counter increments by one per pressed button
      while (!(*myPIN_H & 0x40));

    }
  }

  }



  //if the system is ON
   if()
   {
     //checks whether the button is pushed; checks bit 7
     if (!(*myPIN_B & 0x80))
     {
       //a loop that does nothing to make sure noise is not included (which occurs at a micro second)
       for (volatile unsigned int i = 0; i < 1000; i++);

       //checks again if the button is pressed
       if (!(*myPIN_B & 0x80))
       {

         //DISABLED State
         //not monitoring any temperature or water level

         //Yellow light on
         //*myPORT_B &=  0x00;               //to turn them all off
         //*myPORT_B |=  0x02;               //to turn on Yellow LED

         //makes sure that counter increments by one per pressed button
         while (!(*myPIN_B & 0x80));

      }
    }
  }
}
/*      Serial.println("Increment by 1.");
      //increments the counter by 1
      counter++;
      //1 % 16 = 1, 2 % 16 = 2, ... , 16 % 16 = 0
      //to reset the counter to zero after the value becomes 16 (16 % 16 = 0)
      counter %= 16;

      //prints counter to the serial monitor
      Serial.println(counter);
*/
