#include "ToshiPCH.h"
#include "TXUIElement.h"

void Toshi::XURXUIObjectData::Load(uint8_t* a_pData)
{
	unk = PARSEWORD_BIG(a_pData);
	a_pData += 2;
}

void Toshi::XURXUIObjectData::LoadChildren(uint8_t* a_pData)
{
	TASSERT(PARSEDWORD(a_pData) < (1 << 8), "Not a Word");

}
