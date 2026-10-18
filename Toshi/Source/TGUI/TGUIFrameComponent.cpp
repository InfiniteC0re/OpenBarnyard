#include "ToshiPCH.h"
#include "TGUIFrameComponent.h"
#include "TGUIManager.h"
#include "TGUIDisplayContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUIFrameComponent );

TGUIComponentInfo TGUIFrameComponent::ms_component_info;

// $NicktoonsPS2: FUNCTION 0037d2f8
// $JPOG: FUNCTION TGuiInterface.dll+00028740
void TGUIFrameComponent::InitialiseClass()
{
	ms_component_info.AddElementName( TPString8( "topleft" ) );
	ms_component_info.AddElementName( TPString8( "topright" ) );
	ms_component_info.AddElementName( TPString8( "bottomleft" ) );
	ms_component_info.AddElementName( TPString8( "bottomright" ) );
	ms_component_info.AddElementName( TPString8( "top" ) );
	ms_component_info.AddElementName( TPString8( "bottom" ) );
	ms_component_info.AddElementName( TPString8( "left" ) );
	ms_component_info.AddElementName( TPString8( "right" ) );
	ms_component_info.AddElementName( TPString8( "middle" ) );
	ms_component_info.SetComponentName( TPString8( "TGUIFrameComponent" ) );
	ms_component_info.SetComponentLabel( TPString8( "3x3 Frame" ) );

	TGUIManager::AddComponentClass( &ms_component_info );
}

// $NicktoonsPS2: FUNCTION 0037d6c8
// $JPOG: FUNCTION TGuiInterface.dll+000294d0
void TGUIFrameComponent::DeinitialiseClass()
{
	ms_component_info.Clear();
}

// $NicktoonsPS2: FUNCTION 0037d160
// $JPOG: FUNCTION TGuiInterface.dll+00029480
TGUIFrameComponent::TGUIFrameComponent()
{
}

// $NicktoonsPS2: FUNCTION 0037d1d0
// $JPOG: FUNCTION TGuiInterface.dll+00028af0
TGUIFrameComponent::TGUIFrameComponent( const TGUIFrameComponent& a_rcOther )
    : TGUIComponent( a_rcOther )
{
	CopyElements( m_aElements, a_rcOther.m_aElements, Element_NUMOF );
}

// $NicktoonsPS2: FUNCTION 0037d260
// $JPOG: FUNCTION TGuiInterface.dll+00029440
TGUIFrameComponent::~TGUIFrameComponent()
{
}

// $NicktoonsPS2: FUNCTION 0037d6e8
// $JPOG: FUNCTION TGuiInterface.dll+00029410
TGUIFrameComponent* TGUIFrameComponent::Clone() const
{
	return new TGUIFrameComponent( *this );
}

// $NicktoonsPS2: FUNCTION 0037d150
// $JPOG: FUNCTION TGuiInterface.dll+000294c0
const TGUIComponentInfo* TGUIFrameComponent::GetComponentInfo() const
{
	return &ms_component_info;
}

// $NicktoonsPS2: FUNCTION 0037e348
// $JPOG: FUNCTION TGuiInterface.dll+00029540
TGUIElement* TGUIFrameComponent::GetElements()
{
	return m_aElements;
}

// $NicktoonsPS2: FUNCTION 0037e350
// $JPOG: FUNCTION TGuiInterface.dll+00029530
void TGUIFrameComponent::DestroyCache( void* a_pData ) const
{
	delete TSTATICCAST( Cache, a_pData );
}

// NOTE: the dimensions are always calculated, even if they're replaced by the optimised size
// $NicktoonsPS2: FUNCTION 0037d720
// $JPOG: FUNCTION TGuiInterface.dll+00028a80
TBOOL TGUIFrameComponent::GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const
{
	Cache oCache;
	CalculateDimensions( a_pParameters, &oCache );

	if ( m_bOptimiseSize )
	{
		CalculateOptimisedSize( a_pParameters, a_rWidth, a_rHeight );
		a_rWidth += m_iMarginLeft + m_iMarginRight;
		a_rHeight += m_iMarginTop + m_iMarginBottom;
	}
	else
	{
		a_rWidth  = oCache.uiLeftWidth + oCache.uiRightWidth + m_iMarginLeft + m_iMarginRight;
		a_rHeight = oCache.uiTopHeight + oCache.uiBottomHeight + m_iMarginTop + m_iMarginBottom;
	}

	return TTRUE;
}

// NOTE: the middle height is calculated from the widths of the left, right and middle elements, it's replaced by
// the height of the rectangle when the frame is painted
// $NicktoonsPS2: FUNCTION 0037dff8
// $JPOG: FUNCTION TGuiInterface.dll+00029240
void TGUIFrameComponent::CalculateDimensions( PPropertyValue* a_pParameters, Cache* a_pCache ) const
{
	TINT aWidths[ Element_NUMOF ];
	TINT aHeights[ Element_NUMOF ];

	for ( TINT i = 0; i < Element_NUMOF; i++ )
		m_aElements[ i ].GetPreferredSize( a_pParameters, aWidths[ i ], aHeights[ i ] );

	a_pCache->uiTopHeight    = TUINT16( TMath::Max( aHeights[ Element_TopLeft ], TMath::Max( aHeights[ Element_TopRight ], aHeights[ Element_Top ] ) ) );
	a_pCache->uiBottomHeight = TUINT16( TMath::Max( aHeights[ Element_BottomLeft ], TMath::Max( aHeights[ Element_BottomRight ], aHeights[ Element_Bottom ] ) ) );
	a_pCache->uiLeftWidth    = TUINT16( TMath::Max( aWidths[ Element_TopLeft ], TMath::Max( aWidths[ Element_BottomLeft ], aWidths[ Element_Left ] ) ) );
	a_pCache->uiRightWidth   = TUINT16( TMath::Max( aWidths[ Element_TopRight ], TMath::Max( aWidths[ Element_BottomRight ], aWidths[ Element_Right ] ) ) );

	TINT iMiddleWidth        = TMath::Max( aWidths[ Element_Top ], TMath::Max( aWidths[ Element_Bottom ], aWidths[ Element_Middle ] ) ) - m_iMarginLeft - m_iMarginRight;
	a_pCache->uiMiddleWidth  = TUINT16( TMath::Max( iMiddleWidth, 0 ) );
	TINT iMiddleHeight       = TMath::Max( aWidths[ Element_Left ], TMath::Max( aWidths[ Element_Right ], aWidths[ Element_Middle ] ) ) - m_iMarginTop - m_iMarginBottom;
	a_pCache->uiMiddleHeight = TUINT16( TMath::Max( iMiddleHeight, 0 ) );
}

// a_rWidth and a_rHeight are the size to fit, they get the corners and as many top/left edges as needed to cover it
// $NicktoonsPS2: FUNCTION 0037de88
void TGUIFrameComponent::CalculateOptimisedSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight ) const
{
	TINT aWidths[ Element_NUMOF ];
	TINT aHeights[ Element_NUMOF ];

	for ( TINT i = 0; i < Element_NUMOF; i++ )
		m_aElements[ i ].GetPreferredSize( a_pParameters, aWidths[ i ], aHeights[ i ] );

	TINT iNumColumns = 0;

	if ( aWidths[ Element_Top ] > 0 )
	{
		TINT iSpace = TMath::Max( a_rWidth - aWidths[ Element_TopLeft ] - aWidths[ Element_TopRight ], 0 );
		iNumColumns = TINT( TFLOAT( iSpace ) / TFLOAT( aWidths[ Element_Top ] ) ) + 1;
	}

	TINT iNumRows = 0;

	if ( aHeights[ Element_Left ] > 0 )
	{
		TINT iSpace = TMath::Max( a_rHeight - aHeights[ Element_TopLeft ] - aHeights[ Element_BottomLeft ], 0 );
		iNumRows    = TINT( TFLOAT( iSpace ) / TFLOAT( aHeights[ Element_Left ] ) ) + 1;
	}

	a_rWidth  = aWidths[ Element_TopLeft ] + aWidths[ Element_TopRight ] + iNumColumns * aWidths[ Element_Top ];
	a_rHeight = aHeights[ Element_TopLeft ] + aHeights[ Element_BottomLeft ] + iNumRows * aHeights[ Element_Left ];
}

// $NicktoonsPS2: FUNCTION 0037d800
// $JPOG: FUNCTION TGuiInterface.dll+00028b50
void TGUIFrameComponent::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const
{
	Cache* pCache = TSTATICCAST( Cache, a_pCache->GetData( this ) );

	if ( !pCache )
	{
		pCache = new Cache;
		CalculateDimensions( a_pParameters, pCache );
		a_pCache->SetData( this, pCache );

		// The middle fills the rest of the rectangle
		pCache->uiMiddleWidth  = TUINT16( TMath::Max( a_rcRect.m_iWidth - pCache->uiLeftWidth - pCache->uiRightWidth - m_iMarginLeft - m_iMarginRight, 0 ) );
		pCache->uiMiddleHeight = TUINT16( TMath::Max( a_rcRect.m_iHeight - pCache->uiTopHeight - pCache->uiBottomHeight - m_iMarginTop - m_iMarginBottom, 0 ) );
	}

	TINT iLeftX   = a_rcRect.m_iX + m_iMarginLeft;
	TINT iMiddleX = iLeftX + pCache->uiLeftWidth;
	TINT iRightX  = a_rcRect.m_iX + a_rcRect.m_iWidth - pCache->uiRightWidth - m_iMarginRight;
	TINT iTopY    = a_rcRect.m_iY + m_iMarginTop;
	TINT iMiddleY = iTopY + pCache->uiTopHeight;
	TINT iBottomY = a_rcRect.m_iY + a_rcRect.m_iHeight - pCache->uiBottomHeight - m_iMarginBottom;

	// The elements in the order of the original (columns of the rows)
	const TGUIRectangle aRects[ Element_NUMOF ] = {
		TGUIRectangle( iLeftX, iTopY, pCache->uiLeftWidth, pCache->uiTopHeight ),           // Element_TopLeft
		TGUIRectangle( iRightX, iTopY, pCache->uiRightWidth, pCache->uiTopHeight ),         // Element_TopRight
		TGUIRectangle( iLeftX, iBottomY, pCache->uiLeftWidth, pCache->uiBottomHeight ),     // Element_BottomLeft
		TGUIRectangle( iRightX, iBottomY, pCache->uiRightWidth, pCache->uiBottomHeight ),   // Element_BottomRight
		TGUIRectangle( iMiddleX, iTopY, pCache->uiMiddleWidth, pCache->uiTopHeight ),       // Element_Top
		TGUIRectangle( iMiddleX, iBottomY, pCache->uiMiddleWidth, pCache->uiBottomHeight ), // Element_Bottom
		TGUIRectangle( iLeftX, iMiddleY, pCache->uiLeftWidth, pCache->uiMiddleHeight ),     // Element_Left
		TGUIRectangle( iRightX, iMiddleY, pCache->uiRightWidth, pCache->uiMiddleHeight ),   // Element_Right
		TGUIRectangle( iMiddleX, iMiddleY, pCache->uiMiddleWidth, pCache->uiMiddleHeight )  // Element_Middle
	};

	static constexpr Element s_aPaintOrder[ Element_NUMOF ] = {
		Element_TopLeft, Element_Top, Element_TopRight,
		Element_Left, Element_Middle, Element_Right,
		Element_BottomLeft, Element_Bottom, Element_BottomRight
	};

	if ( !m_apFunctions[ Function_Alpha ] && !m_apFunctions[ Function_Size ] )
	{
		for ( TINT i = 0; i < Element_NUMOF; i++ )
			m_aElements[ s_aPaintOrder[ i ] ].Paint( a_pDisplayContext, a_pParameters, &aRects[ s_aPaintOrder[ i ] ] );

		return;
	}

	TGUIRectangle oOffset( 0, 0, 0, 0 );
	TGUIColour    oColour;

	if ( m_apFunctions[ Function_Alpha ] )
		oColour = TGUIColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( ComputeAlphaFunction( a_pDisplayContext->GetCurrentTime() ) ) );

	if ( m_apFunctions[ Function_Size ] )
		ComputeSizeFunction( a_pDisplayContext->GetCurrentTime(), oOffset );

	for ( TINT i = 0; i < Element_NUMOF; i++ )
		m_aElements[ s_aPaintOrder[ i ] ].Paint( a_pDisplayContext, a_pParameters, &aRects[ s_aPaintOrder[ i ] ], &oOffset, oColour );
}

TOSHI_NAMESPACE_END
