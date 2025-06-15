#include "pch.h"
#include "ADebugFlyState.h"

#include "AGameStateController.h"
#include "Cameras/ACameraManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2FontManager.h"

#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceMouse.h>
#include <Toshi/T2String16.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ADebugFlyState );

ADebugFlyState::ADebugFlyState()
{
	m_aUpAxisInput[ 0 ]      = 0.0f;
	m_aUpAxisInput[ 1 ]      = 0.0f;
	m_aForwardAxisInput[ 0 ] = 0.0f;
	m_aForwardAxisInput[ 1 ] = 0.0f;
	m_aRightAxisInput[ 0 ]   = 0.0f;
	m_aRightAxisInput[ 1 ]   = 0.0f;
	m_pOldHelper             = TNULL;

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	AGUI2Font* pFont = AGUI2FontManager::FindFont( "Rekord18" );
	m_pInfoTextBox   = new AGUI2TextBox;

	m_pInfoTextBox->Create( pFont, 600.0f );
	m_pInfoTextBox->SetText( L"" );
	m_pInfoTextBox->SetColour( TCOLOR( 255, 255, 255 ) );
	m_pInfoTextBox->SetTransform( 0.0f, fHeight / 2 - 32.0f );
	m_pInfoTextBox->SetAlpha( 1.0f );
	m_pInfoTextBox->SetTextAlign( AGUI2Font::TextAlign_Left );
	m_pInfoTextBox->SetInFront();
	m_pInfoTextBox->SetAttachment( AGUI2ATTACHMENT_MIDDLELEFT, AGUI2ATTACHMENT_MIDDLELEFT );

	AGUI2::GetRootElement()->AddChildTail( *m_pInfoTextBox );
}

ADebugFlyState::~ADebugFlyState()
{
	delete m_pInfoTextBox;
}

TBOOL ADebugFlyState::OnUpdate( TFLOAT a_fDeltaTime )
{
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( pCamera )
	{
		TVector4 vecForward = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
		TVector4 vecUp      = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
		TVector4 vecRight   = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_RIGHT );

		vecForward.Multiply( m_fSpeed * ( m_aForwardAxisInput[ 0 ] + m_aForwardAxisInput[ 1 ] ) * a_fDeltaTime );
		vecUp.Multiply( m_fSpeed * ( m_aUpAxisInput[ 0 ] + m_aUpAxisInput[ 1 ] ) * a_fDeltaTime );
		vecRight.Multiply( m_fSpeed * ( m_aRightAxisInput[ 0 ] + m_aRightAxisInput[ 1 ] ) * a_fDeltaTime );

		pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ).Add( vecUp );
		pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ).Add( vecForward );
		pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ).Add( vecRight );

		TVector4 vecTranslation = pCamera->GetMatrix().GetTranslation();
		m_vecRotation.SetFromEulerRollPitchYaw( m_fRoll, m_fPitch, m_fYaw );

		pCamera->GetMatrix().SetFromQuaternion( m_vecRotation );
		pCamera->GetMatrix().SetTranslation( vecTranslation );

		static T2FormatWString512 s_FormatString;
		s_FormatString.Format( L"Speed: %.2f; Position: (%.2f %.2f %.2f)\nPress Escape to leave the fly state.", m_fSpeed, vecTranslation.x, vecTranslation.y, vecTranslation.z );

		m_pInfoTextBox->SetText( s_FormatString.Get() );
	}

	return TTRUE;
}

void ADebugFlyState::OnActivate()
{
	m_pOldHelper       = ACameraManager::GetSingleton()->GetCurrentCameraHelper();
	m_vecLookDirection = ACameraManager::GetSingleton()->GetCurrentCamera()->GetOrientation();
}

void ADebugFlyState::OnRemoval()
{
	if ( m_pOldHelper )
	{
		ACameraManager::GetSingleton()->SetCameraHelper( m_pOldHelper );
		m_pOldHelper = TNULL;
	}
}

TBOOL ADebugFlyState::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	auto        pEventSource = a_pInputEvent->GetSource();
	const TBOOL bIsDown      = a_pInputEvent->GetEventType() == Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN;

	if ( pEventSource->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
	{
		auto iDoodad = a_pInputEvent->GetDoodad();

		switch ( a_pInputEvent->GetDoodad() )
		{
			case Toshi::TInputDeviceKeyboard::KEY_ESCAPE:
				AGameStateController::GetSingleton()->PopCurrentGameState();
				return TTRUE;

			case Toshi::TInputDeviceKeyboard::KEY_W:
				m_aForwardAxisInput[ 0 ] = a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ? 1.0f : 0.0f;
				return TTRUE;
			case Toshi::TInputDeviceKeyboard::KEY_S:
				m_aForwardAxisInput[ 1 ] = a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ? -1.0f : 0.0f;
				return TTRUE;

			case Toshi::TInputDeviceKeyboard::KEY_SPACE:
				m_aUpAxisInput[ 0 ] = a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ? -1.0f : 0.0f;
				return TTRUE;
			case Toshi::TInputDeviceKeyboard::KEY_RSHIFT:
			case Toshi::TInputDeviceKeyboard::KEY_LSHIFT:
				m_aUpAxisInput[ 1 ] = a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ? 1.0f : 0.0f;
				return TTRUE;

			case Toshi::TInputDeviceKeyboard::KEY_A:
				m_aRightAxisInput[ 0 ] = a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ? -1.0f : 0.0f;
				return TTRUE;
			case Toshi::TInputDeviceKeyboard::KEY_D:
				m_aRightAxisInput[ 1 ] = a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ? 1.0f : 0.0f;
				return TTRUE;
		}
	}

	if ( pEventSource->IsA( &TGetClass( TInputDeviceMouse ) ) )
	{
		auto iDoodad = a_pInputEvent->GetDoodad();

		if ( iDoodad == TInputDeviceMouse::AXIS_CURSOR )
		{
			m_fPitch += a_pInputEvent->GetMagnitudeInt( 0 ) / 512.0f;
			m_fRoll -= a_pInputEvent->GetMagnitudeInt( 1 ) / 512.0f;

			TMath::Clip( m_fRoll, -TMath::PI / 2, TMath::PI / 2 );
		}

		if ( iDoodad == TInputDeviceMouse::AXIS_WHEEL )
		{
			m_fSpeed += a_pInputEvent->GetMagnitudeInt( 1 ) / 2.0f;
		}
	}

	return TFALSE;
}

//TFLOAT ADebugFlyState::GetFOV()
//{
//	return *(TFLOAT*)0x007822ac;
//}
