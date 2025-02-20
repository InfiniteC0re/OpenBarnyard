#include "ToshiPCH.h"
#include "Render/TShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006d5a60
void TOrderTable::CreateStaticData( TUINT a_uiMaxMaterials, TUINT a_uiMaxRenderPackets )
{
	s_uiMaxRenderPackets = a_uiMaxRenderPackets;
	s_uiNumRenderPackets = 0;

	s_pRenderPackets         = new TRenderPacket[ a_uiMaxRenderPackets ];
	s_uiOrigMaxRenderPackets = a_uiMaxRenderPackets;

	s_pRegMaterials  = new TRegMaterial[ a_uiMaxMaterials ];
	s_uiMaxMaterials = a_uiMaxMaterials;

	for ( TSIZE i = 0; i < s_uiMaxMaterials; i++ )
	{
		s_pRegMaterials[ i ].SetOrderTable( TNULL );
		s_llRegMatFreeList.InsertTail( &s_pRegMaterials[ i ] );
	}
}

TRenderPacket* TOrderTable::AllocRenderPacket()
{
	TASSERT( s_uiNumRenderPackets + 1 < s_uiMaxRenderPackets );
	TRenderPacket* packet = &s_pRenderPackets[ s_uiNumRenderPackets++ ];
	return s_uiNumRenderPackets < s_uiMaxRenderPackets ? &s_pRenderPackets[ s_uiNumRenderPackets ] : TNULL;
}

// $Barnyard: FUNCTION 006c4610
TOrderTable::TOrderTable()
{
	m_pLastRegMat = TNULL;
	m_pShader     = TNULL;
}

// $Barnyard: FUNCTION 006d5d60
TOrderTable::~TOrderTable()
{
	DestroyStaticData();
}

// $Barnyard: FUNCTION 006d5910
void TOrderTable::Render()
{
	TPROFILER_SCOPE();

	for ( auto it = m_pLastRegMat; it != TNULL; it = it->GetNextRegMat() )
	{
		it->Render();
	}

	s_uiNumRenderPackets = 0;
	m_pLastRegMat        = TNULL;
}

// $Barnyard: FUNCTION 006d5970
void TOrderTable::Flush()
{
	TPROFILER_SCOPE();

	if ( s_uiMaxNumRenderPackets < s_uiNumRenderPackets )
	{
		s_uiMaxNumRenderPackets = s_uiNumRenderPackets;
	}

	m_pShader->StartFlush();

	for ( auto it = m_pLastRegMat; it != TNULL; it = it->GetNextRegMat() )
	{
		it->Render();
	}

	m_pShader->EndFlush();

	s_uiNumRenderPackets = 0;
	m_pLastRegMat        = TNULL;
}

// $Barnyard: FUNCTION 006d5be0
TRegMaterial* TOrderTable::RegisterMaterial( TMaterial* pMat )
{
	TASSERT( s_uiNumRenderPackets == 0 );

	TRegMaterial* pCurrentRegMat = pMat->GetRegMaterial();
	if ( pCurrentRegMat != TNULL )
		return pCurrentRegMat;

	TASSERT( TFALSE == s_llRegMatFreeList.IsEmpty() );

	if ( s_llRegMatFreeList.IsEmpty() )
		return TNULL;

	TRegMaterial* pRegMat = s_llRegMatFreeList.RemoveHead();
	s_llRegMatRegisteredList.InsertTail( pRegMat );
	s_uiNumRegisteredMaterials += 1;

	pRegMat->SetFlags( TRegMaterial::State_Registered );
	pRegMat->SetMaterial( pMat );
	pRegMat->SetOrderTable( this );
	pMat->SetRegMaterial( pRegMat );

	return pRegMat;
}

// $Barnyard: FUNCTION 006d5c60
void TOrderTable::DeregisterMaterial( TRegMaterial* pRegMat )
{
	if ( HASANYFLAG( pRegMat->GetFlags(), TRegMaterial::State_Registered ) )
	{
		TASSERT( s_uiNumRenderPackets == 0 );

		TMaterial* pMaterial = pRegMat->GetMaterial();

		pRegMat->Remove();
		s_llRegMatFreeList.InsertHead( pRegMat );
		s_uiNumRegisteredMaterials -= 1;

		pRegMat->SetFlags( 0 );
		pRegMat->SetMaterial( TNULL );
		pRegMat->SetOrderTable( TNULL );
		pMaterial->SetRegMaterial( TNULL );
	}
}

// $Barnyard: FUNCTION 006d5cd0
void TOrderTable::DeregisterAllMaterials()
{
	while ( !s_llRegMatRegisteredList.IsEmpty() )
	{
		DeregisterMaterial( s_llRegMatRegisteredList.Head() );
	}
}

void TOrderTable::UseMaterial( TRegMaterial* a_pRegMat )
{
	if ( !HASANYFLAG( a_pRegMat->GetFlags(), TRegMaterial::State_Used ) )
	{
		a_pRegMat->SetFlags( a_pRegMat->GetFlags() | TRegMaterial::State_Used );
		a_pRegMat->SetNextRegMat( m_pLastRegMat );
		m_pLastRegMat = a_pRegMat;
	}
}

void TOrderTable::DestroyStaticData()
{
	if ( s_pRegMaterials )
	{
		DeregisterAllMaterials();
		s_llRegMatFreeList.RemoveAll();

		delete[] s_pRegMaterials;
		s_pRegMaterials = TNULL;
	}

	delete[] s_pRenderPackets;
	s_pRenderPackets = TNULL;
}

void TRegMaterial::Render()
{
	TPROFILER_SCOPE();

	m_pMaterial->PreRender();

	for ( auto pPacket = m_pLastRenderPacket; pPacket != TNULL; pPacket = pPacket->GetNextPacket() )
	{
		pPacket->GetMesh()->GetOwnerShader()->Render( pPacket );
	}

	m_pLastRenderPacket = TNULL;
	SetFlags( m_State & ~State_Used );
	m_pMaterial->PostRender();
}

// $Barnyard: FUNCTION 006d59f0
TRenderPacket* TRegMaterial::AddRenderPacket( TMesh* pMesh )
{
	TRenderPacket* pPreviousPacket = m_pLastRenderPacket;

	m_pLastRenderPacket = TOrderTable::AllocRenderPacket();
	m_pLastRenderPacket->SetNextPacket( pPreviousPacket );

	m_pOrderTable->UseMaterial( this );
	m_pLastRenderPacket->SetMesh( pMesh );
	m_pLastRenderPacket->SetMaterial( pMesh->GetMaterial() );

	return m_pLastRenderPacket;
}

TOSHI_NAMESPACE_END
