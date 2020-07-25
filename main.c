// CONFIG
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital input, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has digital I/O function, HV on MCLR must be used for programming)
#pragma config CPD = ON         // Data EE Memory Code Protection bit (Data memory code-protected)
#pragma config CP = ON          // Flash Program Memory Code Protection bit (0000h to 07FFh code-protected)

#define _XTAL_FREQ 16000000 // 16 Mhz


#include <xc.h>         //libreria propia de pics de 8 bits, estan las ctes de cada pata del pic
#include <string.h>     //maneja cadena de textos en c
#include <stdint.h>     // libreria para usar tipos de datosque no soporta datos de forma directa 
#include "dallas.h"     //header del sensor



// Variables para la temperatura
char temp[] = "000.00 C\n\0";
uint16_t raw_temp;

const char comando1[]="AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n";      //comandos AT permite iniciar comunicacion tipo TCP al al dir y puerto que especifico, inicio el canal de cominicacion
const char comando2[]="AT+CIPSEND=51\r\n";                                      //comando que especifica cuantos bytes le voy a mandar
const char comando3[]="GET /update?api_key=S4T6NHJ8JYCIRPM6&field1=";           
const char saltoLinea[]="\r\n";
const char comando4[]="AT+CIPCLOSE\r\n";                                        //cierro la comunicacion TCP, sino se puede colapsar el modulo   
const char comandoA[]="AT+CWMODE=3";                                            //Especifico que modo va a adoptar, 3 es access point y station
const char comandoB[]="AT+CWJAP=\"Fibertel WiFi805 2.4GHz\",\"0043180983\"";    


void configureESP8266();
void send_USART_data(char* comando);
void cargarTemperatura();

void main(void) {
    
    // Configuracion para el puerto Serial
    TRISB2 = 0; // TX
    TRISB1 = 1; // RX
    RCSTA = 0b10010000; // 0x90 (SPEN RX9 SREN CREN ADEN FERR OERR RX9D)
    TXSTA = 0b00100000; // 0x20 (CSRC TX9 TXEN SYNC - BRGH TRMT TX9D)
    SPBRG = 25; // Con esto tengo 9600 baudios
    

    TRISA1 = 0;
    RA1 = 0;
    __delay_ms(1000);
    RA1 = 1;
    __delay_ms(1000);
    RA1 = 0;
    
    // Inicio la comunicacion con el web server por el puerto 80
    
    char temperaturaNueva[6] = "     ";
    configureESP8266();
    int t = 0;
    
      
    while(1){
        //send_USART_data(&emilio);
        cargarTemperatura();
        temperaturaNueva[0] = temp[0];
        temperaturaNueva[1] = temp[1];
        temperaturaNueva[2] = temp[2];
        temperaturaNueva[3] = temp[3];
        temperaturaNueva[4] = temp[4];
        temperaturaNueva[5] = '\0';
        //send_USART_data(&temp);
        // Abro conexion
        send_USART_data(&comando1);
        __delay_ms(500);
        // Establezco cantidad a enviar
        send_USART_data(&comando2);
        __delay_ms(500);
        // Envio datos
        send_USART_data(&comando3);
        send_USART_data(&temperaturaNueva);
        send_USART_data(&saltoLinea);
        __delay_ms(500);
        //Mato conexion
        send_USART_data(&comando4);
        
        __delay_ms(1000);
        RA1 = 0;
        __delay_ms(1000);
        RA1 = 1;
    
        for(t=0;t<300000;t++){                                                  //tiempo de envio
        __delay_ms(1000);}
    }
}

void send_USART_data(char* comando){
    int i=0;
    while(comando[i] != '\0'){
        while(!TXIF)
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

void configureESP8266(){
    
    send_USART_data(&comandoA);
    __delay_ms(4000);
    send_USART_data(&comandoB);
    __delay_ms(4000);
}

