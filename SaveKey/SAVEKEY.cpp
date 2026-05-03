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
	HAL_UART_Receive_IT(uart,Port->Buffer,BUFFER_SIZE);
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

void SaveKeyProtocol::CheckTimeOut(void)
{
	if(Port->TimeOut > 0)
	{
		Port->TimeOut--;
		if(Port->TimeOut == 0)
		{
			__HAL_UART_DISABLE_IT(uart, UART_IT_RXNE);
			/* Check if a transmit process is ongoing or not */
      if(uart->RxState == HAL_UART_STATE_BUSY_TX_RX) {
        uart->RxState = HAL_UART_STATE_BUSY_TX;
      }
      else{
        /* Disable the UART Parity Error Interrupt */
        __HAL_UART_DISABLE_IT(uart, UART_IT_PE);
        __HAL_UART_DISABLE_IT(uart, UART_IT_ERR);
        uart->RxState = HAL_UART_STATE_READY;
      }
			Port->Len = uart->RxXferSize - uart->RxXferCount;
			Port->LineState = Recieved;
		}
	}
}

void SaveKeyProtocol::Process(void)
{
	if(Port->LineState == Nothing)
		return;
	DataRecieved(Port);
	Port->LineState = Nothing;
	HAL_UART_Receive_IT(uart,Port->Buffer,BUFFER_SIZE);
}

void SaveKeyProtocol::SetID(uint8_t _ID)
{
	ID = _ID;
}

uint8_t SaveKeyProtocol::GetID(void)
{
	return ID;
}
