#pragma once

#include "TXUIResource.h"
#include "TXUIElement.h"


namespace Toshi
{
	class TXUIFigure
	{
	};


	class XURXUIFigureData : public XURXUIElementData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUIFigureData";

	public:
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t propType);

		virtual bool IsFloatPropType(uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};

	class XURXUIFillData : public XURXUIObjectData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUIFillData";

		/* 0 */ uint32_t m_fillType;
		/* 1 */ uint32_t m_fillColor;
		/* 2 */ uint16_t m_fillTextureFileName;
		/* 4 */ uint32_t m_fillTranslation;
		/* 5 */ uint32_t m_fillScale;
		/* 6 */ float m_fillRotation;
		/* 7 */ uint32_t m_fillWrapX;
		/* 8 */ uint32_t m_fillWrapY;
		/* 9 */ uint32_t m_fillBrushFlags;


	public:
		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t propType);

		virtual bool IsFloatPropType(uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};

	class XURXUIStrokeData : public XURXUIObjectData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUIStrokeData";

		/* 0 */ float m_strokeWidth;
		/* 1 */ uint32_t m_strokeColor;

	public:

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t propType);

		virtual bool IsFloatPropType(uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};

	class XURXUIGradientData : public XURXUIObjectData
	{
		static constexpr const char* sm_sTypeInfo = "XURXUIGradientData";

		/* 0 */ bool m_radial;
		/* 1 */ uint32_t m_numStops;
		/* 2 */ uint32_t m_stopPos;
		/* 3 */ uint32_t m_stopColor;

		uint32_t* m_stops;
		uint32_t* m_stops2;

	public:

		virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

		virtual bool IsColourPropType(uint32_t propType);

		virtual bool IsFloatPropType(uint32_t propType);

		virtual uint32_t GetTimelinePropSize(uint32_t propType);

		virtual bool TranslateTimelineProp(const char* param_1, uint32_t& param_2, uint32_t& param_3);
		virtual bool ValidateTimelineProp(uint32_t param_2);

		virtual bool Load(TXUIResource& resource, uint8_t*& a_pData);
	};
}

