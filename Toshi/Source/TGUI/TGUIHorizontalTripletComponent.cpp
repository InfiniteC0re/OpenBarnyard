#include "ToshiPCH.h"
#include "TGUIHorizontalTripletComponent.h"
#include "TGUIManager.h"
#include "TGUIDisplayContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUIHorizontalTripletComponent );

TGUIComponentInfo TGUIHorizontalTripletComponent::ms_component_info;

// $NicktoonsPS2: FUNCTION 0037e5b0
// $JPOG: FUNCTION TGuiInterface.dll+000295d0
void TGUIHorizontalTripletComponent::InitialiseClass()
{
	ms_component_info.AddElementName( TPString8( "left" ) );
	ms_component_info.AddElementName( TPString8( "middle" ) );
	ms_component_info.AddElementName( TPString8( "right" ) );
	ms_component_info.SetComponentName( TPString8( "TGUIHorizontalTripletComponent" ) );
	ms_component_info.SetComponentLabel( TPString8( "1x3 Horizontal Stretch" ) );

	TGUIManager::AddComponentClass( &ms_component_info );
}

// $NicktoonsPS2: FUNCTION 0037e788
// $JPOG: FUNCTION TGuiInterface.dll+00029e90
void TGUIHorizontalTripletComponent::DeinitialiseClass()
{
	ms_component_info.Clear();
}

// $NicktoonsPS2: FUNCTION 0037e418
// $JPOG: FUNCTION TGuiInterface.dll+00029e40
TGUIHorizontalTripletComponent::TGUIHorizontalTripletComponent()
{
}

// $NicktoonsPS2: FUNCTION 0037e488
// $JPOG: FUNCTION TGuiInterface.dll+000297d0
TGUIHorizontalTripletComponent::TGUIHorizontalTripletComponent( const TGUIHorizontalTripletComponent& a_rcOther )
    : TGUIComponent( a_rcOther )
{
	CopyElements( m_aElements, a_rcOther.m_aElements, Element_NUMOF );
}

// $NicktoonsPS2: FUNCTION 0037e518
// $JPOG: FUNCTION TGuiInterface.dll+00029e00
TGUIHorizontalTripletComponent::~TGUIHorizontalTripletComponent()
{
}

// $NicktoonsPS2: FUNCTION 0037e7a8
// $JPOG: FUNCTION TGuiInterface.dll+00029dd0
TGUIHorizontalTripletComponent* TGUIHorizontalTripletComponent::Clone() const
{
	return new TGUIHorizontalTripletComponent( *this );
}

// $NicktoonsPS2: FUNCTION 0037e408
// $JPOG: FUNCTION TGuiInterface.dll+00029e80
const TGUIComponentInfo* TGUIHorizontalTripletComponent::GetComponentInfo() const
{
	return &ms_component_info;
}

// $NicktoonsPS2: FUNCTION 0037ede8
// $JPOG: FUNCTION TGuiInterface.dll+00029ef0
TGUIElement* TGUIHorizontalTripletComponent::GetElements()
{
	return m_aElements;
}

// $NicktoonsPS2: FUNCTION 0037ec38
// $JPOG: FUNCTION TGuiInterface.dll+00029cb0
void TGUIHorizontalTripletComponent::CalculateDimensions( PPropertyValue* a_pParameters, TINT& a_rHeight, TINT& a_rLeftWidth, TINT& a_rMiddleWidth, TINT& a_rRightWidth ) const
{
	TINT aWidths[ Element_NUMOF ];
	TINT aHeights[ Element_NUMOF ];

	for ( TINT i = 0; i < Element_NUMOF; i++ )
		m_aElements[ i ].GetPreferredSize( a_pParameters, aWidths[ i ], aHeights[ i ] );

	a_rHeight      = TMath::Max( aHeights[ Element_Left ], TMath::Max( aHeights[ Element_Middle ], aHeights[ Element_Right ] ) );
	a_rLeftWidth   = aWidths[ Element_Left ];
	a_rMiddleWidth = aWidths[ Element_Middle ];
	a_rRightWidth  = aWidths[ Element_Right ];
}

// $NicktoonsPS2: FUNCTION 0037e7e0
// $JPOG: FUNCTION TGuiInterface.dll+00029750
TBOOL TGUIHorizontalTripletComponent::GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const
{
	TINT iHeight      = 0;
	TINT iLeftWidth   = 0;
	TINT iMiddleWidth = 0;
	TINT iRightWidth  = 0;
	CalculateDimensions( a_pParameters, iHeight, iLeftWidth, iMiddleWidth, iRightWidth );

	a_rWidth  = iLeftWidth + iMiddleWidth + iRightWidth + m_iMarginLeft + m_iMarginRight;
	a_rHeight = iHeight + m_iMarginTop + m_iMarginBottom;

	return TTRUE;
}

// NOTE: with a function, the left and right elements are moved away by the size the function adds and they're
// painted even if their width is 0
// $NicktoonsPS2: FUNCTION 0037e880
// $JPOG: FUNCTION TGuiInterface.dll+00029830
void TGUIHorizontalTripletComponent::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const
{
	TINT iHeight      = 0;
	TINT iLeftWidth   = 0;
	TINT iMiddleWidth = 0;
	TINT iRightWidth  = 0;
	CalculateDimensions( a_pParameters, iHeight, iLeftWidth, iMiddleWidth, iRightWidth );

	// The left and right elements shrink proportionally if they don't fit
	TINT iAvailable = a_rcRect.m_iWidth - m_iMarginLeft - m_iMarginRight;

	if ( iAvailable < iLeftWidth + iRightWidth )
	{
		TINT iSides = iLeftWidth + iRightWidth;
		iLeftWidth  = TINT( TFLOAT( iLeftWidth ) * ( TFLOAT( iAvailable ) / TFLOAT( iSides ) ) );
		iRightWidth = iAvailable - iLeftWidth;
	}

	iMiddleWidth = a_rcRect.m_iWidth - iLeftWidth - iRightWidth - m_iMarginLeft - m_iMarginRight;
	iHeight      = a_rcRect.m_iHeight - m_iMarginTop - m_iMarginBottom;

	TINT iLeftX   = a_rcRect.m_iX + m_iMarginLeft;
	TINT iY       = a_rcRect.m_iY + m_iMarginTop;
	TINT iMiddleX = iLeftX + iLeftWidth;
	TINT iRightX  = a_rcRect.m_iX + a_rcRect.m_iWidth - iRightWidth - m_iMarginRight;

	if ( !m_apFunctions[ Function_Alpha ] && !m_apFunctions[ Function_Size ] )
	{
		if ( iLeftWidth > 0 )
		{
			TGUIRectangle oRect( iLeftX, iY, iLeftWidth, iHeight );
			m_aElements[ Element_Left ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		if ( iMiddleWidth > 0 )
		{
			TGUIRectangle oRect( iMiddleX, iY, iMiddleWidth, iHeight );
			m_aElements[ Element_Middle ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		if ( iRightWidth > 0 )
		{
			TGUIRectangle oRect( iRightX, iY, iRightWidth, iHeight );
			m_aElements[ Element_Right ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		return;
	}

	TGUIRectangle oOffset( 0, 0, 0, 0 );
	TGUIColour    oColour;

	if ( m_apFunctions[ Function_Alpha ] )
		oColour = TGUIColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( ComputeAlphaFunction( a_pDisplayContext->GetCurrentTime() ) ) );

	if ( m_apFunctions[ Function_Size ] )
		ComputeSizeFunction( a_pDisplayContext->GetCurrentTime(), oOffset );

	iRightX += oOffset.m_iWidth;
	iLeftX -= oOffset.m_iWidth;

	if ( iLeftWidth >= 0 )
	{
		TGUIRectangle oRect( iLeftX, iY, iLeftWidth, iHeight );
		m_aElements[ Element_Left ].Paint( a_pDisplayContext, a_pParameters, &oRect, &oOffset, oColour );
	}

	if ( iMiddleWidth >= 0 )
	{
		TGUIRectangle oRect( iMiddleX, iY, iMiddleWidth, iHeight );
		m_aElements[ Element_Middle ].Paint( a_pDisplayContext, a_pParameters, &oRect, &oOffset, oColour );
	}

	if ( iRightWidth >= 0 )
	{
		TGUIRectangle oRect( iRightX, iY, iRightWidth, iHeight );
		m_aElements[ Element_Right ].Paint( a_pDisplayContext, a_pParameters, &oRect, &oOffset, oColour );
	}
}

TOSHI_NAMESPACE_END
