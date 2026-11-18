#ifndef STRUCTURES_H
#define STRUCTURES_H

//#include "stm32f1xx_hal.h"
#include "main.h"

#define BUFFER_SIZE 100
#define	UARTTimeOut	10

typedef enum
{
	Nothing		=0,
	Recieved
}UARTSTATE;


typedef struct 
{
	uint8_t Buffer[BUFFER_SIZE];
	uint8_t Len;
	uint8_t TimeOut;
	UARTSTATE	LineState;
}SerialPort;

#endif
