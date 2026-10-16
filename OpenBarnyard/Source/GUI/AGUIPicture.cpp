#include "pch.h"
#include "AGUIPicture.h"
#include "AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT_NORUNTIME( AGUIPicture );

// $Barnyard: FUNCTION 0059d530
AGUIPicture::AGUIPicture()
{
	m_iUnknown1  = 0;
	m_iNumRects1 = 0;
	m_iNumRects2 = 0;
	m_bFlag      = TFALSE;
	m_bLoaded    = TFALSE;
	m_bIsVisible = TTRUE;
	m_fUnknown3  = -10.0f;
}

// NOTE: retail fills split_rules in a dynamic initializer (thunk 0059d520)
// $Barnyard: FUNCTION 0059d400
void AGUIPicture::InitialiseClass()
{
	constexpr TFLOAT TEX_SIZE = 256.0f;

	TFLOAT fValA;
	TFLOAT fValB = -240.0f;

	for ( TINT i = 0; i < 2; i++ )
	{
		fValA = ( i == 0 ) ? TEX_SIZE - 1.0f : 240.0f - fValB;

		split_rules[ i * 3 + 0 ].iImageIndex = i * 3 + 0;
		split_rules[ i * 3 + 0 ].Pos1        = { -320.0f, -65.0f };
		split_rules[ i * 3 + 0 ].Pos2        = { fValB, fValA + fValB };
		split_rules[ i * 3 + 0 ].UV1         = { 0.0f, 0.9960938f };
		split_rules[ i * 3 + 0 ].UV2         = { 0.0f, fValA / TEX_SIZE };
		split_rules[ i * 3 + 0 ].iUnk1       = 0;
		split_rules[ i * 3 + 0 ].iUnk2       = 0;

		split_rules[ i * 3 + 1 ].iImageIndex = i * 3 + 1;
		split_rules[ i * 3 + 1 ].Pos1        = { -65.0f, 190.0f };
		split_rules[ i * 3 + 1 ].Pos2        = { fValB, fValA + fValB };
		split_rules[ i * 3 + 1 ].UV1         = { 0.0f, 0.9960938f };
		split_rules[ i * 3 + 1 ].UV2         = { 0.0f, fValA / TEX_SIZE };
		split_rules[ i * 3 + 1 ].iUnk1       = 0;
		split_rules[ i * 3 + 1 ].iUnk2       = 0;

		split_rules[ i * 3 + 2 ].iImageIndex = i * 3 + 2;
		split_rules[ i * 3 + 2 ].Pos1        = { 190.0f, 320.0f };
		split_rules[ i * 3 + 2 ].Pos2        = { fValB, fValA + fValB };
		split_rules[ i * 3 + 2 ].UV1         = { 0.0f, 1.015625f };
		split_rules[ i * 3 + 2 ].UV2         = { 0.0f, fValA / TEX_SIZE };
		split_rules[ i * 3 + 2 ].iUnk1       = 0;
		split_rules[ i * 3 + 2 ].iUnk2       = 0;

		fValB += fValA;
	}

#ifdef BARNYARD_LATEST
	InitialiseWiiSplitRules();
#endif
}

#ifdef BARNYARD_LATEST

// NOTE: this is the dynamic initializer of split_rules in the Wii build
// $BarnyardWiiJP: FUNCTION 8031f830
void AGUIPicture::InitialiseWiiSplitRules()
{
	// Width of the textures of the three columns, all are 256 pixels high
	constexpr TFLOAT aTexWidths[ 3 ] = { 256.0f, 256.0f, 128.0f };
	constexpr TFLOAT TEX_HEIGHT      = 256.0f;

	TFLOAT fY = -224.0f;

	for ( TINT i = 0; i < 2; i++ )
	{
		const TFLOAT fHeight = ( i == 0 ) ? TEX_HEIGHT - 1.0f : 224.0f - fY;
		TFLOAT       fX      = -320.0f;

		for ( TINT k = 0; k < 3; k++ )
		{
			const TFLOAT fWidth = ( k < 2 ) ? aTexWidths[ k ] - 1.0f : 320.0f - fX;
			SplitRule&   rRule  = ms_aWiiSplitRules[ i * 3 + k ];

			rRule.Pos1        = { fX, fX + fWidth };
			rRule.Pos2        = { fY, fY + fHeight };
			rRule.UV1         = { 0.0f, fWidth / aTexWidths[ k ] };
			rRule.UV2         = { 0.0f, fHeight / TEX_HEIGHT };
			rRule.iImageIndex = i * 3 + k;
			rRule.iUnk1       = 0;
			rRule.iUnk2       = 0;

			fX += fWidth;
		}

		fY += fHeight;
	}
}

#endif // BARNYARD_LATEST

// NOTE: retail resets m_iUnknown1 after m_Name is destroyed
// $Barnyard: FUNCTION 0059d5e0
// $Barnyard: FUNCTION 0059d570
AGUIPicture::~AGUIPicture()
{
	m_iUnknown1 = -1;
}

void AGUIPicture::DeinitialiseClass()
{
}

// $Barnyard: FUNCTION 0059d2b0
void AGUIPicture::Delete()
{
	Flush();
	delete this;
}

// $Barnyard: FUNCTION 0059d2a0
void AGUIPicture::SetFlag()
{
	m_bFlag = TTRUE;
}

// $Barnyard: FUNCTION 0059d310
void AGUIPicture::Render()
{
	if ( m_bIsVisible )
	{
		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

		T2GUITransform transform;
		transform.SetTranslation( 0.0f, 0.0f );

#ifdef BARNYARD_LATEST
		// The Wii build (8031f69c) scales by 638x445
		if ( m_bWiiLayout )
			transform.Scale( fWidth / 638.0f, fHeight / 445.0f );
		else
#endif
			transform.Scale( fWidth / 638.0f, fHeight / 477.0f );

		auto pRenderer = AGUI2::GetRenderer();
		pRenderer->PushTransform( transform, TVector2::VEC_ZERO, TVector2::VEC_ZERO );

		for ( TINT i = 0; i < m_iNumRects1; i++ )
		{
			pRenderer->SetMaterial( *m_pRectangles[ i ].ppMaterial );
			pRenderer->RenderRectangle( m_pRectangles[ i ].Pos1, m_pRectangles[ i ].Pos2, m_pRectangles[ i ].UV1, m_pRectangles[ i ].UV2 );
		}

		pRenderer->PopTransform();
	}
}
