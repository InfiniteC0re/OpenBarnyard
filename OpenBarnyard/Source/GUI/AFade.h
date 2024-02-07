#pragma once
#include "GUI/AGUI2Rectangle.h"

#include <Toshi/TColor.h>
#include <Toshi/TQList.h>
#include <Toshi/T2ObjectPool.h>

class AFade :
	public Toshi::TQList<AFade>::TNode
{
public:
	friend class AFadeManager;

	struct Color
	{
		TUINT8 A;
		TUINT8 R;
		TUINT8 G;
		TUINT8 B;

		constexpr Color() { }

		constexpr Color(TUINT8 a_uiA, TUINT8 a_uiR, TUINT8 a_uiG, TUINT8 a_uiB) :
			A(a_uiA), R(a_uiR), G(a_uiG), B(a_uiB) { }

		constexpr Color(const Toshi::TColor& a_rColor) :
			A(a_rColor.A), R(a_rColor.G), G(a_rColor.G), B(a_rColor.B) { }

		constexpr Color(TUINT32 a_uiColor) :
			A(TCOLOR_GET_A(a_uiColor)),
			R(TCOLOR_GET_R(a_uiColor)),
			G(TCOLOR_GET_G(a_uiColor)),
			B(TCOLOR_GET_B(a_uiColor)) 
		{ }

		constexpr TUINT32 GetTColor32() const
		{
			return TCOLOR_ALPHA(R, G, B, A);
		}
	};

public:
	AFade();
	virtual ~AFade() = default;

	void Update(TFLOAT a_fDeltaTime)
	{
		m_fCurrentTime += a_fDeltaTime;
	}

	void SetFadeTime(TFLOAT a_fFadeTime);
	void SetFadeFromColor(const Color& a_rFadeFromColor);
	void SetFadeToColor(const Color& a_rFadeToColor);

	Color* GetCurrentColor(Color& a_rColor);
	TBOOL IsStillFading(TBOOL a_bThrowFadeOver);

	AGUI2Rectangle& GetRectangleElement()
	{
		return m_Overlay;
	}

	Toshi::TEmitter<AFade, TBOOL>& GetFadeOverEmitter()
	{
		return m_FadeOverEmitter;
	}

private:
	Color m_FadeFromColor;
	Color m_FadeToColor;
	TFLOAT m_fCurrentTime;
	TFLOAT m_fFadeTime;
	AGUI2Rectangle m_Overlay;
	Toshi::TEmitter<AFade, TBOOL> m_FadeOverEmitter;
};
