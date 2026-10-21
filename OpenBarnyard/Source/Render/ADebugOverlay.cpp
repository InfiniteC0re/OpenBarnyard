#include "pch.h"
#include "ADebugOverlay.h"

#if !defined( TOSHI_FINAL ) && ( defined( TRENDERINTERFACE_DX8 ) || defined( TRENDERINTERFACE_DX11 ) )
#  include "ARenderer.h"
#  include "Cameras/ACameraManager.h"
#  include "Tasks/ARootTask.h"
#  include "Units/AUnit.h"
#  include "Units/AUnitManager.h"
#  include "Units/AUnitNameManager.h"
#  include "Units/AUnitBehaviour.h"
#  include "Units/AUnitType.h"
#  include "UnitMovement/Steering/ASteer.h"
#  include "FSM/AFSMState.h"

#  include <Render/TBackend.h>

#  include <stdarg.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ADebugOverlay::ADebugOverlay()
    : m_pDebugText( TNULL )
    , m_bEnabled( TFALSE )
    , m_iNumLines( 0 )
    , m_iNumShownLines( 0 )
{
	// "-debugoverlay" starts with the overlay on
	if ( TStringManager::String8FindString( GetCommandLineA(), "-debugoverlay" ) )
		m_bEnabled = TTRUE;
}

ADebugOverlay::~ADebugOverlay()
{
	if ( m_pDebugText )
	{
		TRenderD3DInterface::Interface()->DestroyDebugText();
		m_pDebugText = TNULL;
	}
}

void ADebugOverlay::SetEnabled( TBOOL a_bEnabled )
{
	m_bEnabled       = a_bEnabled;
	m_iNumLines      = 0;
	m_iNumShownLines = 0;
}

TDebugText* ADebugOverlay::GetDebugText()
{
	if ( !m_pDebugText )
		m_pDebugText = TRenderD3DInterface::Interface()->CreateDebugText();

	return m_pDebugText;
}

void ADebugOverlay::PrintLine( LINE a_eLine, TUINT32 a_uiColour, const TCHAR* a_szFormat, ... )
{
	if ( m_iNumLines >= MAX_LINES )
		return;

	Line& rLine    = m_aLines[ m_iNumLines ];
	rLine.eLine    = a_eLine;
	rLine.uiColour = a_uiColour;

	va_list args;
	va_start( args, a_szFormat );
	_vsnprintf_s( rLine.szText, MAX_LINE_LENGTH, _TRUNCATE, a_szFormat, args );
	va_end( args );

	// The systems updated more than once a frame print the same line again
	for ( TINT i = 0; i < m_iNumLines; i++ )
	{
		if ( m_aLines[ i ].eLine == a_eLine && !TStringManager::String8Compare( m_aLines[ i ].szText, rLine.szText ) )
			return;
	}

	m_iNumLines++;
}

void ADebugOverlay::PrintWorldText( ACamera* a_pCamera, const TVector4& a_rcPosition, const TCHAR* a_szText, TUINT32 a_uiColour )
{
	TDebugText* pDebugText = GetDebugText();
	TVector2    vScreenPos;

	if ( !pDebugText || !ARenderer::GetSingleton()->ProjectToScreen( a_pCamera, a_rcPosition, vScreenPos ) )
		return;

	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderD3DInterface::Interface()->GetCurrentDisplayParams();

	if ( vScreenPos.x < 0.0f || vScreenPos.x >= TFLOAT( pDisplayParams->uiWidth ) || vScreenPos.y < 0.0f || vScreenPos.y >= TFLOAT( pDisplayParams->uiHeight ) )
		return;

	pDebugText->PrintString( TINT( vScreenPos.x ), TINT( vScreenPos.y ), a_szText, a_uiColour, TDebugText::ENTRYFLAGS_CENTERED );
}

void ADebugOverlay::Render( TFLOAT a_fDeltaTime )
{
	// The lines of a paused frame are the ones of the last frame that wasn't (most systems don't update)
	if ( !ARootTask::GetSingleton()->IsPaused() )
	{
		for ( TINT i = 0; i < m_iNumLines; i++ )
			m_aShownLines[ i ] = m_aLines[ i ];

		m_iNumShownLines = m_iNumLines;
	}

	m_iNumLines = 0;

	if ( !m_bEnabled )
	{
		// The labels of the world debug drawing
		if ( m_pDebugText )
			m_pDebugText->Render();

		return;
	}

	TDebugText* pDebugText = GetDebugText();

	if ( !pDebugText )
		return;

	// The lines, by group
	for ( TINT iGroup = 0; iGroup < LINE_NUMOF; iGroup++ )
	{
		for ( TINT i = 0; i < m_iNumShownLines; i++ )
		{
			if ( m_aShownLines[ i ].eLine == iGroup )
				pDebugText->AddString( m_aShownLines[ i ].szText, m_aShownLines[ i ].uiColour );
		}
	}

	ARootTask* pRootTask = ARootTask::GetSingleton();

	if ( pRootTask->IsGameSystemCreated() && pRootTask->IsRenderWorld() && ACameraManager::IsSingletonCreated() )
	{
		// Names and states of the units
		if ( ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
			RenderUnitLabels( pCamera );
	}

	// Frame time and FPS at the bottom
	if ( a_fDeltaTime > 0.0f )
	{
		TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderD3DInterface::Interface()->GetCurrentDisplayParams();

		TCHAR szLine[ 64 ];
		TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Total = %.3f (%.1f FPS)", a_fDeltaTime * 1000.0f, 1.0f / a_fDeltaTime );
		pDebugText->PrintString( pDisplayParams->uiWidth / 2, pDisplayParams->uiHeight - LINE_HEIGHT * 2, szLine, INFO_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );
	}

	pDebugText->Render();
}

void ADebugOverlay::RenderUnitLabels( ACamera* a_pCamera )
{
	if ( !AUnitManager::IsSingletonCreated() )
		return;

	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderD3DInterface::Interface()->GetCurrentDisplayParams();
	ARenderer*                       pRenderer      = ARenderer::GetSingleton();
	const TVector4&                  rcCameraPos    = a_pCamera->GetMatrix().GetTranslation();

	TFLOAT fScreenWidth  = TFLOAT( pDisplayParams->uiWidth );
	TFLOAT fScreenHeight = TFLOAT( pDisplayParams->uiHeight );

	auto& llUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = llUnits.Begin(); it != llUnits.End(); it++ )
	{
		AUnit* pUnit = it;

		if ( pUnit->IsDying() || !pUnit->GetSteer() )
			continue;

		const TVector4* pPosition = pUnit->GetSteer()->GetPosition();

		TFLOAT fX = pPosition->x - rcCameraPos.x;
		TFLOAT fY = pPosition->y - rcCameraPos.y;
		TFLOAT fZ = pPosition->z - rcCameraPos.z;

		if ( fX * fX + fY * fY + fZ * fZ > MAX_UNIT_DISTANCE * MAX_UNIT_DISTANCE )
			continue;

		// The name and the state above the unit (-Y is up)
		TVector4 vAbove( pPosition->x, pPosition->y - UNIT_LABEL_HEIGHT, pPosition->z, 1.0f );
		TVector2 vLabelPos;

		if ( !pRenderer->ProjectToScreen( a_pCamera, vAbove, vLabelPos ) ||
		     vLabelPos.x < 0.0f || vLabelPos.x >= fScreenWidth || vLabelPos.y < 0.0f || vLabelPos.y >= fScreenHeight )
			continue;

		TPString8 strName;

		if ( AUnitNameManager::IsSingletonCreated() )
			strName = AUnitNameManager::GetSingleton()->GetUnitName( pUnit->GetId() );

		if ( strName.IsEmpty() )
			strName = pUnit->GetType()->GetName();

		TINT iX = TINT( vLabelPos.x );
		TINT iY = TINT( vLabelPos.y );
		m_pDebugText->PrintString( iX, iY, strName.GetString(), UNIT_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );

		AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) );
		AFSMState*      pState     = pBehaviour ? pBehaviour->GetFSMInstance()->GetCurrentState() : TNULL;

		if ( pState )
			m_pDebugText->PrintString( iX, iY + LINE_HEIGHT, pState->GetName().GetString(), UNIT_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );

		TVector4 vFeet( pPosition->x, pPosition->y, pPosition->z, 1.0f );
		TVector2 vFeetPos;

		if ( pBehaviour && pRenderer->ProjectToScreen( a_pCamera, vFeet, vFeetPos ) )
		{
			TCHAR szDepth[ 16 ];
			TStringManager::String8Format( szDepth, TARRAYSIZE( szDepth ), "%d", pBehaviour->GetFSMInstance()->GetStackDepth() );
			m_pDebugText->PrintString( TINT( vFeetPos.x ), TINT( vFeetPos.y ), szDepth, UNIT_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );
		}
	}
}

#endif // !TOSHI_FINAL && ( TRENDERINTERFACE_DX8 || TRENDERINTERFACE_DX11 )
