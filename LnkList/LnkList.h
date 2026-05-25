#ifndef LNKLIST_H
#define LNKLIST_H

#include "stm32f1xx_hal.h"
#include "SAVEKEY.h"

struct Node
{
	KeyHeader	KeyHeaderString;
	Node *Next;
};

class LinkedList
{
	public:
		LinkedList();
		void FetchNextNode(KeyHeader *_key);
		HAL_StatusTypeDef SetCommand(HeaderType _Type,KeyState _state,uint8_t _ID,KeyNum	_KeySelect);
		void SendNext(SaveKeyProtocol *_Protocol);
	private:
		Node *First;
		Node *Last;
		KeyHeader CurrentKey;
};

#endif
