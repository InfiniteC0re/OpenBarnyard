#include "pch.h"
#include "ATreeManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "Assets/AAssetLoader.h"

#ifdef TRENDERINTERFACE_DX8
#  include "Platform/DX8/TRenderInterface_DX8.h"
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
TBOOL ATreeManager::Initialise()
{
	TASSERT( m_pTrunks == TNULL && m_pFOBs == TNULL );

	const TCHAR szFOBBoneName[] = "FOB";

	// Create tree trunks
	{
		struct ModelInfo
		{
			const TCHAR* szModelName;
			TINT         iIndex;
		} aModels[] = {
			{ "", 6 },
			{ "", 0 },
			{ "data\\models\\treelittle.trb", 1 },
			{ "data\\models\\tree_bent_trunk.trb", 2 },
			{ "data\\models\\tree_standard_trunk.trb", 3 },
			{ "data\\models\\tree_tall_trunk.trb", 4 },
			{ "data\\models\\treewall_sim_trunks.trb", 5 },
			{ "", 7 }
		};

		m_iNumTrunks = TARRAYSIZE( aModels );
		m_pTrunks    = new Model[ m_iNumTrunks ];

		for ( TINT i = 0; i < m_iNumTrunks; i++ )
		{
			const TCHAR* pchModelName = aModels[ i ].szModelName;
			Model*       pTreeModel   = &m_pTrunks[ i ];

			pTreeModel->m_uiID = aModels[ i ].iIndex;

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

// $Barnyard: FUNCTION 005ee8e0
void ATreeManager::CreateInstances( ATerrainLocatorList* a_pLocatorList )
{
	TVALIDPTR( a_pLocatorList );

	TASSERT( m_pFOBs != TNULL && m_pTrunks != TNULL );

	struct LocatorInfo
	{
		TINT         iIndex;
		const TCHAR* szName;
		TBOOL        bHasCollision;
	} aLocators[] = {
		{ 3, "tree02", TTRUE },
		{ 5, "tree03", TTRUE },
		{ 4, "tree05", TTRUE },
		{ 1, "fob2", TTRUE },
		{ 1, "fobw", TTRUE },
		{ 1, "busha", TFALSE },
		{ 6, "treewall2", TTRUE },
		{ 6, "treewall", TTRUE },
		{ 2, "treelittle", TFALSE },
		{ 3, "tree_bent", TTRUE },
		{ 4, "tree_standard", TTRUE },
		{ 5, "tree_tall", TTRUE },
		{ 6, "treewall_sim", TTRUE },
		{ 0, "fob_bens", TTRUE },
		{ 7, "fob_pond", TTRUE },
	};

	// Initialise transform that is going to be used for each of the tree
	TMatrix44 matGlobalTransform;
	matGlobalTransform.m_f11 = 1.0f;
	matGlobalTransform.m_f12 = 0.0f;
	matGlobalTransform.m_f13 = 0.0f;
	matGlobalTransform.m_f14 = 0.0f;
	matGlobalTransform.m_f21 = 0.0f;
	matGlobalTransform.m_f22 = 0.0f;
	matGlobalTransform.m_f23 = 1.0f;
	matGlobalTransform.m_f24 = 0.0f;
	matGlobalTransform.m_f31 = 0.0f;
	matGlobalTransform.m_f32 = -1.0f;
	matGlobalTransform.m_f33 = 0.0f;
	matGlobalTransform.m_f34 = 0.0f;
	matGlobalTransform.m_f41 = 0.0f;
	matGlobalTransform.m_f42 = 0.0f;
	matGlobalTransform.m_f43 = 0.0f;
	matGlobalTransform.m_f44 = 1.0f;

	TINT iNumCollisionObjects = 0;
	TINT iNumLocators         = a_pLocatorList->GetNumLocators();

	if ( iNumLocators > 0 )
	{
		TBOOL aFlags[ MAX_INSTANCES - 2 ];
		TINT  aUnk[ MAX_INSTANCES ];

		TBOOL* pFlag = aFlags;
		TINT*  pUnk  = aUnk;

		ATerrainLocatorTRBHeader* pLocatorHeader = a_pLocatorList->GetHeader();
		ATerrainLocator*          pLocators      = pLocatorHeader->pLocators;

		for ( TINT i = 0; i < iNumLocators; i++ )
		{
			const TCHAR* pchLocatorName = pLocatorHeader->ppNames[ pLocators[ i ].uiNameId ];
			TVALIDPTR( pchLocatorName );

			for ( TINT k = 0; k < TARRAYSIZE( aLocators ); k++ )
			{
				if ( !TStringManager::String8CompareNoCase(
				         aLocators[ k ].szName,
				         pchLocatorName,
				         TStringManager::String8Length( aLocators[ k ].szName )
				     ) )
				{
					// This locator is valid, so let's create a tree instance now
					if ( m_llFreeTreeInstances.IsEmpty() )
					{
						TERROR( "ATreeManager::CreateInstances - Failed to create all instances due to not having empty instance slots!\n" );
						break;
					}

					TINT  iIndex        = aLocators[ k ].iIndex;
					TBOOL bHasCollision = TFALSE;

					// TODO: figure out this hell
					if ( m_bFlag && m_pTrunks[ iIndex ].m_pManagedModel == TNULL )
					{
						*pFlag = TTRUE;
						iNumCollisionObjects += 1;
					}
					else if ( pLocators[ i ].iFlags2 < 0 )
					{
						*pFlag = TFALSE;
					}
					else
					{
						*pFlag = TTRUE;
						iNumCollisionObjects += 1;
						bHasCollision = TTRUE;
					}

					// Move cursors
					pUnk++;
					pFlag++;

					// Create the instance object
					TreeInstance* pTreeInstance = m_llFreeTreeInstances.PopFront();

					TMatrix44 matLocatorTransform;
					pLocators[ i ].GetMatrix( matLocatorTransform );

					TMatrix44 matFinalTransform;
					matFinalTransform.Multiply( matGlobalTransform, matLocatorTransform );

					// Initialise the instance object
					pTreeInstance->iTreeIndex = iIndex;

					// Get flags for this instance
					if ( iIndex == 7 )
					// force for "fob_pond"
					{
						pTreeInstance->bFlag1 = TTRUE;
						pTreeInstance->bFlag2 = TTRUE;
					}
					else
					// get flags based on the locator name
					{
						TINT iLocatorNameLength = TStringManager::String8Length( pchLocatorName );

						if ( iLocatorNameLength >= 4 && pchLocatorName[ iLocatorNameLength - 4 ] == '_' )
						{
							if ( pchLocatorName[ iLocatorNameLength - 3 ] == 'b' &&
							     pchLocatorName[ iLocatorNameLength - 2 ] == 'r' &&
							     pchLocatorName[ iLocatorNameLength - 2 ] == 'n' )
							// barn?
							{
								pTreeInstance->bFlag1 = TTRUE;
							}
							else if ( pchLocatorName[ iLocatorNameLength - 3 ] == 'd' && pchLocatorName[ iLocatorNameLength - 2 ] == 'k' && pchLocatorName[ iLocatorNameLength - 2 ] == 'g' )
							// dankweed pond and some 'g'?
							{
								pTreeInstance->bFlag2 = TTRUE;
							}
						}
					}

					pTreeInstance->uiLocatorId  = TUINT16( i );
					pTreeInstance->pLocatorList = a_pLocatorList;
					pTreeInstance->uiUnk3       = 255;
					m_llUsedTreeInstances.PushBack( pTreeInstance );

					TTODO( "Spawn FOBs, create collisions and probably something else, idk..." );

					break;
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 005ef3a0
void ATreeManager::Render()
{
	TASSERT( TFALSE );
	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	TMatrix44 matModelView = pRenderContext->GetModelViewMatrix();
	TMatrix44 matLightDir  = TRenderInterface::GetSingleton()->GetLightDirection();

	// something with field9_0x18
	// ...
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
