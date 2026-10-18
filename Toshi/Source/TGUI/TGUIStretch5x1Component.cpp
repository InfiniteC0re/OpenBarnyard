#include "ToshiPCH.h"
#include "TGUIStretch5x1Component.h"
#include "TGUIManager.h"
#include "TGUIDisplayContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUIStretch5x1Component );

TGUIComponentInfo TGUIStretch5x1Component::ms_component_info;

// $NicktoonsPS2: FUNCTION 00380340
void TGUIStretch5x1Component::InitialiseClass()
{
	ms_component_info.AddElementName( TPString8( "top" ) );
	ms_component_info.AddElementName( TPString8( "upper" ) );
	ms_component_info.AddElementName( TPString8( "middle" ) );
	ms_component_info.AddElementName( TPString8( "lower" ) );
	ms_component_info.AddElementName( TPString8( "bottom" ) );
	ms_component_info.SetComponentName( TPString8( "TGUIStretch5x1Component" ) );
	ms_component_info.SetComponentLabel( TPString8( "5x1 Vertical Stretch" ) );

	TGUIManager::AddComponentClass( &ms_component_info );
}

// $NicktoonsPS2: FUNCTION 003805c0
void TGUIStretch5x1Component::DeinitialiseClass()
{
	ms_component_info.Clear();
}

// $NicktoonsPS2: FUNCTION 003801a8
TGUIStretch5x1Component::TGUIStretch5x1Component()
{
}

// $NicktoonsPS2: FUNCTION 00380218
TGUIStretch5x1Component::TGUIStretch5x1Component( const TGUIStretch5x1Component& a_rcOther )
    : TGUIComponent( a_rcOther )
{
	CopyElements( m_aElements, a_rcOther.m_aElements, Element_NUMOF );
}

// $NicktoonsPS2: FUNCTION 003802a8
TGUIStretch5x1Component::~TGUIStretch5x1Component()
{
}

// $NicktoonsPS2: FUNCTION 003805e0
TGUIStretch5x1Component* TGUIStretch5x1Component::Clone() const
{
	return new TGUIStretch5x1Component( *this );
}

// $NicktoonsPS2: FUNCTION 00380198
const TGUIComponentInfo* TGUIStretch5x1Component::GetComponentInfo() const
{
	return &ms_component_info;
}

// $NicktoonsPS2: FUNCTION 00380cc0
TGUIElement* TGUIStretch5x1Component::GetElements()
{
	return m_aElements;
}

// NOTE: the "upper" and "lower" elements have no preferred size
// $NicktoonsPS2: FUNCTION 00380b18
void TGUIStretch5x1Component::CalculateDimensions( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rTopHeight, TINT& a_rMiddleHeight, TINT& a_rBottomHeight ) const
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

// $NicktoonsPS2: FUNCTION 00380618
TBOOL TGUIStretch5x1Component::GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const
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

// The middle element keeps its height in the centre, "upper" and "lower" fill the space around it
// NOTE: with a function, the top and bottom elements are moved away by the size the function adds and the
// "upper" and "lower" elements aren't painted
// $NicktoonsPS2: FUNCTION 003806c0
void TGUIStretch5x1Component::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const
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

	TINT iSpace       = a_rcRect.m_iHeight - iTopHeight - iMiddleHeight - iBottomHeight - m_iMarginTop - m_iMarginBottom;
	TINT iUpperHeight = iSpace / 2;
	TINT iLowerHeight = iSpace - iUpperHeight;

	TINT iTopY    = a_rcRect.m_iY + m_iMarginTop;
	TINT iUpperY  = iTopY + iTopHeight;
	TINT iMiddleY = iUpperY + iUpperHeight;
	TINT iLowerY  = iMiddleY + iMiddleHeight;
	TINT iBottomY = a_rcRect.m_iY + a_rcRect.m_iHeight - iBottomHeight - m_iMarginBottom;
	TINT iX       = a_rcRect.m_iX + m_iMarginLeft;
	iWidth        = a_rcRect.m_iWidth - m_iMarginLeft - m_iMarginRight;

	if ( !m_apFunctions[ Function_Alpha ] && !m_apFunctions[ Function_Size ] )
	{
		if ( iTopHeight > 0 )
		{
			TGUIRectangle oRect( iX, iTopY, iWidth, iTopHeight );
			m_aElements[ Element_Top ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		if ( iUpperHeight > 0 )
		{
			TGUIRectangle oRect( iX, iUpperY, iWidth, iUpperHeight );
			m_aElements[ Element_Upper ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		if ( iMiddleHeight > 0 )
		{
			TGUIRectangle oRect( iX, iMiddleY, iWidth, iMiddleHeight );
			m_aElements[ Element_Middle ].Paint( a_pDisplayContext, a_pParameters, &oRect );
		}

		if ( iLowerHeight > 0 )
		{
			TGUIRectangle oRect( iX, iLowerY, iWidth, iLowerHeight );
			m_aElements[ Element_Lower ].Paint( a_pDisplayContext, a_pParameters, &oRect );
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

	iTopY -= oOffset.m_iWidth;
	iBottomY += oOffset.m_iWidth;

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
