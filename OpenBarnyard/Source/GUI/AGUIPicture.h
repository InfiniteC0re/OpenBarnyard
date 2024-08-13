#pragma once
#include "AGUI2Material.h"

#include <Math/TVector2.h>
#include <Toshi/TPString8.h>

class AGUIPicture : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AGUIPicture, Toshi::TObject );

public:
	struct SplitRule
	{
		Toshi::TVector2 Pos1;
		Toshi::TVector2 Pos2;
		Toshi::TVector2 UV1;
		Toshi::TVector2 UV2;
		TINT			iImageIndex;
		TINT			iUnk1;
		TINT			iUnk2;
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
		AGUI2Material*	 pMaterial;
		Toshi::TTexture* pTexture;
	};

public:
	AGUIPicture();

	virtual void SetFlag();
	virtual void Render();
	virtual void Flush()								   = 0;
	virtual void Cache()								   = 0;
	virtual void Create( const Toshi::TPString8& a_rName ) = 0;

	void SetVisible() { m_bIsVisible = TTRUE; }
	void SetHidden() { m_bIsVisible = TFALSE; }

	const Toshi::TPString8& GetName() const { return m_Name; }

protected:
	inline static SplitRule split_rules[ 6 ];

protected:
	TINT			 m_iUnknown1;
	TINT			 m_iNumRects1;
	TINT			 m_iNumRects2;
	Rectangle*		 m_pRectangles;
	Image*			 m_pImages;
	Toshi::TPString8 m_Name;
	TBOOL			 m_bIsVisible;
	TBOOL			 m_bFlag;
	TBOOL			 m_bLoaded;
	TFLOAT			 m_fUnknown3;
};
