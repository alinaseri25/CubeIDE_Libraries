#include "LnkList.h"

LinkedList::LinkedList()
{
	First = NULL;
}

void LinkedList::FetchNextNode(KeyHeader *_key)
{
	Node *CurNode;
	if(First == NULL)
	{
		return;
	}
	_key->checksum = First->KeyHeaderString.checksum;
	_key->header = First->KeyHeaderString.header;
	_key->ID = First->KeyHeaderString.ID;
	_key->KeySelect = First->KeyHeaderString.KeySelect;
	_key->state = First->KeyHeaderString.state;
	CurNode = First ;
	First = First->Next;
	delete CurNode;
}

HAL_StatusTypeDef  LinkedList::SetCommand(HeaderType _Type,KeyState _state,uint8_t _ID,KeyNum	_KeySelect)
{
	Node *NewNode = new Node();
	
	if(First == NULL)
	{
		Last = First = NewNode;
	}
	Last->Next = NewNode;
	Last = NewNode;
	Last->Next = NULL;
	
	Last->KeyHeaderString.header = _Type;
	Last->KeyHeaderString.state = _state;
	Last->KeyHeaderString.ID = _ID;
	Last->KeyHeaderString.KeySelect = _KeySelect;
	Last->KeyHeaderString.checksum = (uint8_t)(uint8_t)(_Type + _state + _ID + _KeySelect);
	
	return HAL_OK;
}

void LinkedList::SendNext(SaveKeyProtocol *_Protocol)
{
	if(First != NULL)//_Protocol->GetLineState() == LineFree &&
	{
		FetchNextNode(&CurrentKey);
		_Protocol->SendCommand(CurrentKey.header,CurrentKey.state,CurrentKey.ID,CurrentKey.KeySelect);
	}
}
