#include "pch.h"
#include "AHoneyCountHudElement.h"
#include "GUI/AGUI2.h"

#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00453d10, CreateObject 004540f0, CreateObjectInPlace 00454150 (the constructor is inlined in both)
TDEFINE_CLASS( AHoneyCountHudElement );

// $Barnyard: FUNCTION 00453d20
AHoneyCountHudElement::AHoneyCountHudElement()
    : AHudElement( MASK_HONEY_COUNT )
    , m_iUnk78( 0 )
{
}

// $Barnyard: FUNCTION 00453df0
// $Barnyard: FUNCTION 00454190
AHoneyCountHudElement::~AHoneyCountHudElement()
{
}

// $Barnyard: FUNCTION 004541b0
TBOOL AHoneyCountHudElement::OnAttach()
{
	m_oJar.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Jar_Bkg" ) );
	m_oJar.GetMaterial()->SetName( "Jar_Bkg" );

	T2GUIElement::SetAttachment( T2GUIATTACHMENT( 2 ), T2GUIATTACHMENT( 2 ) );
	T2GUIElement::SetDimensions( m_oJar.GetWidth(), m_oJar.GetHeight() );
	m_oJarRoot.SetDimensions( m_oJar.GetWidth(), m_oJar.GetHeight() );

	m_oShadow.T2GUIElement::SetDimensions( m_oJar.GetWidth(), m_oJar.GetHeight() );
	m_oShadow.Hide();
	m_oShadow.SetColour( 0 );

	m_oJarRoot.AddChildTail( m_oJar );
	m_oJarRoot.AddChildTail( m_oShadow );
	m_oJar.Hide();

	m_oJarMask.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Jar_Mask" ) );
	m_oJarMask.SetDimensions( m_oJar.GetWidth(), m_oJar.GetHeight() );
	T2GUIMaterial* pMaskMaterial = m_oJarMask.GetMaterial();
	pMaskMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_MASK );
	pMaskMaterial->SetName( "Jar_Mask" );
	m_oJarRoot.AddChildTail( m_oJarMask );
	m_oJarMask.Hide();

	m_oHoney.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Honey_Rectangle" ) );
	m_oJarMask.AddChildTail( m_oHoney );
	m_oHoney.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	m_oHoney.SetWidth( m_oJarMask.GetWidth() * 0.95f );
	m_oHoney.SetHeight( 0.0f );
	T2GUIMaterial* pHoneyMaterial = m_oHoney.GetMaterial();
	pHoneyMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );
	pHoneyMaterial->SetName( "Honey_Rectangle" );

	m_oGlint.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Jar_Glint" ) );
	m_oGlint.GetMaterial()->SetName( "Jar_Glint" );
	m_oJarRoot.AddChildTail( m_oGlint );
	m_oGlint.Hide();

	T2GUIElement::AddChildTail( m_oJarRoot );
	AGUI2::GetRootElement()->AddChildTail( *TSTATICCAST( T2GUIElement, this ) );
	T2GUIElement::Hide();

	m_oRoyalJelly.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Royal_Jelly" ) );
	m_oRoyalJelly.SetAttachment( T2GUIATTACHMENT( 3 ), T2GUIATTACHMENT( 5 ) );
	m_oJarRoot.AddChildTail( m_oRoyalJelly );
	m_oRoyalJelly.Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 00453ec0
void AHoneyCountHudElement::OnDetach()
{
	m_oJarRoot.RemoveSelf();
	m_oJar.RemoveSelf();
	m_oGlint.RemoveSelf();
	m_oHoney.RemoveSelf();
	m_oJarMask.RemoveSelf();
	m_oShadow.RemoveSelf();
	T2GUIElement::RemoveSelf();
}

// $Barnyard: FUNCTION 00453f40
void AHoneyCountHudElement::Show()
{
	T2GUIElement::Show();
	m_bCurrentlyVisible = TTRUE;
}

// NOTE: folded with AMenuNavHudElement::Hide (00455b60) in retail
void AHoneyCountHudElement::Hide()
{
	T2GUIElement::Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00453f60
void AHoneyCountHudElement::Render()
{
	m_oShadow.PreRender();
	m_oShadow.Render();
	m_oShadow.PostRender();

	m_oJarRoot.PreRender();

	m_oJar.PreRender();
	m_oJar.Render();
	m_oJar.PostRender();

	m_oJarMask.PreRender();
	m_oJarMask.Render();
	m_oJarMask.PostRender();

	m_oGlint.PreRender();
	m_oGlint.Render();
	m_oGlint.PostRender();

	if ( m_bRoyalJelly == TTRUE )
	{
		m_oRoyalJelly.PreRender();
		m_oRoyalJelly.Render();
		m_oRoyalJelly.PostRender();
	}

	m_oJarRoot.PostRender();

	T2GUIElement::Render();
}

// $Barnyard: FUNCTION 00454030
void AHoneyCountHudElement::SetHoney( TFLOAT a_fHeight )
{
	m_oHoney.SetHeight( a_fHeight );
}

// $Barnyard: FUNCTION 00454040
void AHoneyCountHudElement::AddHoney( TFLOAT a_fHeight )
{
	TFLOAT fHeight = m_oHoney.GetHeight() + a_fHeight;
	TMath::Clip( fHeight, 0.0f, m_oJar.GetHeight() * 0.85f );
	m_oHoney.SetHeight( fHeight );
}

// $Barnyard: FUNCTION 004540a0
TBOOL AHoneyCountHudElement::IsFull()
{
	TFLOAT fHeight = m_oHoney.GetHeight();
	return m_oJar.GetHeight() * 0.85f <= fHeight;
}
