#include "pch.h"
#include "AGopherShopCategory.h"
#include "GUI/AGUI2ModelRenderer.h"
#include "Locale/ALocaleManager.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Where the items on the sides are (DAT_00773914, DAT_00773924)
static const TVector4 s_vecLeftPos( -6.0f, 0.0f, 0.0f, 1.0f );
static const TVector4 s_vecRightPos( 6.0f, 0.0f, 0.0f, 1.0f );

// The light and the ambient colour of the models (DAT_00773944, DAT_00773934)
static const TVector4 s_vecLightColour( 1.0f, 1.0f, 1.0f, 1.0f );
static const TVector4 s_vecAmbientColour( 0.2f, 0.2f, 0.2f, 0.0f );

// $Barnyard: FUNCTION 0049a240
AGopherShopCategory::AGopherShopCategory()
    : m_eScroll( SCROLL_NONE )
    , m_iCurrent( -1 )
    , m_iLeft( -1 )
    , m_iRight( -1 )
    , m_pCurrent( TNULL )
    , m_pLeft( TNULL )
    , m_pRight( TNULL )
    , m_bActive( TFALSE )
    , m_iNumItems( 0 )
    , m_iNumRemoved( 0 )
    , m_iSelected( -1 )
    , m_pModelRenderer( TNULL )
{
	// NOTE: retail doesn't clear the arrays
	TUtil::MemClear( m_apItems, sizeof( m_apItems ) );
	TUtil::MemClear( m_apRemoved, sizeof( m_apRemoved ) );
}

// $Barnyard: FUNCTION 0049a1c0
void AGopherShopCategory::SetName( TINT a_iTextId )
{
	m_iTextId = a_iTextId;
	m_wszName = ALocaleManager::Interface()->GetString( a_iTextId );
}

// $Barnyard: FUNCTION 0049a1f0
TFLOAT AGopherShopCategory::EaseOut( TFLOAT a_fTime )
{
	return 1.0f - TMath::Pow( 1.0f - a_fTime, 4.0f );
}

// $Barnyard: FUNCTION 0049a280
void AGopherShopCategory::AddItem( AGopherShopItem* a_pItem )
{
	m_apItems[ m_iNumItems++ ] = a_pItem;
	m_iSelected                = 0;
}

// $Barnyard: FUNCTION 0049a2a0
void AGopherShopCategory::Scroll( TBOOL a_bRight )
{
	if ( m_iNumItems == 1 )
		return;

	// Can't scroll past the ends when there are less than four items
	if ( a_bRight == TTRUE )
	{
		if ( ( m_iNumItems == 2 && m_iCurrent == 1 ) || ( m_iNumItems == 3 && m_iCurrent == 2 ) )
			return;
	}
	else if ( ( m_iNumItems == 2 || m_iNumItems == 3 ) && m_iCurrent == 0 )
	{
		return;
	}

	// Scrolling back while still scrolling
	TBOOL bReverse = ( m_eScroll == SCROLL_RIGHT && !a_bRight ) || ( m_eScroll == SCROLL_LEFT && a_bRight );

	if ( m_eScroll != SCROLL_NONE )
	{
		if ( m_eScroll == SCROLL_RIGHT && a_bRight )
			m_pModelRenderer->SetObjectOpacity( m_iLeft, 0.0f );
		else if ( m_eScroll == SCROLL_LEFT && !a_bRight )
			m_pModelRenderer->SetObjectOpacity( m_iRight, 0.0f );
	}

	m_eScroll = ( a_bRight != TTRUE ) ? SCROLL_LEFT : SCROLL_RIGHT;

	if ( a_bRight == TTRUE )
	{
		m_iLeft    = m_iCurrent;
		m_iCurrent = m_iRight;
		m_iRight   = ( m_iRight + 1 ) % m_iNumItems;

		if ( m_iNumItems == 2 || ( m_iNumItems == 3 && m_iCurrent == 2 ) )
			m_iRight = -1;
	}
	else
	{
		m_iRight   = m_iCurrent;
		m_iCurrent = m_iLeft;

		if ( --m_iLeft < 0 )
			m_iLeft = m_iNumItems - 1;

		if ( m_iNumItems == 2 || ( m_iNumItems == 3 && m_iCurrent == 0 ) )
			m_iLeft = -1;
	}

	m_iSelected = m_iCurrent;
	m_pCurrent  = ( m_iCurrent == -1 ) ? TNULL : m_apItems[ m_iCurrent ];
	m_pLeft     = ( m_iLeft == -1 ) ? TNULL : m_apItems[ m_iLeft ];
	m_pRight    = ( m_iRight == -1 ) ? TNULL : m_apItems[ m_iRight ];

	// The item that comes in from the side
	TINT iEntering = a_bRight == TTRUE ? m_iRight : m_iLeft;

	if ( iEntering != -1 )
	{
		if ( !bReverse )
		{
			m_pModelRenderer->SetObjectVisible( iEntering, TTRUE );
			m_pModelRenderer->SetObjectOpacity( iEntering, 0.0f );
		}

		TVector4 vecPosition = a_bRight ? s_vecRightPos : s_vecLeftPos;
		vecPosition.Multiply( 2.0f );

		AGopherShopItem* pEntering = m_apItems[ iEntering ];
		pEntering->m_fOffset       = vecPosition.x;
		pEntering->m_fFade         = 0.0f;

		if ( !bReverse )
			AGopherShopItem::UpdateTransform( pEntering, TTRUE, vecPosition, TVector4::VEC_ZERO, 1.0f );
	}

	// The item that was selected
	TINT             iLeaving = ( m_eScroll == SCROLL_RIGHT ) ? m_iLeft : m_iRight;
	AGopherShopItem* pLeaving = ( m_eScroll == SCROLL_RIGHT ) ? m_pLeft : m_pRight;

	if ( !bReverse )
	{
		m_pModelRenderer->SetObjectVisible( iLeaving, TTRUE );
		m_pModelRenderer->SetObjectOpacity( iLeaving, 1.0f );
	}

	pLeaving->m_fOffset = 0.0f;
	pLeaving->m_fFade   = 0.0f;

	if ( !bReverse )
	{
		AGopherShopItem::UpdateTransform( pLeaving, TTRUE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );
		m_pModelRenderer->SetObjectVisible( m_iCurrent, TTRUE );
		m_pModelRenderer->SetObjectOpacity( m_iCurrent, 0.0f );
	}

	// The new selected item comes from the other side
	AGopherShopItem* pCurrent   = m_apItems[ m_iCurrent ];
	const TVector4&  rcPosition = a_bRight ? s_vecRightPos : s_vecLeftPos;
	pCurrent->m_fOffset         = rcPosition.x;
	pCurrent->m_fFade           = 0.0f;

	if ( !bReverse )
		AGopherShopItem::UpdateTransform( pCurrent, TTRUE, rcPosition, TVector4::VEC_ZERO, 1.0f );
}

// $Barnyard: FUNCTION 0049a660
void AGopherShopCategory::Update( TFLOAT a_fDeltaTime, TINT a_iUnused )
{
	if ( m_eScroll == SCROLL_NONE )
		return;

	if ( TMath::Abs( m_pCurrent->m_fOffset ) > 0.01f )
	{
		TFLOAT fSpeed = a_fDeltaTime * 5.0f;

		// The selected item moves to the middle and fades in
		TINT             iCurrent = m_iCurrent;
		AGopherShopItem* pCurrent = m_apItems[ iCurrent ];
		pCurrent->m_fOffset       = TMath::LERP( pCurrent->m_fOffset, 0.0f, fSpeed );
		AGopherShopItem::UpdateTransform( pCurrent, TTRUE, TVector4( pCurrent->m_fOffset, 0.0f, 0.0f, 1.0f ), TVector4::VEC_ZERO, 1.0f );

		pCurrent->m_fFade += a_fDeltaTime;
		if ( pCurrent->m_fFade > 1.0f )
			pCurrent->m_fFade = 1.0f;

		// NOTE: retail reads the opacity of the model here and doesn't use it
		m_pModelRenderer->SetObjectOpacity( iCurrent, TMath::LERP( 0.0f, 1.0f, pCurrent->m_fFade ) );

		if ( m_pLeft )
		{
			TINT             iLeft = m_iLeft;
			AGopherShopItem* pLeft = m_apItems[ iLeft ];
			pLeft->m_fOffset       = TMath::LERP( pLeft->m_fOffset, s_vecLeftPos.x, fSpeed );
			AGopherShopItem::UpdateTransform( pLeft, TTRUE, TVector4( pLeft->m_fOffset, 0.0f, 0.0f, 1.0f ), TVector4::VEC_ZERO, 1.0f );

			if ( m_eScroll == SCROLL_RIGHT )
			{
				// The item that was selected fades out
				pLeft->m_fFade += fSpeed;
				if ( pLeft->m_fFade > 1.0f )
					pLeft->m_fFade = 1.0f;

				m_pModelRenderer->SetObjectOpacity( iLeft, TMath::LERP( 1.0f, 0.0f, EaseOut( pLeft->m_fFade ) ) );
			}
		}

		if ( m_pRight )
		{
			TINT             iRight = m_iRight;
			AGopherShopItem* pRight = m_apItems[ iRight ];
			pRight->m_fOffset       = TMath::LERP( pRight->m_fOffset, s_vecRightPos.x, fSpeed );
			AGopherShopItem::UpdateTransform( pRight, TTRUE, TVector4( pRight->m_fOffset, 0.0f, 0.0f, 1.0f ), TVector4::VEC_ZERO, 1.0f );

			if ( m_eScroll == SCROLL_LEFT )
			{
				pRight->m_fFade += fSpeed;
				if ( pRight->m_fFade > 1.0f )
					pRight->m_fFade = 1.0f;

				m_pModelRenderer->SetObjectOpacity( iRight, TMath::LERP( 1.0f, 0.0f, EaseOut( pRight->m_fFade ) ) );
			}
		}
	}
	else
	{
		// Done, put everything in place
		TINT iCurrent         = m_iCurrent;
		m_eScroll             = SCROLL_NONE;
		m_pCurrent->m_fOffset = 0.0f;
		AGopherShopItem::UpdateTransform( m_pCurrent, TFALSE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );
		m_pModelRenderer->SetObjectOpacity( iCurrent, 1.0f );

		if ( m_pLeft )
		{
			m_pLeft->m_fOffset = s_vecLeftPos.x;
			AGopherShopItem::UpdateTransform( m_pLeft, TTRUE, s_vecLeftPos, TVector4::VEC_ZERO, 1.0f );
			m_pModelRenderer->SetObjectOpacity( m_iLeft, 0.0f );
		}

		if ( m_pRight )
		{
			m_pRight->m_fOffset = s_vecRightPos.x;
			AGopherShopItem::UpdateTransform( m_pRight, TTRUE, s_vecRightPos, TVector4::VEC_ZERO, 1.0f );
			m_pModelRenderer->SetObjectOpacity( m_iRight, 0.0f );
		}
	}
}

// $Barnyard: FUNCTION 0049aa80
AGopherShopItem* AGopherShopCategory::GetCurrentItem()
{
	if ( m_iNumItems != 0 )
		return m_apItems[ m_iSelected ];

	return TNULL;
}

// $Barnyard: FUNCTION 0049aaa0
void AGopherShopCategory::Clear()
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
		m_apItems[ i ] = TNULL;

	m_iNumItems = 0;

	for ( TINT i = 0; i < m_iNumRemoved; i++ )
		m_apRemoved[ i ] = TNULL;

	m_iNumRemoved = 0;
}

// $Barnyard: FUNCTION 0049ab00
void AGopherShopCategory::Show()
{
	m_pModelRenderer->RemoveAllObjects();

	m_eScroll  = SCROLL_NONE;
	m_pCurrent = TNULL;
	m_pLeft    = TNULL;
	m_pRight   = TNULL;
	m_iCurrent = 0;

	if ( m_iNumItems == 1 )
	{
		m_iLeft  = -1;
		m_iRight = -1;
	}
	else if ( m_iNumItems == 2 )
	{
		m_iLeft  = -1;
		m_iRight = 1;
	}
	else
	{
		m_iLeft    = 0;
		m_iRight   = 2;
		m_iCurrent = 1;
	}

	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		AGopherShopItem* pItem = m_apItems[ i ];
		m_pModelRenderer->AddObject( pItem->GetObject(), &s_vecLightColour, &s_vecAmbientColour );
		m_pModelRenderer->SetObjectVisible( i, TFALSE );

		if ( i != m_iLeft && i != m_iRight && i != m_iCurrent )
			continue;

		m_pModelRenderer->SetObjectVisible( i, TTRUE );
		m_pModelRenderer->SetObjectOpacity( i, ( i == m_iCurrent ) ? 1.0f : 0.0f );

		TVector4 vecPosition;

		if ( i == m_iLeft )
			vecPosition = s_vecLeftPos;
		else if ( i == m_iRight )
			vecPosition = s_vecRightPos;
		else
			vecPosition = TVector4::VEC_ZERO;

		pItem->m_fOffset = vecPosition.x;
		AGopherShopItem::UpdateTransform( pItem, TTRUE, vecPosition, TVector4::VEC_ZERO, 1.0f );
	}

	// NOTE: retail reads the first item even if there's none
	m_pCurrent = m_apItems[ m_iCurrent ];

	if ( m_iLeft != -1 )
		m_pLeft = m_apItems[ m_iLeft ];

	if ( m_iRight != -1 )
		m_pRight = m_apItems[ m_iRight ];

	m_iSelected = m_iCurrent;
}

// $Barnyard: FUNCTION 0049ad00
void AGopherShopCategory::DestroyItems()
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		if ( m_apItems[ i ] )
			delete m_apItems[ i ];
	}

	for ( TINT i = 0; i < m_iNumRemoved; i++ )
	{
		if ( m_apRemoved[ i ] )
			delete m_apRemoved[ i ];
	}
}

// $Barnyard: FUNCTION 0049ade0
TBOOL AGopherShopCategory::RemoveItem( AGopherShopItem* a_pItem )
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		if ( m_apItems[ i ] == a_pItem )
		{
			for ( TINT k = i; k < m_iNumItems - 1; k++ )
				m_apItems[ k ] = m_apItems[ k + 1 ];

			m_iNumItems--;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0049ae50
void AGopherShopCategory::RemoveItemAndKeep( AGopherShopItem* a_pItem )
{
	RemoveItem( a_pItem );
	m_apRemoved[ m_iNumRemoved++ ] = a_pItem;

	if ( m_iNumItems != 0 )
		Show();
}

// $Barnyard: FUNCTION 0049ae90
AGopherShopItem* AGopherShopCategory::FindItemByModel( TPString8 a_strModel )
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		if ( a_strModel == m_apItems[ i ]->GetModel() )
			return m_apItems[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0049af70
AGopherShopItem* AGopherShopCategory::FindItem( const AGopherShopPurchase& a_rcPurchase )
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		if ( a_rcPurchase.iId == m_apItems[ i ]->GetPurchase().iId )
			return m_apItems[ i ];
	}

	return TNULL;
}
