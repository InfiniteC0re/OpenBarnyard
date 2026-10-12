#include "pch.h"
#include "AParticleSystem.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Render/ASysShader/ASysShader.h"
#include "Render/ASysShader/ASysMaterial.h"

#include <Render/TRenderInterface.h>
#include <Render/TRenderContext.h>
#include <Render/TTexture.h>
#include <Render/TModelManager.h>
#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00677d00
AParticleSystemConfig::AParticleSystemConfig()
{
	iNumFloats    = 16000;
	iNumVectors   = 16000;
	iNumVectors2  = 4096;
	iNumBytes     = 16000;
	iNumDwords    = 12096;
	iNumInstances = 128;
}

// $Barnyard: FUNCTION 00678060
AParticleSystem::MeshNode::~MeshNode()
{
	if ( pMesh )
		pMesh->DestroyResource();
}

// $Barnyard: FUNCTION 00678170
AParticleSystem::EmitterNode::~EmitterNode()
{
	// NOTE: the original destroys every attribute pool when any emitter node is destroyed
	AParticleAttributePool<TFLOAT, AParticleAttributePoolType_Float>::Destroy();
	AParticleAttributePool<TVector4, AParticleAttributePoolType_Vector>::Destroy();
	AParticleAttributePool<TUINT8, AParticleAttributePoolType_Byte>::Destroy();
	AParticleAttributePool<TUINT32, AParticleAttributePoolType_Dword>::Destroy();
	AParticleAttributePool<TVector4, AParticleAttributePoolType_Vector2>::Destroy();
}

// $Barnyard: FUNCTION 006797d0
AParticleSystem::AParticleSystem( const AParticleSystemConfig& a_rcConfig )
{
	m_pEmitterNodes  = TNULL;
	m_pMeshLocks     = TNULL;
	m_piNumParticles = TNULL;
	m_piNumVertices  = TNULL;
	m_pModelData     = TNULL;
	m_pUnk           = TNULL;

	ms_pSingleton = this;

	m_fnOnMaterialLibraryLoaded   = TNULL;
	m_fnOnMaterialLibraryUnloaded = TNULL;
	m_pEffectPool                 = TNULL;
	m_pMaterials                  = TNULL;
	m_pModels                     = TNULL;
	m_eDetail                     = DETAIL_NONE;
	m_fLODStart                   = 0.7f;
	m_uiMaxVertices               = 2000;
	m_pMaterialLibrary            = TNULL;

	m_aFrustumPlanes[ 0 ] = TPlane( TVector3::VEC_POSX, 1.0f );
	m_aFrustumPlanes[ 1 ] = TPlane( TVector3::VEC_NEGX, 1.0f );
	m_aFrustumPlanes[ 2 ] = TPlane( TVector3::VEC_POSY, 1.0f );
	m_aFrustumPlanes[ 3 ] = TPlane( TVector3::VEC_NEGY, 1.0f );
	m_aFrustumPlanes[ 4 ] = TPlane( TVector3::VEC_POSZ, 1.0f );
	m_aFrustumPlanes[ 5 ] = TPlane( TVector3::VEC_NEGZ, 1.0f );

	m_pTestEmitters                       = TNULL;
	m_aNumEmitters[ EMITTERTYPE_TEST ]    = 0;
	m_pGenericEmitters                    = TNULL;
	m_aNumEmitters[ EMITTERTYPE_GENERIC ] = 0;

	ms_pRenderSystem = this;

	CreatePools( a_rcConfig );
}

// $Barnyard: FUNCTION 006799e0
AParticleSystem::~AParticleSystem()
{
	KillAll();

	ms_pSingleton = TNULL;

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	DestroyEmitters();
	UnloadLibrary();

	if ( m_pEffectPool )
		delete m_pEffectPool;
}

// $Barnyard: FUNCTION 00679260
void AParticleSystem::CreatePools( const AParticleSystemConfig& a_rcConfig )
{
	AParticleAttributePool<TFLOAT, AParticleAttributePoolType_Float>::Create( a_rcConfig.iNumFloats / 32 );
	AParticleAttributePool<TVector4, AParticleAttributePoolType_Vector>::Create( a_rcConfig.iNumVectors / 32 );
	AParticleAttributePool<TUINT8, AParticleAttributePoolType_Byte>::Create( a_rcConfig.iNumBytes / 32 );
	AParticleAttributePool<TUINT32, AParticleAttributePoolType_Dword>::Create( a_rcConfig.iNumDwords / 32 );
	AParticleAttributePool<TVector4, AParticleAttributePoolType_Vector2>::Create( a_rcConfig.iNumVectors2 / 32 );

	if ( m_eDetail != DETAIL_HIGH )
	{
		m_eDetail = DETAIL_HIGH;
		DestroyEmitters();

		m_fLODStart                           = 0.7f;
		m_uiMaxVertices                       = 10000;
		m_iNumMeshes                          = 50;
		m_aNumEmitters[ EMITTERTYPE_TEST ]    = 200;
		m_aNumEmitters[ EMITTERTYPE_GENERIC ] = 50;

		CreateEmitters();
	}

	CreateEmitters();
	m_pEffectPool = new AParticleEffectPool( a_rcConfig.iNumInstances );
}

// $Barnyard: FUNCTION 00678360
void AParticleSystem::CreateEmitters()
{
	ASysShader* pShader = ASysShader::GetSingleton();

	// Create or destroy the meshes to have m_iNumMeshes of them
	if ( m_llFreeMeshes.Count() <= m_iNumMeshes )
	{
		for ( TINT i = m_llFreeMeshes.Count(); i < m_iNumMeshes; i++ )
		{
			MeshNode* pNode = new MeshNode();
			pNode->pMesh    = pShader->CreateMesh( TNULL );

			if ( !pNode->pMesh || pNode->pMesh->CreatePools( ASysMesh::FLAGS_DEFAULT, 512, 768 ) != TTRUE )
			{
				if ( pNode->pMesh )
				{
					pNode->pMesh->DestroyResource();
					pNode->pMesh = TNULL;
				}

				delete pNode;
				break;
			}

			m_llFreeMeshes.InsertHead( pNode );
		}
	}
	else
	{
		do
		{
			if ( !m_llFreeMeshes.IsEmpty() )
				m_llFreeMeshes.Delete( m_llFreeMeshes.Head() );

		} while ( m_llFreeMeshes.Count() > m_iNumMeshes );
	}

	if ( !m_pEmitterNodes )
	{
		m_pEmitterNodes = new EmitterNode[ m_aNumEmitters[ EMITTERTYPE_TEST ] + m_aNumEmitters[ EMITTERTYPE_GENERIC ] ];
		TINT iNode      = 0;

		for ( TINT eType = 0; eType < EMITTERTYPE_NUMOF; eType++ )
		{
			if ( m_aNumEmitters[ eType ] > 0 )
			{
				if ( eType == EMITTERTYPE_TEST )
					m_pTestEmitters = new AParticleTestEmitter[ m_aNumEmitters[ eType ] ];
				else if ( eType == EMITTERTYPE_GENERIC )
					m_pGenericEmitters = new AParticleGenericEmitter[ m_aNumEmitters[ eType ] ];

				for ( TINT i = 0; i < m_aNumEmitters[ eType ]; i++ )
				{
					EmitterNode* pNode = &m_pEmitterNodes[ iNode++ ];
					pNode->eType       = eType;

					if ( eType == EMITTERTYPE_TEST )
						pNode->pEmitter = &m_pTestEmitters[ i ];
					else if ( eType == EMITTERTYPE_GENERIC )
						pNode->pEmitter = &m_pGenericEmitters[ i ];

					pNode->pEmitter->Release();
					m_aFreeEmitters[ eType ].InsertHead( pNode );
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 006786d0
void AParticleSystem::DestroyEmitters()
{
	// Put the active emitters back to the free lists
	for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); )
	{
		EmitterNode* pNode = it;
		it                 = pNode->Next();

		m_llActiveEmitters.Remove( pNode );
		pNode->pEmitter->Release();
		m_aFreeEmitters[ pNode->eType ].InsertHead( pNode );
	}

	if ( !m_pEmitterNodes )
		return;

	for ( TINT eType = 0; eType < EMITTERTYPE_NUMOF; eType++ )
	{
		m_aFreeEmitters[ eType ].RemoveAll();

		if ( eType == EMITTERTYPE_TEST )
		{
			if ( m_pTestEmitters )
				delete[] m_pTestEmitters;

			m_pTestEmitters = TNULL;
		}
		else if ( eType == EMITTERTYPE_GENERIC )
		{
			if ( m_pGenericEmitters )
				delete[] m_pGenericEmitters;

			m_pGenericEmitters = TNULL;
		}
	}

	if ( m_pEmitterNodes )
		delete[] m_pEmitterNodes;

	m_pEmitterNodes = TNULL;
}

// $Barnyard: FUNCTION 00679020
void AParticleSystem::LoadLibrary( const TCHAR* a_szName )
{
	TCHAR szFileName[ 256 ];

	KillAll();
	UnloadLibrary();

	// Load the textures
	TStringManager::String8Format( szFileName, sizeof( szFileName ), "%s.ttl", a_szName );
	m_pMaterialLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset( szFileName, AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) );

	if ( m_fnOnMaterialLibraryLoaded )
		m_fnOnMaterialLibraryLoaded( m_pMaterialLibrary );

	// Load the effects
	TStringManager::String8Format( szFileName, sizeof( szFileName ), "%s.trb", a_szName );
	Load( szFileName );

	const TUINT8* pHeader = TSTATICCAST( TUINT8, GetSymbolAddress( "Header" ) );

	if ( pHeader[ 0 ] == 1 && pHeader[ 1 ] == 0 && pHeader[ 2 ] == 0 && pHeader[ 3 ] == 3 )
	{
		m_pMaterials = CastSymbol<AParticleMaterialLibrary>( "MaterialManager" );
		m_pMaterials->CreateMaterials();

		m_pModels = CastSymbol<AParticleModelLibrary>( "ModelManager" );

		// The models are in the directory of the library
		TINT iLastSlash = -1;

		for ( TINT i = 0; a_szName[ i ] != '\0'; i++ )
		{
			if ( a_szName[ i ] == '/' || a_szName[ i ] == '\\' )
				iLastSlash = i;
		}

		if ( a_szName[ 0 ] != '\0' && iLastSlash >= 0 )
			TStringManager::String8Copy( szFileName, a_szName, iLastSlash + 1 );

		szFileName[ iLastSlash + 1 ] = '\0';

		m_pModels->Load( szFileName );
		m_pEffectPool->Load( this );

		TINT iNumMaterials = m_pMaterials->iNumMaterials;
		TINT iNumModels    = m_pModels->iNumModels;

		m_piNumParticles = TSTATICCAST( TINT, TMalloc( iNumMaterials * sizeof( TINT ) ) );
		m_piNumVertices  = TSTATICCAST( TINT, TMalloc( iNumMaterials * sizeof( TINT ) ) );

		if ( iNumModels > 0 )
			m_pModelData = new TUINT32[ iNumModels ];

		m_pMeshLocks = TSTATICCAST( AParticleMeshLock, TMalloc( iNumMaterials * sizeof( AParticleMeshLock ) ) );

		for ( TINT i = 0; i < iNumMaterials; i++ )
			new ( &m_pMeshLocks[ i ] ) AParticleMeshLock();

		m_pUnk = TMalloc( 8 );
	}
}

// $Barnyard: FUNCTION 00678070
void AParticleSystem::UnloadLibrary()
{
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	if ( m_pMaterialLibrary )
	{
		if ( m_fnOnMaterialLibraryUnloaded )
			m_fnOnMaterialLibraryUnloaded( m_pMaterialLibrary );

		AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pMaterialLibrary );
		m_pMaterialLibrary = TNULL;
	}

	TFree( m_piNumParticles );
	TFree( m_piNumVertices );
	TFree( m_pModelData );
	TFree( m_pMeshLocks );
	TFree( m_pUnk );

	m_piNumParticles = TNULL;
	m_piNumVertices  = TNULL;
	m_pModelData     = TNULL;
	m_pMeshLocks     = TNULL;
	m_pUnk           = TNULL;

	m_pEffectPool->Unload();

	if ( m_pMaterials )
		m_pMaterials->DestroyMaterials();

	m_pMaterials = TNULL;

	if ( m_pModels )
		m_pModels->Unload();

	m_pModels = TNULL;

	Close();
}

// $Barnyard: FUNCTION 006789c0
void AParticleSystem::Update( TFLOAT a_fDeltaTime )
{
	if ( m_pMaterials )
	{
		for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); )
		{
			EmitterNode* pNode = it;
			it                 = pNode->Next();

			if ( !pNode->pEmitter->Update( a_fDeltaTime ) )
			{
				m_llActiveEmitters.Remove( pNode );
				pNode->pEmitter->Release();
				m_aFreeEmitters[ pNode->eType ].InsertHead( pNode );
			}
		}

		m_pEffectPool->Update( a_fDeltaTime );
		m_bUpdated = TTRUE;
	}
}

// $Barnyard: FUNCTION 00679380
void AParticleSystem::Render()
{
	if ( !m_pMaterials )
		return;

	UpdateQuadCorners();

	// The emitters are culled with the planes facing out of the frustum
	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();
	m_iNumFrustumPlanes            = 6;
	pRenderContext->GetWorldPlanes( m_aFrustumPlanes, m_iNumFrustumPlanes );

	for ( TINT i = 0; i < m_iNumFrustumPlanes; i++ )
	{
		m_aFrustumPlanes[ i ].AsVector4().x = -m_aFrustumPlanes[ i ].AsVector4().x;
		m_aFrustumPlanes[ i ].AsVector4().y = -m_aFrustumPlanes[ i ].AsVector4().y;
		m_aFrustumPlanes[ i ].AsVector4().z = -m_aFrustumPlanes[ i ].AsVector4().z;
		m_aFrustumPlanes[ i ].AsVector4().w = -m_aFrustumPlanes[ i ].AsVector4().w;
	}

	TVector4 vecCameraPos = TRenderInterface::GetSingleton()->GetCurrentContext()->GetViewWorldMatrix().GetTranslation();
	TFLOAT   fInvLODStart = 1.0f / m_fLODStart;

	TINT  iNumMaterials = m_pMaterials->iNumMaterials;
	TUINT uiNumVertices = 0;

	for ( TINT i = 0; i < iNumMaterials; i++ )
	{
		m_piNumParticles[ i ] = 0;
		m_piNumVertices[ i ]  = 0;
	}

	// Choose number of particles of every emitter depending on the distance
	for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); it++ )
	{
		AParticleEmitter* pEmitter = it->pEmitter;

		if ( !pEmitter->IsVisible() )
		{
			pEmitter->SetNumParticles( 0 );
			continue;
		}

		TFLOAT fLOD = 1.0f - TVector4::DistanceSq( vecCameraPos, pEmitter->m_vecBoundingSphere ) * pEmitter->m_fLODScale;
		TINT   iNumParticles;

		if ( fLOD >= 0.0f )
		{
			TINT   iRange = pEmitter->GetMaxParticles() - pEmitter->m_iMinParticles;
			TFLOAT fScale;

			// Full amount of particles until m_fLODStart, then fading out to the minimum
			if ( fLOD < m_fLODStart )
				fScale = fLOD * fInvLODStart;
			else
				fScale = 1.0f;

			iNumParticles = TINT( iRange * fScale ) + pEmitter->m_iMinParticles;
		}
		else
		{
			iNumParticles = 0;
		}

		pEmitter->SetNumParticles( iNumParticles );

		if ( it->pEmitter->GetMaterial() >= 0 )
		{
			uiNumVertices += pEmitter->GetNumVertices( iNumParticles );
			m_piNumVertices[ pEmitter->GetMaterial() ] += pEmitter->GetNumVertices( iNumParticles );
		}
	}

	// Reduce the number of particles if there are too many vertices
	if ( uiNumVertices > m_uiMaxVertices )
	{
		for ( TINT i = 0; i < iNumMaterials; i++ )
			m_piNumParticles[ i ] = 0;

		TFLOAT fScale = TFLOAT( TINT( m_uiMaxVertices ) ) / TINT( uiNumVertices );

		for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); it++ )
		{
			AParticleEmitter* pEmitter = it->pEmitter;

			if ( pEmitter->GetMaterial() >= 0 )
			{
				pEmitter->SetNumParticles( TINT( pEmitter->GetNumParticles() * fScale ) );
				m_piNumParticles[ it->pEmitter->GetMaterial() ] += it->pEmitter->GetNumParticles();
				m_piNumVertices[ pEmitter->GetMaterial() ] += pEmitter->GetNumVertices( it->pEmitter->GetNumParticles() );
			}
		}
	}

	// Render the emitters grouped by the material
	for ( TINT iMaterial = iNumMaterials - 1; iMaterial >= 0; iMaterial-- )
	{
		for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); it++ )
		{
			if ( iMaterial == it->pEmitter->GetMaterial() )
				it->pEmitter->Render();
		}

		if ( m_pMeshLocks[ iMaterial ].pMesh )
			m_pMeshLocks[ iMaterial ].Flush();
	}

	for ( TINT iMaterial = iNumMaterials - 1; iMaterial >= 0; iMaterial-- )
	{
		if ( m_pMeshLocks[ iMaterial ].pMesh )
			m_pMeshLocks[ iMaterial ].Flush();
	}

	// The emitters without material (models)
	for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); it++ )
	{
		if ( it->pEmitter->GetMaterial() < 0 )
			it->pEmitter->Render();
	}

	m_llFreeMeshes.InsertListTail( m_llUsedMeshes );
	m_bUpdated = TFALSE;
}

// $Barnyard: FUNCTION 006782e0
AParticleEmitter* AParticleSystem::CreateEmitter( EMITTERTYPE a_eType )
{
	if ( m_aFreeEmitters[ a_eType ].Count() <= 0 )
		return TNULL;

	EmitterNode* pNode = m_aFreeEmitters[ a_eType ].IsEmpty() ? TNULL : m_aFreeEmitters[ a_eType ].Remove( m_aFreeEmitters[ a_eType ].Head() );
	m_llActiveEmitters.InsertTail( pNode );

	return pNode->pEmitter;
}

// $Barnyard: FUNCTION 00678bd0
void AParticleSystem::ReleaseEmitter( AParticleEmitter* a_pEmitter )
{
	if ( !a_pEmitter )
		return;

	for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); it++ )
	{
		EmitterNode* pNode = it;

		if ( pNode->pEmitter == a_pEmitter )
		{
			m_llActiveEmitters.Remove( pNode );
			pNode->pEmitter->Release();
			m_aFreeEmitters[ pNode->eType ].InsertHead( pNode );
			return;
		}
	}
}

// $Barnyard: FUNCTION 00678a90
void AParticleSystem::ReleaseEmittersOfSettings( AParticleEmitterSettings* a_pSettings )
{
	for ( auto it = m_llActiveEmitters.Begin(); it != m_llActiveEmitters.End(); )
	{
		EmitterNode* pNode = it;
		it                 = pNode->Next();

		if ( pNode->eType == EMITTERTYPE_GENERIC && TSTATICCAST( AParticleGenericEmitter, pNode->pEmitter )->GetSettings() == a_pSettings )
		{
			m_llActiveEmitters.Remove( pNode );
			pNode->pEmitter->Release();
			m_aFreeEmitters[ pNode->eType ].InsertHead( pNode );
		}
	}
}

// $Barnyard: FUNCTION 00678b40
void AParticleSystem::KillAll()
{
	m_pEffectPool->KillAll();

	while ( !m_llActiveEmitters.IsEmpty() )
	{
		EmitterNode* pNode = m_llActiveEmitters.IsEmpty() ? TNULL : m_llActiveEmitters.Remove( m_llActiveEmitters.Head() );

		pNode->pEmitter->Release();
		m_aFreeEmitters[ pNode->eType ].InsertHead( pNode );
	}
}

// $Barnyard: FUNCTION 00678840
AParticleMeshLock* AParticleSystem::GetMeshLock( TUINT a_uiMaxVertices, TUINT a_uiNumVertices, TINT a_iMaterial )
{
	if ( a_iMaterial < 0 )
		return TNULL;

	ASysMaterial*      pMaterial = m_pMaterials->pMaterials[ a_iMaterial ].pMaterial;
	AParticleMeshLock* pLock     = &m_pMeshLocks[ a_iMaterial ];

	if ( pLock->pMesh )
	{
		if ( TUINT( pLock->pMesh->GetMaxVertices() - pLock->uiNumVertices ) >= a_uiNumVertices )
			return pLock;

		pLock->Flush();
	}

	if ( !m_pMeshLocks[ a_iMaterial ].pMesh && m_llFreeMeshes.Count() > 0 )
	{
		// Look for a mesh that can store every vertex
		for ( auto it = m_llFreeMeshes.Begin(); it != m_llFreeMeshes.End(); it++ )
		{
			if ( it->pMesh->GetMaxVertices() >= a_uiMaxVertices )
			{
				MeshNode* pNode = it;

				m_llFreeMeshes.Remove( pNode );
				m_llUsedMeshes.InsertTail( pNode );

				m_pMeshLocks[ a_iMaterial ].Begin( pNode->pMesh, pMaterial );
				return &m_pMeshLocks[ a_iMaterial ];
			}
		}

		// Take any mesh
		MeshNode* pNode = m_llFreeMeshes.IsEmpty() ? TNULL : m_llFreeMeshes.Remove( m_llFreeMeshes.Tail() );
		m_llUsedMeshes.InsertTail( pNode );

		m_pMeshLocks[ a_iMaterial ].Begin( pNode->pMesh, pMaterial );
		return &m_pMeshLocks[ a_iMaterial ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00677a90
TBOOL AParticleSystem::IsSphereVisible( const TVector4& a_rcCentre, TFLOAT a_fRadius )
{
	TUINT         uiNumPlanes = ms_pRenderSystem->m_iNumFrustumPlanes;
	const TPlane* pPlanes     = ms_pRenderSystem->m_aFrustumPlanes;

	if ( !pPlanes )
		return TFALSE;

	for ( TUINT i = 0; i < uiNumPlanes; i++ )
	{
		TFLOAT fDistance = TVector4::DotProduct3( pPlanes[ i ].AsVector4(), a_rcCentre ) - pPlanes[ i ].AsVector4().w;

		if ( fDistance < -a_fRadius )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00677fb0
const TMatrix44& AParticleSystem::GetModelView()
{
	return TRenderInterface::GetSingleton()->GetCurrentContext()->GetModelViewMatrix();
}

// $Barnyard: FUNCTION 00677fc0
void AParticleSystem::SetModelView( const TMatrix44& a_rcModelView )
{
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();

	// Render what's been rendered with the current matrix
	for ( TINT i = 0; i < m_pMaterials->iNumMaterials; i++ )
	{
		if ( m_pMeshLocks[ i ].pMesh )
			m_pMeshLocks[ i ].Flush();
	}

	pRenderInterface->GetCurrentContext()->SetModelViewMatrix( a_rcModelView );
	UpdateQuadCorners();
}

// $Barnyard: FUNCTION 00677d30
void AParticleSystem::UpdateQuadCorners()
{
	const TMatrix44& rcViewModel = TRenderInterface::GetSingleton()->GetCurrentContext()->GetViewModelMatrix();

	m_vecRight.Multiply( rcViewModel.AsBasisVector4( 0 ), 0.5f );
	m_vecUp.Multiply( rcViewModel.AsBasisVector4( 1 ), 0.5f );
	m_vecForward.Multiply( rcViewModel.AsBasisVector4( 2 ), 0.5f );

	m_aQuadCorners[ 2 ]   = m_vecUp - m_vecRight;
	m_aQuadCorners[ 2 ].w = m_vecUp.w;

	m_aQuadCorners[ 3 ]   = m_vecRight + m_vecUp;
	m_aQuadCorners[ 3 ].w = m_vecUp.w;

	m_aQuadCorners[ 0 ]   = TVector4::VEC_ZERO - m_vecUp - m_vecRight;
	m_aQuadCorners[ 0 ].w = TVector4::VEC_ZERO.w;

	m_aQuadCorners[ 1 ]   = m_vecRight - m_vecUp;
	m_aQuadCorners[ 1 ].w = m_vecRight.w;
}

//-----------------------------------------------------------------------------
// AParticleMeshLock
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00678020
AParticleMeshLock::AParticleMeshLock()
{
	pMesh         = TNULL;
	uiNumVertices = 0;
	uiNumIndices  = 0;
	TUtil::MemClear( &oLockBuffer, sizeof( oLockBuffer ) );
	pIndices  = TNULL;
	pVertices = TNULL;
}

// $Barnyard: FUNCTION 00677660
void AParticleMeshLock::Begin( ASysMesh* a_pMesh, ASysMaterial* a_pMaterial )
{
	pMesh = a_pMesh;
	pMesh->Lock( oLockBuffer );

	uiNumVertices = 0;
	uiNumIndices  = 0;
	pIndices      = oLockBuffer.IndexLock.pBuffer;
	pVertices     = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );

	pMesh->SetMaterial( a_pMaterial );
}

// $Barnyard: FUNCTION 006776a0
void AParticleMeshLock::Advance( TUINT a_uiNumVertices, TUINT a_uiNumIndices )
{
	uiNumIndices += a_uiNumIndices;
	uiNumVertices += a_uiNumVertices;
	pIndices += a_uiNumIndices;
	pVertices += a_uiNumVertices;

	if ( uiNumVertices >= pMesh->GetMaxVertices() || uiNumIndices >= pMesh->GetMaxIndices() )
	{
		pMesh->Unlock( uiNumVertices, uiNumIndices );
		pMesh->Validate();
		pMesh->Render();

		pMesh         = TNULL;
		uiNumVertices = 0;
		uiNumIndices  = 0;
		pIndices      = TNULL;
		pVertices     = TNULL;
	}
}

// $Barnyard: FUNCTION 00677720
void AParticleMeshLock::Flush()
{
	if ( pMesh )
	{
		pMesh->Unlock( uiNumVertices, uiNumIndices );
		pMesh->Validate();
		pMesh->Render();

		pMesh         = TNULL;
		uiNumVertices = 0;
		uiNumIndices  = 0;
		pIndices      = TNULL;
		pVertices     = TNULL;
	}
}

//-----------------------------------------------------------------------------
// Materials and models
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00677480
void AParticleMaterial::Create()
{
	ATexture* pTexture = AMaterialLibraryManager::List::GetSingleton()->FindTexture( szTextureName, TNULL, TNULL );

	if ( pTexture )
	{
		TTextureFactory* pTextureFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );
		this->pTexture                   = pTextureFactory->CreateFromT2Texture( pTexture->pTexture );
	}

	if ( !this->pTexture )
		this->pTexture = TRenderInterface::GetSingleton()->GetInvalidTexture();

	pMaterial = ASysShader::GetSingleton()->CreateMaterial( szName );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: Retail never marks the material as created but Destroy() later destroys it, which
	// fires the TMaterial::OnDestroy assert. Nothing else checks the flag.
	pMaterial->Create();
#endif

	pMaterial->SetTexture( this->pTexture );
	pMaterial->SetTextureNum( 1 );

	switch ( eBlendMode )
	{
		case BLENDMODE_NORMAL:
			pMaterial->SetBlendMode( ASysMaterial::BLENDMODE_1 );
			break;
		case BLENDMODE_ADDITIVE:
			pMaterial->SetBlendMode( ASysMaterial::BLENDMODE_3 );
			break;
		case BLENDMODE_SUBTRACTIVE:
			pMaterial->SetBlendMode( ASysMaterial::BLENDMODE_4 );
			break;
		case BLENDMODE_MULTIPLY:
			pMaterial->SetBlendMode( ASysMaterial::BLENDMODE_10 );
			break;
	}

	pMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
}

// $Barnyard: FUNCTION 00677560
void AParticleMaterial::Destroy()
{
	if ( pMaterial )
	{
		TTexture* pMaterialTexture = pMaterial->GetSysTexture();

		if ( pMaterialTexture && pMaterialTexture != TRenderInterface::GetSingleton()->GetInvalidTexture() )
			pMaterialTexture->DestroyResource();

		pMaterial->Destroy();
	}
}

// $Barnyard: FUNCTION 006775a0
void AParticleMaterialLibrary::CreateMaterials()
{
	for ( TINT i = 0; i < iNumMaterials; i++ )
		pMaterials[ i ].Create();
}

// $Barnyard: FUNCTION 006775d0
void AParticleMaterialLibrary::DestroyMaterials()
{
	for ( TINT i = 0; i < iNumMaterials; i++ )
		pMaterials[ i ].Destroy();

	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();
}

// $Barnyard: FUNCTION 00677610
TINT AParticleMaterialLibrary::FindMaterial( const TCHAR* a_szName ) const
{
	if ( !a_szName )
		return 0;

	for ( TINT i = 0; i < iNumMaterials; i++ )
	{
		if ( TStringManager::String8CompareNoCase( a_szName, pMaterials[ i ].szName ) == 0 )
			return i;
	}

	return 0;
}

// $Barnyard: FUNCTION 00677760
void AParticleModel::Load( const TCHAR* a_szDirectory )
{
	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, sizeof( szFileName ), "%s%s", a_szDirectory, szName );

	pManagedModel = new TManagedModel();
	pManagedModel->Create( szFileName, AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) );
	pSceneObject = pManagedModel->CreateSceneObject();
}

// $Barnyard: FUNCTION 00677830
void AParticleModelLibrary::Load( const TCHAR* a_szDirectory )
{
	pKeylib = TNULL;

	if ( iNumModels > 0 )
	{
		TCHAR szFileName[ 256 ];
		TStringManager::String8Format( szFileName, sizeof( szFileName ), "%s%s", a_szDirectory, szKeylib );

		pKeylib = TRenderInterface::GetSingleton()->GetKeyframeLibraryManager().LoadLibrary( szFileName );
	}

	for ( TINT i = 0; i < iNumModels; i++ )
		pModels[ i ].Load( a_szDirectory );
}

// $Barnyard: FUNCTION 00677920
void AParticleModelLibrary::Unload()
{
	for ( TINT i = 0; i < iNumModels; i++ )
	{
		if ( pModels[ i ].pSceneObject )
			delete pModels[ i ].pSceneObject;

		pModels[ i ].pSceneObject = TNULL;

		if ( pModels[ i ].pManagedModel )
			delete pModels[ i ].pManagedModel;

		pModels[ i ].pManagedModel = TNULL;
	}

	if ( pKeylib )
	{
		TRenderInterface::GetSingleton()->GetKeyframeLibraryManager().UnloadLibrary( pKeylib );
		pKeylib = TNULL;
	}
}

// $Barnyard: FUNCTION 006778d0
TINT AParticleModelLibrary::FindModel( const TCHAR* a_szName ) const
{
	if ( !a_szName )
		return -1;

	for ( TINT i = 0; i < iNumModels; i++ )
	{
		if ( TStringManager::String8CompareNoCase( pModels[ i ].szName, a_szName ) == 0 )
			return i;
	}

	return -1;
}
