#include "ToshiPCH.h"
#include "TXUICanvas.h"

namespace Toshi {

	TBOOL XURXUICanvasData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		XURXUIElementData::Load(resource, a_pData);
		a_pData++;
		return TTRUE;
	}

	void TXUICanvas::SetDimensions(float a_X, float a_Y)
	{

	}

}
