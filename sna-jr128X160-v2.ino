/*********************************************************************
 *
 * Simple SNA for the  HF amateur bands
 * portions of this software are covered by the GNU General Public License ,
 * BEERWARE, or other open source licenses
 * DuWayne  KV4QB
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <duwayne@kv4qb.us> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   DuWayne
 * ----------------------------------------------------------------------------
 *
 *
 * Libraries used Adafruit LCD and Graphics Libraries

 * rotary.h from AD7C
 *
 *
 *************/

#include <SPI.h>
#include <rotary.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // Hardware-specific library
// pins used by TFT Display
#define TFT_CS 9    //6
#define TFT_DC 6  //10
#define TFT_RST 8   //7
#define Backlight 10 //9 // Analog output pin that the LED is attached to

// Color definitions
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_ST7735 display = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);


// AD9850 info
#define W_CLK A5   //  connect to AD9850 module word load clock pin (CLK)
#define FQ_UD A4   //  connect to freq update pin (FQ)
#define DATA A3   // connect to serial data load pin (DATA)
#define DDS_RESET A2 //- connect to reset pin (RST) 
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }

#define AD8307_POWER_CALIBERATION (-872)    // value is determained by checking reading against  known input value
                                            // start with 840   ad8307 log intgerccedpt * 10
#define FreqCorr   0.2897435                // frequency correction   difference in value from 1 and 40 Mhz / 39

//Global variables

// used by sweep and PC input
long int frq;
double Fstart_MHz  = 1000000;  // Start Frequency for sweep
double Fstop_MHz   = 40000000;  // Stop Frequency for sweep
double current_freq_MHz;      // Temp variable used during sweep
double Fstep_MHz = 10000;             //size of Sweep step
double VFOstep_Khz = 100000;             //size of VFO step

int num_steps = 160; // Number of steps to use in the sweep  change for screen size

// used in gain loss calculation


double db;               // db gain loss ref to v out
double dbPoint[160];       //sweep points  db*10
// lookup table for converfting return loss to swr * 10
int SWRPoint[46] =    {375, 375, 375, 375, 337, 301, 262, 232, 220, 208,
                       198, 187, 178, 160, 150, 143, 138, 133, 129, 125,
                       120, 117, 115, 113, 112, 111, 109, 108, 107, 107,
                       106, 105, 105, 104, 104, 103, 103, 103, 102, 102,
                       101, 101, 101, 100, 100, 100
                      };
double max_db;           // used to normalize data

float  powerdB = 0.00; // Calculated power in dBm
float  last_db = 0.00;
float  Peak_db = 0.00;
// used by encoder and pushbutton control
int buttonstate = 0;
double encoder;

//used by various display procedures
int c_pos;
int p;
double f1;
double f2;
double dbValue;


int RunOnce = 1;          // for anything I want to run once at startup

float SupplyVoltage = (0); // Power supply voltage
int backlightTimer = 5000;        // counts the cycles without input (turn off backlight after about 5 seconds of no rf)

// Setup for rotary encoder with push button
Rotary r = Rotary(2, 3); // sets the pins the rotary encoder uses.  Must be interrupt pins.
#define EncoderBtn 4

// Analog input for battery monitor
#define vin_BAT A6


#define R1   (10)  // from GND to vin_BAT, express in 100R  (10 = 1000 Ohm)
#define R2   (39)  // from + power supply to vin_BAT, express in 100R  (47 = 4700 Ohm)
#define VoltSupplyMini (550)       // minimum battery voltage expressed in 100mV (if lower, alarm is generated)

#define LOG_AMP A1

// system modalities
#define mSWEEP 0
#define mNORM  1
#define mFAST  2
#define mVFO   3
#define mDIP   4
#define mSWR   5
#define mWATT  6
int modality = mSWEEP ;

// display modes   either sweep or settings
#define dmSWEEP    0
#define dmSETTING  1
//int  dMODE  = dmSWEEP ;
int  dMODE  = dmSETTING  ;

// settings
#define sSTART  2
#define sEND    3
#define sSTEP   4
#define sMODE   1

int sITEM = sMODE ;

void setup() {

 // Serial.begin(57600);                    // connect to the serial port   only for testing   comment out for final build
 // Serial.println("KV4QB SNA Jr.");

  // initialize interrupts
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();

  // set up encoder and pushbutton pins
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);
  pinMode(EncoderBtn, INPUT);
  digitalWrite(EncoderBtn, HIGH);
  //
  // setup inputs for reading RF_out  RF_in and battery
  pinMode (LOG_AMP, INPUT);
  pinMode(vin_BAT, INPUT);

  // Use this initializer if you're using a 128x160 TFT
  display.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  display.setRotation(1);    // set orientation of display
  analogWrite(Backlight, 120);
  // Control is done with a state machine
  // start out in settings  and select mode
  dMODE = dmSETTING;
  sITEM =  sMODE ;

  // setup AD9850 module pins and initialze
  pinMode(FQ_UD, OUTPUT);
  digitalWrite(FQ_UD, LOW);
  pinMode(W_CLK, OUTPUT);
  digitalWrite(W_CLK, LOW);
  pinMode(DATA, OUTPUT);
  digitalWrite(DATA, LOW);
  pinMode(DDS_RESET, OUTPUT);
  pulseHigh(DDS_RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);  // this pulse enables serial mode on the AD9850
}

//
//   END of Setup
//
//    Main program loop
void loop()
{
  //check battery
  measuresupplyvolt();  // show Splash screen  only do once
  SplashScreen();
  RunOnce = 0;

  //  process pushbutton
  buttonstate = digitalRead(EncoderBtn);
  if (buttonstate == LOW) {
    delay(800);
    buttonstate = digitalRead(EncoderBtn);   //held over .8 second then toggle  sweep screen or settings
    if (buttonstate == LOW) {
      if (dMODE == dmSWEEP) {
        dMODE = dmSETTING;
        display.fillScreen(BLUE);
        dispSettingScreen();
      }
      else  {
        dMODE = dmSWEEP;
        display.fillScreen(BLUE);
        if  (modality  == mVFO) {
          dispVFOScreen();
          VFO_VM();
        }
        else
          dispScanScreen();
      }
      delay(100);
    }
    //   Got  correct display mode from long press now process short push
    //   first take care of settings
    //   Note you have to press button to enable settings change or go to display screen
    else if (dMODE == dmSETTING) {  //  process changes to settings
      if (sITEM == sSTART) {
        setStartFreq();
        sITEM =  sEND ;
      }
      else if (sITEM == sEND) {
        setEndFreq();
        sITEM   = sSTEP;
      }
      else if (sITEM == sSTEP) {
        setStepSize();
        sITEM = sMODE;
      }
      else if (sITEM == sMODE) {
        setModality();
        sITEM = sSTART;
      }
    }
    // Not a settings change
    // start data acquisition
    if (dMODE == dmSWEEP) {
      display.fillScreen(BLUE);
      if   (modality  ==  mVFO)
        VFO_VM();
      else if   (modality  ==  mDIP)
        find_dip();
      else if   (modality  ==  mFAST)
        Fast_Sweep();
      else if   (modality  ==  mWATT)
        WattMeter();
      else if   (modality  ==  mSWR){
        SWRScan();
         display.fillScreen(BLUE);
      }
      else
        Perform_sweep();
      //      dispScanScreen();
      drawSweep();
      sITEM = sMODE;
    }
  }

  //  button press processed  now use rotary
  //  to scroll through acquired data
  if (dMODE == dmSWEEP)
    scrollSweep();
}
//
//   end of loop
//
//                  *******      Functions    ************
//
// interrupt service routine for encoder
ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result) {
    if (result == DIR_CW) {
      encoder--;
    }
    else {
      encoder++;
    }
  }
}
//
//  Do a scan of band  compute gain loss for each point store to array and display sweep
//
void Perform_sweep() {
  int x;
  int x1 = 0;
  int y;
  int y1 ;
  // show screen for selected modality
  if ((modality == mSWEEP) || (modality == mNORM))
    dispScanScreen();
  // else if (modality == mNORM)
  //  dispScanScreen();
  // Find step size for sweep
  Fstep_MHz = (Fstop_MHz - Fstart_MHz) / num_steps;

  // Start scan loop
  max_db = -120;      // set peak finder for possible normalize of data
  for (int i = 0; i <= num_steps; i++) {
    // Calculate frequency for each step in scan
    current_freq_MHz = Fstart_MHz + (i * Fstep_MHz);
    sendFrequency(current_freq_MHz);
    delay(4);  // Wait a little for settling
    read_DB();
    dbPoint[i] = powerdB + (FreqCorr * (current_freq_MHz / 100000)); // store data add frequency correction
    if (max_db < dbPoint[i])  max_db = dbPoint[i];
  }
  if (modality == mNORM) {
    for (int i = 0; i <= num_steps; i++) {
      dbPoint[i] = (dbPoint[i] - (max_db)) ;
    }
  }
  // dispScanScreen();
  // drawSweep();

}

//scroll through sweep
void scrollSweep() {
  if (encoder != 0) {
    //   clear old values from screen
    display.fillRect(1, 1, 159, 16, BLUE);
    display.drawLine(c_pos , 20, c_pos , 127, BLUE);
    c_pos = (encoder + c_pos);
    encoder = 0;
    if (c_pos >= (num_steps - 1)) {
      c_pos = num_steps - 1 ; //Upper scan limit
    }
    if (c_pos <= 1) {
      c_pos = 1;    // Lower scan limit
    }
    p = c_pos;
    f1 =  ( Fstart_MHz  + ( c_pos *  Fstep_MHz ));
    display.setCursor(2, 6);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print((f1 / 1000000), 4);
    display.setCursor(112, 6);
    dbValue =  (dbPoint[p] / 10);
    display.print(dbValue, 1);
    display.print("db");
    drawSweep();
    drawGrid();
    display.drawLine(c_pos , 20, c_pos , 127, RED);
  }
}



//
// Draw the sweep waveform  divide by 50 gives approximate 10 db  per divisions in Sweep mode
//
void drawSweep() {
  int x;
  int x1 = 0;  int y;
  int y1 = 26 - ((dbPoint[0] - 150) / 10) ;
  for (int i = 0; i <= num_steps; i++) {
    y = 26 - ( (dbPoint[i] - 150) / 10) ;
    display.drawLine(x1, y1, i, y, YELLOW);
    x1 = i;
    y1 = y;
  }
}

//
// set output frequency to AD9850
// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
//
void sendFrequency(double frequency) {
  int32_t freq = frequency * 4295067295 / 125000000; // note 125 MHz clock on 9850.  You can make 'slight' tuning variations here by adjusting the clock frequency.
  for (int b = 0; b < 4; b++, freq >>= 8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}
// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte(byte data) {
  for (int i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

//
// display Scan screen with frequency range
//
void dispScanScreen() {
  // display start and stop frequencies on top of display
  //display.fillScreen(BLUE);
  display.setTextSize(1);
  display.setTextColor(YELLOW);
  display.setCursor(2, 6);
  display.print((Fstart_MHz / 1000000), 3);
  display.setCursor(112, 6);
  display.print((Fstop_MHz / 1000000), 3);
  drawGrid();
}

//
// Draw the scan grid  modify for each frequency band width
//
void drawGrid() {
  int xinc;
  display.setTextColor(CYAN);
  xinc = num_steps / 6; // scale for screen
  for (int i = 0; i <= 7; i++)
    display.drawLine((i * xinc) + 1, 20, (i * xinc) + 1, 125, CYAN);
  // these values give approximatly 20 db level markings
  for (int i = 1; i <= 6; i++)
    if (i == 2)
      display.drawLine(2,  (i * 21) - 1, num_steps - 2,  (i * 21) - 1, MAGENTA);  // 0 DB LINE
    else
      display.drawLine(2,  (i * 21) - 1, num_steps - 2,  (i * 21) - 1, CYAN);
}

//
// Power SupplyVoltage measure voltage and test for low volts
//display low voltage
//
void measuresupplyvolt () {
  // Read power supply voltage
  SupplyVoltage = analogRead(vin_BAT);
  SupplyVoltage = map(SupplyVoltage, 0, 1023, 0, (500 * (float R2 + float R1) / float R1)); // compute battery voltage using voltage divider values
  //SupplyVoltage = SupplyVoltage  + diode ;    if you have a reverse voltage diode add the voltage drop
  if (SupplyVoltage <= 500) return;       // not running on batteries !
  if   (SupplyVoltage  <= VoltSupplyMini) {
    //display.fillScreen(BLUE);
    display.setCursor(40, 10);
    display.print("LOW BATTERY !");
    display.setCursor(40, 10);
    delay (500);
    display.setTextColor(BLUE);
    display.print("LOW BATTERY !");
    display.setTextColor(YELLOW);
  }
}

//
// Splash screen  with battery voltage
//

void SplashScreen() {
  if (RunOnce == 1) {
    display.setTextColor(YELLOW);
    display.fillScreen(BLUE);
    display.setCursor(24, 35);
    display.setTextSize(3);
    display.println("KV4QB");
    display.setTextSize(2);
    display.setCursor(5, 70);
    display.print("  SNA Jr");
    display.setCursor(25, 7);
    display.setTextSize(1);
    display.setCursor(30, 100);
    display.print("Batt =");
    display.print((SupplyVoltage / 100));
    display.print("v");
    delay(2500);
    RunOnce = 0;
    display.fillScreen(BLUE);
    dispSettingScreen();
  }
}
//
//  Functions related to settings
//
//
// display settings screen
void dispSettingScreen() {
  display.setTextSize(1);
  if (sITEM == sSTART)                // active setting in  yellow
    display.setTextColor(YELLOW);     // inactiove settings in cyan
  else
    display.setTextColor(CYAN);
  display.setCursor(2, 4);
  display.print("Start Frequency Mhz.");
  display.setCursor(12, 16);
  display.setTextSize(2);
  display.print((Fstart_MHz / 1000000), 3);
  if (sITEM == sEND)
    display.setTextColor(YELLOW);
  else
    display.setTextColor(CYAN);
  display.setTextSize(1);
  display.setCursor(2, 36);
  display.print("Stop Frequency Mhz.");
  display.setTextSize(2);
  display.setCursor(12, 48);
  display.print((Fstop_MHz / 1000000), 3);
  display.setTextSize(1);
  if (sITEM == sSTEP)
    display.setTextColor(YELLOW);
  else
    display.setTextColor(CYAN);
  display.setCursor(2, 66);
  display.print("Step Khz.");
  display.setTextSize(2);
  display.setCursor(12, 76);
  display.print((VFOstep_Khz / 1000), 0);
  display.setTextSize(1);
  if (sITEM == sMODE)
    display.setTextColor(YELLOW);
  else
    display.setTextColor(CYAN);
  display.setCursor(2, 94);
  display.print("Mode");
  display.setTextSize(2);
  display.setCursor(12, 106);
  if (modality == 0)
    display.print("SWEEP.R");
  else  if (modality == 1)
    display.print("SWEEP.N");
  else  if (modality == 2)
    display.print("SWEEP.FAST");
  else if (modality == 3)
    display.print("SIG GEN");
  else if (modality == 4)
    display.print("DIP METER");
  else if (modality == 5)
    display.print("SWR SCAN");
  else if (modality == 6)
    display.print("WATT METER");
}

//
//   change settings functions
//

//   set start frequency
void setStartFreq() {
  dispSettingScreen();                      // mark other settings inactive
  display.setTextColor(YELLOW);
  display.setCursor(2, 16);
  display.setTextSize(2);
  while (1) {
    if (encoder != 0) {
      delay(10);
      Fstart_MHz = Fstart_MHz + (encoder * VFOstep_Khz);
      if (Fstart_MHz > 40000000) Fstart_MHz = 40000000;
      if (Fstart_MHz < 1000000) Fstart_MHz = 1000000;
      encoder = 0;
      display.fillRect(1, 16, 127, 16, BLUE);
      display.setCursor(12, 16);
      display.print((Fstart_MHz / 1000000), 3);
    }
    buttonstate = digitalRead(EncoderBtn);
    if (buttonstate == LOW)
      return;
  }
}

//   setEndfrequency
void setEndFreq() {
  dispSettingScreen();
  display.setTextColor(YELLOW);
  display.setTextSize(2);
  while (1) {                    // use encoder to set frequency   press button to exit
    if (encoder != 0) {
      delay(10);
      Fstop_MHz = Fstop_MHz + (encoder * VFOstep_Khz);
      if (Fstop_MHz > 40000000) Fstop_MHz = 40000000;
      if (Fstop_MHz < 1000000) Fstop_MHz = 1000000;
      encoder = 0;
      display.fillRect(1, 48, 127, 16, BLUE);
      display.setCursor(12, 48);
      display.print((Fstop_MHz / 1000000), 3);
    }
    buttonstate = digitalRead(EncoderBtn);
    if (buttonstate == LOW)
      return;
  }
}

//  set Step Size
void setStepSize() {
  dispSettingScreen();
  display.setTextColor(YELLOW);
  display.setTextSize(2);
  while (1) {                                    // use encoder to set frequency   press button to exit
    if (encoder != 0) {
      delay(10);
      VFOstep_Khz = VFOstep_Khz * 10;
      if (VFOstep_Khz > 1000000) VFOstep_Khz = 1000;
      encoder = 0;
      display.fillRect(1, 76, 127, 16, BLUE);
      display.setCursor(12, 76);
      display.print(( VFOstep_Khz / 1000), 0);
    }
    buttonstate = digitalRead(EncoderBtn);
    if (buttonstate == LOW)
      return;
  }
}

// Set Modality
void setModality() {
  dispSettingScreen();
  display.setTextColor(YELLOW);
  display.setTextSize(2);
  while (1) {
    if (encoder != 0) {
      modality++;
      if (modality  > 6)
        modality  = 0;
      encoder = 0;
      display.fillRect(1, 106, 160, 16, BLUE);
      display.setCursor(12, 106);
      if (modality == 0)
        display.print("SWEEP.R");
      else  if (modality == 1)
        display.print("SWEEP.N");
      else  if (modality == 2)
        display.print("SWEEP.FAST");
      else  if (modality == 3)
        display.print("SIG GEN");
      else  if (modality == 4)
        display.print("DIP METER");
      else  if (modality == 5)
        display.print("SWR SCAN");
      else
        display.print("WATT METER");
    }
    buttonstate = digitalRead(EncoderBtn);
    if (buttonstate == LOW)
      return;
  }
}

// VFO function with simple rf voltmeter

void dispVFOScreen() {
  current_freq_MHz = Fstart_MHz;
  display.setTextSize(1);
  display.setTextColor(YELLOW);     // inactiove settings in cyan
  display.setCursor(2, 8);
  display.print(" VFO Frequency Mhz.");
  display.setCursor(12, 20);
  display.setTextSize(2);
  display.print( (current_freq_MHz / 1000000), 6);
  display.setTextSize(1);
  display.setCursor(12, 40);
  display.print("Step Khz.");
  display.setTextSize(2);
  display.setCursor(12, 52);
  display.print((VFOstep_Khz / 1000), 3);
  display.setTextSize(1);
  display.setTextSize(1);
  display.setCursor(12, 74);
  display.print(" RF Level");
  display.setTextSize(2);
  display.setCursor(12, 86);
  // display.print(db, 1);
  display.fillRect(1, 102, 159, 16, YELLOW);
}


//   Sig Gen Power meter
void VFO_VM() {
  int last_db;
  float scale ;
  current_freq_MHz = Fstart_MHz;
  sendFrequency(current_freq_MHz);
  dispVFOScreen();
  while (1) {
    buttonstate = digitalRead(EncoderBtn);        // check for button press
    if (buttonstate == LOW) {
      delay(250);
      buttonstate = digitalRead(EncoderBtn);      // then check for long press and exit
      if (buttonstate == LOW) return;
      else {            // short press   change step size
        display.fillRect(1, 52, num_steps, 16, BLUE);
        VFOstep_Khz = VFOstep_Khz * 10;
        if (VFOstep_Khz > 1000000) VFOstep_Khz = 10;
        display.setCursor(12, 52);
        display.print((VFOstep_Khz / 1000), 3);
      }
    }
    if (encoder != 0) {             // look for frequency change from encoder
      delay(10);
      current_freq_MHz = current_freq_MHz + (encoder * VFOstep_Khz);
      if (current_freq_MHz > 40000000) current_freq_MHz = 40000000;
      if (current_freq_MHz < 1000000) current_freq_MHz = 1000000;
      encoder = 0;
      display.fillRect(1, 18, 127, 16, BLUE);
      display.setCursor(12, 18);
      display.print((current_freq_MHz / 1000000), 5); // display frequency
      display.print("0");                             // min step is 10 hz. so add a zero to display
      sendFrequency(current_freq_MHz);
    }
    read_DB();                                        // Power meter display
    if (int( powerdB ) != last_db) {                  // only update if changed from last reading
      display.setTextSize(2);
      display.fillRect(1, 86, 127, 16, BLUE);         // clear previous readings
      display.fillRect(1, 102, 127, 16, YELLOW);
      scale = abs((powerdB / 10) - 10) * .8;          // display bargraph
      display.fillRect(10, 104, 100 - int (scale), 12, BLACK);
      display.setCursor(12, 86);                      // display value in dBm
      display.print(powerdB / 10 , 1);
      display.print(" dB ");
      last_db = powerdB;
      delay(50);
    }
  }
}



//     Find dip frequency
void find_dip() {
  double dip_freq;
  double min_db;
  // Start and End frequency full range
  Fstart_MHz  =  1000000;
  Fstop_MHz   = 40000000;
  Fstep_MHz = 25000;       // stgep 50Khz
  min_db = 100;      //    inital value for peak detector
  display.fillScreen(BLUE);
  display.setTextSize(2);
  display.setTextColor(YELLOW);
  display.setCursor(12, 2);
  display.print("Finding Dip");
  display.setCursor(1, 8);

  for (int i = 0; i <= 1559; i++) {
    // Calculate frequency for each step in scan
    current_freq_MHz = Fstart_MHz + (i * Fstep_MHz);
    sendFrequency(current_freq_MHz);
    delay(1);  // Wait a little for settling
    read_DB();
   
    if (min_db > powerdB) {             // look for dip
      min_db = powerdB;
      dip_freq = current_freq_MHz;
    }
  }
  Fstart_MHz  =  dip_freq - 1000000;
  Fstop_MHz   = dip_freq + 1000000;
  display.fillScreen(BLUE);
  c_pos = 80;
  dispScanScreen();
  Perform_sweep();
  drawSweep();
}


//   Fast cpntinusous sweep for aliogiing filters
//
void Fast_Sweep() {
  //  int x;
  //  int x1 = 0;
  int y;
  int y1 ;
  // show screen for selected modality
  display.fillScreen(BLUE);
  dispScanScreen();
  // Find step size for sweep
  Fstep_MHz = (Fstop_MHz - Fstart_MHz) / num_steps;

  // Start scan loop
  buttonstate = digitalRead(EncoderBtn);
  while (buttonstate == HIGH) {
    for (int i = 0; i <= num_steps; i++) {
      // Calculate frequency for each step in scan
      current_freq_MHz = Fstart_MHz + (i * Fstep_MHz);
      sendFrequency(current_freq_MHz);
      delay(1);  // Wait a little for settling
      y = 26 - ((dbPoint[i] / 10) - 150 / 10) ;
      y1 = 26 - ((dbPoint[i + 1] / 10) - 150  / 10) ;
      display.drawLine(i, y, i + 1, y1, BLUE);
      read_DB();
      dbPoint[i] = powerdB + (FreqCorr * (current_freq_MHz / 100000)) ;
      y = 26 - ((dbPoint[i] / 10) - 150 / 10) ;
      y1 = 26 - ((dbPoint[i - 1] / 10) - 150 / 10) ;
      display.drawLine(i - 1, y1, i , y, YELLOW);
      // Check for button pressed during a sweep
      buttonstate = digitalRead(EncoderBtn);
      // if button pressed during sweep exit while loop
    }
    drawGrid();  // refresh grid at end of sweep
  }
  sITEM = sMODE;
  // scanning stopped redraw screen   wait for another short press to restart sweep or long press to exit
  dispScanScreen();
  drawSweep();
}

void read_DB() {
  int RF_in = 0;
  float power_caliberation = AD8307_POWER_CALIBERATION;
  // Read the forawrd power , repeat to help elminate noise and increase voltage for conversion
  RF_in = analogRead(LOG_AMP);
  RF_in = RF_in + analogRead(LOG_AMP);
  powerdB =  RF_in + (power_caliberation ); // convert logamp reading to dB * 10
  // if (abs(new_reading -  powerdB) > 2)                    // only update if reading has changed
  //  powerdB  = new_reading ;                              // value is power in dbm * 10
}

// Power meter display
// Wattmeter function
void WattMeter() {
  float scale ;
  double RFPower;
  double lastRFPower;
  double PeakRFPower = 0;
  float db40;
  display.fillScreen(RED);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 30);
  display.print(" Must use 40dB.");
  display.setCursor(30, 45);
  display.print(" TAP for proper");
  display.setCursor(30, 60);
  display.print("   Reading !");
  delay(2000);
  display.fillScreen(BLUE);

  while (1) {
    buttonstate = digitalRead(EncoderBtn);        // check for button press
    if (buttonstate == LOW) {
      delay(250);
      buttonstate = digitalRead(EncoderBtn);      // then check for long press and exit
      sITEM = sMODE;
      if (buttonstate == LOW) {
        sITEM = sMODE;
        return;
      }

    }
    read_DB();                        // check for RF
    if ( powerdB / 10 <= -20) {      //  No RF sp clear screen and flash Message
      if (PeakRFPower != 0)          //  if previous RF hold display for 2 seconds
        delay(2000);
      display.fillScreen(BLUE);
      display.setTextSize(3);
      display.setTextColor(WHITE);
      display.setCursor(20, 46);
      display.print("No RF !");
      delay(100);
      display.setTextColor(YELLOW);
      PeakRFPower = 0;                // Reset peak
    }
    else {                            // got RF so display values
      db40 = powerdB + 400;                               // add 40 dB to correct for Tap
      if (int( powerdB ) != last_db) {                    // only update if changed from last reading
        display.setTextSize(2);
        display.fillScreen(BLUE);            // clear previous readings
        display.setCursor(20, 8);                         // display value in dBm
        display.print(db40 / 10 , 1);
        display.setTextSize(1);
        display.print("  dBm. ");
        RFPower = pow(10, db40 / 100);
        if (RFPower > PeakRFPower)
          PeakRFPower = RFPower ;
        display.setCursor(8, 46);                        // display value in watts
        if (RFPower <= 2000) {
          display.setTextSize(2);
          display.print(RFPower  , 1);
          display.setTextSize(1);
          display.print("  mWatt ");
        }
        else {
          display.setTextSize(2);
          display.print(RFPower / 1000  , 1);
          display.setTextSize(1);
          display.print("  Watt");
        }

        display.setCursor(8, 86);
        if (PeakRFPower <= 2000) {
          display.setTextSize(2);
          display.print(PeakRFPower  , 1);
          display.setTextSize(1);
          display.print(" mWatt Peak");
        }
        else {
          display.setTextSize(2);
          display.print(PeakRFPower / 1000  , 1);
          display.setTextSize(1);
          display.print(" Watt Peak");
        }
        last_db = powerdB;
        delay(100);
      }
    }
  }
}

void SWRScan() {
  Perform_sweep();
  drawSWRGrid();
   display.setTextSize(1);
  display.setTextColor(YELLOW);
  display.setCursor(2, 6);
  display.print((Fstart_MHz / 1000000), 3);
  display.setCursor(112, 6);
  display.print((Fstop_MHz / 1000000), 3);
  drawSWRSweep();
  while (1) {
    scrollSWR();
    //  drawSWRSweep();
    buttonstate = digitalRead(EncoderBtn);        // check for button press
    if (buttonstate == LOW) {
      delay(350);
      buttonstate = digitalRead(EncoderBtn);      // then check for long press and exit
      if (buttonstate == LOW) {
        display.fillScreen(BLUE);
        sITEM = sMODE;
        return;
      }
    }
  }
}

void drawSWRSweep() {
  int x;
  int x1 = 0;  int y;
  int y1;
  x = abs(dbPoint[0] + 150) / 10;
  y1 = 149 - (SWRPoint[x] / 3 ) ;
  for (int i = 0; i <= num_steps; i++) {
    x = abs(dbPoint[i] + 150) / 10;
    y = 149 - (SWRPoint[x] / 3 ) ;
    display.drawLine(x1, y1, i, y, YELLOW);
    x1 = i;
    y1 = y;
  }
}

//scroll through sweep look up swr from table
void scrollSWR() {
  int x;
  float swr;
  if (encoder != 0) {
    //   clear old values from screen
    display.fillRect(1, 1, 159, 16, BLUE);
    display.drawLine(c_pos , 20, c_pos , 127, BLUE);
    c_pos = (encoder + c_pos);
    encoder = 0;
    if (c_pos >= (num_steps - 1)) {
      c_pos = num_steps - 1 ; //Upper scan limit
    }
    if (c_pos <= 1) {  // Lower scan limit
      c_pos = 1;
    }
    p = c_pos;
    f1 =  ( Fstart_MHz  + ( c_pos *  Fstep_MHz ));
    display.setCursor(2, 6);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print((f1 / 1000000), 3);
    display.setCursor(90, 6);
    display.print("SWR ");
    x = abs(dbPoint[p] + 150) / 10;
    swr = float (SWRPoint[x]);
    if (swr > 375)swr = 375;
    if (swr < 100 )swr = 100;
    display.print(swr / 100, 2);
    display.print(":1");
    drawSWRSweep();
    drawSWRGrid();
    display.drawLine(c_pos , 20, c_pos , 120, RED);
  }
}

void drawSWRGrid() {
  int xinc;
  display.setTextColor(CYAN);
  xinc = num_steps / 6; // scale for screen
  for (int i = 0; i <= 7; i++)
    display.drawLine((i * xinc) + 1, 21, (i * xinc) + 1, 117, CYAN);
  // these values give approximatly 1 SWR level markings
  for (int i = 0; i <= 4; i++)
    display.drawLine(2,  (i * 32) - 11, num_steps - 2,  (i * 32) - 11, CYAN);
}

