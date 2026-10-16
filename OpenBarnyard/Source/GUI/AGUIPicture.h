#pragma once
#include <Math/TVector2.h>
#include <Toshi/TPString8.h>

#include <GUI/T2GUIMaterial.h>

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
		TINT            iImageIndex;
		TINT            iUnk1;
		TINT            iUnk2;
	};

	struct Rectangle
	{
		Toshi::TVector2        Pos1;
		Toshi::TVector2        Pos2;
		Toshi::TVector2        UV1;
		Toshi::TVector2        UV2;
		Toshi::T2GUIMaterial** ppMaterial;
	};

	struct Image
	{
		Toshi::T2GUIMaterial* pMaterial;
		Toshi::TTexture*      pTexture;
	};

public:
	AGUIPicture();
	~AGUIPicture();

	//-----------------------------------------------------------------------------
	// Toshi::TObject
	//-----------------------------------------------------------------------------
	virtual void Delete() override; // +0x04

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void SetFlag();
	virtual void Render();
	virtual void Flush()                                   = 0;
	virtual void Cache()                                   = 0;
	virtual void Create( const Toshi::TPString8& a_rName ) = 0;

	void SetVisible() { m_bIsVisible = TTRUE; }
	void SetHidden() { m_bIsVisible = TFALSE; }

	// $Barnyard: FUNCTION 0059e080
	const Toshi::TPString8& GetName() const { return m_Name; }

	// Number of images (and rectangles) a picture is split into
	TINT GetNumImages() const { return TARRAYSIZE( split_rules ); }

	// $Barnyard: FUNCTION 0059d300
	static const SplitRule* GetSplitRule( TINT a_iIndex ) { return &split_rules[ a_iIndex ]; }

#ifdef BARNYARD_LATEST
	// The pictures of the Wii build are 640x448 (the PC ones 640x480). Set for pictures made from Wii data.
	void SetWiiLayout( TBOOL a_bWiiLayout ) { m_bWiiLayout = a_bWiiLayout; }

	// Split rule a_iIndex of this picture's layout
	const SplitRule* GetLayoutSplitRule( TINT a_iIndex ) const { return m_bWiiLayout ? &ms_aWiiSplitRules[ a_iIndex ] : GetSplitRule( a_iIndex ); }
#endif

protected:
	inline static SplitRule split_rules[ 6 ];

#ifdef BARNYARD_LATEST
	static void InitialiseWiiSplitRules();

	// The split rules of the Wii build
	inline static SplitRule ms_aWiiSplitRules[ 6 ];
#endif

protected:
	TINT             m_iUnknown1;
	TINT             m_iNumRects1;
	TINT             m_iNumRects2;
	Rectangle*       m_pRectangles;
	Image*           m_pImages;
	Toshi::TPString8 m_Name;
	TBOOL            m_bIsVisible;
	TBOOL            m_bFlag;
	TBOOL            m_bLoaded;
	TFLOAT           m_fUnknown3;

#ifdef BARNYARD_LATEST
	TBOOL m_bWiiLayout = TFALSE;
#endif
};
