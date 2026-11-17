#include "jsonpacket.hpp"

Packets::Packets(void)
{
	headerSize = sizeof(DataHeader);
	rng_state = HAL_GetTick();
}

uint8_t *Packets::getrxBuffer(void)
{
	return rxBuffer;
}

uint32_t Packets::getrxBufferSize(void)
{
	return rxBufferSize;
}

void Packets::setRxLoopState(bool _rxLoop)
{
	isRxLoop = _rxLoop;
}

void Packets::readRxPacket(UART_HandleTypeDef *huart,uint8_t **_jsonBody,uint32_t *_jsonLen,uint8_t **_payload,uint32_t *_payloadLen)
{
	//--------------------------
	if(huart == NULL && isRxLoop)
	{
		return;
	}

	uint8_t *tempPtr;
	uint16_t newDataLen = 0;

	//-------------------------

	if(isRxLoop)
	{
		uint16_t currentPos = rxBufferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);

		if(currentPos == lastPos)
		{
			HAL_UART_Receive_DMA(huart, this->getrxBuffer(), this->getrxBufferSize());
			__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
			lastPos = 0;
			return;
		}

		if(currentPos > lastPos)
		{
			newDataLen = currentPos - lastPos;
			memcpy(tempData, rxBuffer + lastPos, newDataLen);
		}
		else
		{
			uint16_t firstPart = rxBufferSize - lastPos;
			uint16_t secondPart = currentPos;

			memcpy(tempData, rxBuffer + lastPos, firstPart);
			memcpy(tempData + firstPart, rxBuffer, secondPart);

			newDataLen = firstPart + secondPart;
		}

		tempPtr = tempData;
		lastPos = currentPos;
	}
	else
	{
		tempPtr = rxBuffer;
		newDataLen = rxBufferSize;
	}

	uint16_t i = 0;

	while(i + headerSize < newDataLen)
	{
		DataHeader *header = (DataHeader *)(tempPtr + i);

		if(header->startBytes != startByte)
		{
			i++;
			continue;
		}

		uint32_t packetSize = headerSize + header->jsonSize + header->payloadSize;

		if(packetSize > newDataLen)
		{
			// packet کامل هنوز نرسیده
			return;
		}

		if((header->jsonSize + header->payloadSize) > (rxBufferSize - headerSize))
		{
			return;
		}

		*_jsonBody = (uint8_t*)((uint8_t *)header + headerSize);
		*_jsonLen = header->jsonSize;

		*_payload = (uint8_t*)((uint8_t *)_jsonBody + header->jsonSize);
		*_payloadLen = header->payloadSize;

		return;
	}
}

bool Packets::addTxPacket(PacketPriority _priority,JsonDocument *json,uint8_t **_payload,uint32_t *_payloadLen)
{
	if(json == NULL || (txPackets.size() > txPacketsSize))
	{
		return false;
	}
	TXPacket packet;
	packet.state = WaitToSend;
	packet.priority = _priority;
	JsonDocument test;
	uint8_t *jsonBuffer = (packet.txBuffer + headerSize);
	uint32_t jsonLen = serializeJson(*json, jsonBuffer,txBufferSize - headerSize);

	DataHeader *header = (DataHeader *)packet.txBuffer;

	header->startBytes = 0xAA55AA55;
	header->jsonSize = jsonLen;
	header->payloadSize = (_payloadLen == NULL)?0:*_payloadLen;
	header->frameCRC = 0xFFFFFFFF;
	if(*_payloadLen > 0 && _payload != NULL)
	{
		memcpy((packet.txBuffer + headerSize + header->jsonSize),*_payload,*_payloadLen);
	}
	packet.timeToSendus = HAL_GetTick() + getRandomNumber(1, 10);
	packet.packetLen = headerSize + header->jsonSize + header->payloadSize;
	txPackets.push(packet);
	return true;
}

TXPacket *Packets::readTxPacket(void)
{
	if(txPackets.size() > 0)
	{
		return &txPackets.front();
	}
	return NULL;
}

void Packets::txPacketCompleted(void)
{
	__disable_irq();
	if(txPackets.size() > 0)
	{
		txPackets.pop();
	}
	__enable_irq();
}

void Packets::packetResetTimer(TXPacket *_packet)
{
	if(_packet != NULL)
	{
		switch (_packet->priority) {
			case High:
				_packet->timeToSendus = getRandomNumber(1, 3);
				break;
			case Medium:
				_packet->timeToSendus = getRandomNumber(3, 5);
				break;
			case Low:
				_packet->timeToSendus = getRandomNumber(5, 10);
				break;
			default:
				_packet->timeToSendus = getRandomNumber(5, 10);
				break;
		}
	}
}

uint32_t Packets::fast_rand(void)
{
	if (rng_state == 0) rng_state = HAL_GetTick();

	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 17;
	rng_state ^= rng_state << 5;
	return rng_state;
}

uint32_t Packets::getRandomNumber(uint32_t _min,uint32_t _max)
{
	uint32_t r = fast_rand();
	return _min + (r % (_max - _min + 1));
}
