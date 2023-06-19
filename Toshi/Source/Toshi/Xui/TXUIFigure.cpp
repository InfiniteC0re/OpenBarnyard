#include "ToshiPCH.h"
#include "TXUIFigure.h"
#include "XURReader.h"

namespace Toshi
{
    TBOOL XURXUIFillData::IsColourPropType(uint32_t propType)
    {
        return propType == PropType_FillType;
    }

    TBOOL XURXUIFillData::IsFloatPropType(uint32_t propType)
    {
        return propType == PropType_FillRotation;
    }

    uint32_t XURXUIFillData::GetTimelinePropSize(uint32_t propType)
    {
        return propType != 2 ? 4 : 2;
    }

    TBOOL XURXUIFillData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
    {
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.FillType", propType, PropType_FillType);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.FillColor", propType, PropType_FillColor);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.TextureFileName", propType, PropType_FillTextureFileName);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Translation", propType, PropType_FillTranslation);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Scale", propType, PropType_FillScale);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Rotation", propType, PropType_FillRotation);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.WrapX", propType, PropType_FillWrapX);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.WrapY", propType, PropType_FillWrapY);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.BrushFlags", propType, PropType_FillBrushFlags);

        return TFALSE;
    }

    TBOOL XURXUIFillData::ValidateTimelineProp(uint32_t param_2)
    {
        return param_2 < PropType_NUMOF;
    }

    TBOOL XURXUIFillData::Load(TXUIResource& resource, uint8_t*& a_pData)
    {
        // TODO:!!!!! NOT FINISHED !!!!!!

        XURXUIObjectData::Load(resource, a_pData);
        
        if (*a_pData++ != 0)
        {
            XURReader reader(a_pData);
            if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

            reader.ReadProperty<XUI_EPT_USHORT32>(PropType_FillType, m_FillType);
            reader.ReadProperty<XUI_EPT_COLOR>(PropType_FillColor, m_FillColor);
            reader.ReadProperty<XUI_EPT_STRING>(PropType_FillTextureFileName, m_FillTextureFileName);
            
            if (reader.ShouldReadThisProp(PropType_Gradient))
            {
                TTODO("XURXUIGradientData");
            }

            reader.ReadProperty<XUI_EPT_VECTOR>(PropType_FillTranslation, m_FillTranslation);
            reader.ReadProperty<XUI_EPT_VECTOR>(PropType_FillScale, m_FillScale);
            reader.ReadProperty<XUI_EPT_FLOAT>(PropType_FillRotation, m_FillRotation);
            reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_FillWrapX, m_FillWrapX);
            reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_FillWrapY, m_FillWrapY);

            if (reader.ShouldReadThisProp(PropType_FillBrushFlags))
            {
                // there's nothing here in globs and de blob
            }

            if (reader.ShouldReadThisProp(PropType_Unknown2))
            {
                // there's nothing here in globs and de blob
                // m_FillRotation = *a_pData++;
            }
        }

        return TTRUE;
    }

    // StrokeData

    TBOOL XURXUIStrokeData::IsColourPropType(uint32_t propType)
    {
        return propType == 1;
    }

    TBOOL XURXUIStrokeData::IsFloatPropType(uint32_t propType)
    {
        return propType == 0;
    }

    uint32_t XURXUIStrokeData::GetTimelinePropSize(uint32_t propType)
    {
        return 4;
    }

    TBOOL XURXUIStrokeData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
    {
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Stroke.StrokeWidth", propType, PropType_StrokeWidth);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Stroke.StrokeColor", propType, PropType_StrokeColor);

        return TFALSE;
    }

    TBOOL XURXUIStrokeData::ValidateTimelineProp(uint32_t param_2)
    {
        return param_2 < 2;
    }

    TBOOL XURXUIStrokeData::Load(TXUIResource& resource, uint8_t*& a_pData)
    {
        XURXUIObjectData::Load(resource, a_pData);

        if (*a_pData++ != 0)
        {
            XURReader reader(a_pData);
            if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

            reader.ReadProperty<XUI_EPT_FLOAT>(PropType_StrokeWidth, m_StrokeWidth);
            reader.ReadProperty<XUI_EPT_COLOR>(PropType_StrokeColor, m_StrokeColor);
        }

        return TTRUE;
    }

    // GradientData

    TBOOL XURXUIGradientData::IsColourPropType(uint32_t propType)
    {
        return propType == 2;
    }

    TBOOL XURXUIGradientData::IsFloatPropType(uint32_t propType)
    {
        return propType == 3;
    }

    uint32_t XURXUIGradientData::GetTimelinePropSize(uint32_t propType)
    {
        return propType == 0 ? 1 : 4;
    }

    TBOOL XURXUIGradientData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
    {
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.Radial", propType, PropType_FillGradientRadial);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.NumStops", propType, PropType_FillGradientNumStops);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.StopPos", propType, PropType_FillGradientStopPos);
        TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.StopColor", propType, PropType_FillGradientStopColor);

        return TFALSE;
    }

    TBOOL XURXUIGradientData::ValidateTimelineProp(uint32_t param_2)
    {
        return param_2 < PropType_NUMOF;
    }

    TBOOL XURXUIGradientData::Load(TXUIResource& resource, uint8_t*& a_pData)
    {
        XURXUIObjectData::Load(resource, a_pData);

        if (*a_pData++ != 0)
        {
            XURReader reader(a_pData);
            if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

            reader.ReadProperty<XUI_EPT_BOOL>(PropType_FillGradientRadial, m_Radial);
            reader.ReadProperty<XUI_EPT_USHORT32>(PropType_FillGradientNumStops, m_NumStops);

            if (reader.ShouldReadThisProp(PropType_FillGradientStopPos))
            {
                XUIEPTUInt8 num = reader.ReadEPTUInt8();
                m_Stops = new XUIEPTFloat[num];

                for (size_t i = 0; i < num; i++)
                {
                    m_Stops[i] = reader.ReadEPTFloat();
                }
            }

            if (reader.ShouldReadThisProp(PropType_FillGradientStopColor))
            {
                XUIEPTUInt8 num = reader.ReadEPTUInt8();
                m_StopColors = new XUIEPTColor[num];

                for (size_t i = 0; i < num; i++)
                {
                    m_StopColors[i] = reader.ReadEPTColor();
                }
            }
        }

        return TTRUE;
    }

    TBOOL XURXUIFigureData::IsColourPropType(uint32_t a_uiObjectIndex, PropType propType)
    {
        if (a_uiObjectIndex == 0)
            return TFALSE;

        TASSERT(a_uiObjectIndex > 0);
        return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
    }

    TBOOL XURXUIFigureData::IsFloatPropType(uint32_t a_uiObjectIndex, PropType propType)
    {
        if (a_uiObjectIndex == 0)
            return TFALSE;

        TASSERT(a_uiObjectIndex > 0);
        return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
    }

    uint32_t XURXUIFigureData::GetTimelinePropSize(uint32_t a_uiObjectIndex, PropType propType)
    {
        if (a_uiObjectIndex == 0)
            return propType == PropType_Closed ? 1 : 4;

        TASSERT(a_uiObjectIndex > 0);
        return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
    }

    TBOOL XURXUIFigureData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
    {
        if (!m_Stroke.TranslateTimelineProp(name, a_uiObjectIndex, propType))
        {
            if (!m_Fill.TranslateTimelineProp(name, a_uiObjectIndex, propType))
            {
                TXUI_TRANSLATE_TIMELINE_PROP(name, Closed, propType);
                TXUI_TRANSLATE_TIMELINE_PROP(name, Points, propType);

                a_uiObjectIndex++;
                return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
            }
        }

        return TTRUE;
    }

    TBOOL XURXUIFigureData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
{
        if (a_uiObjectIndex == 0)
            return a_uiPropIndex < PropType_NUMOF;

        TASSERT(a_uiObjectIndex > 0);
        return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
    }

    TBOOL XURXUIFigureData::Load(TXUIResource& resource, uint8_t*& a_pData)
    {
        XURXUIElementData::Load(resource, a_pData);

        if (*a_pData++ != 0)
        {
            XURReader reader(a_pData);
            if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

            if (reader.ShouldReadThisProp(PropType_Stroke))
            {
                m_Stroke.Load(resource, a_pData);
            }

            if (reader.ShouldReadThisProp(PropType_Fill))
            {
                m_Fill.Load(resource, a_pData);
            }

            XUIEPTCustom points;
            TBOOL hasClosed = reader.ReadProperty<XUI_EPT_BOOL>(PropType_Closed, m_Closed);
            TBOOL hasPoints = reader.ReadProperty<XUI_EPT_CUSTOM>(PropType_Points, points);
        
            if (hasPoints)
            {
                TTODO("Load points");
            }
        }

        return TTRUE;
    }

    TBOOL XURXUIFigureData::ValidateTimelineSubProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
    {
        if (a_uiObjectIndex == 0)
            return m_Stroke.ValidateTimelineProp(a_uiPropIndex);
        else if (a_uiObjectIndex == 3)
            return m_Fill.ValidateTimelineProp(a_uiPropIndex);
        else
            return m_Fill.ValidateTimelineProp(a_uiPropIndex);
    }

    TBOOL XURXUIFigureData::GetTimelineSubPropSize(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
    {
        if (a_uiObjectIndex == 0)
            return m_Stroke.GetTimelinePropSize(a_uiPropIndex);
        else if (a_uiObjectIndex == 3)
            return m_Fill.GetTimelinePropSize(a_uiPropIndex);
        else
            return m_Fill.GetTimelinePropSize(a_uiPropIndex);
    }

    TBOOL XURXUIFigureData::IsFloatSubPropType(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
    {
        if (a_uiObjectIndex == 0)
            return m_Stroke.IsFloatPropType(a_uiPropIndex);
        else if (a_uiObjectIndex == 3)
            return m_Fill.IsFloatPropType(a_uiPropIndex);
        else
            return m_Fill.IsFloatPropType(a_uiPropIndex);
    }

    TBOOL XURXUIFigureData::IsColourSubPropType(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
    {
        if (a_uiObjectIndex == 0)
            return m_Stroke.IsColourPropType(a_uiPropIndex);
        else if (a_uiObjectIndex == 3)
            return m_Fill.IsColourPropType(a_uiPropIndex);
        else
            return m_Fill.IsColourPropType(a_uiPropIndex);
    }

}
