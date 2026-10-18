#include "ToshiPCH.h"
#include "TGUITextComponent.h"
#include "TGUIManager.h"
#include "TGUIDisplayContext.h"
#include "TGUIFormattedText.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUITextComponent );

TGUIComponentInfo TGUITextComponent::ms_component_info;

// $NicktoonsPS2: FUNCTION 00380f08
// $JPOG: FUNCTION TGuiInterface.dll+0002a4b0
void TGUITextComponent::InitialiseClass()
{
	ms_component_info.AddElementName( TPString8( "text" ) );
	ms_component_info.SetComponentName( TPString8( "TGUITextComponent" ) );
	ms_component_info.SetComponentLabel( TPString8( "Multiline Text" ) );

	TGUIManager::AddComponentClass( &ms_component_info );
}

// $NicktoonsPS2: FUNCTION 00381038
// $JPOG: FUNCTION TGuiInterface.dll+0002a8c0
void TGUITextComponent::DeinitialiseClass()
{
	ms_component_info.Clear();
}

// $NicktoonsPS2: FUNCTION 00380d70
// $JPOG: FUNCTION TGuiInterface.dll+0002a890
TGUITextComponent::TGUITextComponent()
{
}

// $NicktoonsPS2: FUNCTION 00380de0
// $JPOG: FUNCTION TGuiInterface.dll+0002a5f0
TGUITextComponent::TGUITextComponent( const TGUITextComponent& a_rcOther )
    : TGUIComponent( a_rcOther )
{
	CopyElements( m_aElements, a_rcOther.m_aElements, Element_NUMOF );
}

// $NicktoonsPS2: FUNCTION 00380e70
// $JPOG: FUNCTION TGuiInterface.dll+0002a870
TGUITextComponent::~TGUITextComponent()
{
}

// $NicktoonsPS2: FUNCTION 00381058
// $JPOG: FUNCTION TGuiInterface.dll+0002a840
TGUITextComponent* TGUITextComponent::Clone() const
{
	return new TGUITextComponent( *this );
}

// $NicktoonsPS2: FUNCTION 00380d60
// $JPOG: FUNCTION TGuiInterface.dll+0002a8b0
const TGUIComponentInfo* TGUITextComponent::GetComponentInfo() const
{
	return &ms_component_info;
}

// $NicktoonsPS2: FUNCTION 003814c8
// $JPOG: FUNCTION TGuiInterface.dll+0002aa20
TGUIElement* TGUITextComponent::GetElements()
{
	return m_aElements;
}

// The size of the text broken into lines that fit a_iMaxWidth, nothing if the maximum width is unknown (-1)
// $NicktoonsPS2: FUNCTION 00381090
// $JPOG: FUNCTION TGuiInterface.dll+0002a5a0
TBOOL TGUITextComponent::GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const
{
	if ( a_iMaxWidth == -1 )
	{
		a_rWidth  = 0;
		a_rHeight = 0;
		return TFALSE;
	}

	m_aElements[ Element_Text ].FormatText(
	    a_pParameters,
	    TNULL,
	    a_rWidth,
	    a_rHeight,
	    TINT16( a_iMaxWidth - ( m_iMarginLeft + m_iMarginRight ) ),
	    TINT16( a_iMaxHeight - ( m_iMarginTop + m_iMarginBottom ) )
	);

	a_rWidth += m_iMarginLeft + m_iMarginRight;
	a_rHeight += m_iMarginTop + m_iMarginBottom;

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00381180
// $JPOG: FUNCTION TGuiInterface.dll+0002a630
void TGUITextComponent::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const
{
	if ( a_rcRect.m_iWidth <= 0 || a_rcRect.m_iHeight <= 0 )
		return;

	TGUIFormattedText* pText = TSTATICCAST( TGUIFormattedText, a_pCache->GetData( this ) );

	if ( !pText )
	{
		pText = BuildCache( a_pParameters, a_rcRect );
		a_pCache->SetData( this, pText );
	}

	a_pDisplayContext->DrawFormattedText(
	    *pText,
	    a_rcRect.m_iX + m_iMarginLeft,
	    a_rcRect.m_iY + m_iMarginTop,
	    a_rcRect.m_iWidth - m_iMarginLeft - m_iMarginRight,
	    a_rcRect.m_iHeight - m_iMarginTop - m_iMarginBottom
	);
}

// $NicktoonsPS2: FUNCTION 00381270
// $JPOG: FUNCTION TGuiInterface.dll+0002a790
void TGUITextComponent::DestroyCache( void* a_pData ) const
{
	if ( a_pData )
		delete TSTATICCAST( TGUIFormattedText, a_pData );
}

// $NicktoonsPS2: FUNCTION 00381298
// $JPOG: FUNCTION TGuiInterface.dll+0002a6a0
TGUIFormattedText* TGUITextComponent::BuildCache( PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect ) const
{
	TGUIFormattedText* pText = new TGUIFormattedText();

	TINT iWidth  = a_rcRect.m_iWidth - m_iMarginLeft - m_iMarginRight;
	TINT iHeight = a_rcRect.m_iHeight - m_iMarginTop - m_iMarginBottom;
	m_aElements[ Element_Text ].FormatText( a_pParameters, pText, iWidth, iHeight, iWidth, iHeight );

	return pText;
}

TOSHI_NAMESPACE_END
