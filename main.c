// CONFIG
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF       //Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital input, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = ON         // Data EE Memory Code Protection bit (Data memory code-protected)
#pragma config CP = ON          // Flash Program Memory Code Protection bit (0000h to 07FFh code-protected)

#define _XTAL_FREQ 16000000 // 16 Mhz


#include <xc.h>         //libreria propia de pics de 8 bits, estan las ctes de cada pata del pic
#include <string.h>     //maneja cadena de textos en c
#include <stdint.h>     //libreria para usar tipos de datos que no soporta datos de forma directa 
#include "dallas.h"     //header del sensor



// Variables para la temperatura
char temp[] = "000.00 C\n\0";
uint16_t raw_temp;

const char saltoLinea[] = "\r\n";

void send_USART_data(char* comando);
void cargarTemperatura();
void enviarTemperatura();

void main(void) {


    // Habilito las interrupciones 
    GIE = 1;
    PEIE = 1;
    INTE = 1;
    INTEDG = 0;
    RBIE = 1; // Habilito interrupts en parte alta puerto B

    // T0IE = 1; // Habilito el timer 0
    // Habilito entrada de interrupcion

    TRISB0 = 1;
    // Configuracion para el puerto Serial
    TRISB2 = 0; // TX
    TRISB1 = 1; // RX
    RCSTA = 0b10010000; // 0x90 (SPEN RX9 SREN CREN ADEN FERR OERR RX9D)
    TXSTA = 0b00100000; // 0x20 (CSRC TX9 TXEN SYNC - BRGH TRMT TX9D)
    SPBRG = 25; // Con esta configuración obtengo 9600 baudios

    // Sensor de corriente
    TRISB5 = 1; // Pull Down

    // Prender y apagar LED
    TRISA1 = 0;
    RA1 = 0;
    __delay_ms(1000);
    RA1 = 1;
    __delay_ms(1000);
    RA1 = 0;
    
    TRISA3 = 1;  //Se configura RA3 para consultar estado de puerta 
    TRISA1 = 1;
    
    int puertaAbierta = 0;
    int sensorCorriente = 0;

    int banderaEnviarTemperatura = 0;
    while (1) {
        banderaEnviarTemperatura++;
        if (banderaEnviarTemperatura == 5) {    //Sacar de aqui para usar un timer
            enviarTemperatura();
            banderaEnviarTemperatura = 0;
        }
        if (RA3==1 && puertaAbierta==0){    //consulta estado de puerta
            puertaAbierta = 1;
            send_USART_data("puerta");
            __delay_ms(250);
            send_USART_data("abierta");     //agragar timer 1 para buzzer 10seg
            enviarTemperatura(); 
            }
        if (RA3==0 && puertaAbierta==1){
            puertaAbierta = 0;
            send_USART_data("puerta");
            __delay_ms(250);
            send_USART_data("cerrada");     //parar el timer del buzzer
            enviarTemperatura(); 
        }
        if (RA1==1 && sensorCorriente==0){    //consulta estado de Sensor de Corriente
            sensorCorriente = 1;
            send_USART_data("corriente");
            __delay_ms(250);
            send_USART_data("si");
                  
            }
        if (RA1==0 && sensorCorriente==1){
            sensorCorriente = 0;
            send_USART_data("corriente");
            __delay_ms(250);
            send_USART_data("no");
            
        }
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
    send_USART_data("temperatura");
    __delay_ms(250);                       //Timer 0 para envinar esto 5 min
    send_USART_data(&temperaturaNueva);

}

void send_USART_data(char* comando) {
    int i = 0;
    while (comando[i] != '\0') {
        while (!TXIF)
            continue;
        TXREG = comando[i];
        i++;
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