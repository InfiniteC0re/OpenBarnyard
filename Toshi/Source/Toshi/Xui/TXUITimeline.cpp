#include "ToshiPCH.h"
#include "TXUITimeline.h"
#include "XURReader.h"

namespace Toshi
{
	void XURXUITimelineData::Load(TXUIResource& a_rXur, uint8_t*& a_pData)
	{
		TIMPLEMENT();
		XURReader reader(a_pData);

		m_StringId = reader.ReadEPTString();
		m_NumTimelineProps = reader.ReadEPTUShort32();
		m_TimelineProps = new TimelineProp[m_NumTimelineProps];

		for (XUIEPTUShort32 i = 0; i < m_NumTimelineProps; i++)
		{
			XUIEPTUInt8 flag = reader.ReadEPTUInt8();

			if (flag == 0x83)
			{
				m_TimelineProps[i].m_unk = reader.ReadEPTUnsigned();
				m_TimelineProps[i].m_unk2 = reader.ReadEPTUnsigned();
			}
			else if (flag == 0x02)
			{
				XUIEPTUInt8 b1 = reader.ReadEPTUInt8();
				XUIEPTUInt8 b2 = reader.ReadEPTUInt8();
				XUIEPTUInt8 b3 = reader.ReadEPTUInt8();

				m_TimelineProps[i].m_unk = ((b1 | 0x200) << 8 | b2) << 8 | b3;
			}
			else
			{
				// some black magic here
				reader.SeekFromCur(2);
			}
		}

		m_NumKeyframes = reader.ReadEPTShort32();
		TASSERT(TFALSE, "We have to implement this shit to continue working on TXUI");
	}
}
