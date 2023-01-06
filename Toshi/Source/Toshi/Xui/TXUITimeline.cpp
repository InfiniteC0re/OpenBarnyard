#include "ToshiPCH.h"
#include "TXUITimeline.h"

void Toshi::XURXUITimelineData::Load(Toshi::TXUIResource& a_rXur, unsigned char*& a_pData)
{
	m_uiNumUnk = PARSEWORD_BIG(a_pData);
	a_pData += 2;
	m_uiNumTimelineProp = PARSEWORD_BIG(a_pData + 2);
	a_pData += 4;
}
