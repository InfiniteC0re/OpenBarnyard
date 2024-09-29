#include "pch.h"
#include "AModelRepos.h"
#include "Assets/AAssetLoader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AModelRepos );

// $Barnyard: FUNCTION 00612340
AModelRepos::AModelRepos()
    : m_UsedModels( AMemory::GetAllocator( AMemory::POOL_Misc ) ), m_AllModels( AMemory::GetAllocator( AMemory::POOL_Misc ) )
{
}

// $Barnyard: FUNCTION 00613040
// $Barnyard: FUNCTION 006130f0
AModelRepos::~AModelRepos()
{
	TIMPLEMENT();
	UnloadAllModels();
	MarkAllModelsUnused();
	UnloadAllUnusedModels();
}

// $Barnyard: FUNCTION 00612440
void AModelRepos::UnloadAllUnusedModels()
{
	while ( !m_UnusedModels.IsEmpty() )
	{
		auto pModel = m_UnusedModels.Head();

		pModel->Remove();
		delete pModel;
	}
}

// $Barnyard: FUNCTION 006123c0
AModel* AModelRepos::GetModel( const Toshi::TPString8& a_rName )
{
	{
		// Look in list of all models
		auto pRes = m_AllModels.Find( a_rName );
		if ( m_AllModels.IsValid( pRes ) ) return pRes->GetSecond();
	}

	{
		// Look in list used models
		auto pRes = m_UsedModels.Find( a_rName );
		if ( m_UsedModels.IsValid( pRes ) ) return pRes->GetSecond();
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00612480
AModel* AModelRepos::GetUnusedModel( const Toshi::TPString8& a_rName )
{
	T2_FOREACH( m_UnusedModels, it )
	{
		if ( it->GetName() == a_rName )
		{
			return it;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00612540
void AModelRepos::Update( TFLOAT a_fDeltaTime )
{
	T2_FOREACH( m_UsedModels, it )
	{
		it->GetSecond()->Update( a_fDeltaTime );
	}

	T2_FOREACH( m_AllModels, it )
	{
		it->GetSecond()->Update( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 00612270
AModel* AModelRepos::AllocateAModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB )
{
	if ( !a_pTRB )
	{
		a_pTRB = AAssetLoader::GetAssetTRB( AAssetType_AssetPack );
	}

	TVALIDPTR( a_pTRB );
	return new AModel( a_rName, a_pTRB );
}

// $Barnyard: FUNCTION 00612fe0
void AModelRepos::MarkAllModelsUnused()
{
	for ( ModelsMap::Iterator it = m_UsedModels.Begin(); it != m_UsedModels.End(); )
	{
		ModelsMap::Iterator next = it.Next();

		MarkModelUnused( it->GetSecond() );
		it = next;
	}
}

// $Barnyard: FUNCTION 00612be0
void AModelRepos::MarkModelUnused( AModel* a_pModel )
{
	ModelsMap::Iterator modelIt = m_UsedModels.FindByValue( a_pModel );
	AModel*             pModel  = modelIt->GetSecond();

	TASSERT( modelIt != m_UsedModels.End() );
	m_UsedModels.Remove( modelIt );
	m_UnusedModels.PushFront( pModel );

	// Delete 1 unused model if there are too many of them
	if ( m_UnusedModels.Size() > MAX_UNUSED_MODELS_NUM )
	{
		AModel* pOldestUnusedModel = m_UnusedModels.Tail();
		pOldestUnusedModel->Remove();

		delete pOldestUnusedModel;
	}
}

// $Barnyard: FUNCTION 00612df0
void AModelRepos::UnloadAllModels()
{
	for ( ModelsMap::Iterator it = m_AllModels.Begin(); it != m_AllModels.End(); )
	{
		ModelsMap::Iterator next   = it.Next();
		AModel*             pModel = it->GetSecond();

		if ( pModel->GetNumInstances() < 1 )
		{
			m_AllModels.Remove( it );

			m_UnusedModels.PushFront( pModel );
		}

		it = next;
	}
}

// $Barnyard: FUNCTION 006127f0
AModelInstance* AModelRepos::InstantiateModel( AModel* a_pModel )
{
	TVALIDPTR( a_pModel );
	return a_pModel->CreateInstance().Get();
}

// $Barnyard: FUNCTION 00612b10
AModelInstance* AModelRepos::InstantiateNewModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB )
{
	AModel*             pModel      = TNULL;
	ModelsMap::Iterator pModels2Res = m_AllModels.Find( a_rName );

	if ( pModels2Res != m_AllModels.End() )
	{
		pModel = pModels2Res->GetSecond();
	}
	else
	{
		ModelsMap::Iterator pUsedModelsRes = m_UsedModels.Find( a_rName );

		if ( m_UsedModels.IsValid( pUsedModelsRes ) )
		{
			pModel = pUsedModelsRes->GetSecond();
		}
	}

	if ( !pModel )
	{
		pModel = AllocateAModel( a_rName, a_pTRB );
		m_UsedModels.Insert( a_rName, pModel );
	}

	TVALIDPTR( pModel );
	return InstantiateModel( pModel );
}

// $Barnyard: FUNCTION 00612f50
void AModelRepos::DestroyModelInstance( AModelInstance* a_pInstance )
{
	{
		// Remove from the list of all models
		auto it = m_AllModels.FindByValue( a_pInstance->GetModel() );

		if ( m_AllModels.IsValid( it ) )
		{
			AModel* pModel = it->GetSecond();
			pModel->DestroyInstance( a_pInstance );
		}
	}

	{
		// Remove from the list of used models
		auto it = m_UsedModels.FindByValue( a_pInstance->GetModel() );

		if ( m_UsedModels.IsValid( it ) )
		{
			AModel* pModel = it->GetSecond();
			pModel->DestroyInstance( a_pInstance );

			if ( pModel->GetNumInstances() < 1 )
			{
				MarkModelUnused( pModel );
			}
		}
	}
}

// $Barnyard: FUNCTION 00612c90
void AModelRepos::LoadModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB )
{
	if ( m_AllModels.Find( a_rName ) == m_AllModels.End() )
	{
		AModel* pModel         = TNULL;
		auto    pUsedModelsRes = m_UsedModels.Find( a_rName );

		if ( !m_UsedModels.IsValid( pUsedModelsRes ) )
		{
			pModel = GetUnusedModel( a_rName );

			if ( pModel )
			{
				pModel->Remove();
			}
			else
			{
				pModel = AllocateAModel( a_rName, a_pTRB );
			}
		}
		else
		{
			pModel = pUsedModelsRes->GetSecond();
			m_UsedModels.Remove( pUsedModelsRes );
		}

		TVALIDPTR( pModel );
		m_AllModels.Insert( a_rName, pModel );
	}
}

// $Barnyard: FUNCTION 00612d60
void AModelRepos::UnloadModel( const Toshi::TPString8& a_rcName, TBOOL a_bDestroy )
{
	auto pModelNode = m_AllModels.Find( a_rcName );

	if ( m_AllModels.IsValid( pModelNode ) && pModelNode->GetSecond()->GetNumInstances() == 0 )
	{
		AModel* pModel = pModelNode->GetSecond();
		m_AllModels.Remove( pModelNode );

		if ( a_bDestroy )
		{
			if ( pModel->IsLinked() )
				pModel->Remove();

			delete pModel;
		}
		else
		{
			m_UnusedModels.PushFront( pModel );
		}
	}
}

// $Barnyard: FUNCTION 006124e0
// Note: this function was replaced with T2Map::FindByValue

// $Barnyard: FUNCTION 00612300
// Note: m_UnusedModels.PushFront

// $Barnyard: FUNCTION 0060fc00
// Note: m_vecInstanceRefs.PopBack
