//-----------------------------------------------------------------------------
// F06x_Timer2_16bitToggle.c
//-----------------------------------------------------------------------------
// Copyright 2005 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// This program presents an example of use of the Timer2 of the C8051F06x's in
// 16-bit output toggle mode. It uses the 'F060DK as HW platform.
//
// This example code counts in timer2 until it overflows. At this moment the T2
// output is toggled. (aproximately every 200msec.)
//
// Pinout:
//
//    P0.0 -> T2 toggle output
//
// How To Test:
//
// 1) Load the F06x_Timer2_16bitToggle.c
// 2) To change the toggling rate modify TOGGLE_RATE value (0 to 255 -> msec)
// 3) Compile and download the code
// 4) Run
// 5) Check the P0.0 pin (T2)
//
// FID:            06X000001
// Target:         C8051F06x
// Tool chain:     KEIL C51 7.20 / KEIL EVAL C51
// Command Line:   None
//
// Release 1.0
//    -Initial Revision (CG)
//    -09 NOV 2005
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <c8051F060.h>                 // SFR declarations
#include <intrins.h>
//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F06x
//-----------------------------------------------------------------------------
sfr16 RCAP3    = 0xCA;                    // Timer3 reload value
sfr16 TMR3     = 0xCC;                    // Timer3 counter

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define SYM_NUM         28   // Number of symbols in Morze alphabet coded
#define DOT_IDX         26
#define SPACE_IDX       27
#define SYSCLK          24500000/8  // SYSCLK in Hz (24.5 MHz internal
                                    // oscillator / 8)
                                    // the internal oscillator has a
                                    // tolerance of +/- 2%
#define DOT_CNT         3
#define DASH_CNT        9
#define DOT_PAUSE       3
#define SPACE_PAUSE     12
#define PAUSE_SYM       0xC000      //mask 2 higher bits in the sym_out, indicates pause required after symbol end

sbit  LED = P1^6;                   // LED: '1' = ON; '0' = OFF

typedef enum {SYM_START, LED_ON, PAUSE } SYM_STATE;

const char out_str[]="Hello all.";
//Encoded symbols, packet 1 symbol into 1 byte. Each element - 2 bits. 
// '.'           - 01
// '-'           - 10
// pause (2x'-') - 11
//round-robin direction - right to left
const unsigned int MORZE_CODE[SYM_NUM]=
//                             Symbol | Morze code
//                                    |
{ 0x0011, //0000 0000 0000 1001 A,a   | .- 
  0x0056, //0000 0000 0101 0110 B,b   | -...
  0x0066, //0000 0000 0110 0110 C,c   | -.-.
  0x0016, //0000 0000 0001 0110 D,d   | -..
  0x0001, //0000 0000 0000 0001 E,e   | .
  0x0065, //0000 0000 0110 0101 F,f   | ..-.
  0x001A, //0000 0000 0001 1010 G,g   | --.
  0x0055, //0000 0000 0101 0101 H,h   | ....
  0x0005, //0000 0000 0000 0101 I,i   | ..
  0x00A9, //0000 0000 1010 1001 J,j   | .---
  0x0026, //0000 0000 0010 0110 K,k   | -.-
  0x0059, //0000 0000 0101 1001 L,l   | .-..
  0x000A, //0000 0000 0000 1010 M,m   | --
  0x0006, //0000 0000 0000 0110 N,n   | -.
  0x001A, //0000 0000 0010 1010 O,o   | ---
  0x0069, //0000 0000 0110 1001 P,p   | .--.
  0x009A, //0000 0000 1001 1010 Q,q   | --.-
  0x0019, //0000 0000 0001 1001 R,r   | .-.
  0x0015, //0000 0000 0001 0101 S,s   | ...
  0x0002, //0000 0000 0000 0010 T,t   | -
  0x0025, //0000 0000 0010 0101 U,u   | ..-
  0x0095, //0000 0000 1001 0101 V,v   | ...-
  0x0029, //0000 0000 0010 1001 W,w   | .--
  0x0096, //0000 0000 1001 0110 X,x   | -..-
  0x00A6, //0000 0000 1010 0110 Y,y   | -.--
  0x005A, //0000 0000 0101 1010 Z,z   | --..
  0x0155, //0000 0001 0101 0101 dot   | ......
  0x0003  //0000 0000 0000 0011 space | silence for period 2x'-'
};

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
volatile unsigned int sym_out = 0;
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void PORT_Init (void);                 // Port initialization routine
void Timer3_Init (int);               // Timer0 initialization routine
char char_to_idx(char c);
void sys_init(void);

//-----------------------------------------------------------------------------
// main() Routine
//-----------------------------------------------------------------------------
void main (void)
{
   int idx = 0;
   int morze_idx;
   sys_init();
      
   while(1){
       morze_idx = (int)char_to_idx(out_str[idx]);
	   if(morze_idx < 0 ) break;
       sym_out = MORZE_CODE[morze_idx];
       while(sym_out){
	   _nop_();
	   }
       sym_out = PAUSE_SYM;
	   while(sym_out){
	   _nop_();
	   }
	   if(MORZE_CODE[morze_idx]=='.') break;
       idx++;
   }
   while(1){
     _nop_();
   }
}

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

void sys_init(void){

   // disable watchdog timer
   WDTCN = 0xde;
   WDTCN = 0xad;

   PORT_Init ();

   Timer3_Init (SYSCLK / 12 / 10);        // Init Timer3 to generate interrupts
                                          // at a 10 Hz rate.
   EA = 1;                                // enable global interrupts

   SFRPAGE = LEGACY_PAGE;                 // Page to sit in for now

}

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function configures the crossbar and GPIO ports.
//
//-----------------------------------------------------------------------------
void PORT_Init (void)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

   SFRPAGE = CONFIG_PAGE;              // set SFR page


   XBR2 = 0x40;                        // Enable crossbar

   P1MDOUT = 0x40;                     // : Port1 Output Mode Register. Set P1.6(LED) to push-pull
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page

}

//------------------------------------------------------------------------------------
// Timer3_Init
//------------------------------------------------------------------------------------
//
// Configure Timer3 to auto-reload and generate an interrupt at interval
// specified by <counts> using SYSCLK/12 as its time base.
//
//
void Timer3_Init (int counts) {
   char old_SFRPAGE;
   old_SFRPAGE = SFRPAGE;                 // Save old SFRPAGE
   SFRPAGE = TMR3_PAGE;                   // Switch to Timer 3 page

   TMR3CN = 0x00;                         // Stop Timer3; Clear TF3;
                                          // use SYSCLK/12 as timebase
   RCAP3   = -counts;                     // Init reload values
   TMR3    = 0xffff;                      // set to reload immediately
   EIE2   |= 0x01;                        // enable Timer3 interrupts
   TR3 = 1;                               // start Timer3

   SFRPAGE = old_SFRPAGE;                 // restore SFRPAGE

}

/* ****************************************************************************
 * @NAME char_to_idx
 * @DESCR Converts input character
 **************************************************************************** */
char char_to_idx(char c){
    int rc = -1;

    if(c >= 0x41 && c <= 0x7A){

	       rc = c - 0x41;

	 } else if(c >= 0x61 && c <= 0x7A){

           rc = c - 0x61;

	 }else if(c == '.'){

         rc = DOT_IDX;
 
	 }else if(c == 0x20){

         rc = SPACE_IDX; 
	 }

	 return rc;
}

//------------------------------------------------------------------------------------
// Timer3_ISR
//------------------------------------------------------------------------------------
// This routine changes the state of the LED whenever Timer3 overflows.
//
// NOTE: The SFRPAGE register will automatically be switched to the Timer 3 Page
// When an interrupt occurs.  SFRPAGE will return to its previous setting on exit
// from this routine.


void Timer3_ISR (void) interrupt 14
{
   static unsigned short int t3_cnt = 0;
   static SYM_STATE sm = SYM_START;
   TF3 = 0;
   if(sym_out == PAUSE_SYM){
       sym_out &= 0x8000;
	   t3_cnt = SPACE_PAUSE;
	   LED = 0;
	   sm = PAUSE;
   }
   switch ( sm ){
       case SYM_START:
	       if(sym_out){
		       t3_cnt = (sym_out & 0x03 == 0x01) ? DOT_CNT : DASH_CNT;
	           sm = LED_ON;
			   LED = 1;
           }
	       break;
       case LED_ON:
	       if(!t3_cnt){
		       sym_out >>= 2;
			   if(sym_out){
		           t3_cnt = DOT_PAUSE;
			       sm = PAUSE;
               } else {
                   sm = SYM_START;
			   }
			   LED = 0;
		   }
	       break;
       case PAUSE:
	       if(!t3_cnt){
		       if(sym_out & PAUSE_SYM){
		          sym_out &= ~PAUSE_SYM;
				  sm = SYM_START;
			   } else if(sym_out){
			       t3_cnt = (sym_out & 0x03 == 0x01) ? DOT_CNT : DASH_CNT;
			       sm = LED_ON;
		           LED = 1;
			   }
		   }
	       break;
   }
   if (t3_cnt) t3_cnt--;

   //LED = ~LED;                            // change state of LED
}
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------