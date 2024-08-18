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
}

void AGUIPicture::DeinitialiseClass()
{
}

void AGUIPicture::SetFlag()
{
	m_bFlag = TTRUE;
}

void AGUIPicture::Render()
{
	if ( m_bIsVisible )
	{
		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

		AGUI2Transform transform;
		transform.SetPosition( 0.0f, 0.0f );
		transform.PreMultiply( fWidth / 638.0f, fHeight / 477.0f );

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
