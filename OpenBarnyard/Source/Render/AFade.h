#pragma once

#include <Toshi/Core/TQList.h>
#include <Toshi2/T2ObjectPool.h>

class AFade :
	public Toshi::TQList<AFade>::TNode
{
public:
	friend class AFadeManager;

public:
	AFade() = default;
	virtual ~AFade() = default;

	TUINT8* GetCurrentColor(TUINT8 a_pColor[4]);
	TBOOL IsStillFading(TBOOL a_bThrowFadeOver);

private:
	TUINT8 m_ui8FadeFromR;
	TUINT8 m_ui8FadeFromG;
	TUINT8 m_ui8FadeFromB;
	TUINT8 m_ui8FadeFromA;
	TUINT8 m_ui8FadeToR;
	TUINT8 m_ui8FadeToG;
	TUINT8 m_ui8FadeToB;
	TUINT8 m_ui8FadeToA;
	TFLOAT m_fCurrentTime;
	TFLOAT m_fFadeTime;
	// AGUI2Rectangle m_Overlay;
	Toshi::TEmitter<AFade, TBOOL> m_FadeOverEmitter; // is it really TBOOL?
};
