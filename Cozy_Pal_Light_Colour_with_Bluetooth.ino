/*Notes

   No Serial Monitor

   Bluetooth Rx                   PIN0
   Bloetooth Tx                   PIN1
   Microphone Control PIN -       PIN2
   IR reciever -                  PIN3
   RED LED -                      PIN5(PWM)
   GREEN LED -                    PIN6(PWM)
   BLUE LED -                     PIN9(PWM)
   Microphone Output -            PIN A0
   Button -                       PIN13

   Free Pins: 4,7,8,10,11,12

   Send device info to app on:
     Connection
     Command sent from app
     Command sent from remote

    ////IR Remote Functions////
    1.ON Light (fade ON)
    2.OFF Light (fade OFF)
    3.Dim light
    8.Activate/deactivate Microphone
    0.Activate/deactivate Flow Mode
    Microphone Status is indicated using LED

    EEPROM
    1. Last status of light
    3. Last status of Sound Sensor
    4. Last status of Flow effect
    7. Delay

       Bluetooth
*/

#include <IRremote.h>
#include <EEPROM.h>
#include <virtuabotixRTC.h>

//PIN Assignment
#define RED_LED 5
#define GREEN_LED 6
#define BLUE_LED 9   //this should be on pin9
#define microphoneControlPin 2
#define RECV_PIN 3
#define pushButton 13
//#define pushButton 4

#define FLOW_DELAY 10

int red_random;
int red_random_previous;
int green_random;
int green_random_previous;
int blue_random;
int blue_random_previous;

int red_randomFlow;
int green_randomFlow;
int blue_randomFlow;

float brightness;
float previousBrightnessValue;

boolean delayStatus;
boolean delayMode = 0;

boolean toggleLightStatus;
boolean toggleREDStatus;
boolean toggleGREENStatus;
boolean toggleBLUEStatus;

int presentDIMcontroller;
int previousDIMcontroller;

int presentREDcontroller;
int previousREDcontroller;

int presentGREENcontroller;
int previousGREENcontroller;

int presentBLUEcontroller;
int previousBLUEcontroller;

char cycleMoods;
//char cycleEffects;

//music visualizer variables
int microphoneValue = 0;
int currentRED = 0;
int currentGREEN = 0;
int currentBLUE = 0;

boolean musicVisualizerMode;
boolean toggleMicrophoneStatus = 0; // variable to toggle the state of Sound Sensor
int mic = A0;
// Variables to find the peak-to-peak amplitude of AUD output
const int sampleTime = 5;
int micOut;
int vCompare = 100;

//flow variables
int random_val;
boolean toggleFlowStatus = 0;
boolean flowMode = 0;

//fire variables
boolean toggleFireStatus = 0;
boolean fireMode = 0;

//siren variables
boolean toggleSirenStatus = 0;
boolean sirenMode = 0;

//disco variables
boolean toggleDiscoStatus = 0;
boolean discoMode;

//pool variables
boolean togglePoolStatus = 0;
boolean poolMode = 0;

int activeMood;

char flowInterval;
long flowIntervalValue;

char brightnessValue;

//Delay Variables
int delaySaver;
unsigned long delay_result;
unsigned long last_operation;

//EEPROM Positions
char allLightsPosition = 1;          //EEPROM position for light status
char redLightposition = 10;
char greenLightposition = 11;
char blueLightposition = 12;
char delayPosition = 6;                //EEPROM position for lights delay

char flowPosition = 8;            //EEPROM position for free dance
char firePosition = 2;
char sirenPosition = 3;
char discoPosition = 4;
char poolPosition = 5;

char microphonePosition = 9;          //EEPROM position for sound sensor
char delayStatusPosition = 13;

char activeMoodPosition = 14;       //EEPROM position for active mood

char brightnessPosition = 15;     //EEPROM position for brightness
char flowIntervalPosition = 16;   //EEPROM position for flow interval position

//App Update Variables
//unsigned long previousMillis = 0;        // will store last time
//const long interval = 50;           // interval at which to delay

unsigned long currentMillis = millis();
unsigned long previousMillis;

//LDR variables
const char ldrPin = A1;
unsigned long LDRtimeKeeper = 0;
int ldrStatus;
boolean nightDetectionMode;

//RTC variables
unsigned long RTCtimeKeeper = 0;
virtuabotixRTC myRTC(4, 7, 8); //If you change the wiring change the pins here also

//IR
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  Serial.begin(9600);
  //Serial.println("READY");

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(microphoneControlPin, OUTPUT);
  pinMode(pushButton, INPUT);   //declares push button pin

  //EEPROM Variables
  presentDIMcontroller = EEPROM.read(allLightsPosition);
  presentREDcontroller = EEPROM.read(redLightposition);
  presentGREENcontroller = EEPROM.read(greenLightposition);
  presentBLUEcontroller = EEPROM.read(blueLightposition);
  delayMode = EEPROM.read(delayStatusPosition);
  delaySaver = EEPROM.read(delayPosition);
  flowMode = EEPROM.read(flowPosition);
  fireMode = EEPROM.read(firePosition);
  poolMode = EEPROM.read(poolPosition);
  discoMode = EEPROM.read(discoPosition);
  sirenMode = EEPROM.read(sirenPosition);
  musicVisualizerMode = EEPROM.read(microphonePosition);
  activeMood = EEPROM.read(activeMoodPosition);
  brightnessValue = EEPROM.read(brightnessPosition);
  flowInterval = EEPROM.read(flowIntervalPosition);

  brightness = brightnessValue * 0.01;
  //Serial.println(brightness);

  irrecv.enableIRIn(); //Start the receiver

  // Set the current date, and time in the following format:
  // seconds, minutes, hours, day of the week, day of the month, month, year
  //myRTC.setDS1302Time(0, 32, 10, 7, 20, 7, 2019);
  //Here you write your actual time/date as shown above
  //but remember to "comment/remove" this function once you're done

  //////Light comes on and goes off quickly (aesthetics)
  ////////////ON////////////////
  for (int i = 0; i <= 20; i++)
  {
    analogWrite(RED_LED, i);
    analogWrite(GREEN_LED, i);
    analogWrite(BLUE_LED, i);
    delay(1);
  }

  delay(100);
  //////////////OFF////////////////
  for (int i = 20; i >= 0; i--)
  {
    analogWrite(RED_LED, i);
    delay(8);
  }
  for (int j = 20; j >= 0; j--)
  {
    analogWrite(GREEN_LED, j);
    delay(8);
  }
  for (int k = 20; k >= 0; k--)
  {
    analogWrite(BLUE_LED, k);
    delay(5);
  }
  delay(200);

  ////Function to return sound sensor to previous status
  if (musicVisualizerMode == 1)
  {
    digitalWrite(microphoneControlPin, HIGH);
    toggleMicrophoneStatus = 1;
    musicVisualizerMode = 1;
  }
  else
  {
    digitalWrite(microphoneControlPin, LOW);
    toggleMicrophoneStatus = 0;
    musicVisualizerMode = 0;
  }

  ////Function to return flow to previous status
  if (flowMode == 1)
  {
    toggleFlowStatus = 1;
    flowMode = 1;
  }
  else if (flowMode == 0)
  {
    toggleFlowStatus = 0;
    flowMode = 0;
  }

  ////Function to return fire to previous status
  if (fireMode == 1)
  {
    toggleFireStatus = 1;
    fireMode = 1;
  }
  else if (fireMode == 0)
  {
    toggleFireStatus = 0;
    fireMode = 0;
  }

  ////Function to return pool to previous status
  if (poolMode == 1)
  {
    togglePoolStatus = 1;
    poolMode = 1;
  }
  else if (poolMode == 0)
  {
    togglePoolStatus = 0;
    poolMode = 0;
  }

  ////Function to return disco to previous status
  if (discoMode == 1)
  {
    toggleDiscoStatus = 1;
    discoMode = 1;
  }
  else if (discoMode == 0)
  {
    toggleDiscoStatus = 0;
    discoMode = 0;
  }

  ////Function to return siren to previous status
  if (sirenMode == 1)
  {
    toggleSirenStatus = 1;
    sirenMode = 1;
  }
  else if (sirenMode == 0)
  {
    toggleSirenStatus = 0;
    sirenMode = 0;
  }

  ////Function to set delay mode
  if (delayMode == 1)
  {
    delayStatus = 1;
  }
  else if (delayMode == 0)
  {
    delayStatus = 0;
  }

  /////Function to set delay
  if (delaySaver == 0)
  {
    delay_result = 60000;
  }
  else if (delaySaver == 1)
  {
    delay_result = 300000;
  }
  else if (delaySaver == 2)
  {
    delay_result = 600000;
  }
  else if (delaySaver == 3)
  {
    delay_result = 1800000;
  }
  else if (delaySaver == 4)
  {
    delay_result = 3600000;
  }
  else if (delaySaver == 5)
  {
    delay_result = 7200000;
  }
  else if (delaySaver == 6)
  {
    delay_result = 10800000;
  }
  else if (delaySaver == 7)
  {
    delay_result = 14400000;
  }
  else if (delaySaver == 8)
  {
    delay_result = 18000000;
  }
  else if (delaySaver == 9)
  {
    delay_result = 21600000;
  }

  //Function to set flow interval
  if (flowInterval == 1) {
    flowIntervalValue = 60000;
  } else if (flowInterval == 2) {
    flowIntervalValue = 300000;
  } else if (flowInterval == 3) {
    flowIntervalValue = 600000;
  } else if (flowInterval == 4) {
    flowIntervalValue = 900000;
  } else if (flowInterval == 5) {
    flowIntervalValue = 1200000;
  } else if (flowInterval == 6) {
    flowIntervalValue = 1800000;
  } else if (flowInterval == 7) {
    flowIntervalValue = 2700000;
  } else if (flowInterval == 8) {
    flowIntervalValue = 3600000;
  }

  //Serial.println(flowIntervalValue);

  ////Function to returun light to previous status(color)
  if ((presentREDcontroller != 0) || (presentGREENcontroller != 0) || (presentBLUEcontroller != 0))
  {
    for (int i = previousREDcontroller; i <= presentREDcontroller; i++)
    {
      analogWrite(RED_LED, i * brightness);
      delay(2);
    }
    for (int j = previousGREENcontroller; j <= presentGREENcontroller; j++)
    {
      analogWrite(GREEN_LED, j * brightness);
      delay(2);
    }
    for (int k = previousBLUEcontroller; k <= presentBLUEcontroller; k++)
    {
      analogWrite(BLUE_LED, k * brightness);
      delay(2);
    }
    RGB_settings();
    toggleLightStatus = 1;
  }
  //Serial.println(musicVisualizerMode);
}

/////////////////////////LOOP//////////////////////////////////////////////
void loop() {
  bluetoothControl();

  myRTC.updateTime();

  if (irrecv.decode(&results))
  {
    translateIR();
    irrecv.resume(); // Receive the next IR value
  }

  buttonControl();
  musicVisualizer(findPTPAmp());
  flow();
  disco();
  fire();
  pool();
  siren();
  checkTimedelay();
  checkAmbientLight();
  //sendStatusToApp();
}


///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////FUNCTIONS//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
void bluetoothControl() {
  delay(20);
  String t; //create an empty string to store messages from Android
  while (Serial.available())
  { //keep reading bytes while they are still more in the buffer
    t += (char)Serial.read(); //read byte, convert to char, and append it to string
  }

  if (t.length()) //if string is not empty do the following
  {
    //Serial.println(t);    //print received character (for debugging)
    //Serial.println(toggleMicrophoneStatus);

    if (t == "a")   // ON/OFF
    {
      if (toggleLightStatus == 0)
      {
        /*offMicrophone();
          offFlow();
          offFire();
          offSiren();
          offDisco();
          offPool();
          presentREDcontroller = 255;
          presentGREENcontroller = 255;
          presentBLUEcontroller = 255;
          RGB_on();
          toggleLightStatus = 1;
          RGB_settings();*/

      }
      /////////////////////////OFF////////////////////////////////////////////
      else
      {
        offMicrophone();
        offFlow();

        offFire();
        offSiren();
        offDisco();
        offPool();
        RGB_off();
        toggleLightStatus = 0;
        RGB_settings();
        Serial.println("...6");
      }
      deactivateAllMoods();
    }

    if (t == "b")   //Microphone
    {
      if (toggleMicrophoneStatus == 0) {
        //Enable microphone
        digitalWrite(microphoneControlPin, HIGH);
        toggleMicrophoneStatus = 1;
        musicVisualizerMode = 1;
        EEPROM.write(microphonePosition, musicVisualizerMode);

        offFlow();
        offFire();
        offSiren();
        offDisco();
        offPool();

        toggleLightStatus = 1;
        Serial.println("....m");
      }
      else {
        offMicrophone();

        toggleLightStatus = 0;

        presentREDcontroller = 0;
        presentGREENcontroller = 0;
        presentBLUEcontroller = 0;

        previousREDcontroller = microphoneValue;
        previousGREENcontroller = microphoneValue;
        previousBLUEcontroller = microphoneValue;

        if (digitalRead(RED_LED) == HIGH) {
          for (int i = previousREDcontroller; i >= presentREDcontroller; i--)
          {
            analogWrite(RED_LED, i * brightness);
            delay(2);
          }
        }
        if (digitalRead(GREEN_LED) == HIGH) {
          for (int j = previousGREENcontroller; j >= presentGREENcontroller; j--)
          {
            analogWrite(GREEN_LED, j * brightness);
            delay(2);
          }
        }
        if (digitalRead(BLUE_LED) == HIGH) {
          for (int k = previousBLUEcontroller; k >= presentBLUEcontroller; k--)
          {
            analogWrite(BLUE_LED, k * brightness);
            delay(2);
          }
        }

        previousREDcontroller = presentREDcontroller;
        previousGREENcontroller = presentGREENcontroller;
        previousBLUEcontroller = presentBLUEcontroller;
        EEPROM.write(redLightposition, previousREDcontroller);
        EEPROM.write(greenLightposition, previousGREENcontroller);
        EEPROM.write(blueLightposition, previousBLUEcontroller);
        Serial.println("....n"); //should be sent by offMicrophone function
        //Serial.println(toggleMicrophoneStatus);
      }
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
    }

    if (t == "r")     /////////////////////FLOW EFFECT
    {
      {
        if (toggleFlowStatus == 0) {
          toggleFlowStatus = 1;
          flowMode = 1;
          EEPROM.write(flowPosition, flowMode);

          offMicrophone();
          offFire();
          offSiren();
          offDisco();
          offPool();

          red_randomFlow = random(0, 255);
          green_randomFlow = random(0, 255);
          blue_randomFlow = random(0, 255);

          if ((digitalRead(RED_LED) == HIGH) || (digitalRead(GREEN_LED) == HIGH) || (digitalRead(BLUE_LED) == HIGH))
          {
            for (int i = previousREDcontroller; i >= red_randomFlow; i--)
            {
              analogWrite(RED_LED, i * brightness);
              delay(2);
            }
            for (int j = previousGREENcontroller; j >= green_randomFlow; j--)
            {
              analogWrite(GREEN_LED, j * brightness);
              delay(2);
            }
            for (int k = previousBLUEcontroller; k >= blue_randomFlow; k--)
            {
              analogWrite(BLUE_LED, k * brightness);
              delay(2);
            }
          }
          else
          {
            for (int i = previousREDcontroller; i <= red_randomFlow; i++)
            {
              analogWrite(RED_LED, i * brightness);
              delay(2);
            }
            for (int j = previousGREENcontroller; j <= green_randomFlow; j++)
            {
              analogWrite(GREEN_LED, j * brightness);
              delay(2);
            }
            for (int k = previousBLUEcontroller; k <= blue_randomFlow; k++)
            {
              analogWrite(BLUE_LED, k * brightness);
              delay(2);
            }
          }
        }
      }
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
      sendLightON();
    }

    if (t == "s")   /////////////////SIREN EFFECT
    {
      {
        if (toggleSirenStatus == 0) {   ////ON
          toggleSirenStatus = 1;
          sirenMode = 1;
          EEPROM.write(sirenPosition, sirenMode);
          offMicrophone();
          offFire();
          offFlow();
          offDisco();
          offPool();

          for (int i = previousREDcontroller; i >= 0; i--)
          {
            analogWrite(RED_LED, i * brightness);
            delay(2);
          }
          for (int j = previousGREENcontroller; j >= 0; j--)
          {
            analogWrite(GREEN_LED, j * brightness);
            delay(2);
          }
          for (int k = previousBLUEcontroller; k >= 0; k--)
          {
            analogWrite(BLUE_LED, k * brightness);
            delay(2);
          }
        }
      }
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
      sendLightON();
    }

    if (t == "t")   /////////////////DISCO EFFECT
    {
      {
        if (toggleDiscoStatus == 0) {
          toggleDiscoStatus = 1;
          discoMode = 1;
          EEPROM.write(discoPosition, discoMode);

          offMicrophone();

          offFire();
          offSiren();
          offFlow();
          offPool();

          red_random = random(0, 255);
          green_random = random(0, 150);
          blue_random = random(0, 150);

          if ((digitalRead(RED_LED) == HIGH) || (digitalRead(GREEN_LED) == HIGH) || (digitalRead(BLUE_LED) == HIGH))
          {
            for (int i = previousREDcontroller; i >= red_random; i--)
            {
              analogWrite(RED_LED, i * brightness);
              delay(2);
            }
            for (int j = previousGREENcontroller; j >= green_random; j--)
            {
              analogWrite(GREEN_LED, j * brightness);
              delay(2);
            }
            for (int k = previousBLUEcontroller; k >= blue_random; k--)
            {
              analogWrite(BLUE_LED, k * brightness);
              delay(2);
            }
          }
          else
          {
            for (int i = previousREDcontroller; i <= red_random; i++)
            {
              analogWrite(RED_LED, i * brightness);
              delay(2);
            }
            for (int j = previousGREENcontroller; j <= green_random; j++)
            {
              analogWrite(GREEN_LED, j * brightness);
              delay(2);
            }
            for (int k = previousBLUEcontroller; k <= blue_random; k++)
            {
              analogWrite(BLUE_LED, k * brightness);
              delay(2);
            }
          }
        }
      }
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
      sendLightON();
    }

    if (t == "u")   /////////////////POOL EFFECT
    {
      if (togglePoolStatus == 0) {   ////ON
        togglePoolStatus = 1;
        poolMode = 1;
        EEPROM.write(poolPosition, poolMode);

        offMicrophone();

        offSiren();
        offFire();
        offFlow();
        offDisco();

        green_random = random(100, 255);

        if ((digitalRead(RED_LED) == HIGH) || (digitalRead(GREEN_LED) > 100) || (digitalRead(BLUE_LED) == HIGH))
        {
          //for (int i = previousREDcontroller; i >= 0; i--)
          //{
          analogWrite(RED_LED, 0);
          //delay(2);
          //}
          for (int j = previousGREENcontroller; j >= green_random; j--)
          {
            analogWrite(GREEN_LED, j * brightness);
            delay(2);
          }
          for (int k = previousBLUEcontroller; k >= 255; k--)
          {
            analogWrite(BLUE_LED, 0 * brightness);
            delay(2);
          }
        }
        else if ((digitalRead(RED_LED) == LOW) || (digitalRead(GREEN_LED) == LOW) || (digitalRead(BLUE_LED) == LOW))
        {
          for (int i = previousGREENcontroller; i <= 100; i++)
          {
            analogWrite(RED_LED, i * brightness);
            delay(2);
          }
          for (int j = previousBLUEcontroller; j <= 255; j++)
          {
            analogWrite(GREEN_LED, j * brightness);
            delay(2);
          }
        }
      }
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
      sendLightON();
    }

    if (t == "v")   ///////////////////FIRE EFFECT
    {
      if (toggleFireStatus == 0) {
        toggleFireStatus = 1;
        fireMode = 1;
        EEPROM.write(firePosition, fireMode);
        offMicrophone();
        offFlow();
        offSiren();
        offDisco();
        offPool();

        analogWrite(BLUE_LED, 0);

        if ((analogRead(RED_LED) > 100) || (analogRead(GREEN_LED) == HIGH))
        {
          for (int i = previousREDcontroller; i >= 100; i--)
          {
            analogWrite(RED_LED, i * brightness);
            delay(2);
          }
          for (int j = previousGREENcontroller; j >= green_random; j--)
          {
            analogWrite(GREEN_LED, j * brightness);
            delay(2);
          }
        }
        else if ((analogRead(RED_LED) < 100) || (analogRead(GREEN_LED) == LOW))
        {
          for (int i = previousREDcontroller; i <= 100; i++)
          {
            analogWrite(RED_LED, i * brightness);
            delay(2);
          }
          for (int j = previousGREENcontroller; j <= green_random; j++)
          {
            analogWrite(GREEN_LED, j * brightness);
            delay(2);
          }
        }
      }
      delay(100);
      sendLightON();
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();

    }

    ////////////Delay Activation
    if (t == "x")
    {
      delay(200);
      if (delayStatus == 0)
      {
        delayStatus = 1;
        EEPROM.write(delayStatusPosition, delayStatus);
        last_operation = millis();    //to reset last operation time
        Serial.println("...1");
      }
      else if (delayStatus == 1)
      {
        delayStatus = 0;
        EEPROM.write(delayStatusPosition, delayStatus);
        Serial.println("...2");
        last_operation = millis();    //to reset last operation time
      }
    }

    ////////Delay Setting
    if (t == "A")
    {
      delay(200);
      delay_result = 60000;
      delaySaver = 0;
      Serial.println(".K");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    else if (t == "B")
    {
      delay(200);
      delay_result = 300000;
      delaySaver = 1;
      Serial.println(".L");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    else if (t == "C")
    {
      delay(200);
      delay_result = 600000;
      delaySaver = 2;
      Serial.println(".M");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    if (t == "D")
    {
      delay(200);
      delay_result = 1800000;
      delaySaver = 3;
      Serial.println(".N");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    if (t == "E")
    {
      delay(200);
      delay_result = 3600000;
      delaySaver = 4;
      Serial.println(".O");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    if (t == "F")
    {
      delay(200);
      delay_result = 7200000;
      delaySaver = 5;
      Serial.println(".P");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    if (t == "G")
    {
      delay(200);
      delay_result = 10800000;
      delaySaver = 6;
      Serial.println(".Q");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    if (t == "H")
    {
      delay(200);
      delay_result = 14400000;
      delaySaver = 7;
      Serial.println(".R");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    if (t == "I")
    {
      delay(200);
      delay_result = 18000000;
      delaySaver = 8;
      Serial.println(".S");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }
    if (t == "J")
    {
      delay(200);
      delay_result = 21600000;
      delaySaver = 9;
      Serial.println(".T");
      EEPROM.write(delayPosition, delaySaver);
      last_operation = millis();    //to reset last operation time
    }

    //////////////All RGB (brightness)
    if (t.startsWith("f")) {
      delay(15);
      int value = t.substring(1).toInt();
      brightness = value * 0.01;
      float presentBrightness = brightness;
      analogWrite(RED_LED, presentREDcontroller * brightness);
      analogWrite(GREEN_LED, presentGREENcontroller * brightness);
      analogWrite(BLUE_LED, presentBLUEcontroller * brightness);

      /*if ((previousBrightnessValue > brightness))
        {
        for (brightness = previousBrightnessValue; brightness <= presentBrightness; brightness += 0.01)
        {
          analogWrite(RED_LED, presentREDcontroller * brightness);
          analogWrite(GREEN_LED, presentGREENcontroller * brightness);
          analogWrite(BLUE_LED, presentBLUEcontroller * brightness);
          delay(100);
        }
        }
        else
        {
        for (brightness = previousBrightnessValue; brightness >= presentBrightness; brightness -= 0.01)
        {
          analogWrite(RED_LED, presentREDcontroller * brightness);
          analogWrite(GREEN_LED, presentGREENcontroller * brightness);
          analogWrite(BLUE_LED, presentBLUEcontroller * brightness);
          delay(100);
        }
        }*/

      EEPROM.write(brightnessPosition, value);

      previousBrightnessValue = brightness;
    }

    //Color picker and custom moods
    //////////////Red alone
    if (t.startsWith("!")) {
      offMicrophone();
      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      delay(15);
      int value = t.substring(1).toInt();
      presentREDcontroller = value;

      if ((presentREDcontroller > 0) || (presentGREENcontroller > 0) || (presentBLUEcontroller > 0))
      {
        toggleLightStatus = 1;
      }
      if ((presentREDcontroller == 0) && (presentGREENcontroller == 0) && (presentBLUEcontroller == 0))
      {
        toggleLightStatus = 0;
      }

      //analogWrite(RED_LED, value);

      //RED
      if (value != "") {
        if (presentREDcontroller > previousREDcontroller)
        {
          for (value = previousREDcontroller; value <= presentREDcontroller; value++)
          {
            analogWrite(RED_LED, value * brightness);
            delay(1);
          }
        }
        else
        {
          for (value = previousREDcontroller; value >= presentREDcontroller; value--)
          {
            analogWrite(RED_LED, value * brightness);
            delay(1);
          }
        }
      }

      previousREDcontroller = presentREDcontroller;
      EEPROM.write(redLightposition, previousREDcontroller);
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
      sendLightON();
    }

    ///////////////Green alone
    else if (t.startsWith("y")) {
      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      delay(15);
      int value = t.substring(1).toInt();
      presentGREENcontroller = value;

      if ((presentREDcontroller > 0) || (presentGREENcontroller > 0) || (presentBLUEcontroller > 0))
      {
        toggleLightStatus = 1;
      }
      if ((presentREDcontroller == 0) && (presentGREENcontroller == 0) && (presentBLUEcontroller == 0))
      {
        toggleLightStatus = 0;
      }

      //analogWrite(GREEN_LED, value);

      //GREEN
      if (value != "") {
        if (presentGREENcontroller > previousGREENcontroller)
        {
          for (value = previousGREENcontroller; value <= presentGREENcontroller; value++)
          {
            analogWrite(GREEN_LED, value * brightness);
            delay(1);
          }
        }
        else
        {
          for (value = previousGREENcontroller; value >= presentGREENcontroller; value--)
          {
            analogWrite(GREEN_LED, value * brightness);
            delay(1);
          }
        }
      }

      previousGREENcontroller = presentGREENcontroller;
      EEPROM.write(greenLightposition, previousGREENcontroller);
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
      sendLightON();
    }


    /////////////Blue alone
    else if (t.startsWith("z")) { ////Blue alone
      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      delay(15);
      int value = t.substring(1).toInt();
      presentBLUEcontroller = value;

      if ((presentREDcontroller > 0) || (presentGREENcontroller > 0) || (presentBLUEcontroller > 0))
      {
        toggleLightStatus = 1;
      }
      if ((presentREDcontroller == 0) && (presentGREENcontroller == 0) && (presentBLUEcontroller == 0))
      {
        toggleLightStatus = 0;
      }

      //analogWrite(BLUE_LED, value);

      //BLUE
      if (value != "") {
        if (presentBLUEcontroller > previousBLUEcontroller)
        {
          for (value = previousBLUEcontroller; value <= presentBLUEcontroller; value++)
          {
            analogWrite(BLUE_LED, value * brightness);
            delay(1);
          }
        }
        else
        {
          for (value = previousBLUEcontroller; value >= presentBLUEcontroller; value--)
          {
            analogWrite(BLUE_LED, value * brightness);
            delay(1);
          }
        }
      }

      previousBLUEcontroller = presentBLUEcontroller;
      EEPROM.write(blueLightposition, previousBLUEcontroller);
      last_operation = millis();    //to reset last operation time
      deactivateAllMoods();
      sendLightON();
    }


    if (t == "j") //COZY
    {
      activeMood = 1;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 255;
      presentGREENcontroller = 65;
      presentBLUEcontroller = 0;

      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".A");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "q") //NATURAL
    {
      activeMood = 2;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 255;
      presentGREENcontroller = 255;
      presentBLUEcontroller = 100;
      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".B");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "k") //tulips
    {
      activeMood = 3;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 236;
      presentGREENcontroller = 0;
      presentBLUEcontroller = 139;

      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".C");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "l") //CANDLE LIGHT
    {
      activeMood = 4;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 100;
      presentGREENcontroller = 10;
      presentBLUEcontroller = 0;

      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".D");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "m") //CLASSIC
    {
      activeMood = 5;
      offMicrophone();
      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 180;
      presentGREENcontroller = 0;
      presentBLUEcontroller = 10;

      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".E");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "n") //ICY
    {
      activeMood = 6;
      offMicrophone();
      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 50;
      presentGREENcontroller = 150;
      presentBLUEcontroller = 255;

      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".F");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "o") //AQUAMARINE
    {
      activeMood = 7;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 0;
      presentGREENcontroller = 255;
      presentBLUEcontroller = 86;

      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".G");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "p") //white
    {
      activeMood = 8;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 255;
      presentGREENcontroller = 255;
      presentBLUEcontroller = 255;
      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".H");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == ",") //warm
    {
      activeMood = 9;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 255;
      presentGREENcontroller = 241;
      presentBLUEcontroller = 200;
      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".X");     ///////////////////////////////////////////////////
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "/") //Snow white
    {
      activeMood = 10;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 255;
      presentGREENcontroller = 253;
      presentBLUEcontroller = 249;
      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".Y");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "=") //pure
    {
      activeMood = 11;

      offMicrophone();

      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();

      previousREDcontroller = presentREDcontroller;
      previousGREENcontroller = presentGREENcontroller;
      previousBLUEcontroller = presentBLUEcontroller;

      presentREDcontroller = 255;
      presentGREENcontroller = 253;
      presentBLUEcontroller = 249;
      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println(".Z");
      EEPROM.write(activeMoodPosition, activeMood);
    }
    if (t == "w") //RANDOMIZE
    {
      randomize();
      deactivateAllMoods();
      Serial.println("..x");
    }

    if (t == "*") {
      if (flowInterval == 1) {
        Serial.println("(......1");
      } else if (flowInterval == 2) {
        Serial.println("(......2");
      } else if (flowInterval == 3) {
        Serial.println("(......3");
      } else if (flowInterval == 4) {
        Serial.println("(......4");
      } else if (flowInterval == 5) {
        Serial.println("(......5");
      } else if (flowInterval == 6) {
        Serial.println("(......6");
      } else if (flowInterval == 7) {
        Serial.println("(......7");
      } else if (flowInterval == 8) {
        Serial.println("(......8");
      }
    }

    //This command is sent immediately after succesfull connection
    if (t == "Y")   //Send device status to app
    {
      delay(100);
      String d2, d, aM, m, fI;
      int br = (int)(brightness * 100);

      //sendStatusToApp();
      if ((presentREDcontroller == 0) && (presentGREENcontroller == 0) && (presentBLUEcontroller == 0)) //if lamp is off
      {
        if ((flowMode == 0) && (sirenMode == 0) && (poolMode == 0) && (fireMode == 0) && (discoMode == 0))  //if no effect is active
        {
          if (musicVisualizerMode == 0)   //if music visualizer is not active
          { //turn on lamp
            presentREDcontroller = 128;
            presentGREENcontroller = 128;
            presentBLUEcontroller = 128;
            RGB_on();
            toggleLightStatus = 1;
            RGB_settings();
          }
        }
      }

      //delay
      if (delaySaver == 0) {
        d2 = "K";
      } else if (delaySaver == 1) {
        d2 = "L";
      } else if (delaySaver == 2) {
        d2 = "M";
      } else if (delaySaver == 3) {
        d2 = "N";
      } else if (delaySaver == 4) {
        d2 = "O";
      } else if (delaySaver == 5) {
        d2 = "P";
      } else if (delaySaver == 6) {
        d2 = "Q";
      } else if (delaySaver == 7) {
        d2 = "R";
      } else if (delaySaver == 8) {
        d2 = "S";
      } else if (delaySaver == 9) {
        d2 = "T";
      }

      //delay ON/OFF
      if (delayStatus == 1)
      {
        d = "...1";
      } else if (delayStatus == 0) {
        d = "...2";
      }

      //active mood
      if (activeMood == 1) {
        aM = "A";
      } else if (activeMood == 2) {
        aM = "B";
      } else if (activeMood == 3) {
        aM = "C";
      } else if (activeMood == 4) {
        aM = "D";
      } else if (activeMood == 5) {
        aM = "E";
      } else if (activeMood == 6) {
        aM = "F";
      } else if (activeMood == 7) {
        aM = "G";
      } else if (activeMood == 8) {
        aM = "H";
      } else if (activeMood == 9) {
        aM = "X";
      } else if (activeMood == 10) {
        aM = "Y";
      } else if (activeMood == 11) {
        aM = "Z";
      }
      else if (activeMood == 0) {
        aM = "I";
      }

      if (musicVisualizerMode == 1)
      {
        m = "3";  //ON
      } else if (musicVisualizerMode == 0) {
        m = "4";  //OFF
      }

      if (flowInterval == 1) {
        fI = "a";
      } else if (flowInterval == 2) {
        fI = "b";
      } else if (flowInterval == 3) {
        fI = "c";
      } else if (flowInterval == 4) {
        fI = "d";
      } else if (flowInterval == 5) {
        fI = "e";
      } else if (flowInterval == 6) {
        fI = "f";
      } else if (flowInterval == 7) {
        fI = "g";
      } else if (flowInterval == 8) {
        fI = "h";
      }

      delay(100);
      Serial.println("`.5" + d + aM + d2 + m + fI + br);
    }

    if (t == "-")
    {
      delay(100);
      String pR = (String)presentREDcontroller;
      String pG = (String)presentGREENcontroller;
      String pB = (String)presentBLUEcontroller;

      //MAKE UP FOR NUMBERS WITH LESS THAN 3 CHARACTERS
      if (pR.length() == 1) {
        pR = "00" + pR;
      }
      else if (pR.length() == 2) {
        pR = "0" + pR;
      }

      if (pG.length() == 1) {
        pG = "00" + pG;
      }
      else if (pG.length() == 2) {
        pG = "0" + pG;
      }

      if (pB.length() == 1) {
        pB = "00" + pB;
      }
      else if (pB.length() == 2) {
        pB = "0" + pB;
      }
      Serial.println("@.............." + pR + pG + pB);
    }

    if (t == "1")
    {
      flowInterval = 1;
      flowIntervalValue = 60000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......1");
    }
    if (t == "2")
    {
      flowInterval = 2;
      flowIntervalValue = 300000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......2");
    }
    if (t == "3")
    {
      flowInterval = 3;
      flowIntervalValue = 600000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......3");
    }
    if (t == "4")
    {
      flowInterval = 4;
      flowIntervalValue = 900000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......4");
    }
    if (t == "5")
    {
      flowInterval = 5;
      flowIntervalValue = 1200000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......5");
    }
    if (t == "6")
    {
      flowInterval = 6;
      flowIntervalValue = 1800000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......6");
    }
    if (t == "7")
    {
      flowInterval = 7;
      flowIntervalValue = 2700000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......7");
    }
    if (t == "8")
    {
      flowInterval = 8;
      flowIntervalValue = 3600000;
      EEPROM.write(flowIntervalPosition, flowInterval);
      Serial.println("(......8");
    }
    //delay(100);
    //sendStatusToApp();
  }
}

void translateIR()
{
  switch (results.value)
  {
    //////ON|OFF light(UP)
    //case 0x106728D7:  //ok
    case 0x38863BF4:
      {
        ///////////////////////////////ON/////////////////////////////////////
        if (toggleLightStatus == 0)
        {
          offMicrophone();
          offFlow();
          offFire();
          offSiren();
          offDisco();
          offPool();
          presentREDcontroller = 255;
          presentGREENcontroller = 255;
          presentBLUEcontroller = 255;
          RGB_on();
          toggleLightStatus = 1;
          RGB_settings();
          Serial.println("...5");
        }
        /////////////////////////OFF////////////////////////////////////////////
        else
        {
          offMicrophone();

          offFlow();
          offFire();
          offSiren();
          offDisco();
          offPool();

          RGB_off();
          toggleLightStatus = 0;
          RGB_settings();
          Serial.println(".6");
        }
        deactivateAllMoods();
        break;
      }
    /*///////////////INCREMENT DIMMER(CH UP)////////////
      //case 0x205DC03F:
      case 0x106728D7:
      {
        stopDanceandFreedance();
        digitalWrite(PIRcontrolPin, LOW);
        togglePIRStatus = 0;
        pirMode = 0;
        EEPROM.write(PIRPosition, pirMode);

        int RGBavg = (previousREDcontroller + previousGREENcontroller + previousBLUEcontroller) / 3;
        if (RGBavg == 255)   //////use BLUE LED values as controller
        {

        }
        else
        {
          RGBavg += 51;
        }

        analogWrite(RED_LED, RGBavg);
        analogWrite(GREEN_LED, RGBavg);
        analogWrite(BLUE_LED, RGBavg);

        previousREDcontroller = RGBavg;
        previousGREENcontroller = RGBavg;
        previousBLUEcontroller = RGBavg;
        EEPROM.write(redLightposition, previousREDcontroller);
        EEPROM.write(greenLightposition, previousGREENcontroller);
        EEPROM.write(blueLightposition, previousBLUEcontroller);
        last_operation = millis();    //to reset last operation time
        break;
      }
      ///////////////DECREMENT DIMMER(CH DOWN)////////////
      //case 0x205D40BF:
      case 0x10676897:
      {
        stopDanceandFreedance();
        digitalWrite(PIRcontrolPin, LOW);
        togglePIRStatus = 0;
        pirMode = 0;
        EEPROM.write(PIRPosition, pirMode);

        int RGBavg = (previousREDcontroller + previousGREENcontroller + previousBLUEcontroller) / 3;
        if (RGBavg == 0)   //////use BLUE LED values as controller
        {

        }
        else
        {
          RGBavg -= 51;
        }

        analogWrite(RED_LED, RGBavg);
        analogWrite(GREEN_LED, RGBavg);
        analogWrite(BLUE_LED, RGBavg);

        previousREDcontroller = RGBavg;
        previousGREENcontroller = RGBavg;
        previousBLUEcontroller = RGBavg;
        EEPROM.write(redLightposition, previousREDcontroller);
        EEPROM.write(greenLightposition, previousGREENcontroller);
        EEPROM.write(blueLightposition, previousBLUEcontroller);
        last_operation = millis();    //to reset last operation time
        break;
      }*/
    /////////Cycle Moods
    //case 0x205D48B7:
    //case 0x1067A857:  //OK
    case 0x38863BFA:  //down
      {
        offMicrophone();
        offFlow();
        offFire();
        offSiren();
        offDisco();
        offPool();

        previousREDcontroller = presentREDcontroller;
        previousGREENcontroller = presentGREENcontroller;
        previousBLUEcontroller = presentBLUEcontroller;

        if (cycleMoods == 11)
        {
          cycleMoods = 1;
        }
        else
        {
          cycleMoods += 1;
        }

        if (cycleMoods == 1)////COZY
        {
          activeMood = 1;
          presentREDcontroller = 255;
          presentGREENcontroller = 65;
          presentBLUEcontroller = 0;
          Serial.println(".A");
        }
        else if (cycleMoods == 2)////////Daylight
        {
          activeMood = 2;
          presentREDcontroller = 255;
          presentGREENcontroller = 225;
          presentBLUEcontroller = 100;
          Serial.println(".B");
        }
        else if (cycleMoods == 3)/////WARM
        {
          activeMood = 3;
          presentREDcontroller = 236;
          presentGREENcontroller = 0;
          presentBLUEcontroller = 139;
          Serial.println(".C");
        }
        else if (cycleMoods == 4)//////DULL
        {
          activeMood = 4;
          presentREDcontroller = 100;
          presentGREENcontroller = 10;
          presentBLUEcontroller = 0;
          Serial.println(".D");
        }
        else if (cycleMoods == 5)///////CLASSIC
        {
          activeMood = 5;
          presentREDcontroller = 180;
          presentGREENcontroller = 0;
          presentBLUEcontroller = 10;
          Serial.println(".E");
        }
        else if (cycleMoods == 6)//////COLD
        {
          activeMood = 6;
          presentREDcontroller = 50;
          presentGREENcontroller = 200;
          presentBLUEcontroller = 255;
          Serial.println(".F");
        }
        else if (cycleMoods == 7)//////AQUAMARINE
        {
          activeMood = 7;
          presentREDcontroller = 0;
          presentGREENcontroller = 255;
          presentBLUEcontroller = 255;
          Serial.println(".G");
        }
        else if (cycleMoods == 8)////////BLUE
        {
          activeMood = 8;
          presentREDcontroller = 0;
          presentGREENcontroller = 25;
          presentBLUEcontroller = 255;
          Serial.println(".H");
        }
        else if (cycleMoods == 9)////////WARM
        {
          activeMood = 9;
          presentREDcontroller = 255;
          presentGREENcontroller = 241;
          presentBLUEcontroller = 200;
          Serial.println(".X");
        }
        else if (cycleMoods == 10)////////SNOW WHITE
        {
          activeMood = 10;
          presentREDcontroller = 255;
          presentGREENcontroller = 253;
          presentBLUEcontroller = 249;
          Serial.println(".Y");
        }
        else if (cycleMoods == 11)////////PURE
        {
          activeMood = 11;
          presentREDcontroller = 255;
          presentGREENcontroller = 253;
          presentBLUEcontroller = 249;
          Serial.println(".Z");
        }
        toggleLightStatus = 1;
        RGB_on();
        RGB_settings();
        EEPROM.write(activeMoodPosition, activeMood);
        break;
      }

    //PIR Mode Setting
    //case 0x205DCA35:
    case 0x106712ED:  //9
      {
        break;
      }

    ///EFFECT (FLOW)
    //case 0x205D00FF:
    //case 0x1067926D:  //8
    case 0x38863BCA:  //left
      {
        if (toggleFireStatus == 0) {
          toggleFireStatus = 1;
          fireMode = 1;
          EEPROM.write(firePosition, fireMode);
          offMicrophone();
          offFlow();
          offSiren();
          offDisco();
          offPool();

          analogWrite(BLUE_LED, 0);

          if ((analogRead(RED_LED) > 100) || (analogRead(GREEN_LED) == HIGH))
          {
            for (int i = previousREDcontroller; i >= 100; i--)
            {
              analogWrite(RED_LED, i * brightness);
              delay(2);
            }
            for (int j = previousGREENcontroller; j >= green_random; j--)
            {
              analogWrite(GREEN_LED, j * brightness);
              delay(2);
            }
          }
          else if ((analogRead(RED_LED) < 100) || (analogRead(GREEN_LED) == LOW))
          {
            for (int i = previousREDcontroller; i <= 100; i++)
            {
              analogWrite(RED_LED, i * brightness);
              delay(2);
            }
            for (int j = previousGREENcontroller; j <= green_random; j++)
            {
              analogWrite(GREEN_LED, j * brightness);
              delay(2);
            }
          }
        }
        delay(100);
        sendLightON();
        last_operation = millis();    //to reset last operation time
        deactivateAllMoods();
        break;
      }

    ///Music Visualizer
    //case 0x205DC23D:
    //case 0x106752AD:  //7
    case 0x38863BC2:  //right
      {
        if (toggleMicrophoneStatus == 0) {
          //Enable microphone
          digitalWrite(microphoneControlPin, HIGH);
          toggleMicrophoneStatus = 1;
          musicVisualizerMode = 1;
          EEPROM.write(microphonePosition, musicVisualizerMode);

          offFlow();
          offFire();
          offSiren();
          offDisco();
          offPool();

          toggleLightStatus = 1;
        }
        else {
          offMicrophone();

          toggleLightStatus = 0;

          presentREDcontroller = 0;
          presentGREENcontroller = 0;
          presentBLUEcontroller = 0;

          previousREDcontroller = microphoneValue;
          previousGREENcontroller = microphoneValue;
          previousBLUEcontroller = microphoneValue;

          if (digitalRead(RED_LED) == HIGH) {
            for (int i = previousREDcontroller; i >= presentREDcontroller; i--)
            {
              analogWrite(RED_LED, i);
              delay(2);
            }
          }
          if (digitalRead(GREEN_LED) == HIGH) {
            for (int j = previousGREENcontroller; j >= presentGREENcontroller; j--)
            {
              analogWrite(GREEN_LED, j);
              delay(2);
            }
          }
          if (digitalRead(BLUE_LED) == HIGH) {
            for (int k = previousBLUEcontroller; k >= presentBLUEcontroller; k--)
            {
              analogWrite(BLUE_LED, k);
              delay(2);
            }
          }
          previousREDcontroller = presentREDcontroller;
          previousGREENcontroller = presentGREENcontroller;
          previousBLUEcontroller = presentBLUEcontroller;
          EEPROM.write(redLightposition, previousREDcontroller);
          EEPROM.write(greenLightposition, previousGREENcontroller);
          EEPROM.write(blueLightposition, previousBLUEcontroller);
        }
        last_operation = millis();    //to reset last operation time
        deactivateAllMoods();
        break;
      }

    //Randomize
    //case 0x1067B24D:  //(0)
    case 0x38863BF2:  //up
      {
        randomize();
        deactivateAllMoods();
        Serial.println("..x");
        break;
      }
  }/////END OF SWITCH FUNCTION
}/////END OF IR TRANSLATE FUNCTION

//Button Control
void buttonControl()
{
  int state = digitalRead(pushButton);
  if (state == HIGH)
  {
    delay(200);
    if (toggleLightStatus == 0)
    {
      offMicrophone();
      offFlow();
      offFire();
      offSiren();
      offDisco();
      offPool();
      presentREDcontroller = 255;
      presentGREENcontroller = 255;
      presentBLUEcontroller = 255;
      RGB_on();
      toggleLightStatus = 1;
      RGB_settings();
      Serial.println("...5");
    }
    /////////////////////////OFF////////////////////////////////////////////
    else
    {
      offMicrophone();
      offFlow();
      RGB_off();
      offFire();
      offSiren();
      offDisco();
      offPool();
      toggleLightStatus = 0;
      RGB_settings();
      Serial.println("...6");
    }
    deactivateAllMoods();

    //sendStatusToApp();
  }
}

void RGB_on()
{
  //RED
  if (presentREDcontroller > previousREDcontroller)
  {
    for (int i = previousREDcontroller; i <= presentREDcontroller; i++)
    {
      analogWrite(RED_LED, i * brightness);
      delay(2);
    }
  }
  else
  {
    for (int i = previousREDcontroller; i >= presentREDcontroller; i--)
    {
      analogWrite(RED_LED, i * brightness);
      delay(2);
    }
  }

  //GREEN
  if (presentGREENcontroller > previousGREENcontroller)
  {
    for (int j = previousGREENcontroller; j <= presentGREENcontroller; j++)
    {
      analogWrite(GREEN_LED, j * brightness);
      delay(2);
    }
  }
  else
  {
    for (int j = previousGREENcontroller; j >= presentGREENcontroller; j--)
    {
      analogWrite(GREEN_LED, j * brightness);
      delay(2);
    }
  }
  //BLUE
  if (presentBLUEcontroller > previousBLUEcontroller)
  {
    for (int k = previousBLUEcontroller; k <= presentBLUEcontroller; k++)
    {
      analogWrite(BLUE_LED, k * brightness);
      delay(2);
    }
  }
  else
  {
    for (int k = previousBLUEcontroller; k >= presentBLUEcontroller; k--)
    {
      analogWrite(BLUE_LED, k * brightness);
      delay(2);
    }
  }
}
void RGB_off()
{
  presentREDcontroller = 0;
  presentGREENcontroller = 0;
  presentBLUEcontroller = 0;
  for (int i = previousREDcontroller * brightness; i >= presentREDcontroller; i--)
  {
    analogWrite(RED_LED, i);
    delay(2);
  }
  for (int j = previousGREENcontroller * brightness; j >= presentGREENcontroller; j--)
  {
    analogWrite(GREEN_LED, j);
    delay(2);
  }
  for (int k = previousBLUEcontroller * brightness; k >= presentBLUEcontroller; k--)
  {
    analogWrite(BLUE_LED, k);
    delay(2);
  }
}

void RGB_settings()
{
  previousREDcontroller = presentREDcontroller;
  previousGREENcontroller = presentGREENcontroller;
  previousBLUEcontroller = presentBLUEcontroller;
  EEPROM.write(redLightposition, previousREDcontroller);
  EEPROM.write(greenLightposition, previousGREENcontroller);
  EEPROM.write(blueLightposition, previousBLUEcontroller);
  last_operation = millis();    //to reset last operation time
}

//Put OFF Microphone
void offMicrophone()
{
  digitalWrite(microphoneControlPin, LOW);
  toggleMicrophoneStatus = 0;
  musicVisualizerMode = 0;
  EEPROM.write(microphonePosition, musicVisualizerMode);
}

//Stop Flow
void offFlow()
{
  toggleFlowStatus = 0;
  flowMode = 0;
  EEPROM.write(flowPosition, flowMode);
}

//Stop Fire
void offFire()
{
  toggleFireStatus = 0;
  fireMode = 0;
  EEPROM.write(firePosition, fireMode);
}

//Stop Siren
void offSiren()
{
  toggleSirenStatus = 0;
  sirenMode = 0;
  EEPROM.write(sirenPosition, sirenMode);
}

//Stop Disco
void offDisco()
{
  toggleDiscoStatus = 0;
  discoMode = 0;
  EEPROM.write(discoPosition, discoMode);
}

//Stop Pool
void offPool()
{
  togglePoolStatus = 0;
  poolMode = 0;
  EEPROM.write(poolPosition, poolMode);
}

void deactivateAllMoods()
{
  activeMood = 0;
  EEPROM.write(activeMoodPosition, activeMood);
}

//Flow
void flow()
{
  if (toggleFlowStatus == 1)
  {
    offMicrophone();
    offFire();
    offSiren();
    offDisco();
    offPool();

    toggleLightStatus = 1;
    if (millis() - previousMillis >= flowIntervalValue) {
      previousMillis = millis();

      //Serial.println("Count");
      //Serial.println(flowIntervalValue);

      red_randomFlow = random(0, 255);
      green_randomFlow = random(0, 255);
      blue_randomFlow = random(0, 255);

      presentREDcontroller = red_randomFlow;
      presentGREENcontroller = green_randomFlow;
      presentBLUEcontroller = blue_randomFlow;

      RGB_on();
      Serial.println(".5");
      RGB_settings();
    }
  }
}

//Fire
void fire()
{
  if (toggleFireStatus == 1)
  {
    analogWrite(BLUE_LED, 0);
    green_random = random(0, 40);
    int fireDelay = random(0, 15);
    analogWrite(RED_LED, 255 * brightness);

    if (green_random > green_random_previous)
    {
      for (int i = green_random_previous; i <= green_random; i++)
      {
        analogWrite(GREEN_LED, i * brightness);
        delay(fireDelay);
      }
      green_random_previous = green_random;
    }
    else
    {
      for (int i = green_random_previous; i >= green_random; i--)
      {
        analogWrite(GREEN_LED, i * brightness);
        delay(fireDelay);
      }
      green_random_previous = green_random;
    }

    toggleLightStatus = 1;

    presentREDcontroller = 255;
    presentGREENcontroller = green_random;

    previousREDcontroller = presentREDcontroller;
    previousGREENcontroller = presentGREENcontroller;
    previousBLUEcontroller = presentBLUEcontroller;
  }
}

//Disco
void disco()
{
  if (toggleDiscoStatus == 1)
  {
    for (int x = 0; x < 4; x++)
    {
      red_random = random(0, 255);
      green_random = random(0, 150);
      blue_random = random(0, 150);

      analogWrite(RED_LED, red_random);
      analogWrite(GREEN_LED, green_random);
      analogWrite(BLUE_LED, blue_random);
      delay(50);

      for (int i = red_random; i >= 0; i--)
      {
        analogWrite(RED_LED, i * brightness);
        delay(1);
      }
      for (int j = green_random; j >= 0; j--)
      {
        analogWrite(GREEN_LED, j * brightness);
        delay(1);
      }
      for (int k = blue_random; k >= 0; k--)
      {
        analogWrite(BLUE_LED, k * brightness);
        delay(1);
      }
      delay(50);
    }
    delay(100);
    for (int x = 0; x < 8; x++)
    {
      red_random = random(0, 255);
      green_random = random(0, 150);
      blue_random = random(0, 150);

      analogWrite(RED_LED, red_random * brightness);
      analogWrite(GREEN_LED, green_random * brightness);
      analogWrite(BLUE_LED, blue_random * brightness);
      delay(70);
      analogWrite(RED_LED, 0);
      analogWrite(GREEN_LED, 0);
      analogWrite(BLUE_LED, 0);
      delay(70);
    }

    toggleLightStatus = 1;
    presentREDcontroller = red_random;
    presentGREENcontroller = green_random;
    presentBLUEcontroller = blue_random;

    previousREDcontroller = presentREDcontroller;
    previousGREENcontroller = presentGREENcontroller;
    previousBLUEcontroller = presentBLUEcontroller;
  }
}

//pool
void pool()
{
  if (togglePoolStatus == 1)
  {
    analogWrite(RED_LED, 0);
    green_random = random(100, 255);
    analogWrite(BLUE_LED, 255 * brightness);
    for (int i = 50; i < green_random; i++)   //ON
    {
      analogWrite(GREEN_LED, i * brightness);
      delay(green_random / 80);
    }

    toggleLightStatus = 1;
    presentGREENcontroller = green_random;
    presentBLUEcontroller = 255;

    previousREDcontroller = presentREDcontroller;
    previousGREENcontroller = presentGREENcontroller;
    previousBLUEcontroller = presentBLUEcontroller;
  }
}

//Siren
void siren()
{
  if (toggleSirenStatus == 1)
  {
    for (int i = 0; i < 2; i++)   //blink twice
    {
      analogWrite(RED_LED, 255 * brightness);
      delay(150);
      analogWrite(RED_LED, 0);
      analogWrite(BLUE_LED, 255 * brightness);
      delay(150);
      analogWrite(BLUE_LED, 0);
    }
    delay(500);
    for (int i = 0; i < 2; i++)   //blink twice
    {
      analogWrite(RED_LED, 255 * brightness);
      delay(150);
      analogWrite(RED_LED, 0);
      analogWrite(BLUE_LED, 255 * brightness);
      delay(150);
      analogWrite(BLUE_LED, 0);
    }
    delay(400);

    toggleLightStatus = 1;
    presentREDcontroller = 0;
    presentGREENcontroller = 0;
    presentBLUEcontroller = 0;

    previousREDcontroller = presentREDcontroller;
    previousGREENcontroller = presentGREENcontroller;
    previousBLUEcontroller = presentBLUEcontroller;
  }
}

// Find the Peak-to-Peak Amplitude Function
int findPTPAmp() {
  // Time variables to find the peak-to-peak amplitude
  unsigned long startTime = millis(); // Start of sample window
  unsigned int PTPAmp = 0;
  // Signal variables to find the peak-to-peak amplitude
  unsigned int maxAmp = 0;
  unsigned int minAmp = 1023;

  // Find the max and min of the mic output within the 50 ms timeframe
  while (millis() - startTime < sampleTime)
  {
    micOut = analogRead(mic);
    if ( micOut < 1023) //prevent erroneous readings
    {
      if (micOut > maxAmp)
      {
        maxAmp = micOut; //save only the max reading
      }
      else if (micOut < minAmp)
      {
        minAmp = micOut; //save only the min reading
      }
    }
  }
  PTPAmp = maxAmp - minAmp; // (max amp) - (min amp) = peak-to-peak amplitude
  //Uncomment this line for help debugging (be sure to also comment out the VUMeter function)
  Serial.println(PTPAmp);
  return PTPAmp;
}
////Music Visualizer
void musicVisualizer(int v)
{
  if (digitalRead(microphoneControlPin) == HIGH)
  {
    if (v < vCompare)    //STAY
  {
    analogWrite(RED_LED, currentRED * brightness);
    analogWrite(GREEN_LED, currentGREEN * brightness);
    analogWrite(BLUE_LED, currentBLUE * brightness);
  }
  if (v >= vCompare)  //RED_LED
  {
    analogWrite(RED_LED, 50 * brightness);
    analogWrite(GREEN_LED, 0);
    analogWrite(BLUE_LED, 0);
    currentRED = 100;
    currentGREEN = 0;
    currentBLUE = 0;
  }
  if (v >= vCompare * 1.2) //GREEN_LED
  {
    analogWrite(RED_LED, 0);
    analogWrite(GREEN_LED, 80 * brightness);
    analogWrite(BLUE_LED, 0);
    currentRED = 0;
    currentGREEN = 150;
    currentBLUE = 0;
  }
  if (v >= vCompare * 2) //BLUE_LED
  {
    analogWrite(RED_LED, 0);
    analogWrite(GREEN_LED, 0);
    analogWrite(BLUE_LED, 80 * brightness);
    currentRED = 0;
    currentGREEN = 0;
    currentBLUE = 180;
  }
  if (v >= vCompare * 3) //PURPLE
  {
    analogWrite(RED_LED, 100 * brightness);
    analogWrite(GREEN_LED, 0);
    analogWrite(BLUE_LED, 100 * brightness);
    currentRED = 200;
    currentGREEN = 0;
    currentBLUE = 200;
  }
  if (v >= vCompare * 5) //YELLOW
  {
    analogWrite(RED_LED, 125 * brightness);
    analogWrite(GREEN_LED, 125 * brightness);
    analogWrite(BLUE_LED, 0);
    currentRED = 230;
    currentGREEN = 230;
    currentBLUE = 0;
  }
  if (v >= vCompare * 7) //AQUAMARINE
  {
    analogWrite(RED_LED, 0);
    analogWrite(GREEN_LED, 125 * brightness);
    analogWrite(BLUE_LED, 125 * brightness);
    currentRED = 0;
    currentGREEN = 230;
    currentBLUE = 230;
  }
  if (v > vCompare * 10) //WHITE
  {
    analogWrite(RED_LED, 125 * brightness);
    analogWrite(GREEN_LED, 125 * brightness);
    analogWrite(BLUE_LED, 125 * brightness);
    currentRED = 255;
    currentGREEN = 255;
    currentBLUE = 255;
  }

    toggleLightStatus = 1;
    presentREDcontroller = 0;
    presentGREENcontroller = 0;
    presentBLUEcontroller = 0;

    previousREDcontroller = presentREDcontroller;
    previousGREENcontroller = presentGREENcontroller;
    previousBLUEcontroller = presentBLUEcontroller;
  }
}

void checkTimedelay()
{
  if ((presentREDcontroller > 0) || (presentGREENcontroller > 0) || (presentBLUEcontroller > 0))
  {
    if (delayStatus == 1)
    {
      if ((millis() - last_operation) >= delay_result)
      {
        //if ((flowMode == 0) && (fireMode == 0) && (poolMode == 0) && (sirenMode == 0) && (discoMode == 0))
        {
          if (musicVisualizerMode == 0)
          {
            presentREDcontroller = 0;
            presentGREENcontroller = 0;
            presentBLUEcontroller = 0;
            for (int i = previousREDcontroller; i >= presentREDcontroller; i--)
            {
              analogWrite(RED_LED, i);
              delay(4);
            }
            for (int j = previousGREENcontroller; j >= presentGREENcontroller; j--)
            {
              analogWrite(GREEN_LED, j);
              delay(4);
            }
            for (int k = previousBLUEcontroller; k >= presentBLUEcontroller; k--)
            {
              analogWrite(BLUE_LED, k);
              delay(4);
            }
            offFlow();
            offFire();
            offSiren();
            offDisco();
            offPool();
            deactivateAllMoods();
            RGB_settings();
            sendStatusToApp();
            toggleLightStatus = 0;
            last_operation = millis();    //to reset last operation time
          }
        }
      }
    }
  }
}

void randomize()
{
  offMicrophone();
  offFlow();
  offFire();
  offSiren();
  offDisco();
  offPool();

  previousREDcontroller = presentREDcontroller;
  previousGREENcontroller = presentGREENcontroller;
  previousBLUEcontroller = presentBLUEcontroller;

  red_random = random(0, 255);
  green_random = random(0, 255);
  blue_random = random(0, 255);

  presentREDcontroller = red_random;
  presentGREENcontroller = green_random;
  presentBLUEcontroller = blue_random;

  RGB_on();
  toggleLightStatus = 1;
  RGB_settings();
}

void factorInBrightness()
{
  presentREDcontroller = presentREDcontroller * brightness;
  presentGREENcontroller = presentGREENcontroller * brightness;
  presentBLUEcontroller = presentBLUEcontroller * brightness;
}

void sendLightON() {
  Serial.println("...5");
}

void sendStatusToApp()
{
  //unsigned long currentMillis = millis();
  //if (currentMillis - previousMillis >= interval) {
  //previousMillis = currentMillis;
  String d, m, d2, l, aM, fI;
  if (delayStatus == 1)
  {
    d = "1";
  } else if (delayStatus == 0) {
    d = "2";
  }

  if (musicVisualizerMode == 1)
  {
    m = "3";
  } else if (musicVisualizerMode == 0) {
    m = "4";
  }

  if (toggleLightStatus == 1)
  {
    l = "5";
  } else if (toggleLightStatus == 0) {
    l = "6";
  }

  if (delaySaver == 0) {
    d2 = "K";
  } else if (delaySaver == 1) {
    d2 = "L";
  } else if (delaySaver == 2) {
    d2 = "M";
  } else if (delaySaver == 3) {
    d2 = "N";
  } else if (delaySaver == 4) {
    d2 = "O";
  } else if (delaySaver == 5) {
    d2 = "P";
  } else if (delaySaver == 6) {
    d2 = "Q";
  } else if (delaySaver == 7) {
    d2 = "R";
  } else if (delaySaver == 8) {
    d2 = "S";
  } else if (delaySaver == 9) {
    d2 = "T";
  }

  if (activeMood == 1) {
    aM = "A";
  } else if (activeMood == 2) {
    aM = "B";
  } else if (activeMood == 3) {
    aM = "C";
  } else if (activeMood == 4) {
    aM = "D";
  } else if (activeMood == 5) {
    aM = "E";
  } else if (activeMood == 6) {
    aM = "F";
  } else if (activeMood == 7) {
    aM = "G";
  } else if (activeMood == 8) {
    aM = "H";
  } else if (activeMood == 0) {
    aM = "I";
  }

  if (flowInterval == 1) {
    fI = "a";
  } else if (flowInterval == 2) {
    fI = "b";
  } else if (flowInterval == 3) {
    fI = "c";
  } else if (flowInterval == 4) {
    fI = "d";
  } else if (flowInterval == 5) {
    fI = "e";
  } else if (flowInterval == 6) {
    fI = "f";
  } else if (flowInterval == 7) {
    fI = "g";
  } else if (flowInterval == 8) {
    fI = "h";
  }

  Serial.println(d + m + l + d2 + aM + fI);
  //}
}

void checkAmbientLight(){
  //check ldr status every 5 seconds
  if ((millis() - LDRtimeKeeper) >= 5000){
    ldrStatus = analogRead(ldrPin);
    Serial.println(ldrStatus);
    LDRtimeKeeper = millis();

    if(ldrStatus <= 200){
      //digitalWrite(WHITE, HIGH);
    }
  }
}
