#include "ToshiPCH.h"
#include "TRenderPacket.h"

namespace Toshi {

	TRenderPacket::TRenderPacket()
	{
		m_pNextPacket = TNULL;
		m_pUnk = TNULL;
		m_pSkeletonInstance = TNULL;
		m_fAmbientColorR = 0.24f;
		m_fAmbientColorG = 0.24f;
		m_fAmbientColorB = 0.24f;
		m_oLightColour.x = 1.0f;
		m_oLightColour.y = 1.0f;
		m_oLightColour.z = 0.65f;
		m_oLightDirection.x = -0.47f;
		m_oLightDirection.y = 0.74f;
		m_oLightDirection.z = 0.47f;
		m_ui8Unk1 = 255;
	}

}

