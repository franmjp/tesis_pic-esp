/*
 * File:   Utils.c
 * Author: Emilio
 *
 * Created on 2 de octubre de 2021, 14:26
 */


#include "Utils.h"

void send_USART_data_custom(char* comando) {
    int i = 0;
    while (comando[i] != '\0') {
        EUSART_Write(comando[i]);
        i++;
    }
}
