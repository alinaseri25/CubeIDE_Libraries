#ifndef SAVEKEY_H
#define SAVEKEY_H

#include "stm32f1xx_hal.h"
#include "Structures.h"
#include <string.h>

#define rxBufferSize 100

typedef enum
{
	Key01						= 0x11,
	Key02						,
	Key03						,
	Key04						,
	Key05						,
	Key06						,
	Key07						,
	Sen01						= 0x01,
	Sen02						,
	Sen03						,
	Sen04						,
	Sen05						,
	Sen06						,
	Open1						= 0x31,
	Stop1						,
	Close1						,
	Open2						,
	Stop2						,
	Close2						,
	Open3						,
	Stop3						,
	Close3						,
	Valve1						= 0x40,
	Fan1						,
	Fan2						,
	Fan3
}KeyNum;

typedef enum
{
	Key							= 0x11,
	Thermostate					= 0x01,
	Curtain						= 0x13
}HeaderType;

typedef struct
{
	GPIO_PinState				PinState;
	uint8_t 					ID;
	KeyNum						KeyNumber;
	HeaderType					KeyType;
	bool						virtualKey;
	uint8_t						DingDongTime;
}switchState;

typedef enum
{
	ReqOn						= 0x81,
	ReqOff						,
	ResOff						= 0x01,
	ResOn							
}KeyState;

typedef struct
{
	HeaderType					header;
	KeyState					state;
	uint8_t 					ID;
	KeyNum						KeySelect;
	uint8_t						checksum;
}KeyHeader;

typedef enum
{
	Heater						= 0x00
}ThermostatState;

typedef enum
{
	One							= 0x01,
	Two							,
	three						,
	Auto
}FanState;

typedef struct
{
	HeaderType					header;
	uint8_t						Fixed0;
	uint8_t						Fixed1;
	uint8_t 					ID;
	ThermostatState				State;
	uint8_t						Fixed2;
	FanState					Fanstate;
	uint8_t						TempSetPoint;
	uint8_t						CurrentTemp;
	uint8_t						FanSpeed_ValveState;
	uint8_t						checksum;
}ThermostatHeader;

typedef struct
{
	HeaderType					header;
	KeyState					state;
	uint8_t 					ID;
	KeyNum						KeySelect;
	uint8_t						Fixed0;
	uint8_t						ID_Replay;
	KeyNum						KeySelect_Replay;
	uint8_t						checksum;
}ScenarioHeader;

typedef enum
{
	LineFree					= 0x00,
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
		void LineFreed(void);
		void Process(void);
		void rxCompleted(void);
		void SetID(uint8_t _ID);
		uint8_t GetID(void);
		uint8_t* getRxBuffer(void);
	private:
		LineState State;
		UART_HandleTypeDef *uart;
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
		uint8_t TxBuffer[100];
		uint8_t rxBuffer[100];
		KeyHeader	*KeyHeaderString;
		SerialPort	*Port;
		uint8_t		ID;
		uint16_t lastPos = 0;
};

#endif
