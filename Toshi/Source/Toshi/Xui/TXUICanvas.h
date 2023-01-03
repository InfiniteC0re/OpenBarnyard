#pragma once
#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUICanvas
	{

	};

	class XURXUICanvasData : public XURXUIElementData
	{
	
		static constexpr const char* sm_sTypeInfo = "XURXUIControlData";
	
		virtual ~XURXUICanvasData() = default;

	public:
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}


