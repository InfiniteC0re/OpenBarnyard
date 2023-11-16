#pragma once
#include "AGUI2Material.h"

#include <Toshi/Math/TVector2.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi2/T2SimpleArray.h>

TOBJECT(AGUIPicture, Toshi::TObject, TFALSE)
{
public:
	struct SplitRule
	{
		Toshi::TVector2 Pos1;
		Toshi::TVector2 Pos2;
		Toshi::TVector2 UV1;
		Toshi::TVector2 UV2;
		TINT iImageIndex;
		TINT iUnk1;
		TINT iUnk2;
	};

	struct Rectangle
	{
		Toshi::TVector2 Pos1;
		Toshi::TVector2 Pos2;
		Toshi::TVector2 UV1;
		Toshi::TVector2 UV2;
		AGUI2Material** ppMaterial;
	};

	struct Image
	{
		AGUI2Material* pMaterial;
		Toshi::TTexture* pTexture;
	};

public:
	AGUIPicture();

	virtual void SetFlag();
	virtual void Render();
	virtual void Flush() = 0;
	virtual void Cache() = 0;
	virtual void Create(const Toshi::TPString8& a_rName) = 0;

	static void InitialiseStatic();

protected:
	inline static SplitRule split_rules[6];

protected:
	TINT m_iUnknown1;
	TINT m_iNumRects1;
	TINT m_iNumRects2;
	Toshi::T2SimpleArray<Rectangle> m_pRectangles;
	Toshi::T2SimpleArray<Image> m_pImages;
	Toshi::TPString8 m_Name;
	TBOOL m_bRender;
	TBOOL m_bFlag;
	TBOOL m_bLoaded;
	TFLOAT m_fUnknown3;
};
