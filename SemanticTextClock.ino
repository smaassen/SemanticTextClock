#include "Wire.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define DS3231_I2C_ADDRESS  0x68
#define LEDPIN              4
#define NUMPIXELS           42

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);



// Variables
int i=0;
int CL=1;  // Update period in seconds

unsigned long timerStart;
unsigned long timerCurrent;
boolean timerEnd=false;

unsigned long clockTimerStart;
unsigned long clockTimerCurr;
boolean clockTimerEnd=false;

int giventime=0;
int hours=0;
int minutes=0;
long totalsec=0L;

// array that keeps track if the LED is ON or OFF
int LEDStates[NUMPIXELS];

const int buttonPin1 = 2;
const int buttonPin2 = 3;     
int buttonState1 = 0;
int buttonState2 = 0;

boolean clockSetMode=false;

//Running variables
byte currSecond;
byte currMinute;
byte currHour;
byte currDay;
byte currDate;
byte currMonth;
byte currYear;

int intensity=95;
int colorIdx=0;
const int N_colors=8;

int currRGB[3]={255,255,255}; // Starting color
// array of colors that can be extended. Also change the fixe size of the array if appending colors
int colorScheme[N_colors][3]=  
  {
  {255,255,255},
  {255,0,0},
  {250,250,0},
  {0,255,0},
  {0,250,250},
  {0,0,255},
  {250,0,250},
  {255,222,173}
  };


void setup() {
  
  pixels.begin();

  // Define buttons
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);

  //Begin I2C communication
  Wire.begin();
  
  // open Serial Communication
  Serial.begin(115200); 
  
  // Initialize LEDS  
  for(i=0;i<42;i++)
  {
    LEDStates[i]=0;
  }
  
  // First time setting: uncomment the follwing line to set the time by code
  //setDS3231time(30, 02, 23, 5, 10, 11, 16);

  // Play Startup Sequence
  startUpSequence();  

  // Read current time from module: comment the follwing line if you want to intially set th time by code
  readDS3231time(&currSecond, &currMinute, &currHour, &currDay, &currDate, &currMonth, &currYear);

  // Start Clocktimer
  clockTimerStart=millis();
}

void loop() {
  
  // Read button pins
  buttonState1 = digitalRead(buttonPin1);
  buttonState2 = digitalRead(buttonPin2);

  // Check what to do
  if (buttonState1 == HIGH) {
    Serial.println("Button 1 was pressed");
    timerStart=millis();
    timerEnd=false;
    while (buttonState1 == HIGH & timerEnd==false){
      buttonState1 = digitalRead(buttonPin1);
      timerCurrent=millis();
      if (timerCurrent-timerStart>3000){
        timerEnd=true;
      }     
    }
    if (timerEnd) {
      // Go into clock setting mode
      // Make LEd Animation to notify the user entering clock mode
      SettingNotifier();
      updateLEDMatrix(LEDStates,currRGB);
      Serial.println("... for 3 seconds");
      Serial.println("  Entering Setting Mode");
      delay(1000);
      currMinute=(currMinute/5*5)%60;
      clockSetMode=true;

      // Enter Setting Mode
      while (clockSetMode)
      {
        // Read button pins
        buttonState1 = digitalRead(buttonPin1);
        buttonState2 = digitalRead(buttonPin2);

        if (buttonState1 == HIGH) 
        {
          Serial.println("Button 1 was pressed in setting mode");
          timerStart=millis();
          timerEnd=false;
          while (buttonState1 == HIGH & timerEnd==false){
            buttonState1 = digitalRead(buttonPin1);
            timerCurrent=millis();
            if (timerCurrent-timerStart>3000){
              timerEnd=true;
            }     
           
          }
          if (timerEnd)
          {
            //Write current setting to clock module and leave setting mode
            currSecond=0;  
            setDS3231time(currSecond, currMinute, currHour, currDay, currDate, currMonth, currYear);
            Serial.println("... new time written to clock module. ... Leaving Clock setting mode");
            clockSetMode=false;
            delay(500);
          }
          else
          {
            // Increment current hour 
            Serial.println("  Hours = Hours + 1");
            currHour=(currHour+1)%12;
            // Update Matrix 
            currSecond=0;  
            setDS3231time(currSecond, currMinute, currHour, currDay, currDate, currMonth, currYear);
            totalsec=currSecond+currMinute*60L+currHour*3600L;
            setLEDStates(LEDStates,giveHour(totalsec),giveMinutes(totalsec));
            updateLEDMatrix(LEDStates,currRGB);
            delay(200);
          }
          
        }

        if (buttonState2 == HIGH) 
        {
          // increment current minute
          Serial.println("Button 2 was pressed in setting mode");
          Serial.println("  minute=minute + 5");
          currMinute=(currMinute+5)%60;
          // Update Matrix 
          totalsec=currSecond+currMinute*60L+currHour*3600L;    // could be polished
          setLEDStates(LEDStates,giveHour(totalsec),giveMinutes(totalsec));
          updateLEDMatrix(LEDStates,currRGB);
          delay(500);
        }

        
      }
      // play sequence to inform the user that he's leaving setting mode
      SettingNotifier();
    }
    else
    {
      // increment Intesity value
      intensity=(intensity+10)%100; 
      Serial.print (" Intesity increased to: ");
      Serial.print(intensity);
      Serial.println(" %");
      // update LED Matrix
      adjustColor(currRGB,colorScheme,intensity,colorIdx);
      updateLEDMatrix(LEDStates,currRGB);

      Serial.print("   current rgb values: ");
      Serial.print(currRGB[0]);
      Serial.print("/");
      Serial.print(currRGB[1]);
      Serial.print("/");
      Serial.println(currRGB[2]);
      
    }
    delay(300); 
  } 

  if (buttonState2 == HIGH) {
    // set to next color in colorscheme
    Serial.println("Button 2 was pressed");
    Serial.println (" Color scheme changed ");
    colorIdx=(colorIdx+1)%N_colors;
   // update LED Matrix
    adjustColor(currRGB,colorScheme,intensity,colorIdx);
    updateLEDMatrix(LEDStates,currRGB);

    Serial.print("   current rgb values: ");
    Serial.print(currRGB[0]);
    Serial.print("/");
    Serial.print(currRGB[1]);
    Serial.print("/");
    Serial.println(currRGB[2]);
    delay(300); 
  }


  

  // Check if there is a coomunictaion from the serial port
  if (Serial.available()>0)
  {
    if (Serial.readString()=="H"){
    // Display time
    displayTime(currSecond, currMinute, currHour, currDay, currDate, currMonth, currYear);
    Serial.print("LED states: ");
    for(i=0;i<42;i++)
      {
        Serial.print(LEDStates[i]);
        Serial.print(" ");
      }
    Serial.println(); 
    } 
  }


  // Check if its time to update the clock 
  clockTimerCurr=millis();
  if (abs(clockTimerCurr-clockTimerStart)>CL*1000)
  {
    // update time from clock module
    readDS3231time(&currSecond, &currMinute, &currHour, &currDay, &currDate, &currMonth, &currYear);
    // update LED matrix;
    totalsec=currSecond+currMinute*60L+currHour*3600L;    // could be polished
    //setLEDStates(LEDStates,giveHour(totalsec),giveMinutes(totalsec));
    setLEDStates(LEDStates,giveHour(totalsec),giveMinutes(totalsec));
    // update the LED Matrix
    updateLEDMatrix(LEDStates,currRGB);
    // restart the timer
    clockTimerStart=millis(); 
  }
}


// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

//Write a time to the clock module 
void setDS3231time(byte second, byte minute, byte hour,byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

// reads the time from the clock module
void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

// Gives Back the hour word that has to be lit
int giveHour(long sec){
  int h_temp=0;
  int criteria=0;

  sec=sec%43200L;
  sec-=1350L;

  if (sec<0)
  {
    return 12;
  }
  else
  {
    h_temp=sec/3600L+1;
    criteria=sec%3600L;
    if (criteria==3599)
    {
      h_temp++;
    }
    return h_temp;
  }
}

// Gives back the minute word that has to be lit
int giveMinutes(long sec){

  int minutes;
  
  sec=sec%3600L;
  sec=sec-150;
  if (sec<0)
  {
    return 0;
  }
  else
  {
    minutes=sec/(300)+1;
    minutes*=5;
    return minutes; 
  }
}

// Gives back an array of the LEDs that have to be lit 
void setLEDStates(int states[NUMPIXELS], int hour,int minutes){
   // reset the LEDS
  for(i=0;i<3;i++)
  {
    LEDStates[i]=1;
  } 
  
  for(i=3;i<42;i++)
  {
    LEDStates[i]=0;
  }
  
  // set the right hour LED
  switch (hour){
    case 1:
      states[16]=1;
      break;
    case 2:
      states[17]=1;
      states[18]=1;
      break;
    case 3:
      states[19]=1;
      states[20]=1;
      break;
    case 4:
      states[21]=1;
      states[22]=1;
      break;
    case 5:
      states[23]=1;
      states[24]=1;
      break;
    case 6:
      states[25]=1;
      states[26]=1;
      states[27]=1;
      break;
    case 7:
      states[28]=1;
      states[29]=1;
      break;
    case 8:
      states[30]=1;
      states[31]=1;
      break;
    case 9:
      states[32]=1;
      states[33]=1;
      break;
    case 10:
      states[34]=1;
      states[35]=1;
      break;
    case 11:
      states[36]=1;
      states[37]=1;
      break;
    case 12:
      states[38]=1;
      states[39]=1;
      states[40]=1;
      break;
  }

  // light up the right minute
  switch(minutes){
    case 0:
        
      break;
    case 5:
      states[3]=1;
      states[4]=1;
      states[13]=1;  
      break; 
    case 10:
      states[8]=1;
      states[13]=1;
      break; 
    case 15:
      states[5]=1;
      states[6]=1;
      states[7]=1;
      states[13]=1;  
      break; 
    case 20:
      states[9]=1;
      states[10]=1;
      states[11]=1;
      states[13]=1; 
      break;
    case 25:
      states[3]=1;
      states[4]=1;
      states[12]=1;
      states[14]=1;
      states[15]=1;  
      break;
    case 30:
      states[14]=1;
      states[15]=1;   
      break; 
    case 35:
      states[3]=1;
      states[4]=1;
      states[13]=1;
      states[14]=1;
      states[15]=1; 
      break; 
    case 40:
      states[9]=1;
      states[10]=1;
      states[11]=1;
      states[12]=1;  
      break; 
    case 45:
      states[5]=1;
      states[6]=1;
      states[7]=1;
      states[12]=1;   
      break;
    case 50:
      states[8]=1;
      states[12]=1;  
      break;
    case 55:
      states[3]=1;
      states[4]=1;
      states[12]=1;
      break;
  }
}

// update LED Matrix 
void  updateLEDMatrix(int states[42], int rgb[3]){
  for (i=0;i<NUMPIXELS;i++)
  {
    if (states[i]==1) 
    {
      pixels.setPixelColor(i, pixels.Color(rgb[0],rgb[1],rgb[2])); 
    }
    else
    {
      pixels.setPixelColor(i, pixels.Color(0,0,0));   
    }
  }
  pixels.show();
}

// Play the startup sequence
void startUpSequence(){
  
  for (int k=0;k<90;k=k+1)
    {
       for (int n=0; n<NUMPIXELS;n++)
       {
        pixels.setPixelColor(n, pixels.Color(0,0,k));
       }
       pixels.show(); 
       delay(5); 
    }
    for (int k=0;k<90;k=k+1)
    {
       for (int n=0; n<NUMPIXELS;n++)
       {
        pixels.setPixelColor(n, pixels.Color(0,k,90-k));
       }
       pixels.show();
       delay(5);  
    }
    for (int k=0;k<90;k=k+1)
    {
       for (int n=0; n<NUMPIXELS;n++)
       {
        pixels.setPixelColor(n, pixels.Color(k,90-k,0));
       }
       pixels.show();
       delay(5);  
    }
    delay(500);

    for (int n=0; n<NUMPIXELS;n++)
    {
        pixels.setPixelColor(n, pixels.Color(90 ,90,90));
    }
    pixels.show();
    delay(1000);
  
}

// Play the Entering/Exiting settings animation
void SettingNotifier(){
  for (int j=0;j<3;j++)
  {
    for (int k=0;k<90;k=k+1)
      {
         for (int n=0; n<NUMPIXELS;n++)
         {
          pixels.setPixelColor(n, pixels.Color(k,k,k));
         }
         pixels.show();
          
      }
    for (int n=0; n<NUMPIXELS;n++)
      {
        pixels.setPixelColor(n, pixels.Color(0,0,0));
      }
    delay(250);  
  }
  delay(250);
  
}

void adjustColor(int currRGB[3],int colorScheme[5][3],int intensity,int colorIdx){
    for (int i=0;i<3;i++){
      currRGB[i]=intensity*0.01*colorScheme[colorIdx][i];  
    }
}

// Display time in serial monitor for debugging
void displayTime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // send it to the serial monitor
  Serial.print(hour,DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute,DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second,DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
    break;
  }
}

