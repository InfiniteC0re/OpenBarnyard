#include "ToshiPCH.h"
#include "TXUICanvas.h"

bool Toshi::XURXUICanvasData::Load(Toshi::TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);
	a_pData++;
	return true;
}
