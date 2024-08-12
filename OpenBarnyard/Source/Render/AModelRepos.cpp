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

AModelRepos::AModelRepos() :
    m_UsedModels( AMemory::GetAllocator( AMemory::POOL_Misc ) ),
    m_Models2( AMemory::GetAllocator( AMemory::POOL_Misc ) )
{
}

AModelRepos::~AModelRepos()
{
    TIMPLEMENT();
    UnloadAllModels();
}

void AModelRepos::UnloadAllModels()
{
    while ( !m_UnusedModels.IsEmpty() )
    {
        auto pModel = m_UnusedModels.Head();

        pModel->Remove();
        delete pModel;
    }
}

AModel* AModelRepos::GetModel( const Toshi::TPString8& a_rName )
{
    auto pRes = m_Models2.FindNode( a_rName );
    if ( pRes != m_Models2.End() ) return pRes->GetValue()->GetSecond();

    pRes = m_UsedModels.FindNode( a_rName );
    if ( pRes != m_Models2.End() ) return pRes->GetValue()->GetSecond();

    return TNULL;
}

AModel* AModelRepos::GetUnusedModel( const Toshi::TPString8& a_rName )
{
    T2_FOREACH ( m_UnusedModels, it )
    {
        if ( it->GetName() == a_rName )
        {
            return it;
        }
    }

    return TNULL;
}

void AModelRepos::Update( TFLOAT a_fDeltaTime )
{
    T2_FOREACH ( m_UsedModels, it )
    {
        it->GetSecond()->Update( a_fDeltaTime );
    }

    T2_FOREACH ( m_Models2, it )
    {
        it->GetSecond()->Update( a_fDeltaTime );
    }
}

AModel* AModelRepos::CreateNewModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB )
{
    if ( !a_pTRB )
    {
        a_pTRB = AAssetLoader::GetAssetTRB( AAssetType_AssetPack );
    }

    TVALIDPTR( a_pTRB );
    return new AModel( a_rName, a_pTRB );
}

AModelInstance* AModelRepos::InstantiateModel( AModel* a_pModel )
{
    TVALIDPTR( a_pModel );

    AModelInstanceRef modelInstance;
    a_pModel->CreateInstance( modelInstance );

    return modelInstance.Get();
}

AModelInstance* AModelRepos::InstantiateNewModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB )
{
    AModel* pModel      = TNULL;
    auto    pModels2Res = m_Models2.FindNode( a_rName );

    if ( pModels2Res == m_Models2.End() )
    {
        auto pUsedModelsRes = m_UsedModels.FindNode( a_rName );

        if ( pUsedModelsRes != m_UsedModels.End() )
        {
            pModel = pUsedModelsRes->GetValue()->GetSecond();
        }
    }
    else
    {
        pModel = pModels2Res->GetValue()->GetSecond();
    }

    if ( !pModel )
    {
        pModel = CreateNewModel( a_rName, a_pTRB );
        m_UsedModels.Insert( a_rName, pModel );
    }

    TVALIDPTR( pModel );
    return InstantiateModel( pModel );
}

void AModelRepos::CreateInModels2( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB )
{
    if ( m_Models2.FindNode( a_rName ) == m_Models2.End() )
    {
        AModel* pModel         = TNULL;
        auto    pUsedModelsRes = m_UsedModels.FindNode( a_rName );

        if ( pUsedModelsRes == m_UsedModels.End() )
        {
            pModel = GetUnusedModel( a_rName );

            if ( pModel )
            {
                pModel->Remove();
            }
            else
            {
                pModel = CreateNewModel( a_rName, a_pTRB );
            }
        }
        else
        {
            pModel = pUsedModelsRes->GetValue()->GetSecond();
            m_UsedModels.RemoveNode( pUsedModelsRes );
        }

        TVALIDPTR( pModel );
        m_Models2.Insert( a_rName, pModel );
    }
}
