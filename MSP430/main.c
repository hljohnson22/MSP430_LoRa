#include <msp430.h> 
#include "main.h"

bool i2c_succeed;

volatile uint8_t pulses;
/**
 * main.c
 */
int main(void)
{


    WDTCTL = WDTPW | WDTHOLD;                     // Stop watchdog timer

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    initClockTo1MHz();

    while (1)
    {
        P1DIR |= BIT0;
        P1OUT &= ~BIT0;
        P1OUT ^= BIT0; //LED1


        P5DIR |= BIT4; //power to LoRa
        P5OUT |= BIT4;
        P4DIR |= BIT5; //in series to add current
        P4OUT |= BIT5;

        P5DIR |= BIT2; //power to sensor
        P5OUT |= BIT2;

        //temperature sensor data line
        P1OUT |= BIT4;                                  // P1.4 output high
        P1DIR |= BIT4;                                  // P1.4 as output

        __delay_cycles(1000); //needs time to get the crystal working properly

        //initialize SPI and I2C
        initGPIO_SPI();
        initSPI();
        initGPIO_I2C();
        initI2C();

        __delay_cycles(10000); //wait for initialization

        //take temp reading
        __delay_cycles(RESET_DELAY*3);

        init_DS18B20();                               // Send reset
        ROM_read();                                   // Send Read ROM command
        for (byte = 0; byte < 8; byte++)
            identification[byte] = Single_read();     // Read full identification number
        function_command_convert();                   // Send function convert temperature command

        __delay_cycles(RESET_DELAY*3);

        init_DS18B20();                               // Send reset
        ROM_read();                                   // Send Read ROM command
        for (byte = 0; byte < 8; byte++)
            identification[byte] = Single_read();     // Read full identification number
        function_command_read();                      // Send function read command
        for (byte = 0; byte < 9; byte++)
          temp[byte] = Single_read();                 // Read output from sensor


        smsbTemp = temp[1] << 4;                      //find temp MSB
        lsbTemp = temp[0] >> 4;                       //find temp lsb
        raw = smsbTemp | lsbTemp;                     //concat temp values
        decimalPlaceValue = (lsbTemp << 4) ^ temp[0]; //find decimal place value
        finalTemp[0] = raw;
        finalTemp[1] = decimalPlaceValue;

        P1OUT |= BIT4;                                //LED update for debugging

        //send over LoRa
        __delay_cycles(1000);
        while(!init_wireless());

        uint16_t CrC = add_crc(finalTemp, 2, 1);      //calculate temp CRC
        finalTemp[2] = (CrC & 0xFF00) >> 8;           //split CRC in two 8 bit variables
        finalTemp[3] = (CrC & 0x00FF);

        wireless_send(finalTemp, 4);                  //send data

        //TEST: without, should be turned off in sleep func
        //P5OUT ^= BIT4; //LoRa power off
        P1OUT ^= BIT0; //LED off
        P5OUT |= BIT4;
        P4OUT |= BIT5;
        P5OUT |= BIT2;


        sleep_10_min();                               //low power mode 4 for 10 min sleep

    }
}

// RTC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(RTCIV,RTCIV_RTCIF))
    {
        case  RTCIV_NONE:   break;          // No interrupt
        case  RTCIV_RTCIF:                  // RTC Overflow
            RTCCTL |= RTCSR; // turn off the timer
            __bic_SR_register_on_exit(LPM4_bits);
            break;
        default: break;
    }
}

//interrupt

