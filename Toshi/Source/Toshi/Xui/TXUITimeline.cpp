#include "ToshiPCH.h"
#include "TXUITimeline.h"
#include "XURReader.h"
#include "TXUIFigure.h"

namespace Toshi
{

	XURXUITimelineData::XURXUITimelineData()
	{
		m_pOwnerData = TNULL;
		m_uiNumTimelineProps = 0;
		m_uiNumKeyframes = 0;
		m_iControlledChildStringID = 0;
		m_aTimelineValues = TNULL;
		m_aTimelineProps = TNULL;
		m_pControlledChild = TNULL;
		m_aTimelineProps = TNULL;
	}

	XURXUITimelineData::~XURXUITimelineData()
	{
		delete[] m_aTimelineProps;
		delete[] m_aTimelineValues;
	}

	TBOOL XURXUITimelineData::Load(TXUIResource& a_rXur, uint8_t*& a_pData)
	{
		XURReader reader(a_pData);

		m_iControlledChildStringID = reader.ReadEPTString();
		m_uiNumTimelineProps = reader.ReadEPTUShort32();
		m_aTimelineProps = new TimelineProp[m_uiNumTimelineProps];

		m_pControlledChild = m_pOwnerData->FindChildElementData(m_iControlledChildStringID);

		for (XUIEPTUShort32 i = 0; i < m_uiNumTimelineProps; i++)
		{
			XUIEPTUInt8 flag = reader.ReadEPTUInt8();

			if (flag == 0x83)
			{
				m_aTimelineProps[i].m_Unk1.Raw = reader.ReadEPTUnsigned();
				m_aTimelineProps[i].m_Unk2 = reader.ReadEPTUnsigned();
			}
			else if (flag == 0x02)
			{
				XUIEPTUInt8 b1 = reader.ReadEPTUInt8();
				XUIEPTUInt8 b2 = reader.ReadEPTUInt8();
				XUIEPTUInt8 b3 = reader.ReadEPTUInt8();

				m_aTimelineProps[i].m_Unk1.Raw = ((b1 | 0x200) << 8 | b2) << 8 | b3;
			}
			else
			{
				m_aTimelineProps[i].m_Unk1.Raw = 0;

				reader.SeekFromCur(-1);
				TUtil::MemCopy(&m_aTimelineProps[i].m_Unk1.Raw, reader.GetPos(), 3);
				m_aTimelineProps[i].m_Unk1.Raw = PARSEDWORD_BIG(m_aTimelineProps[i].m_Unk1.Raw);
				reader.SeekFromCur(3);
			}

			TBOOL bValid = TFALSE;

			if (m_aTimelineProps[i].m_Unk1.Byte4 == 2 ||
				m_aTimelineProps[i].m_Unk1.Raw == 0x10302 ||
				m_aTimelineProps[i].m_Unk1.Raw == 0x10303)
			{
				// Has sub-props
				TASSERT(0 == TStringManager::String16Compare(a_rXur.GetString(m_pControlledChild->m_uiTypeIndex), _TS16("XuiFigure")));
				XURXUIFigureData* pFigureData = TSTATICCAST(XURXUIFigureData*, m_pControlledChild);
				
				bValid = pFigureData->ValidateTimelineSubProp(m_aTimelineProps[i].m_Unk1.Byte2, m_aTimelineProps[i].m_Unk1.Byte1);
			}
			else
			{
				// Doesn't have sub-props
				bValid = m_pControlledChild->ValidateTimelineProp(m_aTimelineProps[i].m_Unk1.Byte3, m_aTimelineProps[i].m_Unk1.Byte2);
			}

			TASSERT(TTRUE == bValid);
		}

		m_uiNumKeyframes = reader.ReadEPTShort32();
		TASSERT(m_uiNumKeyframes < 1000);

		m_aKeyframeData.Create(m_uiNumKeyframes);
		m_aTimelineValues = new XURXUIKeyframeData::Value[m_uiNumTimelineProps * m_uiNumKeyframes];

		for (XUIEPTUShort32 i = 0; i < m_uiNumKeyframes; i++)
		{
			m_aKeyframeData[i].m_uiKeyframeTime = reader.ReadEPTUShort32();
			TASSERT(m_aKeyframeData[i].m_uiKeyframeTime < 5000);

			m_aKeyframeData[i].m_Byte1 = reader.ReadEPTUInt8();
			m_aKeyframeData[i].m_Byte2 = reader.ReadEPTUInt8();
			m_aKeyframeData[i].m_Byte3 = reader.ReadEPTUInt8();
			m_aKeyframeData[i].m_Byte4 = reader.ReadEPTUInt8();
			m_aKeyframeData[i].m_pValues = m_aTimelineValues + m_uiNumTimelineProps * i;

			for (XUIEPTUShort32 k = 0; k < m_uiNumTimelineProps; k++)
			{
				uint32_t propSize;
				TBOOL isFloatProp;
				TBOOL isColourProp;

				if (m_aTimelineProps[k].m_Unk1.Byte4 == 2 ||
					m_aTimelineProps[k].m_Unk1.Raw == 0x10302 ||
					m_aTimelineProps[k].m_Unk1.Raw == 0x10303)
				{
					// Has sub-props
					TASSERT(0 == TStringManager::String16Compare(a_rXur.GetString(m_pControlledChild->m_uiTypeIndex), _TS16("XuiFigure")));
					XURXUIFigureData* pFigureData = TSTATICCAST(XURXUIFigureData*, m_pControlledChild);
					
					propSize = pFigureData->GetTimelineSubPropSize(m_aTimelineProps[k].m_Unk1.Byte2, m_aTimelineProps[k].m_Unk1.Byte1);
					isFloatProp = pFigureData->IsFloatSubPropType(m_aTimelineProps[k].m_Unk1.Byte2, m_aTimelineProps[k].m_Unk1.Byte1);
					isColourProp = pFigureData->IsColourSubPropType(m_aTimelineProps[k].m_Unk1.Byte2, m_aTimelineProps[k].m_Unk1.Byte1);
				}
				else
				{
					// Doesn't have sub-props
					propSize = m_pControlledChild->GetTimelinePropSize(m_aTimelineProps[k].m_Unk1.Byte3, m_aTimelineProps[k].m_Unk1.Byte2);
					isFloatProp = m_pControlledChild->IsFloatPropType(m_aTimelineProps[k].m_Unk1.Byte3, m_aTimelineProps[k].m_Unk1.Byte2);
					isColourProp = m_pControlledChild->IsColourPropType(m_aTimelineProps[k].m_Unk1.Byte3, m_aTimelineProps[k].m_Unk1.Byte2);
				}

				TASSERT(propSize >= 1 && propSize <= 4);

				if (propSize == 1)
				{
					m_aKeyframeData[i].m_pValues[k].Byte = reader.ReadUInt8();
				}
				else if (propSize == 2)
				{
					m_aKeyframeData[i].m_pValues[k].Short = reader.ReadUInt16();
				}
				else if (propSize == 4)
				{
					if (isFloatProp)
						m_aKeyframeData[i].m_pValues[k].Float = reader.ReadFloat();
					else
						m_aKeyframeData[i].m_pValues[k].Int = reader.ReadUInt32();
				}
			}
		}

		return TTRUE;
	}
}
