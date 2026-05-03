#ifndef JSONPACKETCPP_H
#define JSONPACKETCPP_H

#include "packetStructs.hpp"
#include "stm32f1xx_hal.h"
#include "ArduinoJson.h"
#include <string.h>
#include <queue>
#include <random>

#define rxBufferSize 300
#define txBufferSize 300
#define	txPacketsSize 2

enum PacketPriority{
	High			= 0x01,
	Medium			,
	Low
};

enum TXState
{
	Empty			= 0x01,
	WaitToSend		,
	WaitToSent		,
	WaitToResponse	,
	WaitToACK
};

struct TXPacket
{
	TXState state;
	PacketPriority priority;
	uint32_t timeToSendus;
	uint32_t packetLen;
	uint8_t txBuffer[txBufferSize];
};

class Packets{
public:
	explicit Packets(void);
	uint8_t *getrxBuffer(void);
	uint32_t getrxBufferSize(void);

	void setRxLoopState(bool _rxLoop);
	void readRxPacket(UART_HandleTypeDef *huart,uint8_t **_jsonBody = NULL,uint32_t *_jsonLen = NULL,uint8_t **_payload = NULL,uint32_t *_payloadLen = NULL);

	bool addTxPacket(PacketPriority _priority,JsonDocument *json = NULL,uint8_t **_payload = NULL,uint32_t *_payloadLen = NULL);
	TXPacket *readTxPacket(void);
	void txPacketCompleted(void);
	void packetResetTimer(TXPacket *_packet);

private:
	uint8_t currentTxQueue = 0;
	uint16_t headerSize = 0;
	bool isRxLoop = false;
	uint16_t lastPos = 0;
	uint8_t rxBuffer[rxBufferSize];
	uint8_t tempData[rxBufferSize];
	std::queue<TXPacket> txPackets;
	uint32_t rng_state;

	uint32_t fast_rand(void);
	uint32_t getRandomNumber(uint32_t _min,uint32_t _max);
};

#endif
