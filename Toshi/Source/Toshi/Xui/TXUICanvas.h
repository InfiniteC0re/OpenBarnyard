#pragma once
#include "Toshi/Xui/TXUIElement.h"

namespace Toshi
{
	class TXUICanvas : public TXUIElement
	{
	public:
		virtual void SetDimensions(float a_X, float a_Y) override;
	};

	class XURXUICanvasData : public XURXUIElementData
	{
	public:
		static constexpr const char* sm_sTypeInfo = "XURXUIControlData";
	
	public:
		virtual ~XURXUICanvasData() = default;
		
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}