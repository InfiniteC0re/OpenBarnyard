#pragma once
#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUICanvas
	{

	};

	class XURXUICanvasData : public XURXUIElementData
	{
		bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}


