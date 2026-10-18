#include "ToshiPCH.h"
#include "TGUIIconComponent.h"
#include "TGUIManager.h"
#include "TGUIDisplayContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUIIconComponent );

TGUIComponentInfo TGUIIconComponent::ms_component_info;

// $NicktoonsPS2: FUNCTION 0037f030
// $JPOG: FUNCTION TGuiInterface.dll+00029f80
void TGUIIconComponent::InitialiseClass()
{
	ms_component_info.AddElementName( TPString8( "icon" ) );
	ms_component_info.SetComponentName( TPString8( "TGUIIconComponent" ) );
	ms_component_info.SetComponentLabel( TPString8( "1x1 Icon" ) );

	TGUIManager::AddComponentClass( &ms_component_info );
}

// $NicktoonsPS2: FUNCTION 0037f160
// $JPOG: FUNCTION TGuiInterface.dll+0002a3c0
void TGUIIconComponent::DeinitialiseClass()
{
	ms_component_info.Clear();
}

// $NicktoonsPS2: FUNCTION 0037ee98
// $JPOG: FUNCTION TGuiInterface.dll+0002a390
TGUIIconComponent::TGUIIconComponent()
{
}

// $NicktoonsPS2: FUNCTION 0037ef08
// $JPOG: FUNCTION TGuiInterface.dll+0002a0b0
TGUIIconComponent::TGUIIconComponent( const TGUIIconComponent& a_rcOther )
    : TGUIComponent( a_rcOther )
{
	CopyElements( m_aElements, a_rcOther.m_aElements, Element_NUMOF );
}

// $NicktoonsPS2: FUNCTION 0037ef98
// $JPOG: FUNCTION TGuiInterface.dll+0002a370
TGUIIconComponent::~TGUIIconComponent()
{
}

// $NicktoonsPS2: FUNCTION 0037f180
// $JPOG: FUNCTION TGuiInterface.dll+0002a340
TGUIIconComponent* TGUIIconComponent::Clone() const
{
	return new TGUIIconComponent( *this );
}

// $NicktoonsPS2: FUNCTION 0037ee88
// $JPOG: FUNCTION TGuiInterface.dll+0002a3b0
const TGUIComponentInfo* TGUIIconComponent::GetComponentInfo() const
{
	return &ms_component_info;
}

// $NicktoonsPS2: FUNCTION 0037f4b8
// $JPOG: FUNCTION TGuiInterface.dll+0002a420
TGUIElement* TGUIIconComponent::GetElements()
{
	return m_aElements;
}

// $NicktoonsPS2: FUNCTION 0037f1b8
// $JPOG: FUNCTION TGuiInterface.dll+0002a070
TBOOL TGUIIconComponent::GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const
{
	m_aElements[ Element_Icon ].GetPreferredSize( a_pParameters, a_rWidth, a_rHeight );
	a_rWidth += m_iMarginLeft + m_iMarginRight;
	a_rHeight += m_iMarginTop + m_iMarginBottom;

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0037f230
// $JPOG: FUNCTION TGuiInterface.dll+0002a0f0
void TGUIIconComponent::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const
{
	if ( a_rcRect.m_iWidth <= 0 || a_rcRect.m_iHeight <= 0 )
		return;

	TGUIRectangle oRect(
	    a_rcRect.m_iX + m_iMarginLeft,
	    a_rcRect.m_iY + m_iMarginTop,
	    a_rcRect.m_iWidth - ( m_iMarginLeft + m_iMarginRight ),
	    a_rcRect.m_iHeight - ( m_iMarginTop + m_iMarginBottom )
	);

	if ( !m_apFunctions[ Function_Alpha ] && !m_apFunctions[ Function_Size ] )
	{
		m_aElements[ Element_Icon ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		return;
	}

	TGUIRectangle oOffset( 0, 0, 0, 0 );
	TGUIColour    oColour;

	if ( m_apFunctions[ Function_Alpha ] )
		oColour = TGUIColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( ComputeAlphaFunction( a_pDisplayContext->GetCurrentTime() ) ) );

	if ( m_apFunctions[ Function_Size ] )
		ComputeSizeFunction( a_pDisplayContext->GetCurrentTime(), oOffset );

	m_aElements[ Element_Icon ].Paint( a_pDisplayContext, a_pParameters, &oRect, &oOffset, oColour );
}

TOSHI_NAMESPACE_END
