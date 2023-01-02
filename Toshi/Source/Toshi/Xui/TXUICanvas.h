#pragma once
#include "TXUIElement.h"

namespace Toshi
{
	class TXUICanvas
	{

	};

	class XURXUICanvasData : public XURXUIElementData
	{
		bool Load(Toshi::TXUIResource& resource, uint8_t*& a_pData);
	};
}


