#ifndef SAVEKEY_H
#define SAVEKEY_H

#include "stm32f1xx_hal.h"
#include "Structures.h"

typedef enum
{
	Key01			= 0x11,
	Key02						,
	Key03						,
	Key04						,
	Key05						,
	Key06						,
	Key07						,
	Sen01			= 0x01,
	Sen02						,
	Sen03						,
	Sen04						,
	Sen05						,
	Sen06						
}KeyNum;

typedef enum
{
	ReqOn			= 0x81,
	ReqOff							,
	ResOff			= 0x01,
	ResOn							
}KeyState;


typedef enum
{
	Key						= 0x11,
	thermostate		= 0x01,
	Curtain				= 0x13
}HeaderType;

typedef struct
{
	HeaderType	header;
	KeyState		state;
	uint8_t 		ID;
	KeyNum			KeySelect;
	uint8_t			checksum;
}KeyHeader;

typedef enum
{
	LineFree		= 0x00,
	LineBusy					
}LineState;

__weak void DataRecieved(SerialPort	*_Port);


class SaveKeyProtocol
{
	public:
		//SaveKeyProtocol();
		SaveKeyProtocol(UART_HandleTypeDef *_uart,GPIO_TypeDef *_GPIOx,uint16_t _GPIO_Pin,SerialPort	*_Port);
		void init(void);
		HAL_StatusTypeDef SendCommand(HeaderType _Type,KeyState _state,uint8_t _ID,KeyNum	_KeySelect);
		void LineFreed(void);//use it in void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
		void CheckTimeOut(void);//use it in a 1ms
		void Process(void);//use it in a loop
		void SetID(uint8_t _ID);
		uint8_t GetID(void);
	private:
		LineState State;
		UART_HandleTypeDef *uart;
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
		uint8_t TxBuffer[100];
		KeyHeader	*KeyHeaderString;
		SerialPort	*Port;
		uint8_t		ID;
};

#endif
