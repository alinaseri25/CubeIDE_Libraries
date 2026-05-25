#include "KeyBoard.h"

void MKeyBoard::SetRows(RowsCols *_Rows,uint8_t _RowNumber)
{
	this->Key_Rows = _Rows;
	this->RowNumber = _RowNumber;
	for(uint8_t _Count = 0 ; _Count < this->RowNumber ; _Count++)
	{
		set_gpio_output(this->Key_Rows[_Count].GPIO,this->Key_Rows[_Count].GPIO_Pin);
	}
}

void MKeyBoard::SetCols(RowsCols *_Cols,uint8_t _ColNumber)
{
	this->Key_Cols = _Cols;
	this->ColNumber = _ColNumber;
	for(uint8_t _Count = 0 ; _Count < this->RowNumber ; _Count++)
	{
		set_gpio_input(this->Key_Cols[_Count].GPIO,this->Key_Cols[_Count].GPIO_Pin);
	}
}

uint32_t MKeyBoard::ReadKeyBoard(bool invert)
{
	uint32_t RetVal = 0,TempVal = 0;
	for(uint8_t _Count_i = 0 ; _Count_i < this->RowNumber ; _Count_i++)
	{
		HAL_GPIO_WritePin(Key_Rows[_Count_i].GPIO,Key_Rows[_Count_i].GPIO_Pin,GPIO_PIN_SET);
	}
	
	for(uint8_t _Count_j = 0 ; _Count_j < this->RowNumber ; _Count_j++)
	{
		HAL_GPIO_WritePin(Key_Rows[_Count_j].GPIO,Key_Rows[_Count_j].GPIO_Pin,GPIO_PIN_RESET);
		
		for(uint8_t _Cols = 0 ; _Cols < this->ColNumber ; _Cols++)
		{
			if(HAL_GPIO_ReadPin(Key_Cols[_Cols].GPIO,Key_Cols[_Cols].GPIO_Pin) == GPIO_PIN_RESET)
			{
				RetVal |= ((0x00000001)<<((_Count_j * this->RowNumber)+ _Cols));
			}
		}
		
		HAL_GPIO_WritePin(Key_Rows[_Count_j].GPIO,Key_Rows[_Count_j].GPIO_Pin,GPIO_PIN_SET);
	}
	if(invert)
	{
		TempVal = ((RetVal >> 12) & 0x000FFF) | ((RetVal << 12) & 0xFFF000);
		RetVal = TempVal;
	}
	return RetVal;
}

void MKeyBoard::set_gpio_output(GPIO_TypeDef *_GPIOx,uint16_t _GPIO_Pin)
{
	GPIO_InitStruct.Pin = _GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(_GPIOx,&GPIO_InitStruct);
}
	
void MKeyBoard::set_gpio_input(GPIO_TypeDef *_GPIOx,uint16_t _GPIO_Pin)
{
	GPIO_InitStruct.Pin = _GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(_GPIOx,&GPIO_InitStruct);
}
