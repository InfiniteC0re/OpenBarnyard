#include "pch.h"
#include "AGUI2FloatHudElement.h"
#include "GUI/AGUI2.h"
#include "Tasks/ARootTask.h"
#include "Render/ARenderer.h"
#include "Cameras/ACameraManager.h"
#include "Units/AVisibleUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Physics/ACollisionManager.h"

#include <GUI/T2GUIFontManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

//-----------------------------------------------------------------------------
// NOTE: inlined T2GUIFontManager function 006c44f0 (releases a reference taken by
// T2GUIFontManager::FindFont), which the engine doesn't have yet. Retail decrements
// a counter at address 0x10 (crashes) when the font isn't found; that's left out.
//-----------------------------------------------------------------------------
static void ReleaseFont( T2GUIFont* a_pFont )
{
	if ( a_pFont )
	{
		for ( TUINT i = 0; i < T2GUIFontManager::MAX_NUM_FONT_REFS; i++ )
		{
			T2GUIFontRef& rRef = T2GUIFontManager::s_fontrefs[ i ];

			if ( rRef.pFontResource && rRef.pFont == a_pFont )
			{
				rRef.uiNumRefs--;
				return;
			}
		}
	}
}

// $Barnyard: FUNCTION 0063a1d0
// $Barnyard: FUNCTION 00639c50
// $Barnyard: FUNCTION 0047b250
template <class T, TINT MAX_SIZE>
void AGUI2FloatHudElement::SortedArray<T, MAX_SIZE>::Insert( T* a_pItem )
{
	// Binary search for the slot after the items as far as a_pItem (equal distances within 1e-6 keep their order)
	TINT iLower = 0;
	TINT iUpper = iCount - 1;

	while ( iLower <= iUpper )
	{
		TINT   iMiddle = ( iUpper + iLower ) / 2;
		TFLOAT fDiff   = a_pItem->fDistanceSq - apItems[ iMiddle ]->fDistanceSq;

		if ( fDiff < -1e-06f )
		{
			iUpper = iMiddle - 1;
		}
		else if ( fDiff <= 1e-06f )
		{
			// Skip the items at the same distance
			iLower = iMiddle + 1;

			while ( iLower <= iUpper )
			{
				fDiff = a_pItem->fDistanceSq - apItems[ iLower ]->fDistanceSq;
				if ( fDiff < -1e-06f || 1e-06f < fDiff ) break;

				iLower++;
			}

			break;
		}
		else
		{
			iLower = iMiddle + 1;
		}
	}

	// Insert at iLower (retail moves the tail with T2Vector::InsertGap, 00639ba0)
	for ( TINT i = iCount - 1; i >= iLower; i-- )
		apItems[ i + 1 ] = apItems[ i ];

	iCount += 1;
	apItems[ iLower ] = a_pItem;
}

// $Barnyard: FUNCTION 0063aa10
AGUI2FloatHudElement::AGUI2FloatHudElement()
    : m_bCreated( TFALSE )
    , m_pFont( TNULL )
    , m_iNumVisibleTimed( 0 )
    , m_iNumVisibleUnit( 0 )
    , m_bShowUnitFloats( TTRUE )
    , m_bOcclusionTest( TFALSE )
    , m_pAttachedTo( TNULL )
{
	m_oVisibleTimed.iCount = 0;
	m_oVisibleUnit.iCount  = 0;
}

// $Barnyard: FUNCTION 0063ad00
// $Barnyard: FUNCTION 0063aac0
AGUI2FloatHudElement::~AGUI2FloatHudElement()
{
	if ( m_bCreated == TTRUE )
		Destroy();
}

// $Barnyard: FUNCTION 00639e60
void AGUI2FloatHudElement::Create( Toshi::T2GUIElement* a_pParent, const TCHAR* a_szFontName )
{
	m_eAnchor     = T2GUIATTACHMENT_TOPLEFT;
	m_ePivot      = T2GUIATTACHMENT_MIDDLECENTER;
	m_pAttachedTo = a_pParent ? a_pParent : AGUI2::GetRootElement();
	m_pAttachedTo->AddChildTail( *this );

	ReleaseFont( m_pFont );
	m_pFont = T2GUIFontManager::FindFont( a_szFontName );

	SetDimensions( m_pAttachedTo->GetWidth(), m_pAttachedTo->GetHeight() );

	for ( TINT i = 0; i < MAX_TIMED_FLOATS; i++ )
		m_llFreeTimedFloats.InsertTail( new TimedFloat );

	for ( TINT i = 0; i < MAX_UNIT_FLOATS; i++ )
	{
		UnitFloat* pFloat = new UnitFloat;
		pFloat->pUnit     = TNULL;
		m_llFreeUnitFloats.InsertTail( pFloat );
	}

	m_bCreated = TTRUE;
}

// $Barnyard: FUNCTION 0063a210
void AGUI2FloatHudElement::Destroy()
{
	m_bCreated = TFALSE;
	RemoveSelf();
	Reset();

	m_llFreeTimedFloats.DeleteAll();
	m_llTimedFloats.DeleteAll();
	m_oVisibleTimed.Clear();

	m_llFreeUnitFloats.DeleteAll();
	m_llUnitFloats.DeleteAll();
	m_oVisibleUnit.Clear();
}

// $Barnyard: FUNCTION 00639fc0
void AGUI2FloatHudElement::Reset()
{
	while ( !m_llTimedFloats.IsEmpty() )
		m_llFreeTimedFloats.InsertTail( m_llTimedFloats.Remove( m_llTimedFloats.Head() ) );

	while ( !m_llUnitFloats.IsEmpty() )
		m_llFreeUnitFloats.InsertTail( m_llUnitFloats.Remove( m_llUnitFloats.Head() ) );

	m_iNumVisibleTimed = 0;
	m_iNumVisibleUnit  = 0;
}

// $Barnyard: FUNCTION 0063a0c0
TBOOL AGUI2FloatHudElement::AddTimedFloat( const TWCHAR* a_wszText, TUINT32 a_uiColour, const Toshi::TVector4& a_rcPosition, TFLOAT a_fDuration, TFLOAT a_fRiseSpeed, TFLOAT a_fFadeTime )
{
	if ( m_llFreeTimedFloats.IsEmpty() )
		return TFALSE;

	TimedFloat* pFloat  = m_llFreeTimedFloats.Remove( m_llFreeTimedFloats.Head() );
	pFloat->uiColour    = a_uiColour;
	pFloat->wszText     = a_wszText;
	pFloat->vecPosition = a_rcPosition;
	pFloat->fTimeLeft   = a_fDuration;
	pFloat->fRiseSpeed  = a_fRiseSpeed;
	pFloat->fFadeTime   = a_fFadeTime;
	m_llTimedFloats.InsertTail( pFloat );

	return TTRUE;
}

// $Barnyard: FUNCTION 0063ab50
void AGUI2FloatHudElement::Render()
{
	T2GUIElement::Render();

	if ( IsVisible() )
	{
		UpdateTimedFloats();

		for ( TINT i = 0; i < m_iNumVisibleTimed; i++ )
			DrawFloat( m_oVisibleTimed.apItems[ i ] );

		if ( m_bShowUnitFloats == TTRUE )
		{
			UpdateUnitFloats();

			for ( TINT i = 0; i < m_iNumVisibleUnit; i++ )
				DrawFloat( m_oVisibleUnit.apItems[ i ] );
		}
	}
}

// Inlined in Render (0063ab50)
void AGUI2FloatHudElement::DrawFloat( const Float* a_pFloat )
{
	TFLOAT fX = a_pFloat->fScreenX;
	TFLOAT fY = a_pFloat->fScreenY;

	if ( a_pFloat->wszText )
	{
		TFLOAT fWidth = m_pFont->GetTextWidth( a_pFloat->wszText, 1.0f );

		// NOTE: retail uses T2GUIFont::GetLineHeight (006c2dd0). The engine's T2GUIFont doesn't expose
		// the line height, so it's taken from the height of a single line minus the line margin.
		TFLOAT fHeight = m_pFont->GetTextHeightWrapped( L"0", TMath::MAXFLOAT, 1.0f ) - m_pFont->GetLinesMargin();

		m_pFont->DrawTextWrapped( a_pFloat->wszText, fX - fWidth * 0.5f, fY - fHeight * 0.5f, fWidth, fHeight, a_pFloat->uiColour, 1.0f, T2GUIFont::TextAlign_Center, TNULL );
	}
}

// $Barnyard: FUNCTION 0063a2a0
void AGUI2FloatHudElement::UpdateTimedFloats()
{
	TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime(); // the one-line TSystemManager::GetScheduler wrapper 006bc840 is in the ignorelist
	if ( ARootTask::GetSingleton()->IsPaused() ) fDeltaTime = 0.0f;

	TFLOAT   fFarSq          = ARenderer::GetSingleton()->GetFar() * ARenderer::GetSingleton()->GetFar();
	ACamera* pCamera         = ACameraManager::GetSingleton()->GetCurrentCamera();
	TVector4 vCameraPosition = pCamera->GetMatrix().GetTranslation();
	TVector4 vCameraForward( pCamera->GetMatrix().m_f31, pCamera->GetMatrix().m_f32, pCamera->GetMatrix().m_f33, 1.0f );

	m_oVisibleTimed.Clear();
	m_iNumVisibleTimed = 0;

	for ( auto it = m_llTimedFloats.Begin(); it != m_llTimedFloats.End(); )
	{
		TimedFloat* pFloat = it;
		it++;

		pFloat->vecPosition.y += fDeltaTime * pFloat->fRiseSpeed;
		pFloat->fTimeLeft -= fDeltaTime;

		if ( pFloat->fTimeLeft > 0.0f )
		{
			TVector4 vToFloat(
			    pFloat->vecPosition.x - vCameraPosition.x,
			    pFloat->vecPosition.y - vCameraPosition.y,
			    pFloat->vecPosition.z - vCameraPosition.z,
			    pFloat->vecPosition.w
			);

			TFLOAT fDistanceSq = vToFloat.MagnitudeSq();

			// In range and in front of the camera
			if ( fDistanceSq > fFarSq || vCameraForward.DotProduct3( vToFloat ) <= 0.0f )
				continue;

#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: the original compares the number of floats inserted this frame with the maximum, which
			// gets past it after the first replacement so the next inserts overflow the array
			if ( m_oVisibleTimed.iCount >= MAX_TIMED_FLOATS )
#else
			if ( m_iNumVisibleTimed == MAX_TIMED_FLOATS )
#endif
			{
				// Full, replace the furthest float if this one is nearer
				if ( m_oVisibleTimed.apItems[ MAX_TIMED_FLOATS - 1 ]->fDistanceSq <= fDistanceSq )
					continue;

				m_oVisibleTimed.apItems[ --m_oVisibleTimed.iCount ] = TNULL;
			}

			TVector2 vScreenPos;
			ARenderer::GetSingleton()->ProjectToGUI( pCamera, pFloat->vecPosition, vScreenPos );

			TFLOAT fWidth, fHeight;
			AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );

			if ( 0.0f <= vScreenPos.x && vScreenPos.x <= fWidth && 0.0f <= vScreenPos.y && vScreenPos.y <= fHeight )
			{
				pFloat->fDistanceSq = fDistanceSq;
				pFloat->fScreenX    = vScreenPos.x;
				pFloat->fScreenY    = vScreenPos.y;

				// Fades out during the last fFadeTime seconds and with the distance
				TFLOAT fAlpha = ( pFloat->fFadeTime - pFloat->fTimeLeft <= 0.0f ) ? 1.0f : 1.0f - ( pFloat->fFadeTime - pFloat->fTimeLeft ) / pFloat->fFadeTime;

				pFloat->uiColour = ( TUINT32( fAlpha * ( 1.0f - pFloat->fDistanceSq / fFarSq ) * 255.0f ) << 24 ) | ( pFloat->uiColour & 0xffffff );
				m_oVisibleTimed.Insert( pFloat );
				m_iNumVisibleTimed++;
			}
		}
		else
		{
			// Expired
			m_llTimedFloats.Remove( pFloat );
			m_llFreeTimedFloats.InsertHead( pFloat );
		}
	}
}

// $Barnyard: FUNCTION 0063a5e0
void AGUI2FloatHudElement::UpdateUnitFloats()
{
	TFLOAT   fFarSq          = ARenderer::GetSingleton()->GetFar() * ARenderer::GetSingleton()->GetFar();
	ACamera* pCamera         = ACameraManager::GetSingleton()->GetCurrentCamera();
	TVector4 vCameraPosition = pCamera->GetMatrix().GetTranslation();
	TVector4 vCameraForward( pCamera->GetMatrix().m_f31, pCamera->GetMatrix().m_f32, pCamera->GetMatrix().m_f33, 1.0f );

	m_oVisibleUnit.Clear();
	m_iNumVisibleUnit = 0;

	for ( auto it = m_llUnitFloats.Begin(); it != m_llUnitFloats.End(); )
	{
		UnitFloat* pFloat = it;
		it++;

		AUnit*          pUnit         = pFloat->pUnit;
		const TVector4* pUnitPosition = pUnit->GetSteer()->GetPosition();

		TVector4 vToUnit(
		    pUnitPosition->x - vCameraPosition.x,
		    pUnitPosition->y - vCameraPosition.y,
		    pUnitPosition->z - vCameraPosition.z,
		    pUnitPosition->w
		);

		TFLOAT fDistanceSq = vToUnit.MagnitudeSq();

		// In range and in front of the camera
		if ( fDistanceSq > fFarSq || vCameraForward.DotProduct3( vToUnit ) <= 0.0f )
			continue;

		if ( pUnit->IsA( &TGetClass( AVisibleUnit ) ) && !TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
			continue;

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: same overflow as in UpdateTimedFloats
		if ( m_oVisibleUnit.iCount >= MAX_UNIT_FLOATS )
#else
		if ( m_iNumVisibleUnit == MAX_UNIT_FLOATS )
#endif
		{
			// Full, replace the furthest float if this one is nearer
			if ( m_oVisibleUnit.apItems[ MAX_UNIT_FLOATS - 1 ]->fDistanceSq <= fDistanceSq )
				continue;

			m_oVisibleUnit.apItems[ --m_oVisibleUnit.iCount ] = TNULL;
		}

		pUnitPosition = pUnit->GetSteer()->GetPosition();

		TVector4 vPosition = *pUnitPosition;
		vPosition.y        = pUnitPosition->y - pFloat->fHeight;

		if ( m_bOcclusionTest == TTRUE )
		{
			// Hidden behind something
			ARayData oRay( vPosition, vCameraPosition );
			TFLOAT   fHitDistance;

			if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fHitDistance, 0 ) )
				continue;
		}

		// Smooth the height
		TFLOAT fProgress = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() * 10.0f;
		TMath::Clip( fProgress, 0.0f, 1.0f );
		vPosition.y        = TMath::LERP( pFloat->fSmoothedY, vPosition.y, fProgress );
		pFloat->fSmoothedY = vPosition.y;

		TVector2 vScreenPos;
		ARenderer::GetSingleton()->ProjectToGUI( pCamera, vPosition, vScreenPos );

		TFLOAT fWidth, fHeight;
		AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );

		if ( 0.0f <= vScreenPos.x && vScreenPos.x <= fWidth && 0.0f <= vScreenPos.y && vScreenPos.y <= fHeight )
		{
			pFloat->fDistanceSq = fDistanceSq;
			pFloat->fScreenX    = vScreenPos.x;
			pFloat->fScreenY    = vScreenPos.y;

			// Fades out between 10 and 15 units away
			TFLOAT fAlpha;

			if ( !pFloat->bFadeWithDistance || pFloat->fDistanceSq <= 100.0f )
				fAlpha = 1.0f;
			else if ( pFloat->fDistanceSq < 225.0f )
				fAlpha = 1.0f - ( pFloat->fDistanceSq - 100.0f ) * 0.008f;
			else
				fAlpha = 0.0f;

			TMath::Clip( fAlpha, 0.0f, 1.0f );

			pFloat->uiColour = ( TUINT32( fAlpha * 255.0f ) << 24 ) | ( pFloat->uiColour & 0xffffff );
			m_oVisibleUnit.Insert( pFloat );
			m_iNumVisibleUnit++;
		}
	}
}
