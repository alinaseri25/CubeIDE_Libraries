#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stm32f1xx_hal.h"
#include "dwt_stm32_delay.h"

typedef struct
{
	GPIO_TypeDef *GPIO;
	uint16_t GPIO_Pin;
}RowsCols;


class MKeyBoard
{
public:
	void SetRows(RowsCols *_Rows,uint8_t _RowNumber);
	void SetCols(RowsCols *_Cols,uint8_t _ColNumber);
	uint32_t ReadKeyBoard(bool invert = true);
private:
	void set_gpio_output(GPIO_TypeDef *GPIO,uint16_t _GPIO_Pin);
	void set_gpio_input(GPIO_TypeDef *_GPIOx,uint16_t _GPIO_Pin);
	uint8_t RowNumber,ColNumber;
	RowsCols *Key_Rows,*Key_Cols;
	GPIO_InitTypeDef GPIO_InitStruct;
};

#endif
