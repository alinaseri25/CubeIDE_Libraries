#include "SAVEKEY.h"
#include "main.h"

__weak void DataRecieved(SerialPort	*_Port)
{
	
}

SaveKeyProtocol::SaveKeyProtocol(UART_HandleTypeDef *_uart,GPIO_TypeDef *_GPIOx,uint16_t _GPIO_Pin,SerialPort	*_Port)
{
	uart = _uart;
	GPIOx = _GPIOx;
	GPIO_Pin = _GPIO_Pin;
	State = LineFree;
	Port = _Port;
}

void SaveKeyProtocol::init(void)
{
	LineFreed();
	HAL_UART_Receive_DMA(uart, this->getRxBuffer(), rxBufferSize);
	__HAL_UART_ENABLE_IT(uart, UART_IT_IDLE);
}

HAL_StatusTypeDef SaveKeyProtocol::SendCommand(HeaderType _Type,KeyState _state,uint8_t _ID,KeyNum	_KeySelect)
{
	KeyHeaderString = (KeyHeader *)TxBuffer;
	KeyHeaderString->header = _Type;
	KeyHeaderString->state = _state;
	KeyHeaderString->ID = _ID;
	KeyHeaderString->KeySelect = _KeySelect;
	KeyHeaderString->checksum = (uint8_t)(_Type + _state + _ID + _KeySelect);
	
	if(State == LineFree)
	{
		HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_SET);
		
		if(HAL_UART_Transmit_DMA(uart,TxBuffer,5) != HAL_OK)
		{
			return HAL_ERROR;
		}
		State = LineBusy;
	}
	else
	{
		return HAL_BUSY;
	}
	
	return HAL_OK;
}

void SaveKeyProtocol::LineFreed(void)
{
	State = LineFree;
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_RESET);
}

void SaveKeyProtocol::Process(void)
{
	if(Port->LineState == Nothing)
		return;
	DataRecieved(Port);
	Port->LineState = Nothing;
	HAL_UART_Receive_IT(uart,Port->Buffer,BUFFER_SIZE);
}

void SaveKeyProtocol::rxCompleted(void)
{

	uint16_t newDataLen = 0;

	uint16_t currentPos = rxBufferSize - __HAL_DMA_GET_COUNTER(uart->hdmarx);
	if(currentPos == lastPos)
	{
		HAL_UART_Receive_DMA(uart, this->getRxBuffer(), rxBufferSize);
		__HAL_UART_ENABLE_IT(uart, UART_IT_IDLE);
		lastPos = 0;
		Port->Len = newDataLen;
		Port->LineState = Nothing;
		return;
	}
	else if(currentPos > lastPos)
	{
		newDataLen = currentPos - lastPos;
		memcpy(Port->Buffer, rxBuffer + lastPos, newDataLen);
	}
	else
	{
		uint16_t firstPart = rxBufferSize - lastPos;
		uint16_t secondPart = currentPos;

		memcpy(Port->Buffer, rxBuffer + lastPos, firstPart);
		memcpy(Port->Buffer + firstPart, rxBuffer, secondPart);

		newDataLen = firstPart + secondPart;
	}

	lastPos = currentPos;

	Port->Len = newDataLen;
	Port->LineState = Recieved;
}

void SaveKeyProtocol::SetID(uint8_t _ID)
{
	ID = _ID;
}

uint8_t SaveKeyProtocol::GetID(void)
{
	return ID;
}

uint8_t* SaveKeyProtocol::getRxBuffer(void)
{
	return rxBuffer;
}
