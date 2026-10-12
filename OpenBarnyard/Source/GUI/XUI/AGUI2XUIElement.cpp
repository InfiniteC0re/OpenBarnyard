#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIElement.h"
#  include "AGUI2XUIResource.h"
#  include "AGUI2XUICanvas.h"
#  include "AGUI2XUIVisual.h"

#  include <GUI/T2GUI.h>
#  include <Math/TMatrix44.h>
#  include <Math/TQuaternion.h>
#  include <Math/TMathInline.h>
#  include <Toshi/T2String16.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIElement );

// Rotation of the element from the Z angle of a quaternion
static TFLOAT GetQuatAngleZ( const TQuaternion& a_rcQuat )
{
	TMatrix44 oMatrix;
	oMatrix.SetFromQuaternion( a_rcQuat );

	TVector3 vecEuler;
	oMatrix.GetEulerXYZ( vecEuler );

	return vecEuler.z;
}

// $BarnyardWiiJP: FUNCTION 80416cf4
AGUI2XUIElement::AGUI2XUIElement()
{
	m_vecPivot           = TVector2::VEC_ZERO;
	m_fZ                 = 0.0f;
	m_fOpacity           = 1.0f;
	m_uiAnchor           = ANCHOR_NONE;
	m_eState             = STATE_NORMAL;
	m_uiBlendMode        = 0;
	m_bTimelineRecursion = TTRUE;
	m_eFlags |= T2GUIElement::FLAGS_XUIELEMENT;
	m_eAnchor = T2GUIATTACHMENT_TOPLEFT;
	m_ePivot  = T2GUIATTACHMENT_TOPLEFT;
}

// $BarnyardWiiJP: FUNCTION 80416d9c
// $BarnyardWiiJP: FUNCTION 80418110
AGUI2XUIElement::~AGUI2XUIElement()
{
	T2GUIElement* pChild = ChildRBegin();

	while ( pChild )
	{
		pChild->RemoveSelf();

		T2GUIElement* pDeleteChild = pChild;
		pChild                     = ChildRBegin();

		delete pDeleteChild;
	}

	RemoveSelf();
}

// $BarnyardWiiJP: FUNCTION 80416e7c
TBOOL AGUI2XUIElement::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	const TWCHAR* wszId = a_rResource.GetString( a_pData->m_usId );

	// Canvases aren't part of the string table paths
	if ( GetClass() != &TGetClass( AGUI2XUICanvas ) )
		a_rResource.PushID( wszId );

	m_wszId   = wszId;
	m_fWidth  = a_pData->m_fWidth;
	m_fHeight = a_pData->m_fHeight;

	if ( a_pData->m_iPosition >= 0 )
	{
		TVector4 vecPosition = a_rResource.GetVector( a_pData->m_iPosition );
		m_oTransform.SetTranslation( vecPosition.x, vecPosition.y );
		m_fZ = vecPosition.z;
	}

	if ( a_pData->m_iRotation >= 0 )
	{
		TQuaternion quatRotation = a_rResource.GetQuat( a_pData->m_iRotation );
		m_oTransform.Rotate( -GetQuatAngleZ( quatRotation ) );
	}

	if ( a_pData->m_iScale >= 0 )
	{
		TVector4 vecScale = a_rResource.GetVector( a_pData->m_iScale );

		T2GUITransform oScale;
		oScale.Set( TVector2( vecScale.x, 0.0f ), TVector2( 0.0f, vecScale.y ), TVector2( 0.0f, 0.0f ) );
		m_oTransform.PreMultiply( oScale );
	}

	m_fOpacity = a_pData->m_fOpacity;
	TMath::Clip( m_fOpacity, 0.0f, 1.0f );
	m_uiAnchor = a_pData->m_uiAnchor;

	if ( a_pData->m_iPivot >= 0 )
	{
		TVector4 vecPivot = a_rResource.GetVector( a_pData->m_iPivot );
		m_vecPivot.x      = vecPivot.x;
		m_vecPivot.y      = vecPivot.y;
	}

	if ( !a_pData->m_bShow )
		m_eFlags &= ~T2GUIElement::FLAGS_VISIBLE;

	m_uiBlendMode        = a_pData->m_uiBlendMode;
	m_bTimelineRecursion = a_pData->m_bDisableTimelineRecursion == TFALSE;
	m_uiColorWriteFlags  = a_pData->m_uiColorWriteFlags;
	m_oOriginalTransform = m_oTransform;

	if ( a_bCreateChildren == TTRUE )
		CreateChildren( a_rResource, a_pData );

	// NOTE: the original doesn't set a return value here
	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80417244
TBOOL AGUI2XUIElement::CreateChildren( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData )
{
	for ( TUINT32 i = 0; i < a_pData->GetNumChildren(); i++ )
	{
		XURXUIElementData* pChildData = TSTATICCAST( XURXUIElementData, a_pData->GetChild( i ) );
		TObject*           pObject    = pChildData->GetUIClass()->CreateObject();
		AGUI2XUIElement*   pChild     = pObject ? TSTATICCAST( AGUI2XUIElement, pObject ) : TNULL;

		if ( pChild )
		{
			pChild->Create( a_rResource, pChildData, TTRUE );

			if ( pChild->GetClass() != &TGetClass( AGUI2XUICanvas ) )
				a_rResource.PopID();
		}

		AddChildTail( *pChild );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80417320
TUINT32 AGUI2XUIElement::MakeARGB( TINT a_iR, TINT a_iG, TINT a_iB, TINT a_iA )
{
	return TUINT8( a_iB ) | TUINT16( a_iG << 8 ) | ( a_iA << 24 ) | ( ( a_iR << 16 ) & 0xFF0000 );
}

// $BarnyardWiiJP: FUNCTION 80417338
void AGUI2XUIElement::SplitARGB( TUINT32 a_uiColour, TINT& a_rA, TINT& a_rR, TINT& a_rG, TINT& a_rB )
{
	a_rA = ( a_uiColour >> 24 );
	a_rR = ( a_uiColour >> 16 ) & 0xFF;
	a_rG = ( a_uiColour >> 8 ) & 0xFF;
	a_rB = a_uiColour & 0xFF;
}

// $BarnyardWiiJP: FUNCTION 8041735c
void AGUI2XUIElement::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	static TVector2 s_vecOldSize;
	static TVector2 s_vecDelta;

	s_vecOldSize.x = m_fWidth;
	s_vecOldSize.y = m_fHeight;

	T2GUIElement::SetDimensions( a_fWidth, a_fHeight );

	TVector2 vecNewSize( m_fWidth, m_fHeight );

	if ( !( s_vecOldSize == vecNewSize ) )
	{
		s_vecDelta.y = s_vecOldSize.y - vecNewSize.y;
		s_vecDelta.x = s_vecOldSize.x - vecNewSize.x;
		UpdateAnchoring( s_vecDelta );
	}
}

// $BarnyardWiiJP: FUNCTION 80417434
void AGUI2XUIElement::UpdateAnchoring( const TVector2& a_rcDelta )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		AGUI2XUIElement* pChild = TSTATICCAST( AGUI2XUIElement, it );

		if ( pChild->GetClass()->IsA( &TGetClass( AGUI2XUIVisual ) ) )
		{
			pChild->UpdateAnchoring( a_rcDelta );
			continue;
		}

		if ( pChild->m_uiAnchor & ANCHOR_RIGHT )
		{
			if ( pChild->m_uiAnchor & ANCHOR_LEFT )
				pChild->m_fWidth -= a_rcDelta.x;
			else
				pChild->m_oTransform.m_vecTranslation.x -= a_rcDelta.x;
		}

		if ( pChild->m_uiAnchor & ANCHOR_BOTTOM )
		{
			if ( pChild->m_uiAnchor & ANCHOR_TOP )
				pChild->m_fHeight -= a_rcDelta.y;
			else
				pChild->m_oTransform.m_vecTranslation.y -= a_rcDelta.y;
		}

		if ( pChild->m_uiAnchor & ANCHOR_CENTER_X )
		{
			TFLOAT fY = pChild->m_oTransform.m_vecTranslation.y;

			if ( pChild->m_uiAnchor & ANCHOR_CENTER_Y )
				fY = pChild->m_oTransform.m_vecTranslation.y - a_rcDelta.y * 0.5f;

			pChild->m_oTransform.m_vecTranslation.x -= a_rcDelta.x * 0.5f;
			pChild->m_oTransform.m_vecTranslation.y = fY;
		}
		else if ( pChild->m_uiAnchor & ANCHOR_CENTER_Y )
		{
			pChild->m_oTransform.m_vecTranslation.y -= a_rcDelta.y * 0.5f;
		}
	}
}

// $BarnyardWiiJP: FUNCTION 804175b4
void AGUI2XUIElement::SetAlpha( TFLOAT a_fAlpha )
{
	T2GUIElement::SetAlpha( a_fAlpha );

	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
		it->SetAlpha( a_fAlpha );
}

// $BarnyardWiiJP: FUNCTION 80417634
void AGUI2XUIElement::SetFocus( TBOOL a_bFocused )
{
	if ( a_bFocused == TTRUE )
	{
		if ( m_eState != STATE_FOCUSED )
		{
			if ( m_eState != STATE_NORMAL )
				PlayAnimation( L"FocusLoop", TNULL );
			else
				PlayAnimation( L"Focus", L"EndFocus" );
		}

		m_eState = STATE_FOCUSED;
	}
	else
	{
		if ( m_eState != STATE_NORMAL )
			PlayAnimation( L"Normal", L"EndNormal" );

		m_eState = STATE_NORMAL;
	}
}

// $BarnyardWiiJP: FUNCTION 804176ec
void AGUI2XUIElement::Tick( TFLOAT a_fDeltaTime )
{
	T2GUIElement::Tick( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 804176f0
void AGUI2XUIElement::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		AGUI2XUIElement* pChild = TSTATICCAST( AGUI2XUIElement, it );

		if ( pChild->HasFlags( T2GUIElement::FLAGS_XUIELEMENT ) && pChild->m_bTimelineRecursion == TTRUE )
			pChild->PlayAnimation( a_wszStartFrame, a_wszEndFrame );
	}
}

// $BarnyardWiiJP: FUNCTION 804177ac
void AGUI2XUIElement::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		// NOTE: unlike PlayAnimation, this checks the flag of the parent
		if ( it->HasFlags( T2GUIElement::FLAGS_XUIELEMENT ) && m_bTimelineRecursion == TTRUE )
			TSTATICCAST( AGUI2XUIElement, it )->PlayTimelineRange( a_fStart, a_fEnd );
	}
}

// $BarnyardWiiJP: FUNCTION 80417868
void AGUI2XUIElement::StopAnimation()
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		if ( it->HasFlags( T2GUIElement::FLAGS_XUIELEMENT ) && m_bTimelineRecursion == TTRUE )
			TSTATICCAST( AGUI2XUIElement, it )->StopAnimation();
	}
}

// $BarnyardWiiJP: FUNCTION 80417904
AGUI2XUIElement* AGUI2XUIElement::FindChildElement( const TWCHAR* a_wszId )
{
	if ( !a_wszId )
		return TNULL;

	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		if ( it->HasFlags( T2GUIElement::FLAGS_XUIELEMENT ) && T2String16::Compare( TSTATICCAST( AGUI2XUIElement, it )->m_wszId, a_wszId ) == 0 )
			return TSTATICCAST( AGUI2XUIElement, it );
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 804179c0
void AGUI2XUIElement::PreRender()
{
	T2GUITransform oTransform;
	oTransform.SetTranslation( m_vecPivot.x, m_vecPivot.y );

	T2GUI::GetRenderer()->PushTransform( oTransform, TVector2::VEC_ZERO, TVector2::VEC_ZERO );
	T2GUIElement::PreRender();
}

// $BarnyardWiiJP: FUNCTION 80417a2c
void AGUI2XUIElement::PostRender()
{
	T2GUI::GetRenderer()->PopTransform();
	T2GUIElement::PostRender();
}

// $BarnyardWiiJP: FUNCTION 80417a74
void AGUI2XUIElement::UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT )
{
	if ( a_uiProp == XURPROP_ELEMENT_ROTATION )
	{
		TQuaternion quatFrom = a_rResource.GetQuat( a_uiFrom );
		TQuaternion quatTo   = a_rResource.GetQuat( a_uiTo );
		TQuaternion::Slerp( quatFrom, quatFrom, quatTo, a_fT );

		m_oTransform.Rotate( -GetQuatAngleZ( quatFrom ) );
	}
	else if ( a_uiProp == XURPROP_ELEMENT_SHOW )
	{
		if ( a_uiFrom == 1 )
			m_eFlags |= T2GUIElement::FLAGS_VISIBLE;
		else
			m_eFlags &= ~T2GUIElement::FLAGS_VISIBLE;
	}
	else if ( a_uiProp == XURPROP_ELEMENT_SCALE )
	{
		const TVector4& rcFrom = a_rResource.GetVector( a_uiFrom );
		TVector2        vecFrom( rcFrom.x, rcFrom.y );

		const TVector4& rcTo = a_rResource.GetVector( a_uiTo );
		TVector2        vecTo( rcTo.x, rcTo.y );

		vecFrom.Lerp( vecFrom, vecTo, a_fT );

		T2GUITransform oScale;
		oScale.Set( TVector2( vecFrom.x, 0.0f ), TVector2( 0.0f, vecFrom.y ), TVector2( 0.0f, 0.0f ) );
		m_oTransform.PreMultiply( oScale );
	}
	else if ( a_uiProp == XURPROP_ELEMENT_POSITION )
	{
		const TVector4& rcFrom = a_rResource.GetVector( a_uiFrom );
		TVector2        vecFrom( rcFrom.x, rcFrom.y );

		const TVector4& rcTo = a_rResource.GetVector( a_uiTo );
		TVector2        vecTo( rcTo.x, rcTo.y );

		vecFrom.Lerp( vecFrom, vecTo, a_fT );
		m_oTransform.SetTranslation( vecFrom.x, vecFrom.y );
	}
}

// $BarnyardWiiJP: FUNCTION 80417e5c
void AGUI2XUIElement::UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT )
{
	if ( a_uiProp == XURPROP_ELEMENT_HEIGHT )
		SetHeight( TMath::LERP( a_fFrom, a_fTo, a_fT ) );
	else if ( a_uiProp == XURPROP_ELEMENT_OPACITY )
		SetAlpha( TMath::LERP( a_fFrom, a_fTo, a_fT ) );
	else if ( a_uiProp == XURPROP_ELEMENT_WIDTH )
		SetWidth( TMath::LERP( a_fFrom, a_fTo, a_fT ) );
}

// $BarnyardWiiJP: FUNCTION 80417f08
TUINT32 AGUI2XUIElement::InterpColour( TUINT32 a_uiFrom, TUINT32 a_uiTo, TFLOAT a_fT )
{
	TVector4 vecFrom(
	    TFLOAT( ( a_uiFrom >> 24 ) ) / 255.0f,
	    TFLOAT( ( a_uiFrom >> 16 ) & 0xFF ) / 255.0f,
	    TFLOAT( ( a_uiFrom >> 8 ) & 0xFF ) / 255.0f,
	    TFLOAT( a_uiFrom & 0xFF ) / 255.0f
	);

	TVector4 vecTo(
	    TFLOAT( ( a_uiTo >> 24 ) ) / 255.0f,
	    TFLOAT( ( a_uiTo >> 16 ) & 0xFF ) / 255.0f,
	    TFLOAT( ( a_uiTo >> 8 ) & 0xFF ) / 255.0f,
	    TFLOAT( a_uiTo & 0xFF ) / 255.0f
	);

	vecFrom.Lerp4( vecFrom, vecTo, a_fT );

	return ( TINT( 255.0f * vecFrom.x ) << 24 ) | ( TINT( 255.0f * vecFrom.y ) << 16 ) | ( TINT( 255.0f * vecFrom.z ) << 8 ) | TINT( 255.0f * vecFrom.w );
}

// $BarnyardWiiJP: FUNCTION 8040f398
const TVector2& AGUI2XUIElement::GetPivot()
{
	return m_vecPivot;
}

#endif // BARNYARD_LATEST
