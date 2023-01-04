#include "ToshiPCH.h"
#include "TXUICanvas.h"

bool Toshi::XURXUICanvasData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);
	a_pData++;
	return true;
}

void Toshi::TXUICanvas::SetDimensions(float a_X, float a_Y)
{
}
