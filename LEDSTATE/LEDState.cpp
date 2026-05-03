#include "LEDState.h"

LEDS::LEDS()
{
	LEDBlinkTime = 500;
}

void LEDS::ConfigLEDs(uint8_t _LEDNumber,GPIO_TypeDef *_GPIOx,uint16_t _GPIO_Pin)
{
	LEDs[_LEDNumber].GPIOx = _GPIOx;
	LEDs[_LEDNumber].GPIO_Pin = _GPIO_Pin;
}

void LEDS::LEDToggleState(uint8_t _LEDNumber)
{
	(LEDs[_LEDNumber].State == LEDOff) ? LEDs[_LEDNumber].State = LEDOn : LEDs[_LEDNumber].State = LEDOff;
	LEDs[_LEDNumber].Working = OnWorking;
}

void LEDS::AllLEDsOn(void)
{
	for(uint8_t CurLED = 0;CurLED < NumberOfLEDs;CurLED++)
	{
		SetLEDState(CurLED,LEDOn);
	}
	DoWork();
}

void LEDS::AllLEDsOff(void)
{
	for(uint8_t CurLED = 0;CurLED < NumberOfLEDs;CurLED++)
	{
		SetLEDState(CurLED,LEDOff);
	}
	DoWork();
}

void LEDS::AllLEDsBlink(int8_t _blinkCount)
{
	AllLEDsOff();
	for(uint8_t CurLED = 0;CurLED < NumberOfLEDs;CurLED++)
	{
		SetLEDState(CurLED,LEDBlink,_blinkCount);
	}
}

void LEDS::SetLEDState(uint8_t _LEDNumber,LEDStates _State,int8_t _blinkCount)
{
	LEDs[_LEDNumber].State = _State;
	LEDs[_LEDNumber].blinkCount = (_blinkCount > 0)? (_blinkCount * 2) : (_blinkCount);
	LEDs[_LEDNumber].NextTick = HAL_GetTick();
	LEDs[_LEDNumber].Working = OnWorking;
}

LEDStates LEDS::GetLEDState(uint8_t _LEDNumber)
{
	return LEDs[_LEDNumber].State;
}

void LEDS::DoWork(void)
{
	for(uint8_t CurLED = 0;CurLED < NumberOfLEDs;CurLED++)
	{
		if(LEDs[CurLED].Working == OnWorking)
		{
			if(LEDs[CurLED].State == LEDBlink)
			{
				if(HAL_GetTick() >= LEDs[CurLED].NextTick)
				{
					if(LEDs[CurLED].blinkCount <= 0)
					{
						SetLEDState(CurLED,LEDOff);
						LEDs[CurLED].Working = NoWorking;
					}
					else
					{
						LEDs[CurLED].blinkCount--;
						HAL_GPIO_TogglePin(LEDs[CurLED].GPIOx,LEDs[CurLED].GPIO_Pin);
						LEDs[CurLED].NextTick = HAL_GetTick() + LEDBlinkTime;
					}
				}
			}
			else
			{
				HAL_GPIO_WritePin(LEDs[CurLED].GPIOx,LEDs[CurLED].GPIO_Pin,(GPIO_PinState)LEDs[CurLED].State);
				LEDs[CurLED].Working = NoWorking;
			}
		}		
	}
}

