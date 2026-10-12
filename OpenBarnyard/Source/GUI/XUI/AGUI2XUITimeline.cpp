#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUITimeline.h"
#  include "AGUI2XUIElement.h"
#  include "AGUI2XUIResource.h"
#  include "AGUI2XUIShapeCache.h"

#  include <Math/TMathInline.h>
#  include <Toshi/T2String16.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Default directions of the ease curve control points, set by every AGUI2XUITimeline constructor
static TVector2 s_vecEaseIn;
static TVector2 s_vecEaseOut;

static const TWCHAR* s_aStdNamedFrames[ AGUI2XUITimelineControl::STDFRAME_NUMOF ] = {
	L"Normal",
	L"EndNormal",
	L"NormalDisable",
	L"EndNormalDisable",
	L"Focus",
	L"EndFocus",
	L"FocusLoop",
	L"FocusDisable",
	L"EndFocusDisable",
	L"InactiveFocusLoop",
	L"InitFocus",
	L"EndInitFocus",
	L"InitFocusDisable",
	L"EndInitFocusDisable",
	L"KillFocus",
	L"Press",
	L"EndPress",
	L"NormalPress",
	L"EndNormalPress",
	L"PressDisable",
	L"EndPressDisable",
	L"NormalCheck",
	L"EndNormalCheck",
	L"FocusCheck",
	L"EndFocusCheck",
	L"FocusLoop",
	L"InitFocusCheck",
	L"EndInitFocusCheck",
	L"NormalCheckDisable",
	L"EndNormalCheckDisable",
	L"FocusCheckDisable",
	L"EndFocusCheckDisable",
	L"InitFocusCheckDisable",
	L"EndInitFocusCheckDisable",
	L"NormalSel",
	L"EndNormalSel",
	L"NormalSelCheck",
	L"EndNormalSelCheck",
	L"NormalSelDisable",
	L"EndNormalSelDisable",
	L"NormalSelCheckDisable",
	L"EndNormalSelCheckDisable",
	L"ScrollMore",
	L"EndScrollMore",
	L"Scrolling",
	L"EndScrolling",
	L"ScrollMoreDisable",
	L"EndScrollMoreDisable",
	L"TransTo",
	L"TransToEnd",
	L"TransFrom",
	L"TransFromEnd",
	L"TransBackTo",
	L"TransBackToEnd",
	L"TransBackFrom",
	L"TransBackFromEnd",
};

//-----------------------------------------------------------------------------
// AGUI2XUITimeline
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 8041ec00
AGUI2XUITimeline::AGUI2XUITimeline()
{
	m_wszElementId     = TNULL;
	m_pElement         = TNULL;
	m_fDuration        = 0.0f;
	m_fTime            = -1.0f;
	m_iKeyframe        = 0;
	m_pResource        = TNULL;
	m_pData            = TNULL;
	m_bKeyframeChanged = TFALSE;

	s_vecEaseIn.y = TVector2::VEC_POSX.y + TVector2::VEC_POSY.y;
	s_vecEaseIn.x = TVector2::VEC_POSX.x + TVector2::VEC_POSY.x;
	s_vecEaseIn.Normalize();
	s_vecEaseOut.x = -s_vecEaseIn.x;
	s_vecEaseOut.y = -s_vecEaseIn.y;

	m_aEaseCurve[ 0 ] = TVector2::VEC_ZERO;
	m_aEaseCurve[ 1 ] = s_vecEaseIn;
	m_aEaseCurve[ 3 ] = TVector2( 1.0f, 1.0f );
	m_aEaseCurve[ 2 ] = s_vecEaseOut;
}

// $BarnyardWiiJP: FUNCTION 8041ed14
AGUI2XUITimeline::~AGUI2XUITimeline()
{
}

// $BarnyardWiiJP: FUNCTION 8041ed54
TBOOL AGUI2XUITimeline::Create( AGUI2XUIResource& a_rResource, XURXUITimelineData* a_pData, AGUI2XUIElement* a_pOwner )
{
	m_wszElementId = a_rResource.GetString( a_pData->m_usElementId );
	m_pElement     = a_pOwner->FindChildElement( a_rResource.GetString( a_pData->m_usElementId ) );
	m_pData        = a_pData;
	m_pResource    = &a_rResource;
	m_fDuration    = XUI_FRAME_TIME * TFLOAT( a_pData->m_pKeyframes[ a_pData->m_uiNumKeyframes - 1 ].m_uiTime );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041ee0c
void AGUI2XUITimeline::Update( TFLOAT a_fDeltaTime )
{
	if ( m_pData->m_uiNumKeyframes == 0 || m_fTime == -1.0f )
		return;

	if ( m_bKeyframeChanged == TTRUE )
	{
		XURXUIKeyframeData& rKeyframe = m_pData->m_pKeyframes[ m_iKeyframe ];

		if ( rKeyframe.m_ucInterpolation == XURXUIKeyframeData::INTERPOLATION_EASE )
			SetEase( rKeyframe.m_cEaseIn, rKeyframe.m_cEaseOut, TUINT8( rKeyframe.m_cEaseScale ) );
	}

	XURXUIKeyframeData* pKeyframe     = &m_pData->m_pKeyframes[ m_iKeyframe ];
	XURXUIKeyframeData* pPrevKeyframe = &m_pData->m_pKeyframes[ m_iKeyframe - 1 ];

#  ifdef BARNYARD_COMMUNITY_PATCH
	// The original reads the keyframe before the first one when a timeline has a single keyframe
	if ( m_iKeyframe == 0 )
		pPrevKeyframe = pKeyframe;
#  endif

	TFLOAT fPrevTime = XUI_FRAME_TIME * TFLOAT( pPrevKeyframe->m_uiTime );
	TFLOAT fTime     = XUI_FRAME_TIME * TFLOAT( pKeyframe->m_uiTime );
	TFLOAT fProgress = ( m_fTime - fPrevTime ) / ( fTime - fPrevTime );

#  ifdef BARNYARD_COMMUNITY_PATCH
	if ( pPrevKeyframe == pKeyframe )
		fProgress = 1.0f;
#  endif

	TMath::Clip( fProgress, 0.0f, 1.0f );

	m_pElement->RestoreOriginalTransform();

	if ( pKeyframe->m_ucInterpolation == XURXUIKeyframeData::INTERPOLATION_EASE )
	{
		fProgress = GetEaseScalar( fProgress );
	}
	else if ( pKeyframe->m_ucInterpolation == XURXUIKeyframeData::INTERPOLATION_NONE )
	{
		fProgress = ( m_fTime < m_fDuration ) ? 0.0f : 1.0f;
	}

	for ( TUINT32 i = 0; i < m_pData->m_uiNumProps; i++ )
	{
		XURXUITimelineProp& rProp = m_pData->m_pProps[ i ];

		TINT iIndex = ( rProp.uiProp - XURPROP_FIGURE_FILL_GRADIENT_STOPCOLOR > 1 ) ? -1 : rProp.uiIndex;

		if ( m_pData->m_pElementData->IsFloatPropType( rProp.uiProp ) == TTRUE )
		{
			m_pElement->UpdateTimelinePropFloat(
			    *m_pResource,
			    rProp.uiProp,
			    iIndex,
			    m_bKeyframeChanged,
			    pPrevKeyframe->m_pValues[ i ].fValue,
			    pKeyframe->m_pValues[ i ].fValue,
			    fProgress
			);
		}
		else
		{
			m_pElement->UpdateTimelinePropValue(
			    *m_pResource,
			    rProp.uiProp,
			    pPrevKeyframe->m_pValues[ i ].uiValue,
			    pKeyframe->m_pValues[ i ].uiValue,
			    iIndex,
			    m_bKeyframeChanged,
			    fProgress
			);
		}

		m_bKeyframeChanged = TFALSE;
	}
}

// $BarnyardWiiJP: FUNCTION 8041f0b8
void AGUI2XUITimeline::Reset()
{
	m_iKeyframe = 0;
	m_pElement->RestoreOriginalTransform();

	for ( TUINT32 i = 0; i < m_pData->m_uiNumProps; i++ )
	{
		XURXUITimelineProp& rProp = m_pData->m_pProps[ i ];

		TINT iIndex = ( rProp.uiProp - XURPROP_FIGURE_FILL_GRADIENT_STOPCOLOR > 1 ) ? -1 : rProp.uiIndex;

		if ( m_pData->m_pElementData->IsFloatPropType( rProp.uiProp ) == TTRUE )
			m_pElement->UpdateTimelinePropFloat( *m_pResource, rProp.uiProp, iIndex, TTRUE, m_pData->m_pKeyframes[ 0 ].m_pValues[ i ].fValue, 0.0f, 0.0f );
		else
			m_pElement->UpdateTimelinePropValue( *m_pResource, rProp.uiProp, m_pData->m_pKeyframes[ 0 ].m_pValues[ i ].uiValue, 0, iIndex, TTRUE, 0.0f );
	}

	m_bKeyframeChanged = TFALSE;
	m_fTime            = -1.0f;
}

// $BarnyardWiiJP: FUNCTION 8041f228
void AGUI2XUITimeline::SetEase( TINT a_iEaseIn, TINT a_iEaseOut, TUINT a_uiEaseScale )
{
	static TFLOAT s_fMaxAngle = TMath::PI / 4;

	TFLOAT fScale = TFLOAT( a_uiEaseScale ) / 100.0f;

	m_aEaseCurve[ 1 ] = s_vecEaseIn;
	m_aEaseCurve[ 1 ].Rotate( ( TFLOAT( a_iEaseIn ) / 100.0f ) * s_fMaxAngle );
	m_aEaseCurve[ 1 ].x *= fScale;
	m_aEaseCurve[ 1 ].y *= fScale;

	m_aEaseCurve[ 2 ] = s_vecEaseOut;
	m_aEaseCurve[ 2 ].Rotate( ( TFLOAT( a_iEaseOut ) / 100.0f ) * s_fMaxAngle );
	m_aEaseCurve[ 2 ].y = 1.0f + m_aEaseCurve[ 2 ].y * fScale;
	m_aEaseCurve[ 2 ].x = 1.0f + m_aEaseCurve[ 2 ].x * fScale;
}

// $BarnyardWiiJP: FUNCTION 8041f38c
TFLOAT AGUI2XUITimeline::GetEaseScalar( TFLOAT a_fT )
{
	TVector2 vecPoint;
	AGUI2XUIShapeCache::EvaluateBezier( vecPoint, m_aEaseCurve, 3, a_fT );

	return vecPoint.y;
}

// $BarnyardWiiJP: FUNCTION 8041f3c0
void AGUI2XUITimeline::SetTime( TBOOL a_bClamp, TFLOAT a_fTime )
{
	TFLOAT fLastTime = XUI_FRAME_TIME * TFLOAT( m_pData->m_pKeyframes[ m_pData->m_uiNumKeyframes - 1 ].m_uiTime );

	if ( a_bClamp == TTRUE )
		m_fTime = ( fLastTime >= a_fTime ) ? a_fTime : fLastTime;
	else
		m_fTime = fmodf( a_fTime, fLastTime );

	TUINT32 uiKeyframe = 0;

	while ( m_fTime >= XUI_FRAME_TIME * TFLOAT( m_pData->m_pKeyframes[ uiKeyframe ].m_uiTime ) && uiKeyframe < m_pData->m_uiNumKeyframes - 1 )
		uiKeyframe++;

	if ( uiKeyframe > TUINT32( m_iKeyframe ) || ( uiKeyframe < TUINT32( m_iKeyframe ) && m_iKeyframe == m_pData->m_uiNumKeyframes - 1 ) )
		m_bKeyframeChanged = TTRUE;

	m_iKeyframe = uiKeyframe;

	if ( a_bClamp == TTRUE && uiKeyframe > m_pData->m_uiNumKeyframes - 1 )
		m_iKeyframe = m_pData->m_uiNumKeyframes - 1;
}

//-----------------------------------------------------------------------------
// AGUI2XUITimelineControl
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 8040eab0
AGUI2XUITimelineControl::AGUI2XUITimelineControl()
{
	m_pOwner    = TNULL;
	m_fTime     = -1.0f;
	m_fDuration = 0.0f;
}

// $BarnyardWiiJP: FUNCTION 8040eb04
AGUI2XUITimelineControl::~AGUI2XUITimelineControl()
{
	while ( !m_llNamedFrames.IsEmpty() )
	{
		NamedFrame* pFrame = m_llNamedFrames.PopBack();

		if ( pFrame )
			delete pFrame;
	}

	while ( !m_llTimelines.IsEmpty() )
		delete m_llTimelines.PopBack();
}

// $BarnyardWiiJP: FUNCTION 8040eba0
TBOOL AGUI2XUITimelineControl::Create( AGUI2XUIElement* a_pOwner, AGUI2XUIResource& a_rResource, XURXUIObjectData* a_pData )
{
	m_pOwner = a_pOwner;

	if ( a_pData->GetNamedFrames() )
		CreateNamedFrames( a_rResource, a_pData );

	if ( a_pData->GetTimelines() )
		CreateTimelines( a_rResource, a_pData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8040ec1c
TBOOL AGUI2XUITimelineControl::CreateNamedFrames( AGUI2XUIResource& a_rResource, XURXUIObjectData* a_pData )
{
	XURXUINamedFrameData* pData = a_pData->GetNamedFrames();

	for ( TUINT32 i = 0; i < a_pData->GetNumNamedFrames(); i++ )
	{
		NamedFrame* pFrame = new NamedFrame();

		if ( pData->m_usName != 0 )
			pFrame->m_wszName = a_rResource.GetString( pData->m_usName );

		if ( pData->m_usTarget != 0 )
			pFrame->m_wszTarget = a_rResource.GetString( pData->m_usTarget );

		pFrame->m_uiTime   = pData->m_uiTime;
		pFrame->m_eCommand = pData->m_eCommand;

		if ( pFrame->m_wszName )
			pFrame->m_iStdFrame = GetStdNamedFrame( pFrame->m_wszName );
		else
			pFrame->m_iStdFrame = -1;

		m_llNamedFrames.PushBack( pFrame );
		pData++;
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8040ed14
TBOOL AGUI2XUITimelineControl::CreateTimelines( AGUI2XUIResource& a_rResource, XURXUIObjectData* a_pData )
{
	for ( TUINT16 i = 0; i < a_pData->GetNumTimelines(); i++ )
	{
		AGUI2XUITimeline* pTimeline = new AGUI2XUITimeline();

		if ( !pTimeline->Create( a_rResource, &a_pData->GetTimelines()[ i ], m_pOwner ) )
		{
			delete pTimeline;
			return TFALSE;
		}

		if ( pTimeline->GetDuration() > m_fDuration )
			m_fDuration = pTimeline->GetDuration();

		m_llTimelines.PushBack( pTimeline );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8040edec
void AGUI2XUITimelineControl::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fTime == -1.0f )
		return;

	for ( auto it = m_llTimelines.Begin(); it != m_llTimelines.End(); it++ )
	{
		it->SetTime( TFALSE, m_fTime );
		it->Update( a_fDeltaTime );
	}
}

// $BarnyardWiiJP: FUNCTION 8040ee8c
AGUI2XUITimelineControl::NamedFrame* AGUI2XUITimelineControl::FindNamedFrame( const TWCHAR* a_wszName )
{
	if ( !a_wszName )
		return TNULL;

	for ( auto it = m_llNamedFrames.Begin(); it != m_llNamedFrames.End(); it++ )
	{
		if ( T2String16::Compare( it->m_wszName, a_wszName ) == 0 )
			return it.Get();
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 8040ef2c
AGUI2XUITimelineControl::NamedFrame* AGUI2XUITimelineControl::FindNextNamedFrame( NamedFrame* a_pFrame )
{
	auto it = m_llNamedFrames.Begin();

	for ( ;; it++ )
	{
		if ( it == m_llNamedFrames.End() )
			return TNULL;

		if ( it.Get() == a_pFrame )
			break;
	}

	auto itNext = it.Next();

	// Wrap around
	if ( itNext == m_llNamedFrames.End() )
		return m_llNamedFrames.Begin().Get();

	return itNext.Get();
}

// $BarnyardWiiJP: FUNCTION 8040efd8
AGUI2XUITimeline* AGUI2XUITimelineControl::FindTimeline( const TWCHAR* a_wszElementId )
{
	for ( auto it = m_llTimelines.Begin(); it != m_llTimelines.End(); it++ )
	{
		if ( T2String16::Compare( it->GetElementId(), a_wszElementId ) == 0 )
			return it.Get();
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 8040f068
void AGUI2XUITimelineControl::Reset()
{
	for ( auto it = m_llTimelines.Begin(); it != m_llTimelines.End(); it++ )
		it->Reset();

	m_fTime = -1.0f;
}

// $BarnyardWiiJP: FUNCTION 8040f0d8
void AGUI2XUITimelineControl::SetTime( TBOOL a_bClamp, TFLOAT a_fTime )
{
	if ( m_fTime != -1.0f && a_fTime == -1.0f )
		Reset();

	m_fTime = a_fTime;

	if ( a_fTime != -1.0f )
	{
		for ( auto it = m_llTimelines.Begin(); it != m_llTimelines.End(); it++ )
		{
			it->SetTime( a_bClamp, a_fTime );
			it->Update( 0.0f );
		}
	}
}

// $BarnyardWiiJP: FUNCTION 8040f1a4
TINT AGUI2XUITimelineControl::GetStdNamedFrame( const TWCHAR* a_wszName )
{
	for ( TINT i = 0; i < STDFRAME_NUMOF; i++ )
	{
		if ( T2String16::Compare( s_aStdNamedFrames[ i ], a_wszName ) == 0 )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// AGUI2XUITimelinePlayer
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 8041f548
AGUI2XUITimelinePlayer::AGUI2XUITimelinePlayer( AGUI2XUITimelineControl* a_pTimelineControl )
{
	m_pFrame           = TNULL;
	m_fStartTime       = 0.0f;
	m_pNextFrame       = TNULL;
	m_fTime            = -1.0f;
	m_fNextFrameTime   = 0.0f;
	m_pTimelineControl = a_pTimelineControl;
	m_pEndFrame        = TNULL;
	m_fEndTime         = -1.0f;
	m_fRangeStart      = -1.0f;
	m_fRangeEnd        = -1.0f;
	m_bPlaying         = TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041f588
AGUI2XUITimelinePlayer::~AGUI2XUITimelinePlayer()
{
}

// $BarnyardWiiJP: FUNCTION 8041f5c8
void AGUI2XUITimelinePlayer::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fTime == -1.0f || !m_bPlaying )
		return;

	m_fTime += a_fDeltaTime;

	if ( m_pEndFrame && m_fTime >= m_fEndTime && m_pEndFrame->m_iStdFrame == AGUI2XUITimelineControl::STDFRAME_ENDPRESS )
		m_pTimelineControl->GetOwner()->OnPress( TTRUE );

	if ( m_fRangeStart == -1.0f )
	{
		if ( m_pTimelineControl->GetNumNamedFrames() > 0 && m_fTime >= m_fNextFrameTime && m_pNextFrame )
		{
			m_pFrame = m_pNextFrame;

			if ( m_pNextFrame->m_eCommand == XURXUINamedFrameData::COMMAND_STOP )
			{
				m_fTime = m_fNextFrameTime;
				Stop();
			}
			else if ( m_pNextFrame->m_eCommand >= XURXUINamedFrameData::COMMAND_STOP && m_pNextFrame->m_eCommand < XURXUINamedFrameData::COMMAND_GOTOANDSTOP )
			{
				m_pFrame = m_pTimelineControl->FindNamedFrame( m_pNextFrame->m_wszTarget );
				m_fTime  = XUI_FRAME_TIME * TFLOAT( m_pFrame->m_uiTime ) + ( m_fTime - m_fNextFrameTime );
			}

			if ( m_bPlaying == TTRUE )
			{
				m_pNextFrame     = m_pTimelineControl->FindNextNamedFrame( m_pFrame );
				m_fNextFrameTime = XUI_FRAME_TIME * TFLOAT( m_pNextFrame->m_uiTime );
			}
		}
	}
	else if ( m_fTime >= m_fRangeEnd )
	{
		m_fTime = m_fRangeStart;
	}

	m_pTimelineControl->SetTime( TFALSE, m_fTime );
	m_pTimelineControl->Tick( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 8041f7bc
void AGUI2XUITimelinePlayer::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	m_fRangeStart = a_fStart;
	m_fRangeEnd   = a_fEnd;
	m_fTime       = a_fStart;
}

// $BarnyardWiiJP: FUNCTION 8041f7d0
void AGUI2XUITimelinePlayer::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	if ( m_pTimelineControl->GetNumNamedFrames() != 0 && a_wszStartFrame )
	{
		m_pFrame = m_pTimelineControl->FindNamedFrame( a_wszStartFrame );

		if ( m_pFrame )
		{
			TFLOAT fStartTime = XUI_FRAME_TIME * TFLOAT( m_pFrame->m_uiTime );
			m_fTime           = fStartTime;
			m_fStartTime      = fStartTime;
			m_pNextFrame      = m_pTimelineControl->FindNextNamedFrame( m_pFrame );
			m_fNextFrameTime  = XUI_FRAME_TIME * TFLOAT( m_pNextFrame->m_uiTime );
		}
		else
		{
			m_fTime = -1.0f;
		}

		if ( a_wszEndFrame )
		{
			m_pEndFrame = m_pTimelineControl->FindNamedFrame( a_wszEndFrame );
			// NOTE: this is the time of the frame after the start one, not the end one
			m_fEndTime = XUI_FRAME_TIME * TFLOAT( m_pNextFrame->m_uiTime );
		}
	}
	else
	{
		m_fTime = 0.0f;
	}

	m_pTimelineControl->SetTime( TFALSE, m_fTime );
	m_pTimelineControl->Tick( 0.0f );
	m_bPlaying = TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041f924
void AGUI2XUITimelinePlayer::Stop()
{
	m_bPlaying       = TFALSE;
	m_fStartTime     = -1.0f;
	m_fNextFrameTime = -1.0f;
	m_fEndTime       = -1.0f;
	m_pFrame         = TNULL;
	m_pNextFrame     = TNULL;
	m_pEndFrame      = TNULL;
	m_fRangeStart    = -1.0f;
	m_fRangeEnd      = -1.0f;
}

// $BarnyardWiiJP: FUNCTION 8041f954
void AGUI2XUITimelinePlayer::StopAndReset()
{
	m_bPlaying       = TFALSE;
	m_fStartTime     = -1.0f;
	m_fNextFrameTime = -1.0f;
	m_fEndTime       = -1.0f;
	m_pFrame         = TNULL;
	m_pNextFrame     = TNULL;
	m_pEndFrame      = TNULL;
	m_fRangeStart    = -1.0f;
	m_fRangeEnd      = -1.0f;
	m_pTimelineControl->Reset();
}

#endif // BARNYARD_LATEST
