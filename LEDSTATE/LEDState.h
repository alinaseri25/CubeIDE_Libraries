#ifndef LEDSTATE_H
#define LEDSTATE_H

//#include "stm32f1xx_hal.h"
#include "main.h"

#define NumberOfLEDs	6

typedef enum
{
	LEDOff				= 0x00,
	LEDOn								,
	LEDBlink						
}LEDStates;

typedef enum
{
	NoWorking				= 0x00,
	OnWorking
}WorkState;

typedef struct
{
	WorkState	Working;
	LEDStates	State;
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
	uint32_t NextTick;
	int8_t blinkCount;
}EachLEDState;

class LEDS
{
	public:
		LEDS();
		void ConfigLEDs(uint8_t _LEDNumber,GPIO_TypeDef *_GPIOx,uint16_t _GPIO_Pin);
		void LEDToggleState(uint8_t _LEDNumber);
		void AllLEDsOn(void);
		void AllLEDsOff(void);
		void AllLEDsBlink(int8_t _blinkCount = -1);
	
		void SetLEDState(uint8_t _LEDNumber,LEDStates _State,int8_t _blinkCount = -1);
	
		LEDStates GetLEDState(uint8_t _LEDNumber);
	
		void DoWork(void);
	private:
		EachLEDState LEDs[NumberOfLEDs];
		uint32_t LEDBlinkTime;
};

#endif
