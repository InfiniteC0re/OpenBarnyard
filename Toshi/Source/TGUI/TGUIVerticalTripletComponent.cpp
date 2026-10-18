#include "ToshiPCH.h"
#include "TGUIVerticalTripletComponent.h"
#include "TGUIManager.h"
#include "TGUIDisplayContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUIVerticalTripletComponent );

TGUIComponentInfo TGUIVerticalTripletComponent::ms_component_info;

// $NicktoonsPS2: FUNCTION 00381710
// $JPOG: FUNCTION TGuiInterface.dll+0002aab0
void TGUIVerticalTripletComponent::InitialiseClass()
{
	ms_component_info.AddElementName( TPString8( "top" ) );
	ms_component_info.AddElementName( TPString8( "middle" ) );
	ms_component_info.AddElementName( TPString8( "bottom" ) );
	ms_component_info.SetComponentName( TPString8( "TGUIVerticalTripletComponent" ) );
	ms_component_info.SetComponentLabel( TPString8( "3x1 Vertical Stretch" ) );

	TGUIManager::AddComponentClass( &ms_component_info );
}

// $NicktoonsPS2: FUNCTION 003818e8
// $JPOG: FUNCTION TGuiInterface.dll+0002afd0
void TGUIVerticalTripletComponent::DeinitialiseClass()
{
	ms_component_info.Clear();
}

// $NicktoonsPS2: FUNCTION 00381578
// $JPOG: FUNCTION TGuiInterface.dll+0002af80
TGUIVerticalTripletComponent::TGUIVerticalTripletComponent()
{
}

// $NicktoonsPS2: FUNCTION 003815e8
// $JPOG: FUNCTION TGuiInterface.dll+0002ac90
TGUIVerticalTripletComponent::TGUIVerticalTripletComponent( const TGUIVerticalTripletComponent& a_rcOther )
    : TGUIComponent( a_rcOther )
{
	CopyElements( m_aElements, a_rcOther.m_aElements, Element_NUMOF );
}

// $NicktoonsPS2: FUNCTION 00381678
// $JPOG: FUNCTION TGuiInterface.dll+0002af40
TGUIVerticalTripletComponent::~TGUIVerticalTripletComponent()
{
}

// $NicktoonsPS2: FUNCTION 00381908
// $JPOG: FUNCTION TGuiInterface.dll+0002af10
TGUIVerticalTripletComponent* TGUIVerticalTripletComponent::Clone() const
{
	return new TGUIVerticalTripletComponent( *this );
}

// $NicktoonsPS2: FUNCTION 00381568
// $JPOG: FUNCTION TGuiInterface.dll+0002afc0
const TGUIComponentInfo* TGUIVerticalTripletComponent::GetComponentInfo() const
{
	return &ms_component_info;
}

// $NicktoonsPS2: FUNCTION 00381f50
// $JPOG: FUNCTION TGuiInterface.dll+0002b030
TGUIElement* TGUIVerticalTripletComponent::GetElements()
{
	return m_aElements;
}

// $NicktoonsPS2: FUNCTION 00381da8
// $JPOG: FUNCTION TGuiInterface.dll+0002adf0
void TGUIVerticalTripletComponent::CalculateDimensions( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rTopHeight, TINT& a_rMiddleHeight, TINT& a_rBottomHeight ) const
{
	TINT aWidths[ Element_NUMOF ];
	TINT aHeights[ Element_NUMOF ];

	for ( TINT i = 0; i < Element_NUMOF; i++ )
		m_aElements[ i ].GetPreferredSize( a_pParameters, aWidths[ i ], aHeights[ i ] );

	a_rWidth        = TMath::Max( aWidths[ Element_Top ], TMath::Max( aWidths[ Element_Middle ], aWidths[ Element_Bottom ] ) );
	a_rTopHeight    = aHeights[ Element_Top ];
	a_rMiddleHeight = aHeights[ Element_Middle ];
	a_rBottomHeight = aHeights[ Element_Bottom ];
}

// $NicktoonsPS2: FUNCTION 00381940
// $JPOG: FUNCTION TGuiInterface.dll+0002ac30
TBOOL TGUIVerticalTripletComponent::GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const
{
	TINT iWidth        = 0;
	TINT iTopHeight    = 0;
	TINT iMiddleHeight = 0;
	TINT iBottomHeight = 0;
	CalculateDimensions( a_pParameters, iWidth, iTopHeight, iMiddleHeight, iBottomHeight );

	a_rWidth  = iWidth + m_iMarginLeft + m_iMarginRight;
	a_rHeight = iTopHeight + iMiddleHeight + iBottomHeight + m_iMarginTop + m_iMarginBottom;

	return TTRUE;
}

// NOTE: with a function, the top and bottom elements are moved away by the size the function adds
// $NicktoonsPS2: FUNCTION 003819e8
// $JPOG: FUNCTION TGuiInterface.dll+0002acf0
void TGUIVerticalTripletComponent::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const
{
	TINT iWidth        = 0;
	TINT iTopHeight    = 0;
	TINT iMiddleHeight = 0;
	TINT iBottomHeight = 0;
	CalculateDimensions( a_pParameters, iWidth, iTopHeight, iMiddleHeight, iBottomHeight );

	// The top and bottom elements shrink proportionally if they don't fit
	TINT iAvailable = a_rcRect.m_iHeight - m_iMarginTop - m_iMarginBottom;

	if ( iAvailable < iTopHeight + iBottomHeight )
	{
		TINT iSides   = iTopHeight + iBottomHeight;
		iTopHeight    = TINT( TFLOAT( iTopHeight ) * ( TFLOAT( iAvailable ) / TFLOAT( iSides ) ) );
		iBottomHeight = iAvailable - iTopHeight;
	}

	iMiddleHeight = a_rcRect.m_iHeight - iTopHeight - iBottomHeight - m_iMarginTop - m_iMarginBottom;
	iWidth        = a_rcRect.m_iWidth - m_iMarginLeft - m_iMarginRight;

	TINT iX       = a_rcRect.m_iX + m_iMarginLeft;
	TINT iTopY    = a_rcRect.m_iY + m_iMarginTop;
	TINT iMiddleY = iTopY + iTopHeight;
	TINT iBottomY = a_rcRect.m_iY + a_rcRect.m_iHeight - iBottomHeight - m_iMarginBottom;

	if ( !m_apFunctions[ Function_Alpha ] && !m_apFunctions[ Function_Size ] )
	{
		if ( iTopHeight > 0 )
		{
			TGUIRectangle oRect( iX, iTopY, iWidth, iTopHeight );
			m_aElements[ Element_Top ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		if ( iMiddleHeight > 0 )
		{
			TGUIRectangle oRect( iX, iMiddleY, iWidth, iMiddleHeight );
			m_aElements[ Element_Middle ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		if ( iBottomHeight > 0 )
		{
			TGUIRectangle oRect( iX, iBottomY, iWidth, iBottomHeight );
			m_aElements[ Element_Bottom ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		return;
	}

	TGUIRectangle oOffset( 0, 0, 0, 0 );
	TGUIColour    oColour;

	if ( m_apFunctions[ Function_Alpha ] )
		oColour = TGUIColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( ComputeAlphaFunction( a_pDisplayContext->GetCurrentTime() ) ) );

	if ( m_apFunctions[ Function_Size ] )
		ComputeSizeFunction( a_pDisplayContext->GetCurrentTime(), oOffset );

	iBottomY += oOffset.m_iWidth;
	iTopY -= oOffset.m_iWidth;

	if ( iTopHeight > 0 )
	{
		TGUIRectangle oRect( iX, iTopY, iWidth, iTopHeight );
		m_aElements[ Element_Top ].Paint( a_pDisplayContext, a_pParameters, &oRect, &oOffset, oColour );
	}

	if ( iMiddleHeight > 0 )
	{
		TGUIRectangle oRect( iX, iMiddleY, iWidth, iMiddleHeight );
		m_aElements[ Element_Middle ].Paint( a_pDisplayContext, a_pParameters, &oRect, &oOffset, oColour );
	}

	if ( iBottomHeight > 0 )
	{
		TGUIRectangle oRect( iX, iBottomY, iWidth, iBottomHeight );
		m_aElements[ Element_Bottom ].Paint( a_pDisplayContext, a_pParameters, &oRect, &oOffset, oColour );
	}
}

TOSHI_NAMESPACE_END
