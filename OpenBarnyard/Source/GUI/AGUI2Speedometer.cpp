#include "pch.h"
#include "AGUI2Speedometer.h"
#include "AGUI2.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TUINT32 TEXT_COLOUR = 0xFFFFC800;

// NOTE: TMathInline::MoveTowards, moves a_rfValue towards a_fTarget by a_fStep without going past it
static void MoveTowards( TFLOAT& a_rfValue, TFLOAT a_fTarget, TFLOAT a_fStep )
{
	if ( a_rfValue < a_fTarget )
	{
		a_rfValue += a_fStep;
		if ( a_rfValue > a_fTarget ) a_rfValue = a_fTarget;
	}
	else
	{
		a_rfValue -= a_fStep;
		if ( a_rfValue < a_fTarget ) a_rfValue = a_fTarget;
	}
}

// $Barnyard: FUNCTION 0064c310
AGUI2Speedometer::AGUI2Speedometer()
    : m_fMaxAngle( 180.0f )
    , m_fNeedleLength( 1.0f )
    , m_fValue( 0.0f )
    , m_fTargetValue( 0.0f )
    , m_bDigital( TFALSE )
{
}

// $Barnyard: FUNCTION 0064c3e0
AGUI2Speedometer::~AGUI2Speedometer()
{
	if ( m_bDigital == TTRUE )
	{
		m_oDigitalText.RemoveSelf();
		m_oDigitalUnitText.RemoveSelf();
	}
	else
	{
		m_oNeedle.RemoveSelf();
		m_oDial.RemoveSelf();
		m_oDialContainer.RemoveSelf();
		m_oRoot.RemoveSelf();
	}
}

// $Barnyard: FUNCTION 0064c6a0
void AGUI2Speedometer::Create( TBOOL a_bDigital, T2GUIElement* a_pParent )
{
	m_bDigital = a_bDigital;

	if ( a_bDigital == TTRUE )
	{
		// The speed
		m_oDigitalText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 0.0f );
		m_oDigitalText.SetTextAndFitWidth( m_wszSpeed );
		m_oDigitalText.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
		m_oDigitalText.SetTransform( -100.0f, -20.0f, 0.0f );
		m_oDigitalText.SetColour( TEXT_COLOUR );

		// The unit
		m_oDigitalUnitText.Create( T2GUIFontManager::FindFont( "Rekord18" ), 0.0f );
		m_oDigitalUnitText.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 69 ) );
		m_oDigitalUnitText.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
		m_oDigitalUnitText.SetColour( TEXT_COLOUR );

		m_oDigitalText.AddChildTail( m_oDigitalUnitText );
		m_oDigitalText.GetTransform().Scale( 1.25f, 1.25f );
		a_pParent->AddChildTail( m_oDigitalText );
	}
	else
	{
		// The dial is a quarter of the screen wide
		T2GUITextureSection* pDialSection  = T2GUITextureSectionManager::GetTextureSection( "Chase_Speedo" );
		TFLOAT               fSectionWidth = pDialSection->GetWidth();
		TFLOAT               fDialWidth    = AGUI2::GetRootElement()->GetWidth() * 0.25f;
		m_fDialWidth                       = fDialWidth;

		TFLOAT fInvSectionWidth = 1.0f / fSectionWidth;
		TFLOAT fDialHeight      = pDialSection->GetHeight() * fInvSectionWidth * fDialWidth;

		m_oDial.SetDimensions( fDialWidth, fDialHeight );
		m_oDial.SetTextureSection( pDialSection );

		// The needle at value 0 points to the left end of the dial, the pivot is the centre
		// of the circle that goes through both ends and the top of the dial
		m_vNeedlePosition.Set( fDialWidth * -0.5f, 0.0f, 0.0f, 1.0f );
		m_vPivot.Set( 0.0f, 0.0f, ( fDialHeight * fDialHeight - fDialWidth * fDialWidth * 0.25f ) / ( fDialHeight + fDialHeight ), 1.0f );

		m_vNeedleDirection.Set(
		    m_vNeedlePosition.x - m_vPivot.x,
		    m_vNeedlePosition.y - m_vPivot.y,
		    m_vNeedlePosition.z - m_vPivot.z,
		    m_vNeedlePosition.w
		);
		m_fNeedleLength = m_vNeedleDirection.NormaliseMag();

		// Value 1 is the right end of the dial
		TVector4 vMaxDirection( fDialWidth * 0.5f - m_vPivot.x, -m_vPivot.y, -m_vPivot.z, 1.0f );
		vMaxDirection.Normalise();

		m_fMaxAngle          = vMaxDirection.GetAngleBetweenVector( m_vNeedleDirection );
		m_vNeedleDirection.z = -m_vNeedleDirection.z;

		TFLOAT fContainerSize = m_fNeedleLength + m_fNeedleLength;
		m_oDialContainer.SetDimensions( fContainerSize, fContainerSize );

		T2GUITextureSection* pNeedleSection      = T2GUITextureSectionManager::GetTextureSection( "needle" );
		TFLOAT               fNeedleSectionWidth = pNeedleSection->GetWidth();
		TFLOAT               fNeedleWidth        = fInvSectionWidth * fDialWidth * fNeedleSectionWidth;
		TFLOAT               fNeedleHeight       = pNeedleSection->GetHeight() / fNeedleSectionWidth * fNeedleWidth;

		m_oNeedle.SetDimensions( fNeedleWidth, fNeedleHeight );
		m_oNeedle.SetTextureSection( pNeedleSection );
		m_oNeedle.FlipV();

		m_oRoot.SetDimensions( fDialWidth, fNeedleHeight + fDialHeight );

		TStringManager::String16Format( m_wszSpeed, TARRAYSIZE( m_wszSpeed ), L"00:00" );
		TStringManager::String16Format( m_wszUnit, TARRAYSIZE( m_wszUnit ), L"N" );

		T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );
		m_oSpeedText.Create( pFont, fDialWidth );
		m_oSpeedText.SetText( m_wszSpeed );
		m_oUnitText.Create( pFont, fDialWidth );
		m_oUnitText.SetText( m_wszUnit );

		m_oUnitText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		m_oSpeedText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oDial.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oDialContainer.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oNeedle.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_BOTTOMCENTER );

		m_oSpeedText.AddChildTail( m_oUnitText );
		m_oDialContainer.AddChildTail( m_oDial );
		m_oDialContainer.AddChildTail( m_oNeedle );
		m_oRoot.AddChildTail( m_oDialContainer );
		m_oRoot.AddChildTail( m_oSpeedText );
		m_oRoot.SetAlpha( 0.0f );
		m_oDialContainer.SetAlpha( 0.0f );
		a_pParent->AddChildTail( m_oRoot );

		UpdateNeedle();

		if ( m_bDigital != TTRUE )
		{
			m_oSpeedText.SetAlpha( 0.0f );
			m_oUnitText.SetAlpha( 0.0f );
		}
	}
}

// $Barnyard: FUNCTION 0064cb50
void AGUI2Speedometer::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bVisible )
	{
		MoveTowards( m_fValue, m_fTargetValue, a_fDeltaTime );

		if ( m_bDigital )
			m_oDigitalText.SetTextAndFitWidth( m_wszSpeed );
		else
			UpdateNeedle();
	}
}

// $Barnyard: FUNCTION 0064c640
void AGUI2Speedometer::SetVisible( TBOOL a_bVisible )
{
	m_bVisible = a_bVisible;

	if ( m_bDigital == TTRUE )
		m_oDigitalText.SetVisible( a_bVisible == TTRUE );
	else
		m_oRoot.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 0064c610
void AGUI2Speedometer::SetSpeed( TFLOAT a_fSpeed )
{
	TStringManager::String16Format( m_wszSpeed, TARRAYSIZE( m_wszSpeed ), L"%d", TINT( a_fSpeed ) );
}

// $Barnyard: FUNCTION 0064c500
void AGUI2Speedometer::UpdateNeedle()
{
	TVector4 vDirection = m_vNeedleDirection;
	vDirection.AsVector3().RotateY( -( m_fValue * m_fMaxAngle ) );

	m_vNeedlePosition.Multiply( vDirection, m_fNeedleLength );

	TVector4 vCross;
	vCross.CrossProduct( vDirection, TVector4( 0.0f, 0.0f, 1.0f, 1.0f ) );
	TFLOAT fAngle = vDirection.GetAngleBetweenVector( TVector4( 0.0f, 0.0f, 1.0f, 1.0f ) );

	T2GUITransform& rTransform = m_oNeedle.GetTransform();
	rTransform.SetIdentity();
	rTransform.SetTranslation( m_vNeedlePosition.x, m_vNeedlePosition.z );

	if ( vCross.y > 0.0f )
		rTransform.Rotate( fAngle );
	else
		rTransform.Rotate( -fAngle );
}
