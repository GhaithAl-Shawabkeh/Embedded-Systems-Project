void ATD_init(void);  // function to initialize AD conversion
void delayNms(int);   // delay function
unsigned int ATD_read(void); // function to perform AD conversion
void ATD_update();    // function to update the ADCON0 register to switch analog input channels
unsigned int k;  // input analog voltage variable

unsigned int angle;   // Count value of high - pertaining to the angle
unsigned char HL;     // High Low flag

unsigned int waiting; // flag for delay purposes - did not work as bool or char
unsigned int i;  // current finger number

unsigned int fingersADCON0[5]={0x41, 0x49, 0x51, 0x59, 0x61};  // ADCON0 for each finger
float fingersSlope[5] = {250, 20.942, 19.802, 28.169, 26.316}; // slope for each finger's mapping equation
float fingersYintercept[5] = {-196000, -5701.44, -6128.72, -11704.219, -6289.532}; // slope for each finger's mapping equation

void interrupt(void){

       if(PIR1 & 0x04){   // CCP1 interrupt
             if(HL){                                // high
                       CCPR1H = angle >> 8;
                       CCPR1L = angle;
                       HL = 0;                      // next time low
                       CCP1CON = 0x09;              // compare mode, clear output on match
                       TMR1H = 0;
                       TMR1L = 0;
             }
             else{                                          //low
                       CCPR1H = (40000 - angle) >> 8;       // 40000 counts correspond to 20ms
                       CCPR1L = (40000 - angle);
                       CCP1CON = 0x08;             // compare mode, set output on match
                       HL = 1;                     //next time High
                       TMR1H = 0;
                       TMR1L = 0;
             }
            waiting = 0;
             PIR1 = PIR1&0xFB; // clear CCP1 interrupt flag

       }


 }

void main() {
        TRISC = 0x00;           // PWM output at CCP1(RC2)
        PORTC = 0x00;
        TRISD = 0x00;           // for demultiplexor select line
        i=0;
        ATD_init();

        TMR1H = 0;
        TMR1L = 0;

        HL = 1;                // start high
        CCP1CON = 0x08;        // Compare mode, set output on match

        T1CON = 0x01;          // TMR1 On Fosc/4 (inc 0.5uS) with 0 prescaler (TMR1 overflow after 0xFFFF counts == 65535)==> 32.767ms

        INTCON = 0xC0;         // Enable GIE and peripheral interrupts
        PIE1 = PIE1|0x04;      // Enable CCP1 interrupts
        CCPR1H = 2000>>8;      // Value preset in a program to compare the TMR1H value to            - 1ms
        CCPR1L = 2000;         // Value preset in a program to compare the TMR1L value to

        while(1){
              ATD_update();   //     initialize using channel 'i'
              k = ATD_read();  // 0-1023
              
              angle = fingersYintercept[i] + (k*fingersSlope[i]);     // PREV CODE -> angle= 1000 + ((k*2500)/255); 1000count=500uS to 3500count =1750us

              if(angle>5000) angle = 5000;      // 2.5ms
              if(angle<1000) angle = 1000;      // 0.5ms


              PORTD = i;
              
              delayNms(30);
              i = (i+1) % 5;   //move to other channel
        }
}

void ATD_init(void){
      ADCON1=0xCE;           // All Analog, Right Allignment,
      TRISA=0x2F;
}

void ATD_update() {
      ADCON0=fingersADCON0[i];        //iterate through channels 0-4
}

unsigned int ATD_read(void){
      ADCON0=ADCON0 | 0x04;  // GO
      while(ADCON0&0x04);    // wait until DONE
      return (ADRESH<<8)|ADRESL;
}
// 1 ms delay
void delay1ms(void) {
     // set prescalar assignment bit to 0
     // set option reg bit 3 to 0
     OPTION_REG = OPTION_REG & 0xF7;

     // set TMR0 to 6 for 250 counts
     TMR0 = 0x6;

     // wait
     while (waiting==1);
     waiting=1;


    }
void delayNms(int n) {
     while (n--) delay1ms();
}
