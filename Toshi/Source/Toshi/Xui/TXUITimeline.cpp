#include "ToshiPCH.h"
#include "TXUITimeline.h"
#include "XURReader.h"
#include "TXUIFigure.h"

namespace Toshi
{
	void XURXUITimelineData::Load(TXUIResource& a_rXur, uint8_t*& a_pData)
	{
		TIMPLEMENT();
		XURReader reader(a_pData);

        m_iControlledChildStringID = reader.ReadEPTString();
		m_NumTimelineProps = reader.ReadEPTUShort32();
		m_TimelineProps = new TimelineProp[m_NumTimelineProps];

        m_pControlledChild = m_pOwnerData->FindChildElementData(m_iControlledChildStringID);

		for (XUIEPTUShort32 i = 0; i < m_NumTimelineProps; i++)
		{
			XUIEPTUInt8 flag = reader.ReadEPTUInt8();

			if (flag == 0x83)
			{
				m_TimelineProps[i].m_Unk1.Raw = reader.ReadEPTUnsigned();
				m_TimelineProps[i].m_Unk2 = reader.ReadEPTUnsigned();
			}
			else if (flag == 0x02)
			{
				XUIEPTUInt8 b1 = reader.ReadEPTUInt8();
				XUIEPTUInt8 b2 = reader.ReadEPTUInt8();
				XUIEPTUInt8 b3 = reader.ReadEPTUInt8();

				m_TimelineProps[i].m_Unk1.Raw = ((b1 | 0x200) << 8 | b2) << 8 | b3;
			}
			else
			{
                m_TimelineProps[i].m_Unk1.Raw = 0;

				reader.SeekFromCur(-1);
                TUtil::MemCopy(&m_TimelineProps[i].m_Unk1.Raw, reader.GetPos(), 3);
                m_TimelineProps[i].m_Unk1.Raw = PARSEDWORD_BIG(m_TimelineProps[i].m_Unk1.Raw);
                reader.SeekFromCur(3);
			}

            TBOOL bValid = TFALSE;

            if (m_TimelineProps[i].m_Unk1.Byte4 == 2 ||
                m_TimelineProps[i].m_Unk1.Raw == 0x10302 ||
                m_TimelineProps[i].m_Unk1.Raw == 0x10303)
            {
                // Has sub-props
                TASSERT(0 == TStringManager::String16Compare(a_rXur.GetString(m_pControlledChild->m_uiTypeIndex), _TS16("XuiFigure")));
                XURXUIFigureData* pFigureData = TSTATICCAST(XURXUIFigureData*, m_pControlledChild);
                
                bValid = pFigureData->ValidateTimelineSubProp(m_TimelineProps[i].m_Unk1.Byte2, m_TimelineProps[i].m_Unk1.Byte1);
            }
            else
            {
                // Doesn't have sub-props
                bValid = m_pControlledChild->ValidateTimelineProp(m_TimelineProps[i].m_Unk1.Byte2, m_TimelineProps[i].m_Unk1.Byte3);
            }

            TASSERT(TTRUE == bValid);
		}

		m_uiNumKeyframes = reader.ReadEPTShort32();
        TASSERT(m_uiNumKeyframes < 1000);

		TASSERT(TFALSE, "We have to implement this shit to continue working on TXUI");
	}
}
