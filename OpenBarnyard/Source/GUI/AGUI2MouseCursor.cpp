#include "pch.h"
#include "AGUI2MouseCursor.h"
#include "AGUI2TextureSectionManager.h"
#include "AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGUI2MouseCursor::AGUI2MouseCursor()
{
	m_MousePos.x                                = 0.0f;
	m_MousePos.y                                = 0.0f;
	m_CursorCenter                              = { 0.0f, 0.0f };
	m_pMouseDevice                              = TNULL;
	m_pPointerUpSection                         = TNULL;
	m_pPointerDownSection                       = TNULL;
	m_fCursorAccelerations[ Acceleration_Off ]  = 1.0f;
	m_fCursorAccelerations[ Acceleration_Slow ] = 10.0f;
	m_fCursorAccelerations[ Acceleration_Fast ] = 20.0f;
	m_eAccelerationType                         = Acceleration_Fast;
	m_RadialFieldPos                            = TVector2::VEC_ZERO;
	m_fRadialRadius                             = -1.0f;
}

TBOOL AGUI2MouseCursor::Create( const TCHAR* a_szPointerUpTexture, const TCHAR* a_szPointerDownTexture, TFLOAT a_fScalar )
{
	m_pMouseDevice = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 );

	if ( m_pMouseDevice == TNULL )
	{
		return TFALSE;
	}

	m_pPointerUpSection  = AGUI2TextureSectionManager::GetTextureSection( a_szPointerUpTexture );
	TFLOAT fCursorWidth  = m_pPointerUpSection->GetWidth();
	TFLOAT fCursorHeight = m_pPointerUpSection->GetHeight();

	AGUI2Rectangle::SetDimensions( fCursorWidth, fCursorHeight );
	AGUI2Element::GetTransform().Scale( a_fScalar, a_fScalar );
	AGUI2Rectangle::SetTextureSection( m_pPointerUpSection );
	AGUI2Element::SetAttachment( AGUI2ATTACHMENT_MIDDLECENTER, AGUI2ATTACHMENT_TOPLEFT );
	m_CursorCenter = { -fCursorWidth / 2, -fCursorHeight / 2 };

	if ( a_szPointerDownTexture )
	{
		m_pPointerDownSection = AGUI2TextureSectionManager::GetTextureSection( a_szPointerDownTexture );
	}

	AGUI2::GetRootElement()->AddChildTail( *this );
	AGUI2Element::Hide();

	return TTRUE;
}

void AGUI2MouseCursor::Update()
{
	if ( m_pMouseDevice )
	{
		MoveCursor(
		    m_pMouseDevice->GetAxisFloat( TInputDeviceMouse::AXIS_CURSOR, 0 ),
		    m_pMouseDevice->GetAxisFloat( TInputDeviceMouse::AXIS_CURSOR, 1 ) );

		if ( !m_pPointerDownSection || !m_pMouseDevice->IsDown( TInputDeviceMouse::BUTTON_1 ) )
		{
			AGUI2Rectangle::SetTextureSection( m_pPointerUpSection );
			SetDimensions( m_pPointerUpSection->GetWidth(), m_pPointerUpSection->GetHeight() );
			m_bIsMouseDown = TFALSE;
		}
		else
		{
			AGUI2Rectangle::SetTextureSection( m_pPointerDownSection );
			SetDimensions( m_pPointerDownSection->GetWidth(), m_pPointerDownSection->GetHeight() );
			m_bIsMouseDown = TTRUE;
		}

		AGUI2Element::GetTransform().SetTranslation( m_MousePos.x, m_MousePos.y );

		AGUI2Transform oScreenTransform;
		GetScreenTransform( oScreenTransform );
		oScreenTransform.Transform( m_CursorPos, m_CursorCenter );
	}

	return;
}

void AGUI2MouseCursor::MoveCursor( TFLOAT a_fDeltaX, TFLOAT a_fDeltaY )
{
	TFLOAT fCursorAccelerationX;
	TFLOAT fCursorAccelerationY;

	// X
	fCursorAccelerationX = 1.0f;

	if ( m_eAccelerationType != Acceleration_Fast || TMath::Abs( a_fDeltaX ) <= m_fCursorAccelerations[ Acceleration_Fast ] )
	{
		if ( m_eAccelerationType != Acceleration_Off && m_fCursorAccelerations[ Acceleration_Slow ] < TMath::Abs( a_fDeltaX ) )
		{
			fCursorAccelerationX = 2.0f;
		}
	}
	else
	{
		fCursorAccelerationX = 4.0f;
	}

	// Y
	fCursorAccelerationY = 1.0f;

	if ( m_eAccelerationType != Acceleration_Off && m_fCursorAccelerations[ Acceleration_Slow ] < TMath::Abs( a_fDeltaY ) )
	{
		fCursorAccelerationY = 4.0f;
	}

	if ( m_eAccelerationType == Acceleration_Fast && m_fCursorAccelerations[ Acceleration_Fast ] < TMath::Abs( a_fDeltaY ) )
	{
		fCursorAccelerationY = 4.0f;
	}

	m_MousePos.x += fCursorAccelerationX * a_fDeltaX * m_fCursorAccelerations[ Acceleration_Off ];
	m_MousePos.y += fCursorAccelerationY * a_fDeltaY * m_fCursorAccelerations[ Acceleration_Off ];

	TFLOAT fMinsX, fMinsY;
	TFLOAT fMaxsX, fMaxsY;
	AGUI2::GetRootElement()->GetMins( fMinsX, fMinsY );
	AGUI2::GetRootElement()->GetMaxs( fMaxsX, fMaxsY );

	if ( m_MousePos.x < fMinsX ) m_MousePos.x = fMinsX;
	if ( m_MousePos.x > fMaxsX ) m_MousePos.x = fMaxsX;
	if ( m_MousePos.y < fMinsY ) m_MousePos.y = fMinsY;
	if ( m_MousePos.y > fMaxsY ) m_MousePos.y = fMaxsY;

	if ( m_fRadialRadius != -1.0f )
	{
		Toshi::TVector2 vec = m_MousePos - m_RadialFieldPos;

		if ( m_fRadialRadius < vec.Magnitude() )
		{
			vec.Normalize();
			m_MousePos.x = vec.x * m_fRadialRadius + m_RadialFieldPos.x;
			m_MousePos.y = vec.y * m_fRadialRadius + m_RadialFieldPos.y;
		}
	}
}
