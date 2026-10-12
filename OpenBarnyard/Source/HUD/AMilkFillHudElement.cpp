#include "pch.h"
#include "AMilkFillHudElement.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00456450
AMilkFillHudElement::AMilkFillHudElement()
{
}

// $Barnyard: FUNCTION 00456510
// $Barnyard: FUNCTION 00456210
AMilkFillHudElement::~AMilkFillHudElement()
{
}

// $Barnyard: FUNCTION 00456380
void AMilkFillHudElement::Tick( TFLOAT a_fDeltaTime )
{
	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00456390
void AMilkFillHudElement::Render()
{
	if ( !IsVisible() )
		return;

	m_oBackground.PreRender();
	m_oBackground.Render();
	m_oBackground.PostRender();

	m_oGlass.PreRender();

	m_oBackPlate.PreRender();
	m_oBackPlate.Render();
	m_oBackPlate.PostRender();

	m_oMask.PreRender();
	m_oMask.Render();
	m_oMask.PostRender();

	m_oGlint.PreRender();
	m_oGlint.Render();
	m_oGlint.PostRender();

	m_oGlass.PostRender();

	T2GUIElement::Render();
}

// $Barnyard: FUNCTION 00456530
TBOOL AMilkFillHudElement::Create()
{
	T2GUITextureSection* pBottle = T2GUITextureSectionManager::GetTextureSection( "Milk_Bottle" );
	SetDimensions(
	    TMath::Abs( TFLOAT( pBottle->GetPoint2X() - pBottle->GetPoint1X() ) ),
	    TMath::Abs( TFLOAT( pBottle->GetPoint2Y() - pBottle->GetPoint1Y() ) )
	);

	m_oGlass.SetDimensions( GetWidth(), GetHeight() );
	m_oBackground.T2GUIElement::SetDimensions( m_oGlass.GetWidth(), m_oGlass.GetHeight() );
	m_oBackground.SetColour( 0 );

	AddChildTail( m_oGlass );
	m_oGlass.Hide();

	m_oBackPlate.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Glass_Back_Plate" ) );
	m_oGlass.AddChildTail( m_oBackPlate );
	m_oBackPlate.Hide();

	// NOTE: the original gets the materials from the rectangles (T2GUIRectangle::GetMaterial,
	// 006c3960), they are the materials of the texture sections set just before.
	T2GUITextureSection* pMask = T2GUITextureSectionManager::GetTextureSection( "Glass_Mask" );
	m_oMask.SetTextureSection( *pMask );
	m_oMask.Hide();
	pMask->GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASK );
	pMask->GetMaterial()->SetName( "GlassMask" );

	T2GUITextureSection* pMilk = T2GUITextureSectionManager::GetTextureSection( "Glass_Milk" );
	m_oMilk.SetTextureSection( *pMilk );
	m_oMask.AddChildTail( m_oMilk );
	m_oMilk.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oMilk.SetWidth( m_oMask.GetWidth() );
	m_oMilk.SetHeight( 0.0f );
	pMilk->GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );
	pMilk->GetMaterial()->SetName( "GlassMilk" );

	m_oGlint.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Glass_Glint" ) );
	m_oGlass.AddChildTail( m_oGlint );
	m_oGlint.Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 004562d0
void AMilkFillHudElement::Destroy()
{
	m_oBackPlate.RemoveSelf();
	m_oMask.RemoveSelf();
	m_oMilk.RemoveSelf();
	m_oGlint.RemoveSelf();
	m_oGlass.RemoveSelf();
	RemoveSelf();
}

// $Barnyard: FUNCTION 00456340
void AMilkFillHudElement::SetFill( TFLOAT a_fValue, TFLOAT a_fMax )
{
	m_oMilk.SetHeight( m_oBackPlate.GetHeight() * ( a_fValue / a_fMax ) * 0.9f );
}
