#ifndef SAVEKEY_H
#define SAVEKEY_H

#include "stm32f1xx_hal.h"
#include "Structures.h"
#include <string.h>

#define rxBufferSize 100

#pragma pack(push, 1)

/*!
    \enum KeyNum
    \brief Defines all key, sensor, actuator and fan identifiers used in the protocol.

    \value Key01      Main key identifier 1 (Value: 0x11).
    \value Key02      Main key identifier 2.
    \value Key03      Main key identifier 3.
    \value Key04      Main key identifier 4.
    \value Key05      Main key identifier 5.
    \value Key06      Main key identifier 6.
    \value Key07      Main key identifier 7.

    \value Sen01      Sensor identifier 1 (Value: 0x01).
    \value Sen02      Sensor identifier 2.
    \value Sen03      Sensor identifier 3.
    \value Sen04      Sensor identifier 4.
    \value Sen05      Sensor identifier 5.
    \value Sen06      Sensor identifier 6.

    \value Open1      Curtain/door 1 open command (Value: 0x31).
    \value Stop1      Curtain/door 1 stop command.
    \value Close1     Curtain/door 1 close command.
    \value Open2      Curtain/door 2 open command.
    \value Stop2      Curtain/door 2 stop command.
    \value Close2     Curtain/door 2 close command.
    \value Open3      Curtain/door 3 open command.
    \value Stop3      Curtain/door 3 stop command.
    \value Close3     Curtain/door 3 close command.

    \value Valve1     Valve control identifier 1 (Value: 0x40).
    \value Fan1       Fan control identifier 1.
    \value Fan2       Fan control identifier 2.
    \value Fan3       Fan control identifier 3.
*/
typedef enum : uint8_t
{
    Key01                       = 0x11,
    Key02                       ,
    Key03                       ,
    Key04                       ,
    Key05                       ,
    Key06                       ,
    Key07                       ,
    Sen01                       = 0x01,
    Sen02                       ,
    Sen03                       ,
    Sen04                       ,
    Sen05                       ,
    Sen06                       ,
    Open1                       = 0x31,
    Stop1                       ,
    Close1                      ,
    Open2                       ,
    Stop2                       ,
    Close2                      ,
    Open3                       ,
    Stop3                       ,
    Close3                      ,
    Valve1                      = 0x40,
    Fan1                        ,
    Fan2                        ,
    Fan3
} KeyNum;

/*!
    \enum HeaderType
    \brief Protocol frame header types to distinguish incoming packets.

    \value Key              Header type for key and actuator control messages (0x11).
    \value Thermostat       Header type for thermostat configuration and status messages (0x01).
    \value Curtain          Header type for curtain control messages (0x13).
*/
typedef enum : uint8_t
{
    Key                         = 0x11,
    Thermostat                  = 0x01,
    Curtain                     = 0x13
} HeaderType;

/*!
    \enum KeyState
    \brief Key request and response state actions.

    \value ReqOff            Request to turn target key/relay OFF (0x81).
    \value ReqOn           Request to turn target key/relay ON (0x82).
    \value ResOff           Response indicating target state is OFF (0x01).
    \value ResOn            Response indicating target state is ON (0x02).
*/
typedef enum : uint8_t
{
    ReqOff                      = 0x81,
    ReqOn                       ,
    ResOff                      = 0x01,
    ResOn
} KeyState;

/*!
    \struct KeyHeader
    \brief Protocol frame structure for key-related configuration and control messages.
*/
typedef struct
{
    HeaderType header;      /*!< Header identifier (typically HeaderType::Key) */
    KeyState state;         /*!< Requested or reported key state */
    uint8_t ID;             /*!< Device hardware identifier */
    KeyNum KeySelect;       /*!< Target key, sensor or actuator index */
    uint8_t checksum;       /*!< Packet verification checksum */
} KeyHeader;

/*!
    \enum ThermostatState
    \brief Operational state of the thermostat controller.
*/
typedef enum : uint8_t
{
    StateNoChange               = 0xFF, /*!< Do not modify the current state */
    Off                         = 0x00, /*!< Thermostat turned off */
    Heater                      = 0x01, /*!< Heating mode active */
    Cool                        = 0x02, /*!< Cooling mode active */
    constantTemp                = 0x03, /*!< Constant temperature hold mode */
    floorHeating                = 0x04, /*!< Underfloor heating mode */
    fastHeat                    = 0x05, /*!< Fast heating boost mode */
    freshDry                    = 0x06, /*!< Fresh air circulation and dehumidification mode */
    dry                         = 0x07  /*!< Dehumidification dry mode */
} ThermostatState;

/*!
    \enum FanState
    \brief Thermostat fan speed levels and control modes.
*/
typedef enum : uint8_t
{
    FanNoChange                 = 0xFF, /*!< Do not modify current fan speed */
    One                         = 0x01, /*!< Fan speed level 1 (Low) */
    Two                         = 0x02, /*!< Fan speed level 2 (Medium) */
    Three                       = 0x03, /*!< Fan speed level 3 (High) */
    Auto                        = 0x04  /*!< Automatic speed adjustment */
} FanState;

/*!
    \struct ThermostatHeader
    \brief Protocol frame representing the comprehensive status of a thermostat device.
*/
typedef struct
{
    HeaderType header;                  /*!< Header identifier (HeaderType::Thermostat) */
    uint8_t Fixed0;                     /*!< Protocol-defined fixed byte 0 */
    uint8_t Fixed1;                     /*!< Protocol-defined fixed byte 1 */
    uint8_t ID;                         /*!< Unique thermostat device identifier */
    ThermostatState State;              /*!< Active operation mode */
    uint8_t Fixed2;                     /*!< Protocol-defined fixed byte 2 */
    FanState Fanstate;                  /*!< Current fan speed selection */
    uint8_t TempSetPoint;               /*!< Desired setpoint temperature */
    uint8_t CurrentTemp;                /*!< Ambient temperature read by the sensor */
    uint8_t FanSpeed_ValveState;        /*!< Combined state: first 4 bits for valve control, last 4 bits for fan speed */
    uint8_t checksum;                   /*!< Packet verification checksum */
} ThermostatHeader;

/*!
    \struct ThermostatConfigure
    \brief Configuration command packet sent to a thermostat unit.
*/
typedef struct
{
    HeaderType header;                  /*!< Header identifier (HeaderType::Thermostat) */
    uint8_t Fixed0;                     /*!< Protocol-defined fixed byte (typically 0x85) */
    uint8_t Fixed1;                     /*!< Protocol-defined fixed byte (typically 0x00) */
    uint8_t ID;                         /*!< Target thermostat device identifier */
    ThermostatState State;              /*!< Mode of operation to set */
    uint8_t Fixed2;                     /*!< Protocol-defined fixed byte (typically 0xFF) */
    FanState Fanstate;                  /*!< Fan speed level to set */
    uint8_t TempSetPoint;               /*!< Target setpoint temperature to apply */
    uint8_t checksum;                   /*!< Packet verification checksum */
} ThermostatConfigure;

/*!
    \struct ThermostatControl
    \brief Basic command packet to trigger thermostat states.
*/
typedef struct
{
    HeaderType header;                  /*!< Header identifier (HeaderType::Thermostat) */
    KeyState state;                     /*!< Target key or control action state */
    uint8_t Fixed0;                     /*!< Protocol-defined fixed byte (typically 0x00) */
    uint8_t ID;                         /*!< Target thermostat device identifier */
    uint8_t checksum;                   /*!< Packet verification checksum */
} ThermostatControl;

/*!
    \struct ScenarioHeader
    \brief Protocol frame used to execute, record, or relay specific system scenarios.
*/
typedef struct
{
    HeaderType header;                  /*!< Header identifier */
    KeyState state;                     /*!< Operational state of the scenario */
    uint8_t ID;                         /*!< Scenario controller device ID */
    KeyNum KeySelect;                   /*!< Selected scenario trigger key */
    uint8_t Fixed0;                     /*!< Reserved fixed byte */
    uint8_t ID_Replay;                  /*!< Target device ID for replay execution */
    KeyNum KeySelect_Replay;            /*!< Target key identifier to execute in replay */
    uint8_t checksum;                   /*!< Packet verification checksum */
} ScenarioHeader;

/*!
    \struct KeysReport
    \brief Asynchronous or requested report containing system inputs and outputs.
*/
typedef struct
{
    HeaderType header;                  /*!< Header identifier */
    KeyState state;                     /*!< System report state context */
    uint8_t ID;                         /*!< Reporting device identifier */
    uint8_t keys;                       /*!< Bitmask or state values representing physical keys */
    uint8_t sens;                       /*!< Sensor state values or inputs */
    uint8_t unknown0;                   /*!< Reserved for future protocol additions */
    uint8_t unknown1;                   /*!< Reserved for future protocol additions */
    uint8_t unknown2;                   /*!< Reserved for future protocol additions */
    uint8_t unknown3;                   /*!< Reserved for future protocol additions */
    uint8_t unknown4;                   /*!< Reserved for future protocol additions */
    uint8_t checksum;                   /*!< Packet verification checksum */
} KeysReport;

typedef struct
{
	GPIO_PinState				PinState;
	uint8_t 					ID;
	KeyNum						KeyNumber;
	HeaderType					KeyType;
	bool						virtualKey;
	uint8_t						DingDongTime;
}switchState;

#pragma pack(pop)

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
