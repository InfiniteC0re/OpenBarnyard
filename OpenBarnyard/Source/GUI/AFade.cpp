#include "pch.h"
#include "AFade.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

AFade::AFade() :
	m_FadeFromColor(255, 255, 255, 255),
	m_FadeToColor(255, 255, 255, 255),
	m_fCurrentTime(0.0f),
	m_FadeOverEmitter(this)
{
	
}

void AFade::SetFadeTime(TFLOAT a_fFadeTime)
{
	m_fFadeTime = a_fFadeTime;
}

void AFade::SetFadeFromColor(const Color& a_rFadeFromColor)
{
	m_FadeFromColor = a_rFadeFromColor;
}

void AFade::SetFadeToColor(const Color& a_rFadeToColor)
{
	m_FadeToColor = a_rFadeToColor;
}

AFade::Color* AFade::GetCurrentColor(Color& a_rColor)
{
	TFLOAT fProgress = m_fCurrentTime / m_fFadeTime;
	a_rColor.A = TUINT8(Toshi::TMath::LERP(m_FadeFromColor.A / 255.0f, m_FadeToColor.A / 255.0f, fProgress) * 255.0f);
	a_rColor.R = TUINT8(Toshi::TMath::LERP(m_FadeFromColor.R / 255.0f, m_FadeToColor.R / 255.0f, fProgress) * 255.0f);
	a_rColor.G = TUINT8(Toshi::TMath::LERP(m_FadeFromColor.G / 255.0f, m_FadeToColor.G / 255.0f, fProgress) * 255.0f);
	a_rColor.B = TUINT8(Toshi::TMath::LERP(m_FadeFromColor.B / 255.0f, m_FadeToColor.B / 255.0f, fProgress) * 255.0f);

	return &a_rColor;
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
