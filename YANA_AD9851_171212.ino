/*Print Date: 12/12/17
**************************************************

     *****      AD9851 50KHz to 70MHz Scalar Network Analyzer   *****
                 ** Yana: Yet Another Network Analyzer **

  Yana Program: Copyright K1TRB   Thomas Berger (C)2016
  with enhancements by Rudi Reuter DL5FA

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    The GNU General Public License is available at
    <http://www.gnu.org/licenses/>.

  Libraries used have separate copyrights.

  If you modify this program and distribute it, please send a full copy of this
  original program along with your modification. Be sure all modifications are
  well documented in the code.


  K1TRB 5-9-15, 6-17-15
  02-12-16 Switched to AD9851
  11-20-15 Constant min/max on graph (removed)
  11/30/15 Moved from sweep to PHSNA
  12/12/15 Switched to ILI9340 and ST7735
  03/11/16 All boxed up ready for final firmware
  03/25/16 Calibration, SNA in dbm, Power Meter added
  03/31/16 SWR working with simplified short calibration
  04/09/16 Assumed at testing
  02/08/17 Added raw output in PWR
  02/11/17 button interrupt driven
  2017-04-25 DL5FA, VNAJ remote control, DC_IN voltage display (color coded)
  2017-09-16 Corrected string length error in tftPrintCMenu().
  2017-11-16 Removed vnaJ code.
  2017-11-30 added L, C measurement
  2017-12-04 added graph transfer to PC
  2017-12-12 repaired fix_freqs()

  To use as a vfo, operate in the Center or Start Frq Menu
  on the first frequency line.

  ====>   User tweak values: <====

  CALIB_ATTEN30db 28.64 // ideal value for build attenuator
                        // use this value or your measured value
  TUNE_WORD 180000000L // ideal value for 6x crystal frequency
                        // measure output freq at 10MHz and adjust this value
  L_STD              1234                 // 1.2uH // standard L in nH
  C_STD              617                  // 680pF // standard C in pF
  CURSOR_STEP 100L       // cursor step size: you may like 10


  These constants are declared below.

**************************************************

  Operations:

  Turn:    Turn the knob
  Click:   Press the knob for a quick Click
  Push:    Push until the screen goes black
  Hold:    Hold until screen turns blue

  Actions:
  Turn: changes value in a field
        e.g. changes frequency

  Click: changes the increment position on a line
        e.g. changes frequency increment
        Pause scan

  Push:   advances to next field
  Hold:   Go Home
         From Home execute command
 **************************************************
  Wiring:

  UNO (same on Pro Mini and Nano)

  This is the table of jumpers used to wire
  Yana.

  ===> Wire runs:

  (Gnd)Brown and 5v(Red) go to the
  power rail on the breadboard. The rail
  is powered by the 5v regulator on the UNO.

  V+/- (Battery) can be 7-10v.
  V+(Red) and V-(Black) run separately to the
  UNO and the AD8307 module. The AD8307 should
  have its own regulator on board for noise
  reasons. The Display has an on board
  3.3v regulator. The on-board 3.3v regulator
  on the UNO is not used.

  Grouped wire runs (7-10v):
  Use separate wiring.
  Bat voltage: (red/black)
  UNO
  AD8307

  ==> 5v from UNO: (red)
  Use UNO 5v power rail: Vcc.
  UNO
  AD9851
  Display

  Signal Ground: (brown)
  Use UNO ground rail.
  UNO
  AD9851
  AD8307
  Rotary
  Display
  SD-card

  ===> Connections:
***Power:   (To DC plug on UNO, Vin on Nano)

  UNO   Bat Name
  Vin   V+ Bat+(7-10v)    Red
  Gnd   V- Bat-(7-10v)    Black

***Rotary Encoder/Button:
                                Jumper
  UNO   Rotary: Function:         Color:
  2     CLK      Rotary           Orange
  3     DT       Rotary           Yellow
  A0    BTN Button      White
  Gnd   Gnd      Rotary/Button    Brown

***AD8307 Detector:

  UNO   AD8307       Function
  A1    Output       Det out      White
  Gnd   Gnd (sig)    Det Gnd      Brown
  Vin   Vin          Bat volt     Red
  V-    Gnd (Vcc)    Bat Gnd      Black

***AD9851 DDS:

  UNO   DDS
  5v    Vcc   10 AD9851 D-side    Red
  Gnd   D2     7 AD9851 D-side    Brown
  Gnd          1 AD9851 D-side    Brown
  4     W-CLK 9 AD9851            Green
  5     FQ-UD 8 AD9851            Blue
  6     DATA   7 AD9851           White
  7     RESET 6 AD9851            Gray
  Xfrmr:DDS:
  Gnd   Gnd    5 AD9851           White
  -     Zout2 1 AD9851            Red

***Display SPI:

  --Board needs stepdown from 5v to 3.3v
  --for all inputs.
  --Divider instructions:
  --in from UNO --> 220ohm --> LCD in
  --                        |
  --                      470ohm
  --                        |
  --                       Gnd
  --Voltage divider computation:
  --5v x (470/(470+220)) = 3.4v
  --Vcc = 5V

  UNO      LCD    LCD     Name   Routine
         ST7735 ILI9341
  >>>.... step down through divider
  8   ---> SS     CS      CS     TFT_CS     Yellow
  9   ---> RST    RESET RESET    TFT_RST    Orange
  10 ---> A0      D/C     DC     TFT_DC     Purple
  11 ---> SDA     SDI     MOSI   TFT_SID    Blue
  12              SDO     MISO   *** not used
  13 ---> SCK     SCK    CLK    TFT_SCLK   Gray
  <<<.... end of step down
  5v ---> *** LED through 100ohm to Vcc (5v)
  GND ---> GND    GND    GND               Brown
  5v ---> Vcc     Vcc    Vcc (from UNO)    Red

  ***SD-Card: (On display)   !Currently not implemented!
  A3 --->         SD-CS      *** divider      White
  12 --->         SD-MISO                     Green
  11 --->         SD-MOSI    *** divider      Blue
  13 --->         SD-SCK     *** divider      Gray



  **************************************************
  Sources:

  K1TRB website:
  http://personal.colby.edu/personal/t/trberger/pages/hamradio.htm
  Use this LiquidCrystal (1602) (not this project):
  https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
  Graphics library for ST7735 128 x 160 (credit to Adafruit and others):
  https://www.arduino.cc/en/Reference/TFTLibrary
  https://www.arduino.cc/en/Reference/SPI
  DDS library:
  http://m0xpd.blogspot.co.uk/2014/03/dds-and-duedds-libraries.html
  https://github.com/m0xpd/DDS
  Rotary encoder with interrupts
  https://github.com/brianlow/Rotary
  Button:
  https://www.arduino.cc/en/Tutorial/Button
  Adjust power of AD9850, Change Filter:
  http://www.rudiswiki.de/wiki/AmateurRadioDDSgenerator
  Functionality:
  http://pages.suddenlink.net/wa5bdu/PHSNA_QQ_Spring_14.pdf
  https://groups.yahoo.com/neo/groups/PHSNA/info
  http://midnightdesignsolutions.com/nat/

  **************************************************
  Major Parts: (On eBay)    (2/12/16)

  http://www.ebay.com/itm/1MHZ-600MHZ-RF-Signal-Power-Detector-Logarithmic-
  Environmental-Field-Detection-/231629060443?hash=item35ee2af95b:g:HWAAAOSw37tV-rJV
  $11.95 AD8307
  http://www.ebay.com/itm/AD9851-DDS-Signal-Generator-Module-0-70MHz-2-Sine-Wave-
  and-2-Square-Wave-M-/172051686440?hash=item280f146428:g:Sa0AAOSwKtlWi5YO
  $12.49 + $1.98 = $14.47 AD9851
  http://www.ebay.com/itm/1-8-inch-1-8-TFT-LCD-Display-module-ST7735S-128x160-51-
  AVR-STM32-ARM-8-16-bit-/371462246346?hash=item567ce00fca:g:eT4AAOxy79JSZNBv
  $3.78 128x160 TFT
  http://www.ebay.com/itm/12mm-Rotary-Encoder-Push-Button-Switch-Keyswitch-
  Electronic-Components-Easy-Use-/171909824262?hash=item28069fbf06:g:B7UAAOSw~gRVk2YH
  $0.99 Rotary Encoder w/ button
  http://www.ebay.com/itm/Mini-USB-Nano-ATmega328P-5V-16MHz-Micro-controller-board-
  For-Arduino-ATMEGA128P-/291635663891?hash=item43e6d71413:g:FZIAAOSwfZ1WZ~mk
  $2.56 Arduino Nano clone
  http://www.ebay.com/itm/10pcs-3296W-103-3296-W-10K-ohm-Trim-Pot-Trimmer-
  Potentiometer-Square-Shape-/381100607740?hash=item58bb5dc8fc:g:PjYAAOSwg3FUmq7T
  $0.99 10K trimmer
  http://www.ebay.com/itm/IPX-IPEX-to-SMA-Female-Jack-U-FL-RF-Pigtail-Antenna-
  Jumper-Cable-25CM-/361036269737?hash=item540f7048a9:g:SBUAAOSwxH1UAUSM
  $0.99 sma jack with pigtail
  http://www.ebay.com/itm/Waterproof-100-x-68-x-50mm-Plastic-Electronic-Project-
  Box-Enclosure-Case-/161918708804?hash=item25b31b6c44:g:YiUAAOSwZ1lWb8DQ
  $2.15 Project Box 100x68x50 (mm)

  Total         $35.90

  Searching eBay.com may save a bit more.

  Other stuff
  http://www.ebay.com/itm/New-MB102-3-3V-5V-Power-Supply-Module-Set-Breadboard-
  Jumper-Cables-for-Arduino-/141856352405?hash=item21074c2895:g:E40AAOSwyQtVsfwq
  $4.03 + $0.34 Breadboard, power, jumpers

  Output circuit: (14 turns tapped 10 turns above ground)
  In ---||---|
       .1uF C
             C -------- Out
  DDS        C        BN43-2402
             C        14t:10t
             |
  --------------------- GND
*/

/**************************************************/
/*          The Program YANA                     */
/*          T. Berger  2016-03-15                 */
/**************************************************/

// a space is needed ahead of "#include" for it to work.
#include <SPI.h>               // SPI interface
#include <Rotary.h>            // Rotary Encoder
#include <DDS.h>               // works for AD9851
#include <TFT.h>               // TFT library ST7735
#include <EEPROM.h>            // eeprom routines

/********************************************/
/*   Screen Properties Defined Here         */
/********************************************/

//   The ST7735 values for a grid of 12x20 squares
//   landscape would be: 160 x 128 display.
//   portrait values are: 128 x 160 display
//   The first three definitions simplify switching to
//   another display.
#define  TFT_NAME Adafruit_ST7735
#define  TFT_FUNC Adafruit_ST7735
#define  TFT_START tft.initR(INITR_BLACKTAB)
#define  SCREEN_UPRIGHT 2L     // 0,2 Portrait 1,3 Landscape coded: 2
#define  X_GRID_STEP   12L     // grid pixels wide (for 10)
#define  X_GRID_END   128L     // screen pixels wide
#define  Y_GRID_STEP   20L     // grid pixels high (for 7)
#define  Y_GRID_END   140L     // graph pixels high
#define  X_START        7L     // (X_GRID_END-(10*X_GRID_STEP))-1: start x here
#define  X_STEPS      120L     // 10*X_GRID_STEP: pixel x steps in grid
#define  X_STEPSFL    120.0
#define  Y_START        0L
#define  Y_END        141L     // 7*Y_GRID_STEP+1: pixel y steps in grid

#define  BIG_PRT 2           // menu print size
#define  SMALL_PRT 1         // graph print size
// Color definitions for ST7735 and ILI9340
#define  Color_BLACK    ST7735_BLACK    //0x0000
//#define   Color_BLUE     ST7735_BLUE     //0x001F
#define  Color_RED      ST7735_RED      //0xF800
#define  Color_GREEN    ST7735_GREEN    //0x07E0
#define  Color_CYAN     ST7735_CYAN     //0x07FF
#define  Color_MAGENTA ST7735_MAGENTA   //0xF81F
#define  Color_YELLOW ST7735_YELLOW     //0xFFE0
#define  Color_WHITE    ST7735_WHITE    //0xFFFF

// Colors not in library for ST7735
// http://stackoverflow.com/questions/13720937/c-defined-16bit-high-color
#define  Color_ORANGE 0xFD20
#define  Color_BROWN   0x6260
#define  Color_GRAY    0xC618
#define  Color_BLUE    0x00FF
//very light gray C618
//light gray      A514
//mid gray        8410
//gray            4208

/********************************************/
/*   EEPROM Allocations                     */
/*                                          */
/*    Values are uint32_2 (two bytes)       */
/*    Order is low-byte, high-byte          */
/*                                          */
/*    10 Calibration frequencies are:       */
/*    .05,.1,1,10,20,30,40,50,60,70MHz      */
/*    Each freq gets a through reading.     */
/*    -30db reading at 1MHz                 */
/*    Power reading of -10dbm@10MHz         */
/*    Each freq gets a SWR reading.         */
/* Organization:                            */
/* 0 = CALIB_SNA = first freq              */
/*       ...                                */
/* 9 = CALIB_END_F = last freq              */
/* 10 = CALIB_30db = 30db adc value@1MHz    */
/* 11 = CALIB_PWR = adc: -10dbm@1MHz        */
/* 12 = CALIB_SWR                           */
/*       ...                                */
/* 21 = CALIB_END_SWR                       */
/********************************************/

// if CALIB_SNA is changed from 0, then the array index in rawADC,
// ereadINT, and ewriteINT must be changed in some places
// so just leave it be

#define CALIB_SNA 0      // start eeprom address for calibration values
#define CALIB_F_SIZE 10   // number of frequencies
#define CALIB_30db 10     // location where 30db value is stored
#define CALIB_F_30db 2    // 1MHz location in freqs[]
#define CALIB_ATTEN30db 28.57 // build value // Value of 30db attenuator
#define CALIB_PWR 11      // location for -10dbm adc value
#define CALIB_F_PWR 2     // 1MHz location in freqs[]
#define CALIB_SWR 12      // start of SWR table
#define CALIB_SIZE 22     // number of calibration values ([10]SNA+[1]30db+[1]PWR+[10]SWR)
#define NEXT_BASE CALIB_SNA+(2*CALIB_SIZE) // start of next block of eeprom

/********************************************/
/*   Various Constants                      */
/********************************************/

// for changing dbm to volts
#define DBM_VOLT 0.6505149978  // 0 volt offset for dbm to volts
                               // precision is much greater than accuracy

// Yana states
// The program is a finite state machine

#define STATE_INSTR 'I'   // State controls SNA/SWR/PWR/L/C
#define STATE_F_MODE 'F'  // State controls S/F C/R
#define STATE_G_MODE 'G'  // State controls Graph Dims
#define STATE_OP 'O'      // State controls RUN/CAL/LS/CS
#define STATE_F1 'B'      // State controls first frequency 
#define STATE_F2 'E'      // State controls second frequency
#define STATE_L 'L'       // State controls left mark
#define STATE_R 'R'       // State controls right mark
#define STATE_W 'W'       // State: working on running a machine

// Yana Op modes

#define OP_MAX   1   // largest operation number

// Yana Instrument Modes

#define MODE_INSTR_SNA        0   // SNA Grid
#define MODE_INSTR_SWR        1   // SWR Grid
#define MODE_INSTR_PWR        2   // PWR Values
#define MODE_INSTR_SNL        3   // Measure L
#define MODE_INSTR_SNC        4   // Measure C
#define INSTR_MAX             4   // largest instrument number

// Tuning mode display
#define TUNE_SNA   0
#define TUNE_SWR   1
#define TUNE_PWR   2
#define TUNE_L     3
#define TUNE_C     4

// Frequency Modes

#define MODE_FREQ_SF 0            // start/finish frequency menu
#define MODE_FREQ_CR 1            // center radius frequency menu
#define SC_MAX       1            // largest tuning type mode

// Graph Mode Ranges

#define MODE_GR_SMALL 0          // SMAll dimension
#define MODE_GR_BIG   1          // BIG dimension 
#define GR_MAX        1          // largest graphing dimension mode

// Operation Modes

#define MODE_OP_RUN 0            // run the selected instrument
#define MODE_OP_CAL 1            // Calibrate the instrument

// Rotary Encoder pins on UNO

#define ENCODER_B      2         // Encoder pin B
#define ENCODER_A      3         // Encoder pin A

// The AD9851 pins on the UNO.
// On AD9851 infor pins, Vcc=10, and GND=5
// selected so pins line up with Nano pins when modules are side by side

#define W_CLK        4    //on UNO     9 on module
#define FQ_UD        5    //           8
#define DDS_DATA     6    //           7
#define DDS_RESET    7    //           6

// The real 6xclock frequency is entered here
#define TUNE_WORD 180000000L // 180000000 ideal value

// TFT pins on UNO

#define TFT_CS     8    //Yellow
#define TFT_RST    9    //Orange
#define TFT_DC     10   //Purple
// The following pins are predefined in the driver
//#define TFT_MOSI 11   //Blue (11,12,13 coded in driver)
//#define TFT_MISO 12   //
//#define TFT_CLK    13 //Gray

// Button pin on UNO

#define ENCODER_BTN        A0                   // Encoder Button

// AD8307 Detector and supply Voltages into these ADC pins on UNO

#define VOLTAGE_IN         A1                   // Detector voltage
#define DC_AIN             A7                   // supply voltage
#define DC_TO_VOLT         102.4                // convert to Volt
#define L_STD              1234                 // standard L in nH
#define C_STD              617                  // standard C in pF

// Tuning Frequency Limits

#define F_MIN               50000L            // Lower frequency limit
#define F_MAX            70000000L            // Upper frequency limit
#define F_MID            35025000L            // mid range
#define F_INCR_MAX        1000000L            // max size increment in F change
#define PWR10_MAX        10000000L            // maximum power of 10 in frequency

// LC limits (nH, pF)
#define LC_MIN                   1L
#define LC_MAX            10000000L

//char Temp;
unsigned int intTemp;    // Loop variable for scanning

// Timing limits

#define PUSH_TIME               20           // Push button until screen goes blank
#define HOLD_TIME               2*PUSH_TIME  // Push button until the frequency menu appears
#define SWEEPDELAY_TIME         100          // 1.0 sec delay after each sweep
#define MAX_SWEEP_WAIT          2           // max 1.0 seconds to wait

// Big Print Lines

#define LINE_2B 2     // Control Mode Line
#define LINE_3B 3     // Line for Text for Frequency Center or Start
#define LINE_4B 4     // Line for Count for Frequency Center or Start
#define LINE_5B 5     // Line for Text for Text for Frequency Radius or Finish
#define LINE_6B 6     // Line for count for for Frequency Radius or Finish
#define LINE_7B 7     // Line for Text for Left
#define LINE_8B 8     // Line for count for Left
#define LINE_9B 9     // Line for Text for Right
#define LINE_10B 10   // Line for count for Right

// Command Line Field positions

#define LNPOS_INSTR 1 // instrument command field
#define LNPOS_FREQ 5  // frequency mode command field
#define LNPOS_GR 7    // graph span command field
#define LNPOS_OP 10   // operation command field

//   The graph is 140 pixels high
//   Graph is divided into 7 grids of 20 pixels each in height
//   Floating constants are needed to convert
//   swr per pixel and
//   db per pixel
//   on a 10db grid, 2.0 pixels per db would work
//   on a 3db grid, 20.0/3.0 = 6.66667 per db would work
//   On 1 swr unit per grid, 1 unit per 20.0 would work
//   On .5 swr per grid, 1 unit per 40.0 would work

#define DB_BIG_SNA 2.0        // (7 x 10) 10 in modes_gr
#define DB_SMALL_SNA 6.66667  // (7 x 3) 3 in modes_gr
#define UNIT_BIG_SWR 20.0     // (7 x 2) 2 in modes_gr
#define UNIT_SMALL_SWR 40.0   //4.0     // (7 x 0.5) .5 in modes_gr

//    power of 10 for cursor stepping

#define CURSOR_STEP 100L       // cursor step size

// string length (for conversions)

#define STRLEN 16  // Maximum string length for conversion


/********************************************/
/* Hardware Declarations                    */
/********************************************/

// Instantiate TFT

TFT_NAME tft = TFT_FUNC(TFT_CS, TFT_DC, TFT_RST);

// Instantiate AD9851 and set addresses
//    The Library must support the AD9851 in 6xF mode

DDS dds(W_CLK, FQ_UD, DDS_DATA, DDS_RESET);

// Instantiate the Rotary encoder and set pins
Rotary r = Rotary(ENCODER_A, ENCODER_B);

/********************************************/
/*           Variable Declarations          */
/********************************************/
/* Sweep Values Data Type                   */
/********************************************/

struct sweep_type {
  long f;         // current dds setting
  long f1;        // Frequency Center or Start
  long f2;        // Frequency radius or finish
  long ml;        // Left mark
  long mr;        // Right mark
  long ic;        // Increment step size
};

/********************************************/
/* Initial sweep parameter values:          */
/* {f,f1,f2,ml,mr,ic,ir}                    */
/* Set for HF                               */
/********************************************/

sweep_type sweep = {  1000000L,   // current frequency
                      1000000L,   // center/start frequency
                     31000000L,   // radius/finish frequency
                      1800000L,   // left mark
                     29700000L,   // right mark
                      1000000L,   // Increment step size
                   };

// Temporary save parameters
long tmpf, tmpic, tmpf1, tmpf2, tmprcnt, tmprinc, tmprcntmx, tmprcntmn;

// Flags

// boolean tuning             // == true when in graph tuning mode 
// boolean turned_t           //   flag: Turn (declared below)
// boolean down_b             //   flag: Button Down (declared below)
// boolean down_c                 //   flag: Click (not needed)
boolean down_p = false;           //   flag: Push
boolean down_h = false;           //   flag: Hold
boolean usb_on = false;           //   flag: true when usb is on

// Field Commands

// Home screen states
//   yana_state   STATE_INSTR:  Set instrument
//   yana_state   STATE_F_MODE: Set freq mode (start/finish or center/radius)
//   yana_state   STATE_G_MODE: Set graphing range (graph y-axis scale/division)
//   yana_state   STATE_OP:     Set operation (run, cal, usb)
//   yana_state   STATE_F1:     Set center or start (begin) frequency
//   yana_state   STATE_F2:     Set span radius or finish (end) frequency
//   yana_state   STATE_L:      Set left freq
//   yana_state   STATE_R:      Set right freq
// State not on home screen
//   yana_state   STATE_W:      Working: SNA, SWR, PWR, Remote Control
char yana_state;                                // current yana_state
char modes_instr[5][4] = {"SNA", "SWR", "PWR", "SNL", "SNC"}; // Yana instruments
char modes_freq[2][2] = {"S", "C"};             //start/finish or center/radius
char modes_gr[3][2][3] = {{" 3", "10"}, {".5", " 1"}, {"__", "__"}}; //scales for graphs
char modes_op[2][2] = {"R", "C"};     // operations: run, calibrate
// Yana instrument operations: SNA, SWR, PWR
// startup state is assigned here
int8_t mode_instr    = MODE_INSTR_SNA;          // SNA;
int8_t mode_instr_1  = MODE_INSTR_SNA;          // field marker
int8_t mode_freq     = MODE_FREQ_SF;            // frequency setting modes
int8_t mode_freq_now = MODE_FREQ_SF;            // current mode (to detect changes) 
int8_t mode_gr       = MODE_GR_BIG;             // graph range
int8_t mode_op       = MODE_OP_RUN;             // operations: run, cal, set std, usb
int8_t mode_tune     = TUNE_SNA;                // L, C, SNA separator
long l_std          = L_STD;                    // nH
long c_std          = C_STD;                    // pF
// The calibrations frequencies
long freqs[CALIB_F_SIZE] =
{ 50000L, 100000L, 1000000L, 10000000L, 20000000L, 30000000L,
  40000000L, 50000000L, 60000000L, 70000000L
};                                        // .05,.1,1,10,20,30,40,50,60,70MHz
int calibrations[CALIB_SIZE];             // calibration values
float perdb;                              // adc units per db
float orig0dbmfl;                         // adc relative +0dbm value
float orig0rlfl;                          // adc on bridge: RL=0 value
char unitsw[5][3] = {" W", "mW", "uW", "nW", "pW"}; // units for watts
char unitsv[3][3] = {" V", "mV", "uV"};  // units for volts
char outstr[STRLEN];                     // buffer for float to string conversion
long maxfr, minfr;                       // temporary variable


/***********************************************************************/
/* Interrupt set values from rotary encoder and button                 */
/*                                                                     */
/* Variables used both inside and outside an interrupt service routine */
/* must be declared volatile                                           */
/***********************************************************************/

volatile boolean turned_t;             // flag: Knob turned
volatile long rotary_count;            // frequency
volatile long rotary_increment;        // frequency increment
volatile long rotary_count_min;        // min frequency
volatile long rotary_count_max;        // max frequency

volatile boolean down_b;               // button was pushed

/**************************************************************/
/* rotary encoder and button functions                        */
/*                                                            */
/* ISR(PCINT2_vect)                      rotary interrupt     */
/* set_rotary_count(short dir)           rotary counter       */
/* set_rotary_params(rc,rs,rmx,rmn)      rotary parameters    */
/* display_count(line,loc,dig,crs,cnt) display rotary count   */
/* get_button()                          read a button press  */
/* ISR(PCINT1_vect)           button interrupt                */
/* down_b                   interrupt when button is pushed   */
/**************************************************************/

// **** Begin interrupt service routines ****

/****************************************/
/* Interrupt service                    */
/*                                      */
/* dir = 1     Increment                */
/* dir = -1    Decrement                */
/* turned_t = true: knob was turned          */
/****************************************/
/*                                      */
/* Interrupt service routine for        */
/* encoder knob turn                    */
/*                                      */
/* ISR(PCINT2_vect)                     */
/*                                      */
/* returns +1 for CW & -1 for CCW       */
/* If it's not this way, swap the       */
/* A,B connections to the rotary        */
/****************************************/

ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result == DIR_CW)
    set_rotary_count(1);
  else if (result == DIR_CCW)
    set_rotary_count(-1);
}

/****************************************/
/* set_rotary_count(short dir)          */
/*                                      */
/* AD9851 not changed here              */
/* if turned, set t=1 and increment     */
/* do not change if at limit            */
/* parameters:                          */
/* dir = direction +1=CW & -1=CCW       */
/*                                      */
/* variables changed by this routine:   */
/*     rotary_count, turned_t           */
/****************************************/

void set_rotary_count(short dir)
{
  if (rotary_increment != 0)
  {
    if ((dir == 1) && (rotary_count + rotary_increment <= rotary_count_max))
    {
      rotary_count += rotary_increment;
    }
    else if ((dir == -1) && (rotary_count >= rotary_increment + rotary_count_min))
    {
      rotary_count -= rotary_increment;
    }
    turned_t = true;
  }
}

/***************************************/
/* Interrupt service routine for       */
/* button                              */
/*                                     */
/* Only the down_b flag is set         */
/***************************************/

ISR(PCINT1_vect)
{
  boolean button = digitalRead(ENCODER_BTN);
  if (!button)
  {
    down_b = true; // button is down
  }
}

// **** End interrupt service routines ****

/***********************************/
/*    Available SRAM               */
/*     For Testing xxx             */
/***********************************/
/* === Commented Out ===

  int freeRam ()
  {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  }

  void showFree()
  {
     display_count(2,1,4,SMALL_PRT,1000000000L,freeRam());
  }
  //              showFree(); // testing ram left

  // test: used for inserting stubs

  int zzz,a1,a2,a3;
  float n1,n2,n3;

  zzz=10;
       display_count(zzz,1,8,SMALL_PRT,sweep.ic,a1); zzz++;
       display_count(zzz,1,8,SMALL_PRT,sweep.ic,a2); zzz++;
       display_count(zzz,1,8,SMALL_PRT,sweep.ic,a3); zzz++;
       display_db(zzz,12,SMALL_PRT,(int)mcurs);
        while(!get_button()){}
        doPrint(cntln,18,SMALL_PRT,Color_WHITE,"Tune");
        while(!get_button()){}

   zzz === test ... to see values ===
         int zzz;
       zzz=10;
          display_count(zzz,1,8,SMALL_PRT,sweep.ic,(int)(10000.0*perdb)); // zzz++;
          doPrint(zzz,17,SMALL_PRT,Color_WHITE,"perdb"); zzz++;
          display_count(zzz,1,8,SMALL_PRT,sweep.ic,(int)(10000.0*orig0dbmfl));// zzz+
    +;
          doPrint(zzz,18,SMALL_PRT,Color_WHITE,"pwr0");// zzz++;
    //         display_count(zzz,1,8,SMALL_PRT,sweep.ic,a3); zzz++;
    //         display_db(zzz,12,SMALL_PRT,(int)mcurs);
    //         while(!get_button()){}
           while(!get_button()){}
        zzz === end test ... to see values ===        
        
   === End Commented Out === */

/*************************************/
/* set_rotary_params(rc,rs,rmn,rmx)  */
/*                                   */
/* Set the rotary parameters         */
/* parameters:                       */
/* rc  = rotary_count                */
/* rs  = rotary_increment            */
/* rmx = rotary_count_max            */
/* rmn = rotary_count_min            */
/*************************************/

void set_rotary_params( long rc, long rs, long rmx, long rmn)
{
  noInterrupts();     // an interrupt here would be nasty
  rotary_count        = rc;
  rotary_increment    = rs;
  rotary_count_max    = rmx;
  rotary_count_min    = rmn;
  turned_t           = false;
  interrupts();
}

/***********************************/
/* Compute the power 10^E          */
/***********************************/

long ten_to_the(uint8_t E)
{
  long N = 1;

  while (E > 0)
  {
    N = N * 10;
    E--;
  }
  return (N);
}

/***********************************/
/* Compute the power of 10 in N    */
/***********************************/

long pwr10in( long N)
{
  long pwr = PWR10_MAX;

  if (N == 0)
  {
    return (0);
  }
  else
  {
    while (N / pwr == 0)
    {
      pwr = pwr / 10;
    }
  }
  return (pwr);
}


/*************************************/
/* Compute the number of digits in N */
/*************************************/

uint8_t digitsInN( long N)
{
  uint8_t dig = 0;

  while (N > 0)
  {
    N = N / 10;
    dig++;
  }
  return (dig);
}

/**********************************************************/
/* float_fix(float int int)                               */
/*    change floating point to string                     */
/*    val floating point value                            */
/*    len length of final string including decimal point  */
/*    dacc digits after decimal                           */
/*    final string length is len-dacc                     */
/**********************************************************/

void float_fix(float val, int len, int dacc)
{
  int tmp;

  dtostrf(val, len, dacc, outstr);
  while (outstr[0] == ' ')
  {
    for (tmp = 0; tmp < 15; tmp++)
    {
      outstr[tmp] = outstr[tmp + 1];
    }
  }

  len = len - dacc;
  outstr[len] = 0;
}

/*******************************************/
/*    Screen Writing Routines              */
/*                                         */
/* doPrint(line,loc,siz,clr,*txt )         */
/* display_count(line,loc,dig,siz,crs,cnt) */
/* CurrentField(line,loc,siz,txt)          */
/* tftPrintCMenu()                         */
/* tftPrintFMenu()                         */
/* NextFMenuLine(menu,line,loc)            */
/* tftPrintGrid()                          */
/* tftLabelGrid()                          */
/*******************************************/

/************************************/
/* doPrint(line,loc,siz,clr,*txt ) */
/*                                  */
/* Print text at a location         */
/* parameters:                      */
/* line=line on screen              */
/* loc= position in line            */
/* siz= character size              */
/* clr= text color                  */
/* txt= text to print               */
/* Black background                 */
/************************************/

void doPrint(uint8_t line, uint8_t loc, uint8_t siz, uint16_t clr, const char *txt )
{
  tft.setTextSize(siz);
  tft.setTextColor(clr, Color_BLACK); // text and background
  tft.setCursor(siz * 6 * (loc - 1), siz * 8 * (line - 1));
  tft.print(txt);
}

/*******************************************/
/* display_count(line,loc,dig,siz,crs,cnt) */
/*                                         */
/* Displays the counter                    */
/*                                         */
/*    chars are in a 6x8 block             */
/*      128w x 160 h (screen)              */
/* Character counts on the screen:         */
/*    Landscape                            */
/*      size 1 2 3 4 5 7 8 14 17           */
/*      wide 26 13 9 6 5 4 3 2 1           */
/*      high 16 8 5 4 3 2 2 1 1            */
/*    Portrait                             */
/*      size 1 2 3 4 5 7 8 14 17           */
/*      wide 20 10                         */
/*      high 20 10                         */
/* If crs > dig then no cursor is shown.   */
/* parameters:                             */
/*    line: line of display (1 to height)  */
/*    loc: start location: 0 to width-1    */
/*    dig: size of field in digits         */
/*      frequencies: 8                     */
/*                                         */
/*    siz: size of characters (1 or 2)     */
/*    incr: cursor power of 10             */
/*    cnt: The number to display           */
/*******************************************/

void display_count(uint8_t line, uint8_t loc, uint8_t dig, uint8_t siz,   long
                   incr, long cnt)
{
  uint8_t crs;
  long f, d = 1; // 8 digits
  char n = ' '; // Leading zero blanking: changes to '0'

  crs = digitsInN(incr); // cursor position

  tft.setTextSize(siz);
  tft.setTextColor(Color_YELLOW, Color_BLACK); // text and background
  tft.setCursor(siz * 6 * (loc - 1), siz * 8 * (line - 1));

  // compute d: the power of ten in the field
  d = ten_to_the(dig - 1);

  // Pick out each digit and print it
  while (d >= 1)
  {
    f = (cnt % (10 * d)) / d; // current digit
    if (dig == crs)
    {
      tft.setTextColor(Color_GREEN, Color_BLUE); // Set cursor
    }
    if (f > 0)
    {
      tft.print(f);
      n = '0';      // done with leading zeros
    }
    else
    {
      if (d == 1 && cnt == 0) // if cnt==0 all spaces leading
      {
        n = '0';
      }
      tft.print(n); //leading = space, after = 0
    }
    if (dig == crs)
    {
      tft.setTextColor(Color_YELLOW, Color_BLACK); // unset cursor
    }
    if ((d == 1000) || (d == 1000000)) // comma insertion
    {
      if (n == '0')
        tft.print(',');
      else
        tft.print(' '); // no comma, still leading
    }
    dig--;
    d = d / 10;
  }
}

/*************************************/
/*    display db                     */
/*************************************/

void display_db(uint8_t line, uint8_t loc, uint8_t siz, int cnt)
{
  int f, d = 100; // 8 digits
  char n = ' '; // Leading zero blanking: changes to '0'

  tft.setTextSize(siz);
  tft.setTextColor(Color_YELLOW, Color_BLACK); // text and background
  tft.setCursor(siz * 6 * (loc - 1), siz * 8 * (line - 1));

  if (cnt < 0)
  {
    tft.print('-');
    cnt = -cnt;
  }
  else
  {
    tft.print(' ');
  }
  while (d >= 1)
  {
    f = (cnt % (10 * d)) / d; // current digit
    if (f > 0)
    {
      tft.print(f);
      n = '0';      // done with leading zeros
    }
    else
    {
      if (d == 1 && n == ' ') // if cnt==0 all spaces leading
      {
        n = '0';
      }
      tft.print(n);   //leading = space, after = 0
    }
    if (d == 10) // decimal insertion
    {
      tft.print('.');
    }
    d = d / 10;
  }
}

/************************************/
/* CurrentField(line,loc,siz,txt)   */
/*                                  */
/* MARK the current field           */
/*                                  */
/* line: line of field name         */
/* loc: start location on line      */
/* siz: character size (1,2)        */
/* txt: First letter field name     */
/************************************/

void CurrentField(uint8_t line, uint8_t loc, uint8_t siz, char txt)
{
  tft.setTextSize(siz);
  tft.setTextColor(Color_RED, Color_GREEN); // text and background
  tft.setCursor(siz * 6 * (loc - 1), siz * 8 * (line - 1));
  tft.print(txt);
}

/***************************************/
/* Display supply voltage, color coded */
/***************************************/

void dsp_supp_volt()
{
  unsigned int dc_in = 0;
  float dc_volt;
  uint16_t voltColor = Color_GREEN;

  dc_in = dc_in + analogRead(DC_AIN);
  dc_volt = dc_in / DC_TO_VOLT;
  float_fix(dc_volt, 6, 2);
  if (dc_volt < 7.5) voltColor = Color_YELLOW; // less than 20% capacity
  if (dc_volt < 7.2) voltColor = Color_RED;    // less than  5% capacity
  doPrint(1, 15, SMALL_PRT, voltColor, outstr);
  doPrint(1, 19, SMALL_PRT, voltColor, "V");
}

/************************************/
/*      Print Command menu          */
/*                                  */
/* tftPrintCMenu()                  */
/************************************/

void tftPrintCMenu()
{
  int i;
  char strHeader[13] = "*Yana K1TRB ";  // don't forget null at end
  doPrint(1, 3, SMALL_PRT, Color_BLUE, strHeader);
  dsp_supp_volt();
  doPrint(LINE_2B, 1, BIG_PRT, Color_ORANGE, modes_instr[mode_instr_1]);
  doPrint(LINE_2B, 5, BIG_PRT, Color_ORANGE, modes_freq[mode_freq]);
  doPrint(LINE_2B, 7, BIG_PRT, Color_ORANGE, modes_gr[mode_instr][mode_gr]);
  doPrint(LINE_2B, 10, BIG_PRT, Color_ORANGE, modes_op[mode_op]);

}

/*************************************/
/*                                   */
/* print frequencies on home scrreen */
/*                                   */
/*************************************/

void tftPrintFFreqs()
{
  doPrint(LINE_3B, 8, BIG_PRT, Color_WHITE, "F");
  doPrint(LINE_5B, 8, BIG_PRT, Color_WHITE, "F");
  doPrint(LINE_7B, 8, BIG_PRT, Color_WHITE, "F");
  doPrint(LINE_9B, 8, BIG_PRT, Color_WHITE, "F");
  display_count(LINE_4B, 1, 8, BIG_PRT, sweep.ic, sweep.f1);
  display_count(LINE_6B, 1, 8, BIG_PRT, sweep.ic, sweep.f2);
  display_count(LINE_8B, 1, 8, BIG_PRT, sweep.ic, sweep.ml);
  display_count(LINE_10B, 1, 8, BIG_PRT, sweep.ic, sweep.mr);

}
/************************************/
/* Print sweep frequency menu       */
/*                                  */
/* tftPrintFMenu()                  */
/************************************/

void tftPrintFMenu()
{
  if (mode_freq == MODE_FREQ_CR)
  {
    doPrint(LINE_3B, 1, BIG_PRT, Color_WHITE, "Center   "); // Center freq
    doPrint(LINE_5B, 1, BIG_PRT, Color_WHITE, "Radius   "); // Radius freq
  }
  else
  {
    doPrint(LINE_3B, 1, BIG_PRT, Color_WHITE, "Start    "); // Start freq
    doPrint(LINE_5B, 1, BIG_PRT, Color_WHITE, "Finish   "); // Finish freq
  }
  doPrint(LINE_3B, 8, BIG_PRT, Color_WHITE, "Frq");    // Center freq
  doPrint(LINE_5B, 8, BIG_PRT, Color_WHITE, "Frq");    // Radius freq
  doPrint(LINE_7B, 1, BIG_PRT, Color_WHITE, "Left   Frq");  // Left Mark freq
  doPrint(LINE_9B, 1, BIG_PRT, Color_WHITE, "Right  Frq");  // Right Mark freq

  tftPrintFFreqs();
}

/************************************/
/* Next Menu Line                   */
/*                                  */
/* NextFMenuLine(menu,line,loc)     */
/*                                  */
/*   Print frequency menu (i.e.     */
/*   no cursor) then add cursor to  */
/*   current field                  */
/*   menu is the leading character  */
/*   in the cursor field            */
/*   line is the line of this item  */
/************************************/

void NextFMenuLine(char menu, uint8_t line)
{
  yana_state = menu;
  tftPrintFFreqs();
  CurrentField(line, 8, BIG_PRT, 'F');
  tftPrintCMenu();
}

/************************************/
/* Print a grid on the screen       */
/*                                  */
/* tftPrintGrid()                   */
/************************************/

void tftPrintGrid()
{
  tft.fillScreen(Color_BLACK);

  for (uint16_t y = Y_START; y < Y_END; y += Y_GRID_STEP)
  {
    tft.drawFastHLine(X_START, y, Y_GRID_END, Color_GRAY);
  }
  for (uint16_t x = X_START; x < X_GRID_END; x += X_GRID_STEP)
  {
    tft.drawFastVLine(x, Y_START, Y_END, Color_GRAY);
  }
  tft.drawFastVLine((X_START + X_GRID_END) / 2, Y_START, Y_END, Color_RED);
}


/************************************/
/* Label the Grid                   */
/*                                  */
/* tftLabelGrid()                   */
/*                                  */
/* gt is grid type                  */
/************************************/

void tftLabelGrid()
{

  if (mode_instr == MODE_INSTR_SNA)
  {
    doPrint(20, 18, SMALL_PRT, Color_WHITE, "SNA");
    if (mode_gr == MODE_GR_BIG)
    {
      doPrint(1, 1, SMALL_PRT, Color_YELLOW, "0");
      doPrint(5, 1, SMALL_PRT, Color_YELLOW, "20");
      doPrint(10, 1, SMALL_PRT, Color_YELLOW, "40");
      doPrint(15, 1, SMALL_PRT, Color_YELLOW, "60");
    }
    else
    {
      doPrint(1, 1, SMALL_PRT, Color_YELLOW, "0");
      doPrint(5, 1, SMALL_PRT, Color_YELLOW, "6");
      doPrint(10, 1, SMALL_PRT, Color_YELLOW, "12");
      doPrint(15, 1, SMALL_PRT, Color_YELLOW, "18");
    }
  }
  else if (mode_instr == MODE_INSTR_SWR)
  {
    doPrint(20, 18, SMALL_PRT, Color_WHITE, "SWR");
    if (mode_gr == MODE_GR_BIG)
    {
      doPrint(1, 1, SMALL_PRT, Color_YELLOW, "1");
      doPrint(5, 1, SMALL_PRT, Color_YELLOW, "3");
      doPrint(10, 1, SMALL_PRT, Color_YELLOW, "5");
      doPrint(15, 1, SMALL_PRT, Color_YELLOW, "7");
    }
    else
    {
      doPrint(1, 1, SMALL_PRT, Color_YELLOW, "1");
      doPrint(5, 1, SMALL_PRT, Color_YELLOW, "2");
      doPrint(10, 1, SMALL_PRT, Color_YELLOW, "3");
      doPrint(15, 1, SMALL_PRT, Color_YELLOW, "4");
    }
  }
  else // if(mode_op==MODE_INSTR_PWR)
  {
    doPrint(20, 18, SMALL_PRT, Color_WHITE, "PWR");
    doPrint(LINE_2B, 4, BIG_PRT, Color_WHITE, "Power");   // dbm Power
    doPrint(LINE_8B, 1, BIG_PRT, Color_WHITE, "Frequency"); // DDS Freq
  }
}

/********************************************/
/* incr_crsr Move the cursor along the line */
/********************************************/

void incr_crsr()
{
  sweep.ic = sweep.ic / CURSOR_STEP;
  if (sweep.ic < 1) // loop increment
  {
    sweep.ic = F_INCR_MAX;
  }
}


/********************************************/
/* Read the button with debouncing          */
/*                                          */
/* get_button()                             */
/*                                          */
/* Quick click just records and exits       */
/* Screen clears on Push                    */
/* Cleared screen changed to menu on Hold   */
/* return value 1: button depressed         */
/*                                          */
/* click: return 1, p=false,h=false         */
/* push: return 1, p=true, h=false          */
/* hold: return 1, p=false, h=true          */
/* down_h==true: it's a hold                */
/********************************************/

boolean get_button()
{
  short short_p = 0;

  down_p = false;             // ready for push or hold
  down_h = false;
  if (!digitalRead(ENCODER_BTN)) // ! means it's pressed
  {
    delay(20);
    if (!digitalRead(ENCODER_BTN)) // It's down: keep checking
    {
      short_p = 0;
      // Wait for Push
      while ((!digitalRead(ENCODER_BTN)) && (short_p < PUSH_TIME))
      {
        short_p++;
        delay(20);
      };
      // Process Push or hold
      if (short_p >= PUSH_TIME)
      {
        tft.fillScreen(Color_BLACK);   // signal a Push
        down_p = true;                  // Push flag set
        short_p = 0;

        while ((!digitalRead(ENCODER_BTN)) &&    (short_p < HOLD_TIME))
        {
          short_p++;
          delay(20);
        }
        // Process Hold
        if (short_p >= HOLD_TIME)
        {
          tft.fillScreen(Color_BLUE);    //   signal a Hold
          down_h = true;                // Hold    flag
          down_p = false;                // it's    not a Push
          while (!digitalRead(ENCODER_BTN))
          {
            delay(1);
          }; // wait for button release
          delay(1);
        }
      }
      return 1;                          // the button was depressed
    } // down < 20 doesn't count
  }                                      // don't count just a bounce
  return 0;                              // the button was not depressed
}

/**********************************************************/
/* Set sweep parameters in bounds                         */
/*                                                        */
/*   void fix_freqs()                                     */
/*                                                        */
/* switching true==switch center/radius <-> start/finish  */
/**********************************************************/

void fix_freqs(boolean switching)
{
  long mxfr, mnfr;

  // start/finish mode
  if (mode_freq == MODE_FREQ_SF) // in start finish mode
  {
    if (switching)
    {
      // coming from center/radius
      // keep start finish related to center/radius
      mxfr = (sweep.f1 - sweep.f2); // new start
      sweep.f2 = sweep.f1 + sweep.f2; // new finish
      sweep.ml = sweep.f1 - sweep.ml; // new mark positions
      sweep.mr = sweep.f1 + sweep.mr;
      sweep.f1 = mxfr;          // set new start
    }

    // Keep frequencies between F_MIN and F_MAX
    if ((sweep.f1 < F_MIN) || (sweep.f1 > F_MAX))
    {
      sweep.f1 = F_MIN;
    }
    if ((sweep.f2 < sweep.f1) || (sweep.f2 > F_MAX))
    {
      sweep.f2 = F_MAX;
    }
    sweep.f = sweep.f1;

    // check the marks
    if ((sweep.ml < sweep.f1) || (sweep.ml > sweep.f2))
    {
      sweep.ml = sweep.f1; // set it at the beginning
    }
    if ((sweep.mr < sweep.ml) || (sweep.mr > sweep.f2))
    {
      sweep.mr = sweep.f2; // set it at the end
    }
  } // end start/finish mode

  // center/radius mode
  else // mode_freq == MODE_FREQ_CR
  {
    if (switching)
    {
      // coming from start/finish
      // keep center/radius related to start/finish
      mxfr = (sweep.f2 - sweep.f1) / 2L; // radius
      sweep.f1 = (sweep.f2 + sweep.f1) / 2L; // center
      sweep.f2 = mxfr;
      if (sweep.ml <= sweep.f1)
      {
        sweep.ml = sweep.f1 - sweep.ml; // former position
      }
      else
      {
        sweep.ml = sweep.f1 - mxfr; // set at edge
      }
      if (sweep.mr >= sweep.f1)
      {
        sweep.mr = sweep.mr - sweep.f1; // former position
      }
      else
      {
        sweep.mr = sweep.f1 + mxfr; // set at edge
      }
    }

    if (sweep.f1 > F_MAX)
    {
      sweep.f1 = F_MAX;
    }
    else if (sweep.f1 < F_MIN)
    {
      sweep.f1 = F_MIN;
    }

    mxfr = F_MAX - sweep.f1;      // determine max radius
    mnfr = sweep.f1 - F_MIN;
    if (mxfr > mnfr)
    {
      mxfr = mnfr;
    }
    // mxfr is now max radius

    // make sure radius less than mxfr
    if (sweep.f2 > mxfr)
    {
      sweep.f2 = mxfr;
    }
    sweep.f = sweep.f1;

    // check marks
    if (sweep.ml > sweep.f2)
    {
      sweep.ml = sweep.f2; // set at end
    }
    if (sweep.mr > sweep.f2)
    {
      sweep.mr = sweep.f2; // set at end
    }
  }
  mode_freq_now = mode_freq; // no longer need to worry about change of mode
}

/**********************************************************/
/* Setup for menu entries                                 */
/*                                                        */
/* void go_home()                                         */
/* void set_for_f1()                                      */
/**********************************************************/
/* Set for frequency entry (yana_state STATE_F1)          */
/**********************************************************/

void set_for_f1()
{
  set_rotary_params(sweep.f1, sweep.ic, F_MAX, F_MIN);
  sweep.f = sweep.f1;           // tuned freq = top line freq
  dds.setFrequency(sweep.f); // frequency is tuned on the frequency line
  tftPrintFMenu();
  tftPrintCMenu();
  // show results
  NextFMenuLine(STATE_F1, LINE_3B);
}

/**********************************************************/
/* Read and average adc                                   */
/**********************************************************/

int avg_adc(int pin)
{
  int index, adcval = 0;

  for (index = 0; index < 8; index++)
  {
    adcval = adcval + analogRead(pin);
    delay(10);
  }
  return (adcval / 8);
}

/**********************************************************/
/* Write an int to eeprom low-byte, high-byte order       */
/**********************************************************/

void ewriteInt(int addr, int val)
{
  int tmp;

  tmp = val % 0x100;      // Low byte
  EEPROM.write(addr, tmp);
  tmp = val / 0x100;      // High byte
  EEPROM.write(addr + 1, tmp);

}

/**********************************************************/
/* Read an int from eeprom low-byte, high-byte order      */
/**********************************************************/

int ereadInt(int addr)
{
  int tmp1, tmp2;

  tmp1 = EEPROM.read(addr);      // Low byte

  tmp2 = EEPROM.read(addr + 1);  // high byte
  tmp1 = 0x100 * tmp2 + tmp1;    // hex shift the high byte
  return (tmp1);
}

/**********************************************************/
/* go_home Go to home screen, home position               */
/**********************************************************/

void go_home(boolean fix_cr)
{
  fix_freqs(fix_cr);
  set_rotary_params(MODE_OP_RUN, 1, OP_MAX, 0);
  yana_state = STATE_OP;
  mode_op = MODE_OP_RUN;
  tft.fillScreen(Color_BLACK);
  tftPrintFMenu();
  tftPrintCMenu();
  CurrentField(LINE_2B, LNPOS_OP, BIG_PRT, modes_op[mode_op][0]);
}

/**********************************************************/
/*        Calibration                                     */
/**********************************************************/
/**********************************************************/
/* Install a through cable from RF to DET.                */
/* Calibration Frequencies:                               */
/* .05 .1, 1, 10, 20, 30, 40, 50, 60, 70MHz (10)          */
/* At each frequency, read the raw adc output.            */
/* Save these values in an array.                         */
/*                                                        */
/* Install a 30db attenuator from RF to DET.              */
/* Read the value at 1MHz                                 */
/* Save this value in the array.                          */
/*                                                        */
/* Show the values and allow saving.                      */
/* If saved, store the values in eeprom                   */
/**********************************************************/

void calibrate()
{
  int crntval, adcval;
  int rawADC[CALIB_SIZE];
  int8_t index1, index2, index3;

  // Pause here to show data
  tft.fillScreen(Color_BLACK);   // signal a Push
  doPrint(1, 1, SMALL_PRT, Color_WHITE, "Current Calibration");
  for (index3 = 0; index3 < CALIB_SIZE; index3++)
  {
    rawADC[index3] = ereadInt(2 * index3); // transfer calibrations to rawADC
  }
  for (index3 = 0; index3 < CALIB_PWR; index3++)
  {
    display_count(index3 + 2, 1, 4, SMALL_PRT, 1000000000L, rawADC[index3]);
    display_count(index3 + 2, 6, 4, SMALL_PRT, 1000000000L, rawADC[CALIB_PWR + index3]);
  }
  index3 = index3 + 3;
  doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Click: SNA"); index3++;
  doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Push: SWR"); index3++;
  doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Hold: PWR"); index3++;
  while ((!get_button())) {} // wait for button release

  // PWR Calibration

  if (down_h) // do power
  {
    tft.fillScreen(Color_BLACK);
    index3 = 2;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "PWR"); index3++;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Attach -10dbm to DET"); index3++;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Then Click"); index3++;
    while ((!get_button())) {} // wait for button release
    dds.setFrequency(freqs[CALIB_F_PWR]); // calibration freq
    delay(100);
    rawADC[CALIB_PWR] = avg_adc(VOLTAGE_IN); // -10dbm adc value
    dds.setFrequency(sweep.f); // back on freq
    delay(10);

  } // end PWR calibration (down_h)

  // SWR Calibration

  else if (down_p) // do SWR
  {
    index3 = 2;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "SWR"); index3++;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Attach Bridge"); index3++;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "With Short"); index3++;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Then Click"); index3 = index3++;
    while ((!get_button())) {} // wait for button release
    if (down_h) {
      tft.fillScreen(Color_BLACK);
    }
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Working ..."); index3++;

    // set and read averaged adc values
    for (index1 = 0; index1 < CALIB_F_SIZE; index1++)
    {
      dds.setFrequency(freqs[index1]);
      delay(100);
      rawADC[CALIB_SWR + index1] = avg_adc(VOLTAGE_IN);
    }

  } // end SWR calibration (down_p)

  // (Click) SNA Calibration
  else // click do SNA
  {
    // Pause here for "through" installation
    tft.fillScreen(Color_BLACK);
    index3 = 2;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "SNA"); index3++;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Attach Through"); index3++;
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Then Click"); index3++;
    while (!get_button()) {} // wait for button release
    if (down_h) {
      tft.fillScreen(Color_BLACK);
    }
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Working ..."); index3++;
    index3++;

    // set and read averaged adc values
    for (index1 = 0; index1 <= CALIB_F_SIZE - 1; index1++)
    {
      dds.setFrequency(freqs[index1]);
      delay(100);
      rawADC[index1] = avg_adc(VOLTAGE_IN);
    }

    // Pause here for "30db" installation
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Attach 30db Atten"); index3++;
    doPrint( index3, 1, SMALL_PRT, Color_WHITE, "Then Click"); index3++;
    while ((!get_button())) {} // wait for button release

    dds.setFrequency(freqs[CALIB_F_30db]);
    delay(100);
    rawADC[CALIB_30db] = avg_adc(VOLTAGE_IN); // -30db adc value at 1MHz
    dds.setFrequency(sweep.f); // back on freq
    delay(10);

  } // end SNA calibration (Click)

  // Pause here to show data
  tft.fillScreen(Color_BLACK);    // signal a Push
  doPrint(2, 1, SMALL_PRT, Color_WHITE, "New Values");
  for (index3 = 0; index3 < CALIB_PWR; index3++)
  {
    display_count(index3 + 3, 1, 4, SMALL_PRT, 1000000000L, rawADC[index3]);
    display_count(index3 + 3, 6, 4, SMALL_PRT, 1000000000L, rawADC[CALIB_PWR + index3]);
  }
  index3 = index3 + 3;
  doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Click: Skip"); index3++;
  doPrint(index3, 1, SMALL_PRT, Color_WHITE, "Push: Save"); index3++;
  while ((!get_button())) {} // wait for button release
  if (down_h) {
    tft.fillScreen(Color_BLACK);
  }
  if (down_p) // store the readings
  {
    for (index1 = 0; index1 < CALIB_SIZE; index1++)
    {
      ewriteInt(CALIB_SNA + 2 * index1, rawADC[index1]);
      calibrations[index1] = rawADC[index1]; // use calibrations now
    }
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "--> Saved");
  }
  else
  {
    doPrint(index3, 1, SMALL_PRT, Color_WHITE, "!!! NOT Saved");
  }
  delay(2000);

  go_home(false);    // start over; no change to cr/sf
}

/**********************************************************/
/*       Scale to db                                      */
/*                                                        */
/*       scale adc to db                                  */
/*       -as frequency increases, magnitude falls         */
/*       this calculation adjusts for the change          */
/*       -it normalizes so 0db at constant level          */
/**********************************************************/

float scale(long freq1, int adc) //, int base)
{
  int index;
  long temp1, temp2;
  double magnitude;

  // find position of freq between calibration frequencies
  for (index = 0; freq1 > freqs[index]; index++) {};

  // problem: 70MHz overflows float (double) so is rounded, only 7 digits are retained
  // lose the digit as late as possible

  temp1 = freq1 - freqs[index - 1];
  temp2 = freqs[index] - freqs[index - 1];
  magnitude = (float)temp1 / (float)temp2; //    fraction of the way

  // first: fraction of the way (currently: magnitude)
  // second: the change in calibrations[]
  // third: stack the result onto the calibration to obtain a normalized calibration value
  // fourth: subtract the adc reading and change to db

  if (mode_instr == MODE_INSTR_SNA ) //
  {
    // use the calibrations for Yana (loss in db)

    magnitude = (((magnitude *                                 // fraction of the way
                   ((float)calibrations[index] - (float)calibrations[index - 1])) // change in table
                  + (float)calibrations[index - 1])            // start value
                 - (float)adc) * perdb;                        // adc reading and scale to db
  }
  else // mode_instr == MODE_INSTR_SWR
  {
    // use the SWR calibrations for the bridge on Yana (return loss in db)

    magnitude = ((( magnitude *
                    ((float)calibrations[CALIB_SWR + index] -
                     (float)calibrations[CALIB_SWR + index - 1]))
                  + (float)calibrations[CALIB_SWR + index - 1])
                 - (float)adc) * perdb; //+orig0rlfl;
  }
  return (magnitude);
}

/**********************************************************/
/*       Change db to vswr                                */
/*                                                        */
/*       One unit change in adc is about .19db            */
/*         change in return loss.                         */
/*       At swr 1.1 a change by .19db moves to 1.1023     */
/*       At swr 8.0 a change by .19db moves to 8.75       */
/*       Expect swr 8 graphs to be jagged                 */
/**********************************************************/

float tovswr(float rl)
{
  float temp;
  temp = pow(10.0, -rl / 20.0); // this is Gamma

  temp = (1 + temp) / (1 - temp); // this is vswr
  //Serial.println(temp);  // debug
  if(temp < 1.0) temp = 8;  // Limit range
  if(temp > 8.0) temp = 8;
  return (temp);

}

/**********************************************************/
/*       Change db to sceen coordinates                   */
/**********************************************************/

int getcoord(float dbval)
{
  float tmp;

  if (mode_gr == MODE_GR_BIG)
  {
    if (mode_instr == MODE_INSTR_SWR)
    {
      tmp = ((tovswr(dbval) - 1.0) * UNIT_BIG_SWR); //20.0; // Shift up 1 to SWR 1 is at the top
    }
    else // SNA
    {
      tmp = dbval * DB_BIG_SNA; // 2.0;
    }
  }
  else // MODE_GR_SMALL
  {
    if (mode_instr == MODE_INSTR_SWR)
    {
      tmp = ((tovswr(dbval) - 1.0) * UNIT_SMALL_SWR); // 40.0;   // Shift up one
    }
    else // SNA
    {
      tmp = dbval * (DB_SMALL_SNA); // 6.6667
    }
  }
  tmp = round(tmp);
  return ((int)tmp);
}

/**********************************************************/
/*       Refine f1min in Scan                             */
/**********************************************************/

long scale_lc(long fg, long f)  
// fg = scan step size, f = f1min
{
//  boolean found = false; xxxy
  uint16_t templc;
  long currentf, fgaplc, fmn, fmn1;
  int adcval, adcval0;

  currentf = f-4*fg;  // start 4 gap steps lower
  if ((currentf < F_MIN) || ((f + 8*fg) > F_MAX))
  {
    return fg;  // no refinement
  }
  else
  {
    fgaplc = fg/8;  // subdivide gap by 8
    dds.setFrequency(currentf);
    adcval = avg_adc(VOLTAGE_IN);
    adcval0 = adcval;
    fmn = currentf;
    // search for smaller minimum
    // starting 4*8 fgaplc below first guess f
    for (templc = 0; templc <= 64; templc++)
    {
      dds.setFrequency(currentf);
      adcval = avg_adc(VOLTAGE_IN);
      fmn1 = currentf;
      if (adcval < adcval0)
      {
        fmn = currentf;
        adcval0 = adcval;
      }
      else if (adcval > adcval0)
      {
        fmn1 = currentf - fgaplc;  // highest min freq
        templc = 65;  // done searching
      }
      currentf = currentf + fgaplc;
    }
    fmn = (fmn + fmn1)/2L;
    return fmn;
  }
}


//void save_params()
//{
//  tmpf = sweep.f;
//  tmpic = sweep.ic;
//  tmpf1 = sweep.f1;
//  tmpf2 = sweep.f2;
//}

void restore_params()
{
  sweep.f = tmpf;
  sweep.ic = tmpic;
  sweep.f1 = tmpf1;
  sweep.f2 = tmpf2;
}

/**********************************************************/
/*                                                        */
/*       Scan                                             */
/*       sweep the screen: used by SNA,SNL,SNC, and SWR   */
/*                                                        */
/* Steps:                                                 */
/* 1. Set sweep limits and get marks                      */
/* 2. Set rotary parameters for sweep                     */
/* 3. Start outer state W loop                            */
/* 4. Prepare button press & knob turn                    */
/* 5. Put grid on screen and draw marks, center           */
/* 6. Get db values for marks                             */
/* 7. Display battery voltage                             */
/* 8. Compute step size and align to grid                 */
/* 9. Set min and max initial values                      */
/* 10. Obtain values at first point on screen             */
/* 11. Perform one sweep                                  */
/*  11a. Set frequency                                    */
/*  11b. read & scale adc                                 */
/*  11c. Adjust min and max                               */
/*  11d. Plot a graph segment                             */
/*  11e. Set next frequency                               */
/*  11f.  Move prior coordinates                          */
/* 12.  Wait for graphing speed delay                     */
/* 13.  Calculate next speed if knob turned               */
/* 15. Process button press                               */
/*  15a. Enter tuning mode                                */
/*  15b. If hold go home                                  */
/*  15c. If push and LC then process                      */
/*  15d. If click do nothing                              */
/* 16. If tuning then process                             */
/*  16a.  Center frequency & get value                    */
/*  16b. If SNA get values                                */
/*  16c. If SWR get values                                */
/*  16d. If LC, refine f and get value                    */
/*  16e. Display scan data                                */
/*  16f. Set rotary parameters                            */
/*  16g. Do tuning loop                                   */
/*   16gi. Process if knob turn                           */
/*    16gia. Move frequency and read value                */
/*    16gib. Adjust value for mode                        */
/*    16gic. Display new values                           */
/*   16gii. Check button                                  */
/*    16giia. If hold then go home                        */
/*    16giib. If push then turn off tuning                */
/*    16giic. If click, increment cursor                  */
/* End of above loops                                     */
/*                                                        */
/**********************************************************/

void scan()
{
  long start, finish, lmark, rmark, tempf; // frequencies
  long fgap, frem;                 // sweep frequency increment
  long f1min, f1max; // marked frequencies in plot
  int xcoord, xcoord0, ycoord, ycoord0; // coordinates for plotting
  int adcval;
  int cntln;                       // lines for display
  float rangefl;                   // float variables
  float tempfl;                    //
  float wmin, wmax, ytest, mcurs;  // min, max, and cursor in sweep
  float lmarkyfl, rmarkyfl;
  int toggle = 1;                  // delay time on sweep
  boolean setting, tuning = false;              // used for tuning along the curve
  char lcstr[3] = "xx";

//  save_params();
  
  // 1. Set limits: if in center/radius, switch to start/finish parameters
  
  if (mode_freq == MODE_FREQ_SF)
  {
    start = sweep.f1;
    finish = sweep.f2;
    lmark = sweep.ml;
    rmark = sweep.mr;
  }
  else // mode_freq == MODE_FREQ_CR
  {
    start = (sweep.f1 - sweep.f2);
    finish = (sweep.f1 + sweep.f2);
    lmark = (sweep.f1 - sweep.ml);
    rmark = (sweep.f1 + sweep.mr);
  }

  // 3. start outer state W loop
  
  yana_state = STATE_W;
  while (yana_state == STATE_W) // sweeping SNA or SWR
  {
 // 2.  set rotary parameters for sweep
  
    sweep.f = (start + finish) / 2L;
    sweep.ic = 1000000L;
    set_rotary_params(sweep.f, sweep.ic, finish, start); // adjust start and finish
    
  // 4. Prepare for button press & knob turn
  
    noInterrupts();
    down_b   = false;                 // no button press
    turned_t = false;               // no knob turn
    interrupts();

  // 5. Put the grid on the screen & draw marks, center

    tftPrintGrid();
    tftLabelGrid();

    // floating value for sweep range
    // used to calculate distance into sweep range
    rangefl = (float)finish - (float)start;

    //draw left mark
    // compute the fraction into the range then add it onto the start
    tempfl = (((float)lmark - (float)start) / rangefl) * X_STEPSFL;
    xcoord = (int)tempfl + X_START; //
    tft.drawFastVLine(xcoord, Y_START, Y_END, Color_GREEN);

    //draw right mark
    tempfl = (((float)rmark - (float)start) / rangefl) * X_STEPSFL;
    xcoord = (int)tempfl + X_START;      //
    tft.drawFastVLine(xcoord, Y_START, Y_END, Color_GREEN);

  // 6. Get the db values for the marks

    // get db on left mark
    dds.setFrequency(lmark);
    delay(10);
    adcval = avg_adc(VOLTAGE_IN);
    lmarkyfl = scale(lmark, adcval); // lmark in db
    // get db on right mark
    dds.setFrequency(rmark);
    delay(10);
    adcval = avg_adc(VOLTAGE_IN);
    rmarkyfl = scale(rmark, adcval); // rmark in db

  // 7.  display battery voltage
  
    dsp_supp_volt();

  // 8. Compute step size and align to grid
  
    // compute the sweep step size:
    // tempf seems necessary to force the compiler
    // to do the arithmetic
    tempf = (finish - start);
    fgap = tempf / X_STEPS; // frequency change for stepping the frequency
    frem = (tempf % X_STEPS) / 2L; // 1/2 remainder
    // adjust starting point so that grid marks come out right
    sweep.f = start + frem;

  // 9. Set min and max initial values
    
    // maximum and minimum attenuation
    wmin = 0.0;   // move it down to min signal (max value)
    wmax = 1000.0; // move it up to max signal    (min value)

  // 10. Obtain values at prior point on screen
  
    dds.setFrequency(sweep.f);
    delay(10);
    xcoord0 = X_START;         // screen coords for start of first segment
    adcval = avg_adc(VOLTAGE_IN);
    ycoord0 = getcoord(scale(sweep.f, adcval)); // screen coordinate

  // 11. Perform one sweep
  
    // *** below is one sweep ***

    for (xcoord = X_START; xcoord <= X_GRID_END; xcoord++)
    {
      // 11a. Set frequency
      
      dds.setFrequency(sweep.f);

      // 11b. read & scale adc

      ycoord = analogRead(VOLTAGE_IN);

      ytest = scale(sweep.f, ycoord); // value in db or swr
      if (usb_on)
      {
        Serial.print(sweep.f);
        Serial.print(",");
        Serial.println(ytest);
      }
      
      
      ycoord = getcoord(ytest);       // screen y-value
      if (ycoord < 0)                 // set lower limit 0
      {
        ycoord = 0;
      }

      // 11c. Adjust min and max

      if (ytest > wmin)
      {
        wmin = ytest;
        f1min = sweep.f;
      }
      if (ytest < wmax)
      {
        wmax = ytest;
        f1max = sweep.f;
      }

      // 11d. Plot a graph segment
      
      // Can choose either segments or points at compile time
      tft.drawLine(xcoord0, ycoord0, xcoord, ycoord, Color_WHITE); // plot line segments
      // xxx        tft.drawPixel(xcoord, ycoord, Color_WHITE);         // just plot points

  // 11e. Set next frequency
  
      sweep.f = fgap * (xcoord - X_START) + start; // increment frequency

  // 11f.  Move prior coordinates
  
      xcoord0 = xcoord;    // position for next segment start
      ycoord0 = ycoord;
    }
    usb_on = false;

    //    *** One sweep has finished ***

  // 12.  Wait graphing speed delay
  
    if ((toggle > 0) && !(down_b))  // process graph speed change
    {
      for (xcoord = 0; xcoord <= SWEEPDELAY_TIME*toggle; xcoord++) // scan delay
      {
        if (turned_t)   // in case turned, skip this wait time
        {
          xcoord = SWEEPDELAY_TIME*toggle + 1;
        }
        else
        {
          delay(20);  // was 10
        }
      }
    }

  // 13.  Calculate next speed if turned
  
    // turn_t == 1 when knob has been turned
    // toggle is the current sweep speed
    
    if (turned_t)  // increment graphing speed
    {
      toggle++;
      if (toggle == MAX_SWEEP_WAIT+1)
      {
        toggle = 0;
      }
      noInterrupts();
      turned_t = false;
      interrupts();
    }

  // 15. Process button press
  
    if (down_b) // stop graphing: button press 
    {
      tuning = true;
      
  //  15a. Check for button states push or hold
  
      if (get_button())
      {

  //  15b.  If hold go home
  
        if (down_h) // hold: quit sweeping, go home
        {
          tuning = false;     // not tuning
          go_home(false); // start over: no change to cr/sf
        }

  //  15c.  If push and LC then process standard
        
        else if ((down_p) && ((mode_tune == TUNE_L) || (mode_tune == TUNE_C))) 
        {
          
  //   15ci. Set units for standard
         // save_params();
  tmpf = sweep.f;
  tmpic = sweep.ic;
  tmpf1 = sweep.f1;
  tmpf2 = sweep.f2;
  
          if (mode_tune == TUNE_L)
          {
            sweep.f = c_std;
            lcstr[0] = 'p';
            lcstr[1] = 'F';
          }
          else if (mode_tune == TUNE_C)
          {
            sweep.f = l_std;
            lcstr[0] = 'n';
            lcstr[1] = 'H';
          }

  //    15cii. Show current values for standard
          
          sweep.ic = CURSOR_STEP;
          sweep.f1 = LC_MIN;
          sweep.f2 = LC_MAX;
          set_rotary_params(sweep.f,sweep.ic,sweep.f2,sweep.f1);
          doPrint(16, 2, SMALL_PRT, Color_WHITE,"New Standard Value");
          doPrint(17, 14, SMALL_PRT, Color_WHITE, lcstr);

          setting = true;  
          while (setting) // tuning an LC value
          {
            display_count(17, 1, 8, SMALL_PRT, sweep.ic, sweep.f);
            
  //      15ciia.  Set new value from knob turn
        
            if (turned_t)  // set standard to tuned value
            {
               noInterrupts();
               sweep.f = rotary_count;
               turned_t = false;
               interrupts();
               if (mode_tune == TUNE_L)
               {
                 c_std = sweep.f;  // use C to measure L
               }
               else if (mode_tune == TUNE_C)
               {
                 l_std = sweep.f;  // use L to measure C
               }
            } // end turning knob
            
  //      15ciib.  Check for button

            if (get_button())
            {
              if (down_h) // hold: go home
              {
                setting = false;
                tuning = false;
                restore_params();
                go_home(false); // no change to sf/cr
              }
              else if (down_p) // push: go back to graphing
              {
                tuning = false;
                setting = false;
                restore_params();
              }
              else // click: move cursor
              {

  //        15ciic.  Increment the cursor
  
                incr_crsr();
                noInterrupts();
                rotary_increment = sweep.ic;
                interrupts();
                display_count(17, 1, 8, SMALL_PRT, sweep.ic, sweep.f);

              }
            } // end button in LC setting

          } // end setting LC standard value
        } // end processing L, C standard
        else if (down_p) // push but not SNL or SNC
        {
          usb_on = true; // one trace out 
          tuning = false; // back to graphing
          setting = false;
        }
        //else // it's a click: do graph tuning mode
      }  // end if get_button 

  // 16. Graph tuning mode
      
      if (tuning)
      {

  //  16a.  Center frequency & get value
  
         sweep.f = (finish + start)/2L; //
         dds.setFrequency(sweep.f);     // get the value at center
         delay(10);
         ycoord0 = avg_adc(VOLTAGE_IN);
         mcurs = scale(sweep.f, ycoord0);

  //  16b. If SNA get values
        
         if (mode_instr == MODE_INSTR_SNA)  // set values for sna
         {
            lmarkyfl = -lmarkyfl * 10.0;  // left value
            rmarkyfl = -rmarkyfl * 10.0;  // right value
            mcurs = -mcurs * 10.0;  // value at midscreen
            wmin = -wmin * 10.0;    // min value
            wmax = -wmax * 10.0;    // max value
         }
           else // mode_instr == MODE_INSTR_SWR  // set values for swr

  //  16c. If SWR get values
        
           {
             lmarkyfl = tovswr(lmarkyfl) * 10.0;  // left swr
             rmarkyfl = tovswr(rmarkyfl) * 10.0;  // right swr
             mcurs = tovswr(mcurs) * 10.0;  // swr at midscreen
             wmin = tovswr(wmin) * 10.0;    // swr at min
             wmax = tovswr(wmax) * 10.0;    // swr at max
           }

  //  16d. If LC, refine f and get value
      
        // If LC, refine resonant frequency

           if ((mode_tune == TUNE_L) || (mode_tune == TUNE_C))
           {
             doPrint(16, 1, SMALL_PRT, Color_WHITE, "** Wait **");
             f1min = scale_lc(fgap,f1min);  // refining freq takes time
             dds.setFrequency(f1min);
             ycoord = avg_adc(VOLTAGE_IN);
             wmin = -10.0*scale(f1min, ycoord); // value in db
           }

  //  16e. Display scan data
  
        // Show scan Data
      
           cntln = 16;
           display_count(cntln, 1, 8, SMALL_PRT, sweep.ic, sweep.f);
           display_db(cntln, 12, SMALL_PRT, (int)mcurs);
           doPrint(cntln, 18, SMALL_PRT, Color_WHITE, "Tune");

           cntln++;
           display_count(cntln, 1, 8, SMALL_PRT, 1000000000L, f1min);
           display_db(cntln, 12, SMALL_PRT, (int)wmin);
           doPrint(cntln, 18, SMALL_PRT, Color_WHITE, "Min");

           cntln++;
           // show fmax, db max, max name 
           if ((mode_tune == TUNE_SNA) || (mode_tune == TUNE_SWR) )  
           {
             display_count(cntln, 1, 8, SMALL_PRT, 1000000000L, f1max);
             display_db(cntln, 12, SMALL_PRT, (int)wmax);
             doPrint(cntln, 18, SMALL_PRT, Color_WHITE, "Max");
           }
           else if (mode_tune == TUNE_L)  // show fmin, L value, nH
           {
             wmax = round(pow(10,21)/((4*PI*PI*(double)f1min*(double)f1min)*c_std));
             display_count(cntln, 1, 8, SMALL_PRT, 1000000000L, wmax);
             doPrint(cntln, 12, SMALL_PRT, Color_WHITE, "      nH ");
           } 
           else if (mode_tune == TUNE_C)  // show fmin, C value, pF
           {
             wmax = round(pow(10,21)/((4*PI*PI*(double)f1min*(double)f1min)*l_std));
             display_count(cntln, 1, 8, SMALL_PRT, 1000000000L, wmax);
             doPrint(cntln, 12, SMALL_PRT, Color_WHITE, "      pF ");
           } 
      
        // lmark value, rmark value
  
           cntln++;
           display_count(cntln, 1, 8, SMALL_PRT, 1000000000L, lmark);
           display_db(cntln, 12, SMALL_PRT, (int)lmarkyfl);
           doPrint(cntln, 18, SMALL_PRT, Color_WHITE, "L");

           cntln++;
           display_count(cntln, 1, 8, SMALL_PRT, 1000000000L, rmark);
           display_db(cntln, 12, SMALL_PRT, (int)rmarkyfl);
           doPrint(cntln, 18, SMALL_PRT, Color_WHITE, "R  ");

  //  16f. Set rotary parameters
  
           set_rotary_params(sweep.f, sweep.ic, finish, start); // sets turned_t = false

  //  16g. Do tuning loop
  
           cntln = 16;
           while (tuning)
           {

  //   16gi. Process if knob turn
          
             // while waiting check for rotary turn
             if (turned_t) // tuning on curve
             {
 
  //   16gia. Move frequency and read value
            
               // Tune along the trace
               // steps are about, but less than one pixel on trace
               noInterrupts();
               sweep.f = rotary_count;
               turned_t = false;
               interrupts();
               dds.setFrequency(sweep.f);
               delay(10);
               ycoord0 = (float)avg_adc(VOLTAGE_IN); // do a multiple read
               mcurs = scale(sweep.f, ycoord0);

  //   16gib. Adjust value for mode

               if (mode_instr == MODE_INSTR_SNA)
               {
                 mcurs = -mcurs * 10.0;
               }
               else // mode_instr==MODE_INSTR_SWR
               {
                 mcurs = tovswr(mcurs) * 10.0;
               }

  //   16gic. Display new values

               display_count(cntln, 1, 8, SMALL_PRT, sweep.ic, sweep.f);
               display_db(cntln, 12, SMALL_PRT, (int)mcurs);
               doPrint(cntln, 18, SMALL_PRT, Color_WHITE, "Tune");
             }    // end turned_t

  //  16gii. Check button
        
             // process button in tuning mode
        
             if (get_button()) // process the button
             {
               if (down_h) // hold: quit sweeping
               {

  //    16giia. If hold then go home
              
                 tuning = false;
                 go_home(false); // start over: no change to cr/sf
               }
               else if (down_p) // push: back to sweeping
               {

  //    16giib. If push then turn off tuning
              
                 tuning = false; // turn off tuning (back to sweeping)
               }
               else // click
               {

  //    16giic. If click, increment cursor
              
                 incr_crsr();
                 noInterrupts();
                 rotary_increment = sweep.ic;
                 interrupts();
                 display_count(cntln, 1, 8, SMALL_PRT, sweep.ic, sweep.f);
               }
             } // end process the button in tuning mode
           } // end of tuning loop
        } // end if tuning
      } // end of if down_b
  }  // end STATE_W sweeping
}

/**********************************************************/
/* do_pwr()                                               */
/*                                                        */
/* present the power screen                               */
/**********************************************************/

void do_pwr()
{
  long counter; //
  int adcval, tmp, cnt, raw;
  float pwrdbm, pwrval, pwrvlt;

  // if in center/radius, switch to start/finish parameters
  if (mode_freq == MODE_FREQ_SF)
  {
    counter = (sweep.f1 + sweep.f2) / 2;
    set_rotary_params(counter, sweep.ic, sweep.f2, sweep.f1);
  }
  else // mode_freq == MODE_FREQ_CR
  {
    counter = sweep.f1;
    set_rotary_params(counter, sweep.ic, sweep.f1 + sweep.f2, sweep.f1 - sweep.f2);
  }

  tft.fillScreen(Color_BLACK);

  display_count(LINE_9B, 1, 8, BIG_PRT, sweep.ic, counter);

  yana_state = STATE_W;
  while (yana_state == STATE_W) // reading PWR
  {
    noInterrupts();
    down_b = false;   // no button press
    interrupts();

    dsp_supp_volt();                              // show supply voltage
    raw = avg_adc(VOLTAGE_IN);
    pwrdbm = (perdb * (float)raw) - orig0dbmfl;   //dbm value
    pwrval = pow(10.0, pwrdbm / 10.0);            // the power
    pwrvlt = pow(10.0, pwrdbm / 20.0 - DBM_VOLT); // the power

    // display the dbm
    dtostrf(pwrdbm, 4, 1, outstr);
    doPrint(LINE_3B, 2, BIG_PRT, Color_YELLOW, outstr);
    doPrint(LINE_3B, 8, BIG_PRT, Color_WHITE, "dbm");

    // display power in watts
    tmp = 0;
    pwrval = pwrval / 1000.0;
    if (pwrval > 0.0)
    {
      while ((int)pwrval == 0)
      {
        pwrval = pwrval * 1000.0;
        tmp++;
      }
    }

    // display pwrval
    float_fix(pwrval, 7, 3);
    doPrint(LINE_5B, 3, BIG_PRT, Color_YELLOW, outstr);
    doPrint(LINE_5B, 9, BIG_PRT, Color_WHITE, unitsw[tmp]);

    // display voltage
    tmp = 0;
    if (pwrvlt > 0.0)
    {
      while ((int)pwrvlt == 0)
      {
        pwrvlt = pwrvlt * 1000.0;
        tmp++;
      }
    }

    // display pwrvlt
    float_fix(pwrvlt, 7, 3);
    doPrint(LINE_6B, 3, BIG_PRT, Color_YELLOW, outstr);
    doPrint(LINE_6B, 9, BIG_PRT, Color_WHITE, unitsv[tmp]);
    tftLabelGrid();

    // display raw ADC reading
    display_count(LINE_7B, 3, 3, BIG_PRT, 1000000000L, raw);

    // now delay .1 sec while polling the button
    for (tmp = 0; tmp < 20; tmp++)
    {
      if ((turned_t) || (down_b))
      {
        tmp = 21;
      }
      else
      {
        delay(10);
      }
    } // end delay
    if (turned_t) // check for freq change
    {
      // Set DDS & display frequency on the bottom
      noInterrupts();
      counter = rotary_count;
      turned_t = false;
      interrupts();
      dds.setFrequency(counter);
      delay(10);
      // display frequency
      display_count(LINE_9B, 1, 8, BIG_PRT, sweep.ic, counter);
    }
    if (get_button())
    {
      noInterrupts();
      down_b = true; // button press
      interrupts();
    }

    if (down_b)
    {
      if (down_h) // quit: power
      {
        go_home(false); // start over; no change to cr/sf
      }
      if (down_p) // quit: power
      {
        go_home(false); // start over; no chnage to cr/sf
      }
      else // click: increment cursor
      {
        incr_crsr();
        noInterrupts();
        rotary_increment = sweep.ic;
        turned_t = false;
        interrupts();
        // display frequency
        display_count(LINE_9B, 1, 8, BIG_PRT, sweep.ic, counter);

      }
    }    // end down_b

  } // end command STATE_W
}

/**********************************************************/
/* setup                                                  */
/**********************************************************/

void setup()
{
  
  int tmp;

   Serial.begin(115200); // set serial port
   
  // start the display
  TFT_START;

  // fix screen rotation
  tft.setRotation(SCREEN_UPRIGHT);
  // start up the DDS...

  dds.init();

  // trim your xtal with TUNE_WORD...

  dds.trim(TUNE_WORD);     //set TUNE_WORD for 6x actual osc freq

  // Set the encoder button pin

  pinMode(ENCODER_BTN, INPUT);          // using Analog pin
  digitalWrite(ENCODER_BTN, HIGH);      // enable pullup

  // Set the ADC voltage reference

  analogReference(DEFAULT);            // INTERNAL = 1.1 V AREF, DEFAULT = 5 V AREF

  // Enable pin change interrupt for the button

  PCICR |= 0x02;
  PCMSK1 |= 0b00000001;

  // Enable pin change interrupt for the rotary encoder

  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei(); // turn on interrupts

  // get the calibration data

  for (tmp = 0; tmp < CALIB_SIZE; tmp++)
  {
    calibrations[tmp] = ereadInt(CALIB_SNA + 2 * tmp);
  }

  // calculate the perdb adc coordinates (adcval/db)
  // value from 4/6/16 calibration: .189 adc step/db

  tmp = calibrations[CALIB_F_30db] - calibrations[CALIB_30db]; // range for 30db at 1MHz
  perdb = CALIB_ATTEN30db / ((float)tmp);                      // adc units per db

  // calculate the 0dbm origin (adcval/db) with 30db from Out to Det

  // subtract this value from converted adc readings to get actual dbm
  // value from 4/6/16 calibration: .9948 is org0dbmfl

  tmp = calibrations[CALIB_PWR];                   // -10dbm adc value
  orig0dbmfl = 10.0 + (perdb * (float)tmp);        // adc relative value at +0dbm

   // Intro Screen

  tft.fillScreen(Color_BLACK);   // background black

  doPrint(3, 3, BIG_PRT, Color_YELLOW, "*Yana");
  doPrint(5, 3, BIG_PRT, Color_BLUE, "K1TRB");
  doPrint(7, 3, BIG_PRT, Color_BLUE, ">2017");

  delay(700);
  tft.fillScreen(Color_BLACK);

  // Set up for yana_state STATE_OP: operations

  go_home(false);   // start over; no change to cr/sf
}


/**********************************************************/
/* loop, main program                                     */
/**********************************************************/

void loop()
{
  /*******************************************************/
  /* yana_state STATE_INSTR: Set the Analyzer Instrument */
  /*******************************************************/

  while (yana_state == STATE_INSTR)
  {
    // Check for knob rotation
    if (turned_t)
    {
      noInterrupts();
      mode_instr_1 = rotary_count; 
      turned_t = false;
      interrupts();
      switch(mode_instr_1)
      {
        case  MODE_INSTR_SNA:
          mode_tune = TUNE_SNA;
          mode_instr = MODE_INSTR_SNA;
        break;

        case MODE_INSTR_SWR:
          mode_tune = TUNE_SWR;
          mode_instr = MODE_INSTR_SWR;
        break;

        case MODE_INSTR_PWR:
          mode_tune = TUNE_PWR;
          mode_instr = MODE_INSTR_PWR;
        break;

        case MODE_INSTR_SNL:
          mode_tune = TUNE_L;
          mode_instr = MODE_INSTR_SNA;
        break;

        case MODE_INSTR_SNC:
          mode_tune = TUNE_C;
          mode_instr = MODE_INSTR_SNA;
        break;

      }
        
      tftPrintCMenu();
      CurrentField(LINE_2B, LNPOS_INSTR, BIG_PRT, modes_instr[mode_instr][0]);
    }

    if (get_button())
    {
      /* Process hold: */
      if (down_h)
      {
        go_home(false); // start over; no change to cr/sf
      }

      /* Process push:   */
      else if (down_p)  // push:
      {
        set_for_f1();
      }

      /* Process a click: increment f increment */
      else   // click, increment changed
      {
        // set for frequency modes STATE_F_MODE
        yana_state = STATE_F_MODE;
        mode_freq_now = mode_freq; // to know this mode when leaving STATE_F_MODE
        set_rotary_params(mode_freq, 1, SC_MAX, 0);
        tftPrintCMenu();
        CurrentField(LINE_2B, LNPOS_FREQ, BIG_PRT, modes_freq[mode_freq][0]);
      }
    }
  }    // end of STATE_INSTR

  /*****************************************************************/
  /* yana_state STATE_F_MODE: Choose Center/Radius or Start/Finish */
  /*****************************************************************/

  while (yana_state == STATE_F_MODE)
  {
    // Check for knob rotation
    if (turned_t)
    {
      noInterrupts();
      mode_freq = rotary_count;
      turned_t = false;
      interrupts();
      tftPrintCMenu();
      CurrentField(LINE_2B, LNPOS_FREQ, BIG_PRT, modes_freq[mode_freq][0]);
    }

    if (get_button())
    {
      /* Process hold: stay in yana_state STATE_F1: f start */
      if (down_h)
      {
        go_home(mode_freq_now != mode_freq); // start over; no change to cr/sf
      }

      /* Process push: go to yana_state STATE_F1 freq Center or Start */
      else if (down_p)   // push: go to next yana_state
      {
        fix_freqs((mode_freq_now != mode_freq));  // switching sf/cr?
        set_for_f1();
      }

      /* Process a click: increment f increment */
      else     // click, increment changed
      {
        // set for instrument STATE_G_MODE
        if (mode_freq_now != mode_freq)
        {
          tftPrintFMenu();
        }
        fix_freqs(mode_freq_now != mode_freq); // switching between S/F, C/R
        tftPrintFFreqs();
        yana_state = STATE_G_MODE;
        set_rotary_params(mode_gr, 1, GR_MAX, 0);
        tftPrintCMenu();

        CurrentField(LINE_2B, LNPOS_GR, BIG_PRT, modes_gr[mode_instr][mode_gr][0]);
      } // end click
    } // end get_button
  } // end of STATE_F_MODE

  /***********************************************************/
  /*    yana_state STATE_G_MODE: Set the Analyzer Graph Mode */
  /***********************************************************/
  
  while (yana_state == STATE_G_MODE)
  {
    // Check for knob rotation
    if (turned_t)
    {
      noInterrupts();
      mode_gr = rotary_count;
      turned_t = false;
      interrupts();
      tftPrintCMenu();
      CurrentField(LINE_2B, LNPOS_GR, BIG_PRT, modes_gr[mode_instr][mode_gr][0]);
    }

    if (get_button())
    {
      /* Process hold: goto yana_state STATE_OP */
      if (down_h)
      {
        go_home(false); // start over; no change to cr/sf
      }

      /* Process push: go to yana_state STATE_F1 freq Center or Start */
      else if (down_p)   // : go to next yana_state
      {
        // goto yana_state STATE_F1
        set_for_f1();
      }

      /* Process a click: increment f increment */
      else   // click, increment changed
      {
        // set for instrument STATE_OP
        go_home(false); // start over; no change to cr/sf
      }
    }
  }    // end of STATE_G_MODE

  /*******************************************************/
  /*   yana_state STATE_OP: Set the Analyzer Operation   */
  /*******************************************************/
  
  while (yana_state == STATE_OP)
  {
    // Check for knob rotation
    if (turned_t)
    {
      noInterrupts();
      mode_op = rotary_count;
      turned_t = false;
      interrupts();
      tftPrintCMenu();
      CurrentField(LINE_2B, LNPOS_OP, BIG_PRT, modes_op[mode_op][0]);
    }

    if (get_button())
    {
      /* Process hold: execute mode_instr chosen instrument       */
      if (down_h)  // it's a hold:
      {
        switch(mode_op) // do the operation
        {
          case MODE_OP_RUN :
            if (mode_instr == MODE_INSTR_SNA)
            {
              scan() ; // scan in SNA instrument
            }
            else if (mode_instr == MODE_INSTR_SWR)
            {
              scan(); // scan in SWR instrument
            }
            else //mode_instr == MODE_INSTR_PWR
            {
              do_pwr();
            }
            break;

          case MODE_OP_CAL :
            calibrate();
            break;
            
        }
      }

      /* Process push: go to yana_state STATE_F1 freq Center or Start */
      else if (down_p)   // push: go to next yana_state
      {
        // Go to the STATE_F1 frequency setting
        set_for_f1();
      }

      /* Process a click: increment f increment */
      else   // click, increment changed
      {
        // set for instrument STATE_INSTR
        yana_state = STATE_INSTR;
        set_rotary_params(mode_instr, 1, INSTR_MAX, 0);
        tftPrintCMenu();
        CurrentField(LINE_2B, LNPOS_INSTR, BIG_PRT, modes_instr[mode_instr][0]);
      }
    }
  }    // end of STATE_OP

  /*******************************************************/
  /*   yana_state STATE_F1: Set the Analyzer Frequency   */
  /*******************************************************/
  
  while (yana_state == STATE_F1) //yana_state is center/start freq
  {
    /* Check for knob rotation */
    if (turned_t) // freq was changed
    {
      noInterrupts();
      sweep.f1 = rotary_count;
      turned_t = false;
      interrupts();
      display_count(LINE_4B, 1, 8, BIG_PRT, sweep.ic, sweep.f1);
      dds.setFrequency(sweep.f1);    // set the frequency of dds
    }

    /* Check for button down */
    if (get_button())  // check for button down
    {
      /* Process hold: goto yana_state STATE_OP */
      if (down_h)  // it's a hold:
      {
        go_home(false); // start over; no change to cr/sf
      }

      /* Process push: go to yana_state STATE_F2 freq radius/finish */
      else if (down_p)    // push: go to next yana_state
      {
        // Go to the yana_state STATE_F2 screen (Radius or Finish)
        fix_freqs(false);    // no change to SF/CR

        // set limits for sweep.f2
        if (mode_freq == MODE_FREQ_CR)
        {
          minfr = 0;  // lower limit of radius
          if ((F_MIN + F_MAX)/2L > sweep.f1) 
          // F_Min is closer
          {
            maxfr = sweep.f1 - F_MIN;
          }
          else // F_MAX closer
          {
            maxfr = F_MAX - sweep.f1;
          }
        }
        else // mode_freq==MODE_FREQ_SF
        {
          maxfr = F_MAX;
          minfr = sweep.f1;
        }

        set_rotary_params(sweep.f2, sweep.ic, maxfr, minfr);
        tftPrintFMenu();
        tftPrintCMenu();
        NextFMenuLine(STATE_F2, LINE_5B);  // switch to STATE_F2
      }

      /* Process a click: increment f increment */
      else   // click, increment changed
      {
        incr_crsr();
        noInterrupts();
        rotary_increment = sweep.ic;
        turned_t = false;
        interrupts();

        display_count(LINE_4B, 1, 8, BIG_PRT, sweep.ic, sweep.f1);
        //                 display_count(LINE_6B,1,8,BIG_PRT,sweep.ic,sweep.f2);
      }
    }
  }    // end of STATE_F1

  /***********************************************/
  /* yana_state STATE_F2: Set the analyzer span  */
  /***********************************************/
  
  while (yana_state == STATE_F2)    //yana_state is r freq
  {

    /* Check for knob rotation */
    if (turned_t) // freq was changed
    {
      noInterrupts();
      sweep.f2 = rotary_count; // retrieve new r freq
      turned_t = false;
      interrupts();
      display_count(LINE_6B, 1, 8, BIG_PRT, sweep.ic, sweep.f2);
      // dds doesn't move on change to finish f

    }

    /* Check for button down */
    if (get_button())
    {
      /* Process a hold: goto yana_state STATE_OP */
      if (down_h)    // hold:
      {
        go_home(false); // start over; no change to cr/sf
      }

      // Process a push: go to yana_state STATE_L: left marker
      else if (down_p)    // push: go to next yana_state
      {
        fix_freqs(false);    // No change to SF/CR

        // set rotary parameters
        // set limits for sweep.ml
        if (mode_freq == MODE_FREQ_CR)
        {
          set_rotary_params(sweep.ml, sweep.ic, sweep.f2, 0);
        }
        else // mode_freq==MODE_FREQ_SF
        {
          set_rotary_params(sweep.ml, sweep.ic, sweep.f2, sweep.f1);
        }

        tftPrintFMenu();
        tftPrintCMenu();
        NextFMenuLine(STATE_L, LINE_7B);
      }

      /* Process a click: increment radius/finish increment */
      else   // click, increment changed
      {
        incr_crsr();
        noInterrupts();
        rotary_increment = sweep.ic;
        turned_t = false;
        interrupts();
        display_count(LINE_6B, 1, 8, BIG_PRT, sweep.ic, sweep.f2);
      }
    }
  }    // end of STATE_F2

  /***********************************************/
  /* yana_state STATE_L: Set Left Marker         */
  /***********************************************/
  
  while (yana_state == STATE_L) //yana_state is left
  {
    // Check for knob rotation
    if (turned_t) // rotating changes repeat
    {
      noInterrupts();
      sweep.ml = rotary_count; // retrieve new L freq
      turned_t = false;
      interrupts();
      display_count(LINE_8B, 1, 8, BIG_PRT, sweep.ic, sweep.ml);
      // dds doesn't move on change to L
    }

    // Check for button down
    if (get_button())
    {
      /* Process a hold: goto yana_state STATE_OP */
      if (down_h)    // hold:
      {
        go_home(false); // start over; no change to cr/sf
      }

      // Process a push: go to yana_state R: right marker
      else if (down_p)
      {
        fix_freqs(false);    // No change to SF/CR

        // set rotary parameters of sweep.mr
        if (mode_freq == MODE_FREQ_CR)
        {
          set_rotary_params(sweep.mr, sweep.ic, sweep.f2, 0);
        }
        else // mode_freq==MODE_FREQ_SF
        {
          set_rotary_params(sweep.mr, sweep.ic, sweep.f2, sweep.ml);
        }

        tftPrintFMenu();
        tftPrintCMenu();
        NextFMenuLine(STATE_R, LINE_9B);
      }

      /* Process a click: increment radius/finish increment */
      else   // click, increment changed
      {
        incr_crsr();
        noInterrupts();
        rotary_increment = sweep.ic;
        turned_t = false;
        interrupts();
        display_count(LINE_8B, 1, 8, BIG_PRT, sweep.ic, sweep.ml);
      }
    }
  }    // end of STATE_L

  /***********************************************/
  /* yana_state STATE_R: Set Right Marker         */
  /***********************************************/

  while (yana_state == STATE_R) //yana_state is left
  {
    // Check for knob rotation
    if (turned_t) // rotating changes repeat
    {
      noInterrupts();
      sweep.mr = rotary_count; // retrieve new M freq
      turned_t = false;
      interrupts();
      display_count(LINE_10B, 1, 8, BIG_PRT, sweep.ic, sweep.mr);
      // dds doesn't move on change to M

    }

    // Check for button down
    if (get_button())
    {
      /* Process a hold: Go Home */
      if (down_h)    // hold
      {
        go_home(false); // start over; no change to cr/sf
      }

      // Process a push: go to yana_state STATE_F1
      else if (down_p)
      {
        fix_freqs(false);    // No change to SF/CR
        set_for_f1();
      }

      /* Process a click: increment radius/finish increment */
      else   // click, increment changed
      {
        incr_crsr();
        noInterrupts();
        rotary_increment = sweep.ic;
        turned_t = false;
        interrupts();
        display_count(LINE_10B, 1, 8, BIG_PRT, sweep.ic, sweep.mr);
      }
    }
  } // end of STATE_R

} // end loop


