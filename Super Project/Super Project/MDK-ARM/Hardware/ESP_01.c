#include "usart.h"
#include <stdio.h>
#include <string.h>

extern char usart_data[128];
const char ESP_01_Command[][50] = {
    {"AT+CIPSTART=\"TCP\",\"192.168.137.1\",8081\r\n"},
    {"AT+CIPMODE=1\r\n"},
    {"AT+CIPSEND\r\n"}
    //{"ESP_01 Init Success!"}
};

void ESP_01_Init(void){
    
    for(int i=0;i<3;i++){
        HAL_UART_Transmit(&huart2,"\0",1,100);
        HAL_UART_Transmit(&huart2,(uint8_t *)ESP_01_Command[i],strlen(ESP_01_Command[i])+5,100);
        printf(ESP_01_Command[i]);
        printf(usart_data);
        memset(usart_data,0,128);
        HAL_Delay(2000);
    }
}