#include "ToshiPCH.h"
#include "TGUIComponentCache.h"
#include "TGUIComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TINT TGUIComponentCache::ms_iNumCaches = 0;

// $NicktoonsPS2: FUNCTION 0037af90
// $JPOG: FUNCTION TGuiInterface.dll+00006ba0
TGUIComponentCache::TGUIComponentCache()
    : m_oBBox( 0, 0, 0, 0 )
{
	m_oRoot.pNext      = &m_oRoot;
	m_oRoot.pPrev      = &m_oRoot;
	m_oRoot.pComponent = TNULL;
	m_oRoot.pData      = TNULL;

	ms_iNumCaches++;
}

// NOTE: the instance count is also increased here (the destructor decreases it), JPOG has no count
// $JPOG: FUNCTION TGuiInterface.dll+00006b70
TGUIComponentCache::TGUIComponentCache( const TGUIComponentCache& a_rcOther )
    : m_oBBox( a_rcOther.m_oBBox )
{
	m_oRoot.pNext      = &m_oRoot;
	m_oRoot.pPrev      = &m_oRoot;
	m_oRoot.pComponent = TNULL;
	m_oRoot.pData      = TNULL;

	ms_iNumCaches++;
}

// $NicktoonsPS2: FUNCTION 0037b008
// $JPOG: FUNCTION TGuiInterface.dll+00006b20
TGUIComponentCache::~TGUIComponentCache()
{
	ms_iNumCaches--;
	Invalidate();
}

// $NicktoonsPS2: FUNCTION 0037afc0
// $JPOG: FUNCTION TGuiInterface.dll+00006b40
TGUIComponentCache& TGUIComponentCache::operator=( const TGUIComponentCache& a_rcOther )
{
	m_oBBox = a_rcOther.m_oBBox;
	Invalidate();

	return *this;
}

// $NicktoonsPS2: FUNCTION 0037b140
// $JPOG: FUNCTION TGuiInterface.dll+00025d00
void TGUIComponentCache::Invalidate()
{
	while ( m_oRoot.pNext != &m_oRoot )
	{
		Data* pData = m_oRoot.pNext;
		pData->pComponent->DestroyCache( pData->pData );

		pData->pPrev->pNext = pData->pNext;
		pData->pNext->pPrev = pData->pPrev;
		pData->pNext        = pData;
		pData->pPrev        = pData;

		delete pData;
	}
}

// $NicktoonsPS2: FUNCTION 0037b1d0
// $JPOG: FUNCTION TGuiInterface.dll+00025d60
TGUIComponentCache::Data* TGUIComponentCache::FindData( const TGUIComponent* a_pComponent ) const
{
	for ( Data* pData = m_oRoot.pNext; pData != &m_oRoot; pData = pData->pNext )
	{
		if ( pData->pComponent == a_pComponent )
			return pData;
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 0037b060
// $JPOG: FUNCTION TGuiInterface.dll+00026110
void* TGUIComponentCache::GetData( const TGUIComponent* a_pComponent ) const
{
	Data* pData = FindData( a_pComponent );
	return pData ? pData->pData : TNULL;
}

// $NicktoonsPS2: FUNCTION 0037b088
// $JPOG: FUNCTION TGuiInterface.dll+00025c80
void TGUIComponentCache::SetData( const TGUIComponent* a_pComponent, void* a_pData )
{
	Data* pData = FindData( a_pComponent );

	if ( pData )
	{
		if ( pData->pData )
			a_pComponent->DestroyCache( pData->pData );
	}
	else
	{
		// Insert at the head
		pData               = new Data;
		pData->pComponent   = a_pComponent;
		pData->pPrev        = &m_oRoot;
		pData->pNext        = m_oRoot.pNext;
		m_oRoot.pNext       = pData;
		pData->pNext->pPrev = pData;
	}

	pData->pData = a_pData;
}

// The data is invalidated if the size changes
// $JPOG: FUNCTION TGuiInterface.dll+00006ac0
void TGUIComponentCache::SetBBox( const TGUIRectangle& a_rcBBox, const TGUIComponent* a_pComponent )
{
	if ( a_rcBBox.m_iWidth != m_oBBox.m_iWidth || a_rcBBox.m_iHeight != m_oBBox.m_iHeight )
		Invalidate();

	m_oBBox = a_rcBBox;
}

TOSHI_NAMESPACE_END
