#include "mcc_generated_files/mcc.h"
#include "Dallas.h"
#include "Utils.h"

// Variables para la temperatura
char temp[] = "000.00 C\n\0";
uint16_t raw_temp;


void cargarTemperatura();
void enviarTemperatura();

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();

    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    while (1)
    {
        // Add your application code
        RA2=0;
        __delay_ms(1000);
        RA2=1;
        __delay_ms(5000);
        enviarTemperatura();
    }
}


void cargarTemperatura() {
    temp[10] = (char) 223;
    if (ds18b20_read(&raw_temp)) {
        if (raw_temp & 0x8000) // if the temperature is negative
        {
            temp[0] = '-'; // put minus sign (-)
            raw_temp = (~raw_temp) + 1; // change temperature value to positive form
        } else {
            if ((raw_temp >> 4) >= 100) // if the temperature >= 100 °C
                temp[0] = '1'; // put 1 of hundreds
            else // otherwise
                temp[0] = '0'; // put zero '0'
        }
        // put the first two digits ( for tens and ones)
        temp[1] = ((raw_temp >> 4) / 10) % 10 + '0'; // put tens digit
        temp[2] = (raw_temp >> 4) % 10 + '0'; // put ones digit

        // put the 4 fraction digits (digits after the point)
        // why 625?  because we're working with 12-bit resolution (default resolution)
        temp[4] = ((raw_temp & 0x0F) * 625) / 1000 + '0'; // put thousands digit
        temp[5] = (((raw_temp & 0x0F) * 625) / 100) % 10 + '0'; // put hundreds digit
        // Add break line to end
        temp[8] = '\n';
    }
}

void enviarTemperatura() {
    char temperaturaNueva[6] = "     ";
    cargarTemperatura();
    temperaturaNueva[0] = temp[0];
    temperaturaNueva[1] = temp[1];
    temperaturaNueva[2] = temp[2];
    temperaturaNueva[3] = temp[3];
    temperaturaNueva[4] = temp[4];
    temperaturaNueva[5] = '\0';

    // Mando comando "temperatura"
    send_USART_data_custom("temperatura$");
    //__delay_ms(1000);                       //Timer 0 para enviar esto 5 min
    send_USART_data_custom(&temperaturaNueva);
    send_USART_data_custom("$");
}
/**
 End of File
*/