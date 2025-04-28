#include "pch.h"
#include "ATreeManager.h"
#include "Assets/AAssetLoader.h"

#ifdef TRENDERINTERFACE_DX8
#  include "Platform/DX8/ASkinShader/ASkinMaterial_DX8.h"
#endif // TRENDERINTERFACE_DX8

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005efdf0
ATreeManager::ATreeManager()
    : m_pTrunks( TNULL )
    , m_pFOBs( TNULL )
    , m_bFlag( TFALSE )
    , m_bCreateCollision( TTRUE )
    , m_fUnk1( 30.0f )
    , m_fUnk2( 110.0f )
    , m_fUnk3( 98.0f )
{
	// TODO: initialise other members
}

ATreeManager::~ATreeManager()
{
}

// $Barnyard: FUNCTION 005ee170
TBOOL ATreeManager::LoadModels()
{
	const TCHAR szFOBBoneName[] = "FOB";

	// Create tree trunks
	{
		const TCHAR* aTrunkModelNames[] = {
			"",
			"",
			"data\\models\\treelittle.trb",
			"data\\models\\tree_bent_trunk.trb",
			"data\\models\\tree_standard_trunk.trb",
			"data\\models\\tree_tall_trunk.trb",
			"data\\models\\treewall_sim_trunks.trb",
			""
		};

		TINT aTrunkModelIndices[] = {
			6,
			0,
			1,
			2,
			3,
			4,
			5,
			7
		};

		TSTATICASSERT( TARRAYSIZE( aTrunkModelNames ) == TARRAYSIZE( aTrunkModelIndices ) );

		m_iNumTrunks = TARRAYSIZE( aTrunkModelNames );
		m_pTrunks    = new Model[ m_iNumTrunks ];

		for ( TINT i = 0; i < m_iNumTrunks; i++ )
		{
			const TCHAR* pchModelName = aTrunkModelNames[ i ];
			Model*       pTreeModel   = &m_pTrunks[ i ];

			pTreeModel->m_uiID = aTrunkModelIndices[ i ];

			if ( pchModelName[ 0 ] == '\0' )
			// No model specified for this trunk
			{
				pTreeModel->m_iNumFOBs     = 1;
				pTreeModel->m_pFOBMatrices = new TMatrix44[ 1 ];
				pTreeModel->m_pFOBMatrices[ 0 ].Identity();
				pTreeModel->m_fUnk1   = 0.0f;
				pTreeModel->m_fRadius = 1.6f;

				if ( i != 1 ) // ?
					pTreeModel->m_fRadius = 4.4f;
			}
			else
			// This is definetely a tree trunk and it HAS a model
			{
				// Load the model
				pTreeModel->Load( pchModelName );

				// Do some other setups
				TModel* pModel = pTreeModel->m_pManagedModel->GetModel();
				TASSERT( pModel->GetNumLODs() >= 1 );

				pTreeModel->m_fUnk1   = -pModel->GetLOD( 0 ).BoundingSphere.GetOrigin().z;
				pTreeModel->m_fRadius = pModel->GetLOD( 0 ).BoundingSphere.GetRadius() * 1.5f;

				TSkeletonInstance* pSkeletonInstance = pTreeModel->m_pSceneObject->GetInstance()->GetSkeletonInstance();
				TSkeleton*         pSkeleton         = pSkeletonInstance->GetSkeleton();

				// Count number of FOBs (foliage objects)
				pTreeModel->m_iNumFOBs = 0;
				for ( TINT i = 0; i < pSkeleton->GetBoneCount(); i++ )
				{
					if ( !TStringManager::String8CompareNoCase( pSkeleton->GetBone( i )->GetName(), szFOBBoneName, TARRAYSIZE( szFOBBoneName ) - 1 ) )
					// Name of the bone starts with FOB
					{
						pTreeModel->m_iNumFOBs += 1;
					}
				}

				// Create matrices for FOBs
				TINT iCurrentFOB           = 0;
				pTreeModel->m_pFOBMatrices = new TMatrix44[ pTreeModel->m_iNumFOBs ];
				for ( TINT i = 0; i < pSkeleton->GetBoneCount(); i++ )
				{
					if ( !TStringManager::String8CompareNoCase( pSkeleton->GetBone( i )->GetName(), szFOBBoneName, TARRAYSIZE( szFOBBoneName ) - 1 ) )
					// Name of the bone starts with FOB
					{
						pTreeModel->m_pFOBMatrices[ iCurrentFOB++ ] = pSkeletonInstance->GetBoneTransformCurrent( i );
					}
				}
			}
		}
	}

	// Create foliage objects
	{
		const TCHAR* aFOBModelNames[] = {
			"data\\models\\busha.trb",
			"data\\models\\busha_LOD1.trb",
			"data\\models\\foblittle.trb",
			TNULL,
			"data\\models\\tree_bent_fob.trb",
			TNULL,
			"data\\models\\tree_standard_fob.trb",
			TNULL,
			"data\\models\\tree_tall_fob.trb",
			TNULL,
			"data\\models\\treewall_sim_fobs.trb",
			TNULL,
			"data\\models\\fob_bens.trb",
			TNULL,
			"data\\models\\Fob_pond.trb",
			TNULL
		};

		m_iNumFOBs = TARRAYSIZE( aFOBModelNames );
		m_pFOBs    = new Model[ m_iNumFOBs ];

		for ( TINT i = 0; i < m_iNumFOBs; i++ )
		{
			if ( const TCHAR* pchModelName = aFOBModelNames[ i ] )
			{
				// Load the model
				m_pFOBs[ i ].Load( pchModelName );
			}
		}
	}

	// Add all tree instances to the list of free instances
	for ( TINT i = 0; i < MAX_INSTANCES; i++ )
	{
		m_llFreeTreeInstances.PushFront( &m_aTreeInstances[ i ] );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005edd80
ATreeManager::Model::Model()
{
	m_pManagedModel = TNULL;
	m_pSceneObject  = TNULL;
	m_uiID          = 0;
	m_iNumFOBs      = 0;
	m_pFOBMatrices  = TNULL;
}

// $Barnyard: FUNCTION 005edfe0
ATreeManager::Model::~Model()
{
	if ( m_pSceneObject )
		m_pSceneObject->Delete();

	if ( m_pManagedModel )
		delete m_pManagedModel;

	TVALIDPTR( m_pFOBMatrices );
	delete[] m_pFOBMatrices;
}

void ATreeManager::Model::Load( const TCHAR* a_szModelName )
{
	m_pManagedModel = new TManagedModel();
	m_pManagedModel->Create( a_szModelName, AAssetLoader::GetAssetTRB( AAssetType_Startup ) );

	// Create scene object
	m_pSceneObject = m_pManagedModel->CreateSceneObject();
	m_pSceneObject->GetTransform().SetMatrix( TMatrix44::IDENTITY );
	m_pSceneObject->GetTransform().SetEuler( TVector3( TMath::PI / 2, 0.0f, 0.0f ) );
	m_pSceneObject->GetTransform().SetTranslate( TVector3::VEC_ZERO );

	// ?????????
	TMatrix44 matTransform;
	m_pSceneObject->GetTransform().GetLocalMatrixImp( matTransform );
	m_pSceneObject->GetTransform().SetMatrix( matTransform );
	m_pSceneObject->DisableSkeletonUpdate();

	TModel* pModel = m_pManagedModel->GetModel();
	for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
	{
		TModelLOD* pLOD = &pModel->GetLOD( i );

		for ( TINT k = 0; k < pLOD->iNumMeshes; k++ )
		{
			TMesh* pMesh = pLOD->ppMeshes[ k ];

			// Use HD lighting on skinned materials
			if ( ASkinMaterialHAL* pSkinMat = TDYNAMICCAST( ASkinMaterialHAL, pMesh->GetMaterial() ) )
				pSkinMat->SetHDLighting( TTRUE );
		}
	}
}
