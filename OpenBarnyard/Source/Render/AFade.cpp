#include "pch.h"
#include "AFade.h"

TUINT8* AFade::GetCurrentColor(TUINT8 a_pColor[4])
{
	TFLOAT fProgress = m_fCurrentTime / m_fFadeTime;
	a_pColor[0] = TUINT8(Toshi::TMath::LERP(m_ui8FadeFromR / 255.0f, m_ui8FadeToR / 255.0f, fProgress) * 255.0f);
	a_pColor[1] = TUINT8(Toshi::TMath::LERP(m_ui8FadeFromG / 255.0f, m_ui8FadeToG / 255.0f, fProgress) * 255.0f);
	a_pColor[2] = TUINT8(Toshi::TMath::LERP(m_ui8FadeFromB / 255.0f, m_ui8FadeToB / 255.0f, fProgress) * 255.0f);
	a_pColor[3] = TUINT8(Toshi::TMath::LERP(m_ui8FadeFromA / 255.0f, m_ui8FadeToA / 255.0f, fProgress) * 255.0f);

	return a_pColor;
}

TBOOL AFade::IsStillFading(TBOOL a_bThrowFadeOver)
{
	if (m_fFadeTime <= m_fCurrentTime)
	{
		if (a_bThrowFadeOver)
		{
			m_FadeOverEmitter.Throw(TFALSE);
		}

		return TFALSE;
	}

	return TTRUE;
}
