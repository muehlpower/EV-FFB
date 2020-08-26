//you have to modify:
//SPI.h line 84 to "uint16_t transfer(byte _pin, uint16_t _data, SPITransferMode _mode = SPI_LAST);" 
//SPI.cpp line 175 to "uint16_t SPIClass::transfer(byte _pin, uint16_t _data, SPITransferMode _mode) {"
//SPI.cpp line 176 to " return d & 0xFFFF;"
//to accept 16bit data
//you found it at C:\Users\MB\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\libraries\SPI\src

// inslude the SPI library:
#include <SPI.h>

#include "C:\Users\MB\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system\CMSIS\Device\ATMEL\sam3xa\include\component\component_spi.h"
#include "C:\Users\MB\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system\CMSIS\Device\ATMEL\sam3xa\include\instance\instance_spi0.h"


#define CS 2
#define SS 3

uint16_t WakeUp[2] = {0x2AD4, 0x0000};
uint16_t Com0[2] = {0x2710, 0x0000};
uint16_t Com1[2] = {0x21F2, 0x0000};
uint16_t Vref[2] = {0x2BFB, 0x0000};
uint16_t Slp[3] ={0xF300, 0x0000, 0x38DC};
uint16_t reqVolt[5] = {0x0763, 0x08F9, 0x09D6, 0x0AA7, 0x0B88};
uint16_t reqTemp = 0x0E1B;
uint16_t padding = 0x0000;
uint32_t receive1 = 0;
uint16_t receive2 = 0;
uint16_t Fluffer[24][2];
byte count1 = 0;
byte count2 = 0;
byte count3 = 0;

unsigned long LoopTimer1 = 0;

uint16_t Voltage[4][30] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


uint16_t Temps [4][6] = {
  {0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0},
};



uint16_t Temps2 [4][4] =  {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
};        
  
bool debug = 0;
bool SPIdebug = 1;


void SPIbegin (void)
{
  SPI.begin(10);  
  REG_SPI0_WPMR=0x53504900;  //Unlocks Write protection
  REG_SPI0_CR=0x1;           // enables SPI send/receive
  SPI0->SPI_CSR[0]=0x00005481;   //54hex = 84dec   =>1MHz / 15hex = 21dec   =>4MHz
}

void Generic_Send_Once(uint16_t Command[], uint8_t len)
{
  //SPIbegin (); 
  digitalWrite (SS, LOW);        // assert Slave Select
  for (int h = 0; h < len; h++)
  {
    receive1 = SPI.transfer(10,Command[h], SPI_LAST);  // do a transfer
  }
  digitalWrite (SS, HIGH);       // de-assert Slave Select
    //SPI.end(10);
  delayMicroseconds(15);
  //delay(1);
 }



void setup()
 {
  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(CS, OUTPUT); //select pin
  pinMode(SS, OUTPUT); //select pin
  //pinMode(4, OUTPUT); //select pin
  pinMode(10, OUTPUT); //select pin
  //pinMode(52, OUTPUT); //select pin
  //SerialUSB.begin(9600);//(115200);//normal port
  Serial.begin(115200);//normal port
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
    SPIbegin ();
}









void GetTempData ()  //request
{
  //SPIbegin ();
  digitalWrite (SS, LOW);        // assert Slave Select
  receive1 = SPI.transfer(10, reqTemp, SPI_LAST);  // do a transfer
  count1=0;
  count2=0;
  for (count3 = 0; count3 < 32; count3 ++)
  {
    receive1 = SPI.transfer(10, padding, SPI_LAST);  // do a transfer  
    count1++;
    if (count1<4)
     {
      //Fluffer[count2][0] =receive1;
      Fluffer[count2][0] = highByte(receive1);
      Fluffer[count2][1] = lowByte(receive1);
      count2++;
     }
    else count1=0;  
  }
  digitalWrite (SS, HIGH);       // de-assert Slave Select
  //SPI.end(10);
  uint16_t tempval = 0;
  if (Fluffer[0][1] != 0xff) 
  {
    for (int p = 0; p < 4; p++)   //pack
    {
      for (int c = 0; c < 6; c++)  //sens
        {
           if (Fluffer[p*6+c][1] != 0xff)
            {
              //tempvol = 0xC412; 
              //tempvol = Fluffer[p*6+c][0];
              tempval = Fluffer[p*6+c][1] * 256 + Fluffer [p*6+c] [0];
              Temps[p][c] = tempval;
            }
        }
    }     
  }
 delayMicroseconds(50);
}

void GetVoltData (uint16_t r)  //request
{
  //SPIbegin ();
  digitalWrite (SS, LOW);        // assert Slave Select
  receive1 = SPI.transfer(10, reqVolt[r], SPI_LAST);  // do a transfer
  count1=0;
  count2=0;
  for (count3 = 0; count3 < 32; count3 ++)
  {
    receive1 = SPI.transfer(10, padding, SPI_LAST);  // do a transfer  
    count1++;
    if (count1<4)
     {
      //Fluffer[count2][0] =receive1;
      Fluffer[count2][0] = highByte(receive1);
      Fluffer[count2][1] = lowByte(receive1);
      count2++;
     }
    else count1=0;  
  }
  digitalWrite (SS, HIGH);       // de-assert Slave Select
  //SPI.end(10);
  uint16_t tempval = 0;
  if (Fluffer[0][1] != 0xff) 
  {
    for (int p = 0; p < 4; p++)   //pack
    {
      for (int c = 0; c < 6; c++)  //cell
        {
           if (Fluffer[p*6+c][1] != 0xff)
            {
              //tempvol = 0xC412; 
              //tempvol = Fluffer[p*6+c][0];
              tempval = Fluffer[p*6+c][1] * 256 + Fluffer [p*6+c] [0];
              Voltage[p][r*6+c] = tempval / 12.5;
            }
        }
    }     
  }
 delayMicroseconds(50);
}


void Display ()
{
    Serial.println();
    for (int h = 0; h < 4; h++)
    {
      Serial.print("Pack ");
      Serial.print(h + 1);
      Serial.print(" : ");
      for (int g = 0; g < 30; g++)
      {
        if (Voltage[h][g]>5)
        {
         Serial.print("| ");
         Serial.print(Voltage[h][g]);
         Serial.print("mV");
        }
        if (g==12)  
        {
         Serial.println();
         Serial.print("         ");
        }
      }
      Serial.println();
      Serial.print("         Temps ");
      uint16_t tempval1 = 0;
      uint16_t tempval2 = 0;
      for (int g = 0; g < 2; g++)
      {
         tempval1=Temps[h][g*3+1];
         if (tempval1 >= (1131)) 
         {
          tempval1 = tempval1-1131;
          tempval2 = tempval1/10;
          Serial.print("|  ");
          Serial.print(tempval2);
          Serial.print(".");
          Serial.print(tempval1-10*tempval2);
          Serial.print("°C");
         }
         else 
         {
          tempval1 = 1131-tempval1;
          tempval2 = tempval1/10;
          Serial.print("| -");
          Serial.print(tempval2);
          Serial.print(".");
          Serial.print(tempval1-10*tempval2);
          Serial.print("°C");
         }
      }
      Serial.println();
    }   
}
          

void loop()
{
  if (millis() - LoopTimer1 > 100)
  //delay (5);
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    LoopTimer1 = millis();

    for (int h = 0; h < 9; h++)
     {
      Generic_Send_Once(WakeUp, 2);
      if (receive1==0x0000) h=9;
     }
/*
    for (int h = 0; h < 2; h++)
     {
      Generic_Send_Once(Com0, 2);
      if (receive1==0x0000) h=2;
     }

    for (int h = 0; h < 9; h++)
     {
      Generic_Send_Once(WakeUp, 2);
      if (receive1==0x0000) h=9;
     }
     
    for (int h = 0; h < 2; h++)
     {
      Generic_Send_Once(Com1, 2);
      if (receive1==0x0000) h=2;
     }
*/
    for (int h = 0; h < 2; h++)
     {
      //Generic_Send_Once(Vref, 2);
      if (receive1==0x0000) h=2;
     }
    
    for (int r = 0; r < 5; r++)
    GetTempData ();

    for (int r = 0; r < 5; r++)
    GetVoltData (r);
   
    //for (int h = 0; h < 8; h++)
    // Generic_Send_Once(Slp, 3);

    Display ();
    delay (1);
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    //digitalWrite (CS, HIGH);
    }
}



 
