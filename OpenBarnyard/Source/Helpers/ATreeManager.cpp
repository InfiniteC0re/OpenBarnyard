#include "pch.h"
#include "ATreeManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "Render/AGlowViewport.h"
#include "Assets/AAssetLoader.h"
#include "Helpers/ARegrowthManager.h"
#include "Physics/ACollisionModelSet.h"
#include "Physics/ACollisionModelInstance.h"

// NOTE: Opcode defines PI
#undef PI

#include "Render/ABackend.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005efdf0
ATreeManager::ATreeManager()
    : m_ppKeyLibs( TNULL )
    , m_pTrunks( TNULL )
    , m_pFOBs( TNULL )
    , m_pRenderData( TNULL )
    , m_pTrunkCollisionObjects( TNULL )
    , m_fLODSwitchDistance( 30.0f )
    , m_fRenderDistance( 110.0f )
    , m_fBlendingDistance( 98.0f )
    , m_bCreateCollision( TTRUE )
    , m_bCollisionEnabled( TFALSE )
    , m_iNumCollisionSets( 0 )
{
	m_apUnk6670[ 0 ] = TNULL;
	m_apUnk6670[ 1 ] = TNULL;
}

// $Barnyard: FUNCTION 005eff00
ATreeManager::~ATreeManager()
{
	m_llUsedTreeInstances.Clear();
	Destroy();

	// NOTE: the lists at +0x6678/+0x6688, the locked meshes and the instance lists are destroyed by their destructors
}

// $Barnyard: FUNCTION 005ee7f0
void ATreeManager::Destroy()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apUnk6670 ); i++ )
	{
		if ( m_apUnk6670[ i ] )
		{
			m_apUnk6670[ i ]->Destroy();
			delete m_apUnk6670[ i ];
			m_apUnk6670[ i ] = TNULL;
		}
	}

	for ( TINT i = 0; i < m_iNumKeyLibs; i++ )
		TRenderInterface::GetSingleton()->GetKeyframeLibraryManager().UnloadLibrary( m_ppKeyLibs[ i ] );

	TFree( m_ppKeyLibs );
	m_ppKeyLibs = TNULL;

	// Destroy the models (this releases their materials)
	delete[] m_pTrunks;
	delete[] m_pFOBs;

	// NOTE: the original doesn't reset these pointers
	m_pTrunks    = TNULL;
	m_iNumTrunks = 0;
	m_pFOBs      = TNULL;
	m_iNumFOBs   = 0;

	delete[] m_pRenderData;
	m_pRenderData = TNULL;

	delete[] m_pTrunkCollisionObjects;
	m_pTrunkCollisionObjects = TNULL;

	DestroyCollisionSets();
}

// $Barnyard: FUNCTION 005edf90
void ATreeManager::DestroyCollisionSets()
{
	while ( m_iNumCollisionSets != 0 )
	{
		ACollisionModelSet* pSet                 = m_apCollisionSets[ --m_iNumCollisionSets ];
		m_apCollisionSets[ m_iNumCollisionSets ] = TNULL;

		if ( pSet )
			delete pSet;
	}
}

// $Barnyard: FUNCTION 005ee020
ACollisionModelSet* ATreeManager::FindCollisionSet( TINT a_iTrunk ) const
{
	for ( TINT i = 0; i < m_iNumCollisionSets; i++ )
	{
		if ( m_apCollisionSets[ i ]->GetModel() == m_pTrunks[ a_iTrunk ].m_pManagedModel->GetModel() )
			return m_apCollisionSets[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005ee070
void ATreeManager::SetCollisionEnabled( TBOOL a_bEnable )
{
	if ( m_bCollisionEnabled == a_bEnable )
		return;

	m_bCollisionEnabled = a_bEnable;

	if ( a_bEnable )
	{
		for ( TINT i = 0; i < m_iNumTrunks; i++ )
		{
			Model*              pTrunk = &m_pTrunks[ i ];
			ACollisionModelSet* pSet   = TNULL;

			if ( pTrunk->m_pManagedModel && m_iNumCollisionSets < MAX_COLLISION_SETS && pTrunk->m_pManagedModel->GetModel()->GetNumCollisionMeshes() > 0 )
			{
				pSet = new ACollisionModelSet();
				pSet->Create( pTrunk->m_pManagedModel->GetModel() );

				m_apCollisionSets[ m_iNumCollisionSets++ ] = pSet;
			}
		}
	}
	else
	{
		DestroyCollisionSets();
	}
}

// $Barnyard: FUNCTION 005ee170
TBOOL ATreeManager::Initialise()
{
	TASSERT( m_pTrunks == TNULL && m_pFOBs == TNULL );

	m_iNumKeyLibs = 0;

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
				pTreeModel->m_fGroundOffset = 0.0f;
				pTreeModel->m_fRadius       = 1.6f;

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

				pTreeModel->m_fGroundOffset = -pModel->GetLOD( 0 ).BoundingSphere.GetOrigin().z;
				pTreeModel->m_fRadius       = pModel->GetLOD( 0 ).BoundingSphere.GetRadius() * 1.5f;

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

	m_pRenderData = new TreeInstanceRenderData*[ m_iNumTrunks * 2 ];

	return TTRUE;
}

// $Barnyard: FUNCTION 005ee8e0
void ATreeManager::CreateInstances( ATerrainLocatorList* a_pLocatorList )
{
	TVALIDPTR( a_pLocatorList );

	TASSERT( m_pFOBs != TNULL && m_pTrunks != TNULL );

	static constexpr TINT NUM_LOCATOR_TYPES = 15;

	// The trunk of the locator types
	const TINT aTrunkIndices[ NUM_LOCATOR_TYPES ] = { 3, 5, 4, 1, 1, 1, 6, 6, 2, 3, 4, 5, 6, 0, 7 };

	const TCHAR* aLocatorNames[ NUM_LOCATOR_TYPES ] = {
		"tree02",
		"tree03",
		"tree05",
		"fob2",
		"fobw",
		"busha",
		"treewall2",
		"treewall",
		"treelittle",
		"tree_bent",
		"tree_standard",
		"tree_tall",
		"treewall_sim",
		"fob_bens",
		"fob_pond",
	};

	// Whether the FOBs of the locator types get collisions
	const TBOOL aHasFOBCollision[ NUM_LOCATOR_TYPES ] = {
		TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TFALSE, TTRUE, TTRUE, TFALSE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE
	};

	// The locators are Z up
	TMatrix44 matGlobalTransform = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	TINT iNumTrunkCollisions = 0;
	TINT iNumLocators        = a_pLocatorList->GetNumLocators();

	// For each created instance (in the order of creation): whether its trunk gets a collision and its locator type
	TBOOL aTrunkCollision[ MAX_INSTANCES - 2 ];
	TINT  aLocatorTypes[ MAX_INSTANCES ];
	TINT  iNumCreated = 0;

	ATerrainLocatorTRBHeader* pLocatorHeader = a_pLocatorList->GetHeader();
	ATerrainLocator*          pLocators      = pLocatorHeader->pLocators;

	for ( TINT i = 0; i < iNumLocators; i++ )
	{
		const TCHAR* pchLocatorName = pLocatorHeader->ppNames[ pLocators[ i ].uiNameId ];

		TINT iType = 0;
		while ( iType < NUM_LOCATOR_TYPES && TStringManager::String8CompareNoCase( aLocatorNames[ iType ], pchLocatorName, TStringManager::String8Length( aLocatorNames[ iType ] ) ) != 0 )
			iType++;

		if ( iType >= NUM_LOCATOR_TYPES || m_llFreeTreeInstances.IsEmpty() )
			continue;

		TINT  iTrunkIndex   = aTrunkIndices[ iType ];
		TBOOL bFOBCollision = TFALSE;

		if ( m_bCollisionEnabled != TTRUE || m_pTrunks[ iTrunkIndex ].m_pManagedModel != TNULL )
		{
			// A negative flag means no collisions
			if ( pLocators[ i ].iFlags2 < 0 )
			{
				aTrunkCollision[ iNumCreated ] = TFALSE;
			}
			else
			{
				aTrunkCollision[ iNumCreated ] = TTRUE;
				iNumTrunkCollisions++;
				bFOBCollision = TTRUE;
			}
		}
		else
		{
			aTrunkCollision[ iNumCreated ] = TTRUE;
			iNumTrunkCollisions++;
		}

		TreeInstance* pTreeInstance = m_llFreeTreeInstances.PopFront();

		TMatrix44 matLocatorTransform;
		pLocators[ i ].GetMatrix( matLocatorTransform );

		TMatrix44 matFinalTransform;
		matFinalTransform.Multiply( matGlobalTransform, matLocatorTransform );

		pTreeInstance->iTrunkIndex = iTrunkIndex;

		if ( iTrunkIndex == 7 )
		{
			// fob_pond
			pTreeInstance->iTrunkIndex = 7;
			pTreeInstance->iColor      = 3;
		}
		else
		{
			// The colour comes from the suffix of the name
			TINT iLocatorNameLength = TStringManager::String8Length( pchLocatorName );

			if ( iLocatorNameLength >= 4 && pchLocatorName[ iLocatorNameLength - 4 ] == '_' )
			{
				if ( pchLocatorName[ iLocatorNameLength - 3 ] == 'b' && pchLocatorName[ iLocatorNameLength - 2 ] == 'r' && pchLocatorName[ iLocatorNameLength - 1 ] == 'n' )
					pTreeInstance->iColor = 1;
				else if ( pchLocatorName[ iLocatorNameLength - 3 ] == 'd' && pchLocatorName[ iLocatorNameLength - 2 ] == 'k' && pchLocatorName[ iLocatorNameLength - 1 ] == 'g' )
					pTreeInstance->iColor = 2;
			}
		}

		pTreeInstance->uiLocatorId     = TUINT16( i );
		pTreeInstance->pLocatorList    = a_pLocatorList;
		pTreeInstance->uiRegrowthGroup = 255;
		m_llUsedTreeInstances.PushFront( pTreeInstance );

		aLocatorTypes[ iNumCreated++ ] = iType;

		// The fruits of the tree (ARegrowthManager), the type is in the flags of the locator
		TUINT uiRegrowthType = pLocators[ i ].iFlags2 & 0x7f;

		if ( HASANYFLAG( ARegrowthManager::GetSingleton()->GetFlags(), ARegrowthManager::FLAGS_IN_USE ) && ARegrowthManager::HasTypeFlag( uiRegrowthType, ARegrowthManager::TYPEFLAG_FRUIT ) )
		{
			pTreeInstance->uiRegrowthGroup = ARegrowthManager::GetSingleton()->AllocateGroupId();

			Model*     pTrunk    = &m_pTrunks[ pTreeInstance->iTrunkIndex ];
			TSkeleton* pSkeleton = m_pFOBs[ pTrunk->m_uiID * 2 ].m_pManagedModel->GetModel()->GetSkeleton();

			// The bones of the FOBs are where the fruits grow
			for ( TINT k = 0; k < m_pTrunks[ pTreeInstance->iTrunkIndex ].m_iNumFOBs; k++ )
			{
				TMatrix44 matFOB;
				matFOB.Multiply( matFinalTransform, m_pTrunks[ pTreeInstance->iTrunkIndex ].m_pFOBMatrices[ k ] );

				ARegrowthManager::GetSingleton()->CreateFromSkeleton( pSkeleton, matFOB, pTreeInstance->uiRegrowthGroup, uiRegrowthType, TFALSE );
			}
		}

		// Collisions of the FOBs
		Model* pTrunk   = &m_pTrunks[ pTreeInstance->iTrunkIndex ];
		TINT   iNumFOBs = pTrunk->m_iNumFOBs;

		if ( m_bCreateCollision && bFOBCollision && aHasFOBCollision[ iType ] && iNumFOBs > 0 )
		{
			pTreeInstance->pSimpleCollisionObjects = new ACollisionObjectSimple[ iNumFOBs ];

			TFLOAT fRadius = 1.3f;

			for ( TINT k = 0; k < iNumFOBs; k++ )
			{
				TMatrix44 matFOBLocal = pTrunk->m_pFOBMatrices[ k ];

				if ( pTrunk->m_uiID > 1 )
				{
					// One big sphere above the trunk
					matFOBLocal = TMatrix44::IDENTITY;
					matFOBLocal.SetTranslation( TVector4( 0.0f * 5.0f, 0.0f * 5.0f, 1.0f * 5.0f, 1.0f * 5.0f ) );
					fRadius = 2.5f;
				}

				TMatrix44 matFOB;
				matFOB.Multiply( matFinalTransform, matFOBLocal );

				pTreeInstance->pSimpleCollisionObjects[ k ].CreateActive( matFOB.GetTranslation(), fRadius, 0.2f, 0.0f, ACollisionObject::TYPE_FOLIAGE, TNULL );
			}
		}
	}

	// Collisions of the trunks
	delete[] m_pTrunkCollisionObjects;
	m_pTrunkCollisionObjects = TNULL;

	if ( iNumTrunkCollisions <= 0 )
		return;

	m_pTrunkCollisionObjects = new ACollisionObjectSimple[ iNumTrunkCollisions ];

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original starts at the number of all the used instances, which reads past the
	// arrays above when instances of other locator lists exist (the list has the newest instances first)
	TINT iInstance = iNumCreated - 1;
#else
	TINT iInstance = -1;
	T2_FOREACH( m_llUsedTreeInstances, it )
	iInstance++;
#endif

	TINT iCollision = iNumTrunkCollisions;

	for ( auto it = m_llUsedTreeInstances.Begin(); it != m_llUsedTreeInstances.End(); it = it->Next(), iInstance-- )
	{
		if ( !aTrunkCollision[ iInstance ] )
			continue;

		const ATerrainLocator* pLocator = it->pLocatorList->GetLocator( it->uiLocatorId );
		TVector4               vecPosition( pLocator->vecPosition.x, 0.7f - pLocator->vecPosition.z, pLocator->vecPosition.y, 1.0f );

		ACollisionModelInstance* pModelInstance = TNULL;
		TINT                     iType          = aLocatorTypes[ iInstance ];
		TINT                     iTrunkIndex    = aTrunkIndices[ iType ];

		if ( m_bCollisionEnabled == TTRUE && m_pTrunks[ iTrunkIndex ].m_pManagedModel )
		{
			ACollisionModelSet* pSet = FindCollisionSet( iTrunkIndex );
			pModelInstance           = new ACollisionModelInstance();
			pModelInstance->Create( pSet, m_pTrunks[ iTrunkIndex ].m_pSceneObject );
			it->pTrunkCollision = pModelInstance;
		}

		TFLOAT fScale = pLocator->GetScale();
		TFLOAT fRadius;
		TFLOAT fHeight;
		TUINT  eType;

		if ( iType == 8 )
		{
			// treelittle
			eType   = ACollisionObject::TYPE_TRUNK;
			fRadius = 0.3f * fScale;
			fHeight = fScale * 3.7f;
		}
		else if ( iType == 5 )
		{
			// busha
			eType   = ACollisionObject::TYPE_BUSH;
			fRadius = 1.1f * fScale;
			vecPosition.y += 0.5f;
			fHeight = fScale * 0.6f + 0.5f;
		}
		else
		{
			eType   = ACollisionObject::TYPE_TRUNK;
			fRadius = 1.3f * fScale;
			fHeight = fScale * 5.7f;
		}

		m_pTrunkCollisionObjects[ --iCollision ].CreateActive( vecPosition, fRadius, fHeight, fRadius * 0.5f, eType, pModelInstance );

		if ( iCollision <= 0 )
			break;
	}
}

static constexpr TUINT                      MAX_RENDERED_INSTANCES = 240;
static ATreeManager::TreeInstanceRenderData s_aInstanceMatrices[ MAX_RENDERED_INSTANCES ];

// $Barnyard: FUNCTION 005ef2b0
void ATreeManager::DestroyInstances( ATerrainLocatorList* a_pLocatorList )
{
	// NOTE: the collisions of the trunks of all the locator lists are destroyed
	delete[] m_pTrunkCollisionObjects;
	m_pTrunkCollisionObjects = TNULL;

	for ( auto it = m_llUsedTreeInstances.Begin(); it != m_llUsedTreeInstances.End(); )
	{
		TreeInstance* pTreeInstance = it;
		it                          = it->Next();

		if ( pTreeInstance->pLocatorList != a_pLocatorList )
			continue;

		// The regrowth objects (fruits) of the tree
		if ( HASANYFLAG( ARegrowthManager::GetSingleton()->GetFlags(), ARegrowthManager::FLAGS_IN_USE ) &&
		     pTreeInstance->uiRegrowthGroup != 0xff && pTreeInstance->uiRegrowthGroup != 0 )
		{
			ARegrowthManager::GetSingleton()->DestroyGroup( pTreeInstance->uiRegrowthGroup );
		}

		if ( pTreeInstance->pSimpleCollisionObjects )
		{
			delete[] pTreeInstance->pSimpleCollisionObjects;
			pTreeInstance->pSimpleCollisionObjects = TNULL;
		}

		// NOTE: the original doesn't delete the collision model instance of the trunk
		pTreeInstance->pTrunkCollision = TNULL;
		m_llUsedTreeInstances.Erase( pTreeInstance );
		m_llFreeTreeInstances.PushFront( pTreeInstance );
	}
}

// $Barnyard: FUNCTION 005efc80
ATreeManager::TreeInstance* ATreeManager::FindTreeInFront( const TVector4& a_rcPosition, const TVector2& a_rcDirection, TFLOAT a_fMaxDistance )
{
	TreeInstance* pFound = TNULL;

	T2_FOREACH( m_llUsedTreeInstances, it )
	{
		// NOTE: the locators are Z up
		const ATerrainLocator* pLocator = it->pLocatorList->GetLocator( it->uiLocatorId );
		TVector2               vecToTree( pLocator->vecPosition.x - a_rcPosition.x, pLocator->vecPosition.y - a_rcPosition.z );

		if ( vecToTree.y * a_rcDirection.y + vecToTree.x * a_rcDirection.x > 0.0f )
		{
			TFLOAT fDistance = vecToTree.NormalizeMag();

			if ( fDistance <= a_fMaxDistance && vecToTree.y * a_rcDirection.y + vecToTree.x * a_rcDirection.x > 0.8f )
			{
				Model* pTrunk = &m_pTrunks[ it->iTrunkIndex ];

				if ( pTrunk->m_pSceneObject && pTrunk->m_iNumFOBs > 0 )
				{
					a_fMaxDistance = fDistance;
					pFound         = it;
				}
			}
		}
	}

	return pFound;
}

// $Barnyard: FUNCTION 005efda0
ATreeManager::TreeInstance* ATreeManager::FindTreeByCollision( void* a_pCollisionInstance )
{
	T2_FOREACH( m_llUsedTreeInstances, it )
	{
		if ( it->pTrunkCollision == a_pCollisionInstance )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005edde0
TINT ATreeManager::GetNumFOBs( const TreeInstance* a_pTree ) const
{
	return m_pTrunks[ a_pTree->iTrunkIndex ].m_iNumFOBs;
}

// $Barnyard: FUNCTION 005ede00
TVector4 ATreeManager::GetFOBPosition( const TreeInstance* a_pTree, TINT a_iFOB ) const
{
	// The locators are Z up
	TMatrix44 matZUp = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	TMatrix44 matLocator;
	a_pTree->pLocatorList->GetLocator( a_pTree->uiLocatorId )->GetMatrix( matLocator );

	TMatrix44 matTree;
	matTree.Multiply( matZUp, matLocator );

	TMatrix44 matFOB;
	matFOB.Multiply( matTree, m_pTrunks[ a_pTree->iTrunkIndex ].m_pFOBMatrices[ a_iFOB ] );

	return matFOB.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $Barnyard: FUNCTION 005ef3a0
void ATreeManager::Render()
{
	AGlowViewport*    pGlowViewport    = AGlowViewport::GetSingleton();
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();
	TRenderContext*   pRenderContext   = pRenderInterface->GetCurrentContext();

	TMatrix44 matModelView = pRenderContext->GetModelViewMatrix();
	TMatrix44 matLightDir  = pRenderInterface->GetLightDirection();

	TINT iNumLocatorsToRender = 0;

	// Reset render data
	for ( TINT i = 0; i < m_iNumTrunks * 2; i++ )
		m_pRenderData[ i ] = 0;

	// Get terrain related objects
	ATerrainInterface* pTerrainInterface = ATerrainInterface::GetSingleton();
	ATerrainVIS*       pTerrainVIS       = pTerrainInterface->GetVIS();
	TINT               iNumSections      = pTerrainVIS->GetNumSections();
	ATerrainSection*   pCurrentSection   = &pTerrainVIS->GetSections()[ pTerrainInterface->GetSectionID() ];

	ATerrainLocatorList* pLocatorList            = pTerrainInterface->GetLocatorList();
	TINT                 iLocatorsNumVISSections = pLocatorList->GetNumVISSections();

	// Collect info about the locators we should render
	constexpr TUINT MAX_SECTIONS = 64;

	TUINT uiNumSectionsToRender = 0;
	struct SectionRenderData
	{
		TUINT uiStart;
		TUINT uiEnd;
	} aSectionRenderData[ MAX_SECTIONS ];

	for ( TINT i = 0; i < iLocatorsNumVISSections; i++ )
	{
		ATerrainLocatorVISSection* pLocatorVISSection = &pLocatorList->GetVISHeader()->pSections[ i ];

		if ( !TStringManager::String8CompareNoCase( "undefined", pLocatorVISSection->pName ) )
		// undefined cell - always visible
		{
			TUINT uiFirstLocator = pLocatorVISSection->uiFirstLocator;
			TUINT uiNumLocators  = pLocatorVISSection->uiNumLocators;

			aSectionRenderData[ uiNumSectionsToRender ].uiStart = uiFirstLocator;
			aSectionRenderData[ uiNumSectionsToRender ].uiEnd   = uiFirstLocator + uiNumLocators;

			uiNumSectionsToRender++;
		}
		else
		// some other cell - should check visibility
		{
			for ( TINT k = 0; k < iNumSections; k++ )
			{
				// If the section is visible from the current one and locator VIS name matches it, add to the draw list
				if ( pCurrentSection->IsOtherSectionVisible( k ) &&
				     !TStringManager::String8CompareNoCase( pTerrainVIS->GetSections()[ k ].GetName(), pLocatorVISSection->pName ) )
				{
					TUINT uiFirstLocator = pLocatorVISSection->uiFirstLocator;
					TUINT uiNumLocators  = pLocatorVISSection->uiNumLocators;

					aSectionRenderData[ uiNumSectionsToRender ].uiStart = uiFirstLocator;
					aSectionRenderData[ uiNumSectionsToRender ].uiEnd   = uiFirstLocator + uiNumLocators;

					uiNumSectionsToRender++;

					break;
				}
			}
		}
	}

	if ( iLocatorsNumVISSections == 0 )
	{
		// Compare against all locators if no VIS data is provided
		aSectionRenderData[ uiNumSectionsToRender ].uiStart = 0;
		aSectionRenderData[ uiNumSectionsToRender ].uiEnd   = 2147483647;
		uiNumSectionsToRender += 1;
	}

	TASSERT( uiNumSectionsToRender < MAX_SECTIONS );

	// Do culling
	TFLOAT fRenderDistanceSq    = m_fRenderDistance * m_fRenderDistance;
	TFLOAT fLODSwitchDistanceSq = m_fLODSwitchDistance * m_fLODSwitchDistance;
	TFLOAT fBlendingFactor      = 1.0f / ( m_fRenderDistance - m_fBlendingDistance );

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

	TVector4 vecViewPos = pRenderContext->GetViewWorldMatrix().GetTranslation();

	struct LocatorRenderData
	{
		TUINT8 uiFlags;
		TUINT8 uiUnused;
		TUINT8 uiLightId;
	} aLocatorRenderData[ MAX_RENDERED_INSTANCES ];

	if ( !m_llUsedTreeInstances.IsEmpty() )
	{
		TINT iInstanceIndex = 0;

		T2_FOREACH( m_llUsedTreeInstances, pTreeInstance )
		{
			TUINT uiInstanceLocatorId = pTreeInstance->uiLocatorId;

			for ( TUINT i = 0; i < uiNumSectionsToRender; i++ )
			{
				if ( aSectionRenderData[ i ].uiStart <= uiInstanceLocatorId && uiInstanceLocatorId < aSectionRenderData[ i ].uiEnd )
				// this tree instance relates to this VIS section
				{
					Model*           pTrunk    = &m_pTrunks[ pTreeInstance->iTrunkIndex ];
					ATerrainLocator* pLocators = pTreeInstance->pLocatorList->GetHeader()->pLocators;

					ATerrainLocator* pLocator = &pLocators[ uiInstanceLocatorId ];
					TFLOAT           fScale   = pLocator->GetScale();

					TSphere boundingVolume( pLocator->vecPosition.x, -pLocator->vecPosition.z + pTrunk->m_fGroundOffset, pLocator->vecPosition.y, fScale * pTrunk->m_fRadius );
					TFLOAT  fDistanceSq = TVector4::DistanceSq( boundingVolume.AsVector4(), vecViewPos );

					if ( fDistanceSq < fRenderDistanceSq )
					{
						const TBOOL bIsLocatorFar = fLODSwitchDistanceSq <= fDistanceSq;

						if ( TRenderContext::CullSphereToFrustumSimple( boundingVolume, pRenderContext->GetWorldPlanes(), 6 ) &&
						     ( !bIsLocatorFar || bIsLocatorFar ) // yeah, that's stupid, but it's probably a leftover from testing
						)
						{
							// Get transform of this locator
							TMatrix44 matLocator;
							pLocator->GetMatrix( matLocator );

							// Go render if exceeded the max limit of instances to render
							if ( iInstanceIndex >= MAX_RENDERED_INSTANCES )
								goto START_RENDERING;

							TreeInstanceRenderData* pInstanceRenderData = &s_aInstanceMatrices[ iInstanceIndex ];
							TINT                    iRenderDataIndex    = pTreeInstance->iTrunkIndex * 2;

							// Use second LOD if the locator is away
							if ( bIsLocatorFar ) iRenderDataIndex += 1;

							aLocatorRenderData[ iNumLocatorsToRender ].uiFlags  = pLocators[ uiInstanceLocatorId ].GetFlags1();
							aLocatorRenderData[ iNumLocatorsToRender ].uiUnused = 0x3F;

							// Get list of lights
							TLightIDList oLightIdList;
							pGlowViewport->GetInfluencingLightIDs( boundingVolume, oLightIdList );

							aLocatorRenderData[ iNumLocatorsToRender ].uiFlags |= pTreeInstance->iColor << 4;
							aLocatorRenderData[ iNumLocatorsToRender ].uiLightId = oLightIdList[ 0 ];

							iNumLocatorsToRender += 1;
							iInstanceIndex += 1;

							// Build instance matrix
							TMatrix44 matTransform;
							matTransform.Multiply( matGlobalTransform, matLocator );

							if ( pTreeInstance->pTrunkCollision )
							{
								// Move the trunk's scene object to this tree, so its collision model instance follows it
								TTransformObject& rTransform = m_pTrunks[ pTreeInstance->iTrunkIndex ].m_pSceneObject->GetTransform();

								matTransform.Scale( fScale );
								rTransform.SetMatrix( matTransform );
								rTransform.SetScale( fScale, fScale, fScale );

								pTreeInstance->pTrunkCollision->SetTransformObject( &rTransform );
							}

							pInstanceRenderData->oMatrix.Multiply( matModelView, matTransform );
							pInstanceRenderData->oMatrix.Scale( fScale );

							pInstanceRenderData->pPrev        = m_pRenderData[ iRenderDataIndex ];
							m_pRenderData[ iRenderDataIndex ] = pInstanceRenderData;
						}
					}
				}
			}
		}
	}

START_RENDERING:
	static TVector4 s_vecUnused1 = TVector4( 0.7372549f, 0.8156863f, 0.5254902f );
	static TVector4 s_vecUnused2 = TVector4( 0.54509807f, 0.60784316f, 0.47058824f );
	static TVector4 s_vecUnused3 = TVector4( 0.9529412f, 0.75686276f, 0.54509807f );

	// Calculate light direction matrices
	TMatrix44 aLightMatrices[ 16 ];
	for ( TINT i = 0; i < TARRAYSIZE( aLightMatrices ); i++ )
	{
		aLightMatrices[ i ].ScaleAll( matLightDir, i * ( 1.0f / ( TARRAYSIZE( aLightMatrices ) - 1 ) ) );
	}

	// Finally, do the rendering!!!
	for ( TINT i = 0; i < m_iNumTrunks * 2; i++ )
	{
		if ( m_pRenderData[ i ] == TNULL )
			continue;

		Model*        pTrunkModel  = &m_pTrunks[ i / 2 ];
		TSceneObject* pSceneObject = pTrunkModel->m_pSceneObject;

		// Decide which LOD to render
		TINT iLODIndex = i & 1;

		// Setup skeleton instance and make sure the LOD exists
		if ( pSceneObject )
		{
			iLODIndex = TMath::Min( pSceneObject->GetInstance()->GetModel()->GetNumLODs() - 1, iLODIndex );
			TASSERT( iLODIndex >= 0 );

			pRenderContext->SetSkeletonInstance( pSceneObject->GetInstance()->GetSkeletonInstance() );
		}

		// Render recursively
		TreeInstanceRenderData* pInstanceRenderData = m_pRenderData[ i ];
		while ( pInstanceRenderData != TNULL )
		{
			// Set blending or don't draw instance that is completely blend out
			if ( pInstanceRenderData->oMatrix.m_f43 >= m_fBlendingDistance )
			{
				TFLOAT fBlend = ( pInstanceRenderData->oMatrix.m_f43 - m_fBlendingDistance ) * fBlendingFactor;

				if ( fBlend < 1.0f )
				{
					pRenderContext->SetAlphaBlend( 1.0f - fBlend );
				}
				else
				{
					pInstanceRenderData = pInstanceRenderData->pPrev;
					continue;
				}
			}
			else
			{
				pRenderContext->SetAlphaBlend( 1.0f );
			}

			// Setup lighting settings
			TINT iInstanceIndex = pInstanceRenderData - s_aInstanceMatrices;
			pRenderInterface->SetLightDirectionMatrix( aLightMatrices[ aLocatorRenderData[ iInstanceIndex ].uiFlags % TARRAYSIZE( aLightMatrices ) ] );

			// Setup model view matrix
			pRenderContext->SetModelViewMatrix( pInstanceRenderData->oMatrix );

			// Render trunk model
			if ( pSceneObject )
			{
				TModel*    pModel = pSceneObject->GetInstance()->GetModel();
				TModelLOD& rLOD   = pModel->GetLOD( iLODIndex );

				for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
					rLOD.ppMeshes[ k ]->Render();
			}

			// Render FOB model
			Model* pFOBModel = &m_pFOBs[ pTrunkModel->m_uiID * 2 ];
			pRenderContext->SetSkeletonInstance( pFOBModel->m_pSceneObject->GetInstance()->GetSkeletonInstance() );

			for ( TINT k = 0; k < pTrunkModel->m_iNumFOBs; k++ )
			{
				TMatrix44 matFOBTransform;
				matFOBTransform.Multiply( pInstanceRenderData->oMatrix, pTrunkModel->m_pFOBMatrices[ k ] );

				pRenderContext->SetModelViewMatrix( matFOBTransform );

				TModel*    pModel = pFOBModel->m_pSceneObject->GetInstance()->GetModel();
				TModelLOD& rLOD   = pModel->GetLOD( 0 );

				for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
					rLOD.ppMeshes[ k ]->Render();
			}

			pInstanceRenderData = pInstanceRenderData->pPrev;
		}
	}

	pRenderInterface->SetLightDirectionMatrix( matLightDir );
	pRenderContext->SetAlphaBlend( 1.0f );
	pRenderContext->SetModelViewMatrix( matModelView );
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

	// NOTE: the models of the FOBs have no matrices
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
