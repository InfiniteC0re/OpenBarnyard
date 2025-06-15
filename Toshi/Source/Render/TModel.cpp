#include "ToshiPCH.h"
#include "TModel.h"

#include "Render/TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006cdf20
TModel::TModel()
{
	m_eFlags              = Flags_None;
	m_iNumInstances       = 0;
	m_iLODCount           = 0;
	m_iNumCollisionMeshes = 0;
	m_pSkeleton           = TNULL;
	m_pCollisionMeshes    = TNULL;
	m_pTRB                = TNULL;
	m_bIsAssetFile        = TFALSE;
	m_aLODDistances[ 0 ]  = 5.0f;
	m_aLODDistances[ 1 ]  = 20.0f;
	m_aLODDistances[ 2 ]  = m_aLODDistances[ 1 ] + m_aLODDistances[ 1 ];
	m_aLODDistances[ 3 ]  = m_aLODDistances[ 2 ] + m_aLODDistances[ 2 ];
}

// $Barnyard: FUNCTION 006cdff0
// $Barnyard: FUNCTION 006ce130
TModel::~TModel()
{
}

// $Barnyard: FUNCTION 006ce3e0
TBOOL TModel::LoadTRB()
{
	TASSERT( m_eFlags & Flags_Created );

	// Load skeleton
	if ( auto pSkeletonHeader = TSTATICCAST( TTMDBase::SkeletonHeader, GetSymbol( "SkeletonHeader" ) ) )
	{
		m_pSkeleton                = TSTATICCAST( TSkeleton, GetSymbol( "Skeleton" ) );
		TKeyframeLibrary* pLibrary = TRenderInterface::GetSingleton()->GetKeyframeLibraryManager().GetLibrary( pSkeletonHeader->m_szTKLName );

		if ( !pLibrary )
		{
			TERROR( "TModel: loading a model without required keyframe library: %s\n", pSkeletonHeader->m_szTKLName );
		}

		m_pSkeleton->GetKeyLibraryInstance().CreateEx(
		    pLibrary,
		    pSkeletonHeader->m_iTKeyCount,
		    pSkeletonHeader->m_iQKeyCount,
		    pSkeletonHeader->m_iSKeyCount,
		    pSkeletonHeader->m_iTBaseIndex,
		    pSkeletonHeader->m_iQBaseIndex,
		    pSkeletonHeader->m_iSBaseIndex
		);
	}

	// Load collision
	auto pCollisionHeader = TSTATICCAST( TTMDBase::CollisionHeader, GetSymbol( "Collision" ) );
	TASSERT( TNULL != pCollisionHeader );

	m_iNumCollisionMeshes = pCollisionHeader->m_iNumMeshes;
	m_pCollisionMeshes    = new TModelCollisionData[ pCollisionHeader->m_iNumMeshes ];

	// Copy info about the collision meshes
	for ( TINT i = 0; i < pCollisionHeader->m_iNumMeshes; i++ )
	{
		auto&       collisionMesh       = m_pCollisionMeshes[ i ];
		const auto& collisionMeshHeader = pCollisionHeader->m_pMeshes[ i ];

		// Copy base info
		collisionMesh.m_iBoneID       = collisionMeshHeader.m_iBoneID;
		collisionMesh.m_pVertices     = collisionMeshHeader.m_pVertices;
		collisionMesh.m_uiNumVertices = collisionMeshHeader.m_uiNumVertices;
		collisionMesh.m_pIndices      = collisionMeshHeader.m_pIndices;
		collisionMesh.m_uiNumIndices  = collisionMeshHeader.m_uiNumIndices;

		// Reserve space for collision types
		collisionMesh.m_vecCollGroups.SetSize( collisionMeshHeader.m_uiNumCollTypes, TModelCollisionGroup{} );

		// Copy collision types
		for ( TUINT k = 0; k < collisionMeshHeader.m_uiNumCollTypes; k++ )
		{
			collisionMesh.m_vecCollGroups[ k ].strName    = collisionMeshHeader.m_pCollGroups[ k ].pszName;
			collisionMesh.m_vecCollGroups[ k ].uiUnk1     = collisionMeshHeader.m_pCollGroups[ k ].iUnk1;
			collisionMesh.m_vecCollGroups[ k ].uiNumFaces = collisionMeshHeader.m_pCollGroups[ k ].uiNumFaces;

			// TODO: figure out what it is
			collisionMesh.m_vecCollGroups[ k ].vecS1.SetSize( collisionMeshHeader.m_pCollGroups[ k ].iSomeCount );
			collisionMesh.m_vecCollGroups[ k ].uiUnk3     = 0;
			collisionMesh.m_vecCollGroups[ k ].eCollGroup = TCollisionCommon::TOSHICGROUP_SUPPORT;

			for ( TINT j = 0; j < collisionMeshHeader.m_pCollGroups[ k ].iSomeCount; j++ )
			{
				collisionMesh.m_vecCollGroups[ k ].vecS1[ j ].uiUnk1 = collisionMeshHeader.m_pCollGroups[ k ].pS1[ j ].uiUnk1;
				collisionMesh.m_vecCollGroups[ k ].vecS1[ j ].uiUnk2 = collisionMeshHeader.m_pCollGroups[ k ].pS1[ j ].uiUnk2;
			}
		}
	}

	// Now let some client modelloader handle this
	TASSERT( ms_cbModelLoaderTRB != TNULL, "Loader callback is not specified" );
	ms_cbModelLoaderTRB( this );

	m_eFlags |= Flags_Loaded;
	return TTRUE;
}

// $Barnyard: FUNCTION 006ce8c0
TBOOL TModel::LoadTRB( TTRB* a_pTRB )
{
	m_bIsAssetFile = TFALSE;
	m_pTRB         = a_pTRB;

	return LoadTRB();
}

// $Barnyard: FUNCTION 006ce820
TBOOL TModel::LoadTRB( const TCHAR* a_szFileName, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	if ( !m_bIsAssetFile && m_pTRB )
	{
		delete m_pTRB;
	}

	if ( a_pAssetTRB )
	{
		m_pTRB                 = a_pAssetTRB;
		m_bIsAssetFile         = TTRUE;
		m_szSymbolPrefix       = a_szFileName;
		m_szSymbolPrefixLength = a_ui8FileNameLen;
	}
	else
	{
		m_bIsAssetFile = TFALSE;
		m_pTRB         = new TTRB();

		if ( m_pTRB->Load( a_szFileName, 0 ) != TTRB::ERROR_OK )
		{
			return TFALSE;
		}
	}

	return LoadTRB();
}

// $Barnyard: FUNCTION 006ce780
void TModel::UnloadTRB()
{
	if ( m_pTRB )
	{
		// Reset pointers in collision meshes, since we don't own the data and don't want to free it
		for ( TINT i = 0; i < m_iNumCollisionMeshes; i++ )
		{
			m_pCollisionMeshes[ i ].m_pVertices = TNULL;
			m_pCollisionMeshes[ i ].m_pIndices  = TNULL;
		}

		if ( !m_bIsAssetFile )
		{
			m_pTRB->Close();
			delete m_pTRB;
		}

		m_pTRB = TNULL;
	}

	m_bIsAssetFile = TFALSE;

	if ( m_pCollisionMeshes )
		delete[] m_pCollisionMeshes;

	m_pCollisionMeshes = TNULL;
	m_eFlags &= Flags_Loaded;
}

// $Barnyard: FUNCTION 006ce010
void* TModel::GetSymbol( const TCHAR* a_szSymbolName )
{
	if ( m_bIsAssetFile )
	{
		TCHAR symbolName[ 64 ];
		TStringManager::String8Copy( symbolName, m_szSymbolPrefix, m_szSymbolPrefixLength );
		TStringManager::String8Copy( symbolName + m_szSymbolPrefixLength, a_szSymbolName );

		return m_pTRB->GetSymbolAddress( symbolName );
	}

	return m_pTRB->GetSymbolAddress( a_szSymbolName );
}

// $Barnyard: FUNCTION 006ce180
TModelInstance* TModel::CreateInstance()
{
	auto pInstance = new TModelInstance( this );

	if ( m_pSkeleton )
	{
		pInstance->m_pSkeletonInstance = m_pSkeleton->CreateInstance( TTRUE );

		if ( TNULL == pInstance->m_pSkeletonInstance )
		{
			TASSERT( !"Couldn't create skeleton instance" );
			delete pInstance;
			return TNULL;
		}
	}

	m_iNumInstances++;
	return pInstance;
}

// $Barnyard: FUNCTION 006c1b40
TBOOL TModel::GetSkeletonAssetSymbolName( const TCHAR* a_szFileName, const TCHAR*& a_rSymbolName, TUINT8& a_rNameLen, TTRB* a_pTRB )
{
	auto iFilePathLength = TUINT8( TStringManager::String8Length( a_szFileName ) );
	auto iFileNamePos    = iFilePathLength - 1;
	a_rNameLen           = iFilePathLength;

	while ( a_szFileName[ iFileNamePos ] != '\\' && a_szFileName[ iFileNamePos ] != '/' )
		iFileNamePos--;

	iFileNamePos++;
	TCHAR symbolName[ 64 ];

	auto iFileNameLength = iFilePathLength - iFileNamePos - 4;
	a_rNameLen           = iFileNameLength;

	TStringManager::String8Copy( symbolName, a_szFileName + iFileNamePos, iFileNameLength );
	symbolName[ iFileNameLength ]     = '_';
	symbolName[ iFileNameLength + 1 ] = '\0';
	a_rNameLen                        = iFileNameLength + 1;

	TStringManager::String8ToLowerCase( symbolName );
	TStringManager::String8Copy( symbolName + a_rNameLen, "Skeleton" );

	auto pSymbol = a_pTRB->GetSymbol( symbolName );

	if ( pSymbol )
	{
		a_rSymbolName = a_pTRB->GetSymbolName( pSymbol );
		return TTRUE;
	}
	else
	{
		return TFALSE;
	}
}

TModelInstance::TModelInstance( TModel* a_pModel )
{
	m_pModel                  = a_pModel;
	m_pSkeletonInstance       = TNULL;
	m_fnPreRenderCb           = 0;
	m_fnCustomRenderCb        = TNULL;
	m_pCustomRenderCbUserData = TNULL;
	m_iCurrentLOD             = 0;
}

TModelInstance::~TModelInstance()
{
}

void TModelInstance::Update( TFLOAT a_fDeltaTime )
{
	m_pSkeletonInstance->UpdateTime( a_fDeltaTime );
}

// $Barnyard: FUNCTION 006ce090
void TModelInstance::Render()
{
	if ( m_fnPreRenderCb )
	{
		m_fnPreRenderCb( this );
	}

	if ( !m_fnCustomRenderCb )
	{
		m_pSkeletonInstance->UpdateState( TTRUE );
		TRenderInterface::GetSingleton()->GetCurrentContext()->SetSkeletonInstance( m_pSkeletonInstance );

		for ( TINT i = 0; i < m_pModel->m_LODs[ m_iCurrentLOD ].iNumMeshes; i++ )
		{
			auto pMesh = m_pModel->m_LODs[ m_iCurrentLOD ].ppMeshes[ i ];
			pMesh->Render();
		}
	}
	else
	{
		m_fnCustomRenderCb( this, m_pCustomRenderCbUserData );
	}
}

void TModelInstance::SetPreRenderCallback( t_PreRenderCB a_fnCallback )
{
	m_fnPreRenderCb = a_fnCallback;
}

void TModelInstance::SetCustomRenderMethod( t_CustomRenderCB a_fnCallback, void* a_pUserData )
{
	m_fnCustomRenderCb        = a_fnCallback;
	m_pCustomRenderCbUserData = a_pUserData;
}

void TModelInstance::SetLOD( TINT32 a_iLOD )
{
	m_iCurrentLOD = a_iLOD;
}

TINT32 TModelInstance::GetLOD()
{
	return m_iCurrentLOD;
}

// $Barnyard: FUNCTION 006ce150
void TModelInstance::Delete()
{
	if ( m_pSkeletonInstance )
	{
		m_pSkeletonInstance->Delete();
		m_pSkeletonInstance = TNULL;
	}

	m_pModel->m_iNumInstances--;
	m_pModel = TNULL;
	delete this;
}

TModel* TModelInstance::GetModel() const
{
	return m_pModel;
}

TSkeletonInstance* TModelInstance::GetSkeletonInstance() const
{
	return m_pSkeletonInstance;
}

TOSHI_NAMESPACE_END
