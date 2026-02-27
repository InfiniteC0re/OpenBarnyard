#include "pch.h"
#include "AInstanceManager.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "ALoadScreen.h"

#ifdef TRENDERINTERFACE_DX8
#  include "Platform/DX8/ASkinShader/ASkinMaterial_DX8.h"
#endif

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005dfd60
AInstanceManager::AInstanceManager()
    : m_ppKeyLibs( TNULL )
    , m_pModels( TNULL )
    , m_pSomeBuffer( TNULL )
    , m_iInstanceLibIndex( -1 )
    , m_pCollObjectModelNodes( TNULL )
    , m_pCollisionModelInstances( TNULL )
    , m_uiFlags( 0 )
    , m_iNumKeyLibraries( 0 )
    , m_flUnk1( 30.0f )
    , m_flUnk2( 85.0f )
    , m_flUnk3( 75.0f )
{
	m_iNumModels = NUM_MODELS;
	m_pModels    = new Model[ NUM_MODELS ];

	for ( TSIZE i = 0; i < TARRAYSIZE( m_aInstances ); i++ )
		m_llFreeInstances.PushFront( &m_aInstances[ i ] );

	m_pSomeBuffer = TMalloc( m_iNumModels << 3 );
}

// $Barnyard: FUNCTION 005e1d80
AInstanceManager::~AInstanceManager()
{
	m_llUsedInstances.Clear();
	Reset();
}

static const TCHAR* s_ppszInstanceLibs[] = {
	TNULL,
	TNULL,
	TNULL,
	"instancescc",
	TNULL,
	"instancescc",
	"instancescc",
	"instancescc",
	TNULL
};

static const TCHAR* s_pszMainLibraryName = "instancessh";

// $Barnyard: FUNCTION 005e1e80
TBOOL AInstanceManager::LoadModels( TINT a_iInstanceLibIndex, TINT a_iNumModels, TUINT* a_pModelIndices, TBOOL a_bCreateCollisionModelNodes, TBOOL a_bCreateCollisionModelInstances, TINT a_iNumCollisionModelNodes )
{
	TASSERT( a_iInstanceLibIndex > -1 );

	// Update flags
	if ( a_bCreateCollisionModelNodes )
	{
		m_uiFlags |= 4;
		m_uiFlags |= 12;
	}
	else
	{
		m_uiFlags &= ~4;
		m_uiFlags &= ~12;
	}

	Reset();
	TTRB* pAssetTRB = AAssetLoader::GetAssetTRB( AAssetType_AssetPack );

	// Load necessary libraries
	m_iInstanceLibIndex = a_iInstanceLibIndex;
	if ( s_ppszInstanceLibs[ m_iInstanceLibIndex ] )
	{
		AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( TPS8D( s_ppszInstanceLibs[ m_iInstanceLibIndex ] ), pAssetTRB );
		AMaterialLibraryManager::GetSingleton()->LoadLibrary( TPS8D( s_ppszInstanceLibs[ m_iInstanceLibIndex ] ), pAssetTRB, TTRUE );
	}

	AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( TPS8D( s_pszMainLibraryName ), pAssetTRB );
	AMaterialLibraryManager::GetSingleton()->LoadLibrary( TPS8D( s_pszMainLibraryName ), pAssetTRB, TTRUE );

	// Load all requested models
	for ( TINT k = 0; k < a_iNumModels; k++ )
	{
		// Update once in 16 times
		if ( ( k % 16 ) == 0 ) g_oLoadScreen.Update();

		const TUINT uiModelIndex = a_pModelIndices[ k ];

		TManagedModel* pManagedModel = new TManagedModel;
		TBOOL          bLoadedModel  = pManagedModel->Create( ms_aModelDefinitions[ uiModelIndex ].pchPath, pAssetTRB );

#ifdef TOSHI_DEBUG
		if ( !bLoadedModel )
			TERROR( "Couldn't load instance model '%s'\n", ms_aModelDefinitions[ uiModelIndex ].pchPath );
		else
			TINFO( "Loaded instance model '%s'\n", ms_aModelDefinitions[ uiModelIndex ].pchPath );
#endif // TOSHI_DEBUG

		// Create scene object
		TSceneObject* pSceneObject   = pManagedModel->CreateSceneObject();
		m_pModels[ uiModelIndex ].pManagedModel = pManagedModel;
		m_pModels[ uiModelIndex ].pSceneObject  = pSceneObject;

		// Setup object
		pSceneObject->GetTransform().SetMatrix( TMatrix44::IDENTITY );
		pSceneObject->GetTransform().SetEuler( TVector3( TMath::DegToRad( 90.0f ), 0.0f, 0.0f ) );
		pSceneObject->GetTransform().SetTranslate( TVector3::VEC_ZERO );

		// Bake transform into matrix
		TMatrix44 matTransform;
		pSceneObject->GetTransform().GetLocalMatrixImp( matTransform );
		pSceneObject->GetTransform().SetMatrix( matTransform );

		// Disable HD lighting for instances
		TModel* pModel = pManagedModel->GetModel();
		for (TINT i = 0; i < pModel->GetNumLODs(); i++)
		{
			TModelLOD* pLOD = &pModel->GetLOD( i );

			for ( TINT k = 0; k < pLOD->iNumMeshes; k++ )
			{
				TMesh* pMesh = pLOD->ppMeshes[ k ];
				TMaterial* pMaterial = pMesh->GetMaterial();

				TASSERT( pMaterial->IsA( &TGetClass( ASkinMaterial ) ) );
				ASkinMaterialHAL* pSkinMaterial = TSTATICCAST( ASkinMaterialHAL, pMaterial );

				pSkinMaterial->SetHDLighting( TFALSE );
			}
		}
	}

	// Initialise collision
	SetupCollisions( a_bCreateCollisionModelInstances );
	
	// Make sure collision nodes are created if they are needed
	if ( a_bCreateCollisionModelNodes && m_llFreeCollisionModelNodes.IsEmpty() )
	{
		// Remove free nodes if they for some reason exist
		if ( m_pCollObjectModelNodes ) delete[] m_pCollObjectModelNodes;

		// Allocate nodes
		m_pCollObjectModelNodes = new ACollisionObjectModelNode[ a_iNumCollisionModelNodes ];
		for ( TINT i = 0; i < a_iNumCollisionModelNodes; i++ )
			m_llFreeCollisionModelNodes.PushFront( &m_pCollObjectModelNodes[ i ] );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005e1ae0
void AInstanceManager::Reset()
{
	for ( TSIZE i = 0; i < TARRAYSIZE( m_aLists ); i++ )
		ClearList( m_aLists[ i ] );

	delete[] m_pCollObjectModelNodes;
	m_pCollObjectModelNodes = TNULL;

	delete[] m_pCollisionModelInstances;
	m_pCollisionModelInstances = TNULL;

	m_llFreeCollisionModelNodes.Clear();
	m_llFreeCollisionModelInstances.Clear();

	// Wait until GPU is done with the resources before models are destroyed
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	// Destroy models
	for ( TINT i = 0; i < m_iNumModels; i++ )
		m_pModels[i].Reset();

	// Destroy key libs
	if ( m_ppKeyLibs )
	{
		for (TINT i = 0; i < m_iNumKeyLibraries; i++)
		{
			TRenderInterface::GetSingleton()->GetKeyframeLibraryManager().UnloadLibrary( m_ppKeyLibs[ i ] );
		}

		delete[] m_ppKeyLibs;
		m_ppKeyLibs = TNULL;
	}

	// Unload other libraries
	if ( m_iInstanceLibIndex > -1 )
	{
		if ( s_ppszInstanceLibs[ m_iInstanceLibIndex ] )
		{
			AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPS8D( s_ppszInstanceLibs[ m_iInstanceLibIndex ] ) );
			AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPS8D( s_ppszInstanceLibs[ m_iInstanceLibIndex ] ), TTRUE );
		}

		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPS8D( s_pszMainLibraryName ) );
		AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPS8D( s_pszMainLibraryName ), TTRUE );

		m_iInstanceLibIndex = -1;
	}

	DestroyCollisionSets();
}

static TMatrix44 s_mInstanceTransform = {
	1.0f,  0.0f, 0.0f, 0.0f,
	0.0f,  0.0f, 1.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f,  0.0f, 0.0f, 1.0f
};

// $Barnyard: FUNCTION 005e0300
void AInstanceManager::CreateInstances( ATerrainLocatorList* a_pLocatorList )
{
	TSTATICASSERT( 255 > NUM_MODELS );

	// Per-locator spawn flags (parallel array to the locator list)
	enum EInstanceFlags : TCHAR
	{
		InstanceFlags_None          = 0,
		InstanceFlags_RenderOnly    = 1,
		InstanceFlags_WithCollision = 2,
	};

	//-----------------------------------------------------------------------------
	// Build a per-model-slot lookup: model index -> ACollisionModelSet*
	//-----------------------------------------------------------------------------
	ACollisionModelSet* apModelCollSets[ NUM_MODELS ];
	for ( TINT i = 0; i < NUM_MODELS; i++ )
	{
		apModelCollSets[ i ] = TNULL;

		if ( m_pModels[ i ].pManagedModel == TNULL )
			continue;

		TModel* pModel = m_pModels[ i ].pManagedModel->GetModel();
		for ( TINT j = 0; j < m_vecCollisionSets.Size(); j++ )
		{
			if ( m_vecCollisionSets[ j ]->GetModel() == pModel )
			{
				apModelCollSets[ i ] = m_vecCollisionSets[ j ];
				break;
			}
		}
	}

	//-----------------------------------------------------------------------------
	// 1. Match every terrain locator to a model definition and
	// decide whether/how to create an instance for it.
	//-----------------------------------------------------------------------------
	const TINT iNumLocators = a_pLocatorList->GetHeader()->iNumLocators;
	if ( iNumLocators < 1 )
		return;

	TCHAR aCollisionFlags[ MAX_INSTANCES ];

	// Counts used in later phases
	TINT iNumCollisionModels = 0; // unique model slots needing a collision instance
	TINT iNumInstancesWithCollision     = 0; // total instances queued (early-out guard)

	// Tracks which model slots have had their collision instance allocated this call.
	TBOOL abCollInstAllocated[ NUM_MODELS ];
	TUtil::MemSet( abCollInstAllocated, 0, sizeof( abCollInstAllocated ) );

	// The list slot we will write instances into (resolved below)
	List* pTargetList = TNULL;

	TCHAR* pCollisionFlagCursor = aCollisionFlags;
	for ( TINT iLocIdx = 0; iLocIdx < iNumLocators; ++iLocIdx )
	{
		const ATerrainLocator* pLocator = a_pLocatorList->GetLocator( iLocIdx );
		const TCHAR*           pszName  = a_pLocatorList->GetLocatorName( iLocIdx );

		// ---- Find matching model definition (case-insensitive linear scan) ----
		TUINT8 iModelIndex = -1;

		for ( TUINT8 k = 0; k < NUM_MODELS; k++ )
		{
			TSIZE uLen = TStringManager::String8Length( ms_aModelDefinitions[ k ].pchName );
			if ( TStringManager::String8CompareNoCase( ms_aModelDefinitions[ k ].pchName, pszName, uLen ) == 0 )
			{
				iModelIndex = k;
				break;
			}
		}

		if ( iModelIndex == -1 ) continue;

		// ---- Regrowth check ----
		// If the regrowth manager is active (flag 0x20 set), flower/plant locators
		// are skipped here and handled by ARegrowthManager instead.
		static const TCHAR* s_apszRegrowthNames[] = {
			"flwry01",
			"flwry02",
			"bale01",
			"milkcan",
			"rock01",
			"trashcan",
			"barrel",
		};

		TBOOL bIsRegrowthObject = TFALSE;
		TTODO( "ARegrowthManager skip here" );
// 		if ( ARegrowthManager::GetSingleton() != TNULL && ( ARegrowthManager::GetSingleton()->GetFlags() & 0x20 ) != 0 )
// 		{
// 			for ( TSIZE f = 0; f < TARRAYSIZE( s_apszRegrowthNames ); f++ )
// 			{
// 				TSIZE uLen = TStringManager::String8Length( s_apszRegrowthNames[ f ] );
// 				if ( TStringManager::String8CompareNoCase( s_apszRegrowthNames[ f ], pszName, uLen ) == 0 )
// 				{
// 					bIsRegrowthObject = TTRUE;
// 					break;
// 				}
// 			}
// 		}

		if ( bIsRegrowthObject ) continue;

		// ---- Validate prerequisites ----
		Model& rModel = m_pModels[ iModelIndex ];

		// Flag 1: model must be loaded before an instance can be created
		if ( ( m_uiFlags & 1 ) && rModel.pManagedModel == TNULL ) continue;

		// Must have a free instance node and a valid model pointer
		if ( m_llFreeInstances.IsEmpty() || rModel.pManagedModel == TNULL ) continue;

		TModel* pModel = rModel.pManagedModel->GetModel();

		// ---- Setup collision type if needed ----
		if ( ( m_uiFlags & 4 ) && ( m_uiFlags & 8 ) )
		{
			const TBOOL bHasCollisionSet = ( m_uiFlags & 2 ) && ( apModelCollSets[ iModelIndex ] != TNULL ) && ( pModel->GetNumCollisionMeshes() > 0 );

			// "radius" bone presence is a secondary collision indicator
			const TINT iBoneRadius = pModel->GetSkeleton()->GetBoneID( "radius", 0 );

			const TBOOL bIsInIgnoreList1 =
			    ( T2String8::FindString( pszName, "_coll" ) == 0 ) ||
			    ( T2String8::FindString( pszName, "bale01" ) == 0 ) ||
			    ( T2String8::FindString( pszName, "rock01" ) == 0 );

			const TBOOL bIsInIgnoreList2 =
			    ( T2String8::FindString( pszName, "FenceRBrown" ) == 0 ) ||
			    ( T2String8::FindString( pszName, "FenceRGrey" ) == 0 ) ||
			    ( T2String8::FindString( pszName, "roadbarrier" ) == 0 ) ||
			    ( T2String8::FindString( pszName, "car_treewall" ) == 0 );

			// Check whether this instance is actually useful for rendering
			if ((!bHasCollisionSet && iBoneRadius == -1 && !bIsInIgnoreList1) || (pLocator->iFlags2 < 0) || bIsInIgnoreList2)
			{
				*( pCollisionFlagCursor++ ) = InstanceFlags_None;
				continue;
			}

			if ( bHasCollisionSet )
			{
				*pCollisionFlagCursor = InstanceFlags_WithCollision;

				// Only allocate one collision model instance per unique model slot
				if ( !abCollInstAllocated[ iModelIndex ] )
				{
					abCollInstAllocated[ iModelIndex ] = TTRUE;
					iNumCollisionModels++;

					pCollisionFlagCursor++;
				}
			}
			else
			{
				// No collision for this instance
				*( pCollisionFlagCursor++ ) = InstanceFlags_RenderOnly;
			}
			
			iNumInstancesWithCollision++;
		}

		// Re encode scale and rotation???
		const TUINT8 uiYawPacked   = (TUINT8)TMath::Round( ( pLocator->uiFlags1 & 0b1111 ) * 20.0f * 0.05f );
		const TUINT8 uiScalePacked = (TUINT8)TMath::Round( ( pLocator->uiFlags1 >> 4 ) * 6.667f * 0.15f );

		// Pop a free instance node
		InstanceEntry* pNode = m_llFreeInstances.PopFront();

		// Pack instance metadata into the node
		pNode->pLocatorList     = a_pLocatorList;
		pNode->uiLocatorIndex   = iLocIdx;
		pNode->uiModelIndex     = iModelIndex;
		pNode->uiPackedScaleYaw = ( uiScalePacked << 4 ) | ( uiYawPacked != 0 ? uiYawPacked : 5 );

		m_llUsedInstances.PushBack( pNode );
	}

	if ( iNumInstancesWithCollision <= 0 ) return;

	//-----------------------------------------------------------------------------
	// NOTE: At this point aCollisionFlags is not filled with garbage!!!
	//-----------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------
	// Find a free List slot for this locator batch
	//-----------------------------------------------------------------------------
	for ( TSIZE i = 0; i < TARRAYSIZE( m_aLists ); i++ )
	{
		if ( m_aLists[ i ].pLocators == TNULL )
		{
			pTargetList = &m_aLists[ i ];
			break;
		}
	}

	// Store locator list pointer in the slot (original uses iUnknown for this)
	if ( pTargetList ) pTargetList->pLocators = a_pLocatorList;

	//-----------------------------------------------------------------------------
	// 2. Create ACollisionModelInstances
	//-----------------------------------------------------------------------------
	ACollisionModelInstanceNode* aCollInstances[ NUM_MODELS ];
	TUtil::MemSet( aCollInstances, 0, sizeof( aCollInstances ) );

	if ( iNumCollisionModels > 0 )
	{
		for ( TSIZE iMdl = 0; iMdl < NUM_MODELS; iMdl++ )
		{
			if ( !abCollInstAllocated[ iMdl ] )
				continue;

			if ( m_llFreeCollisionModelInstances.IsEmpty() )
			{
				aCollInstances[ iMdl ] = TNULL;
				continue;
			}

			ACollisionModelInstanceNode* pInstNode = m_llFreeCollisionModelInstances.PopBack();

			pTargetList->llCollModelInstances.PushBack( pInstNode );

			pInstNode->GetNodeValue().Create(
			    apModelCollSets[ iMdl ],
			    m_pModels[ iMdl ].pSceneObject
			);

			aCollInstances[ iMdl ] = pInstNode;
		}
	}

	//-----------------------------------------------------------------------------
	// 3. Create ACollisionObjectModels
	//
	// For each node the function derives a capsule/sphere collision shape:
	//
	//   * World matrix    – locator matrix * g_mInstanceTransform, then scaled
	//   * Origin          – LOD0 bounding sphere centre in world space
	//                       (overridden by "centre" bone if present)
	//   * rock01_01 bone  – use raw locator position; derive radius/length directly
	//   * end bone        – derive capsule axis direction + length
	//   * radius bone     – compute capsule radius via point-to-segment distance
	//                       (sphere radius if no axis)
	//-----------------------------------------------------------------------------

	TINT iIndex = 0;
	for ( auto pNode = ( m_llUsedInstances ).Begin(); pNode != ( m_llUsedInstances ).End(); ++pNode, ++iIndex )
	{
		if ( aCollisionFlags[ iIndex ] == InstanceFlags_None ) continue;

		// ---- Retrieve locator data ----
		ATerrainLocatorList*   pNodeLocList = pNode->pLocatorList;
		const TUINT16          uiLocatorIdx = pNode->uiLocatorIndex;
		const ATerrainLocator* pLoc         = pNodeLocList->GetLocator( uiLocatorIdx );

		// Y is negated — terrain uses a left-handed coordinate convention
		const TFLOAT fLocX = pLoc->vecPosition.x;
		const TFLOAT fLocY = -pLoc->vecPosition.y;
		const TFLOAT fLocZ = pLoc->vecPosition.z;

		const TUINT8 uiModelIndex = pNode->uiModelIndex;
		TModel*      pModel       = m_pModels[ uiModelIndex ].pManagedModel->GetModel();

		// ---- LOD0 bounding sphere ----
		TVector4 vOrigin( pModel->GetLOD( 0 ).BoundingSphere.GetOrigin() );

		const TFLOAT fScale = (TFLOAT)( pNode->uiPackedScaleYaw & 0b1111 ) * 0.2f;

		// ---- Capsule shape parameters ----
		TVector4 vCapsuleDir    = TVector4::VEC_NEGY;
		TFLOAT   fCapsuleLength = 0.0f;
		TFLOAT   fCapsuleRadius = 0.0f;
		TBOOL    bHasAxis       = TFALSE;

		// ---- World transform: locator * global instance matrix, then scaled ----
		TMatrix44 mLocWorld;
		pLoc->GetMatrix( mLocWorld );

		TMatrix44 mWorld;
		mWorld.Multiply( s_mInstanceTransform, mLocWorld );

		TMatrix44 mWorldUnscaled = mWorld; // keep unscaled copy for TTransformObject

		mWorld.Scale( fScale, fScale, fScale );
		TMatrix44::TransformVector( vOrigin, mWorld, vOrigin );

		TSkeleton* pSkel = pModel->GetSkeleton();

		// ---- "rock01_01" bone: use raw locator pos, fixed capsule sizing ----
		TINT iBone = pSkel->GetBoneID( "rock01_01", 0 );
		if ( iBone != -1 )
		{
			vOrigin        = TVector4( fLocX, fLocY, fLocZ );
			fCapsuleRadius = ( pNode->uiPackedScaleYaw & 0b1111 ) * 0.12f;
			fCapsuleLength = ( pNode->uiPackedScaleYaw & 0b1111 ) * 0.04f;
		}

		// ---- "centre" bone: move sphere origin to bone world position ----
		iBone = pSkel->GetBoneID( "centre", 0 );
		if ( iBone != -1 && pSkel->GetBone( iBone ) != TNULL )
		{
			TMatrix44 mBone;
			mBone.Multiply( mWorld, pSkel->GetBone( iBone )->m_Transform );

			vOrigin = mBone.GetTranslation();
		}

		// ---- "end" bone: compute capsule axis direction and length ----
		iBone = pSkel->GetBoneID( "end", 0 );
		if ( iBone != -1 && pSkel->GetBone( iBone ) != TNULL )
		{
			TMatrix44 mBone;
			mBone.Multiply( mWorld, pSkel->GetBone( iBone )->m_Transform );
			TVector4 vEnd( mBone.m_f41, mBone.m_f42, mBone.m_f43, mBone.m_f44 );

			if ( !vEnd.IsEqual( vOrigin ) )
			{
				vCapsuleDir    = vEnd - vOrigin;
				fCapsuleLength = vCapsuleDir.NormaliseMag();
				bHasAxis       = TTRUE;
			}
		}

		// ---- "radius" bone: compute capsule/sphere radius ----
		TTransformObject             transformForNode;
		TTransformObject*            pTransformForNode = TNULL;
		ACollisionModelInstance*     pCollInstance     = TNULL;

		iBone = pSkel->GetBoneID( "radius", 0 );
		if ( iBone != -1 && pSkel->GetBone( iBone ) != TNULL )
		{
			TMatrix44 mBone;
			mBone.Multiply( mWorld, pSkel->GetBone( iBone )->m_Transform );
			TVector4 vRadiusPt( mBone.m_f41, mBone.m_f42, mBone.m_f43, mBone.m_f44 );

			// Far end of the capsule axis
			TVector4 vScaledDir;
			vScaledDir.Multiply( vCapsuleDir, fCapsuleLength );

			TVector4 vCapsuleEnd = vScaledDir + vOrigin;

			if ( bHasAxis )
			{
				if ( !vOrigin.IsEqual( vCapsuleEnd ) )
				{
					TFLOAT fDistSq = 0.0f;
// 					TFLOAT fDistSq = TMath::DistancePointToSegmentSq(
// 					    (TVector3*)&vRadiusPt,
// 					    (TVector3*)&vOrigin,
// 					    (TVector3*)&vCapsuleEnd
// 					);

					if ( fDistSq > 0.0001f )
					{
						fCapsuleRadius = TMath::Sqrt( fDistSq );

						if ( aCollisionFlags[ iIndex ] == InstanceFlags_WithCollision )
							fCapsuleRadius *= 1.2f;
					}
				}
			}
			else
			{
				// Sphere: distance from origin to radius bone
				TFLOAT fDistSq = TVector4::DistanceSq( vRadiusPt, vOrigin );
				if ( fDistSq > 0.0001f )
				{
					fCapsuleLength = 0.0f;
					fCapsuleRadius = TMath::Sqrt( fDistSq ) * 1.2f;

					if ( aCollisionFlags[ iIndex ] == InstanceFlags_WithCollision )
						fCapsuleRadius *= 1.2f;

					goto CreateNode; // no transform needed for a sphere shape
				}
			}

			// Set up a TTransformObject for shaped (capsule) collision nodes
			transformForNode.SetMatrix( TMatrix44::IDENTITY );
			transformForNode.SetScale( TVector3( 1.0f, 1.0f, 1.0f ) );

			if ( fScale != 1.0f )
			{
				TMatrix44 mScaled = mWorldUnscaled;
				mScaled.Scale( fScale, fScale, fScale );
				transformForNode.SetMatrix( mScaled );
			}
			else
			{
				transformForNode.SetMatrix( mWorldUnscaled );
			}

			transformForNode.SetScale( TVector3( fScale, fScale, fScale ) );
			pTransformForNode = &transformForNode;
		}
		else
		{
			// No radius bone — only InstanceFlags_WithCollision nodes get a transform
			if ( aCollisionFlags[ iIndex ] == InstanceFlags_WithCollision )
			{
				pCollInstance = ( aCollInstances[ uiModelIndex ] != 0 ) ? &aCollInstances[ uiModelIndex ]->GetNodeValue() : TNULL;

				if ( fScale != 1.0f )
				{
					TMatrix44 mScaled = mWorldUnscaled;
					mScaled.Scale( fScale, fScale, fScale );
					transformForNode.SetMatrix( mScaled );
				}
				else
				{
					transformForNode.SetMatrix( mWorldUnscaled );
				}

				transformForNode.SetScale( TVector3( fScale, fScale, fScale ) );
				pTransformForNode = &transformForNode;
			}
		}

	CreateNode:
		if ( !m_llFreeCollisionModelNodes.IsEmpty() )
		{
			ACollisionObjectModelNode* pCollNode = m_llFreeCollisionModelNodes.PopBack();

			if ( pCollNode != TNULL )
			{
				TVALIDPTR( pTargetList );
				pTargetList->llCollModelNodes.PushBack( pCollNode );

				TTODO( "FUN_0061e730 = ACollisionObjectModel::Create?" );
				// (origin, radius, length, direction, maxSubNodes, collInstHandle, transform)
// 				pCollNode->GetNodeValue().Create(
// 				    vOrigin,
// 				    fCapsuleRadius,
// 				    fCapsuleLength,
// 				    vCapsuleDir,
// 				    64,
// 				    iCollInstHandle,
// 				    pTransformForNode
// 				);
			}
		}
	}
}

// $Barnyard: FUNCTION 005e00f0
void AInstanceManager::DestroyCollisionSets()
{
	while ( !m_vecCollisionSets.IsEmpty() )
	{
		ACollisionModelSet* pModelSet = *m_vecCollisionSets.Back();
		delete pModelSet;

		m_vecCollisionSets.PopBack();
	}

	m_uiFlags &= ~2;
}

// $Barnyard: FUNCTION 005e0050
void AInstanceManager::ClearList( List& a_rList )
{
	while ( !a_rList.llCollModelNodes.IsEmpty() )
	{
		auto pLast = a_rList.llCollModelNodes.Back().Get();
		a_rList.llCollModelNodes.PopBack();

		ACollisionObjectModel* pModel = &pLast->GetNodeValue();

		m_llFreeCollisionModelNodes.PushBack( pLast );
		if ( pModel->IsCollisionObjectLinked() )
			pModel->UnlinkCollisionObject();
	}

	a_rList.pLocators = TNULL;
}

// $Barnyard: FUNCTION 005e0140
void AInstanceManager::SetupCollisions( TBOOL a_bCreateCollisionSets )
{
	const TBOOL bHasCollisionSets = ( ( m_uiFlags & 2 ) == 2 );

	// Check if new flag value is different
	if ( bHasCollisionSets == a_bCreateCollisionSets ) return;

	// Update flags
	if ( bHasCollisionSets ) m_uiFlags |= 2;
	else m_uiFlags &= ~2;

	if ( bHasCollisionSets )
	{
		if ( m_llFreeCollisionModelInstances.IsEmpty() )
		{
			// Need to (re)allocate model instance nodes
			if ( m_pCollisionModelInstances ) delete[] m_pCollisionModelInstances;

			// Create nodes
			m_pCollisionModelInstances = new ACollisionModelInstanceNode[ 34 ];

			// Add all to the free list
			for ( TINT i = 0; i < 34; i++ )
				m_llFreeCollisionModelInstances.PushFront( &m_pCollisionModelInstances[ i ] );
		}

		// Create collision sets for the models
		for ( TINT i = 0; i < m_iNumModels; i++ )
		{
			TModel*    pModel         = m_pModels[ i ].pManagedModel->GetModel();
			const TINT iNumCollMeshes = pModel->GetNumCollisionMeshes();

			if ( iNumCollMeshes > 0 )
			{
				ACollisionModelSet* pCollModelSet = new ACollisionModelSet;
				pCollModelSet->Create( pModel );

				m_vecCollisionSets.PushBack( pCollModelSet );
			}
		}
	}
	else
	{
		// Doesn't need collision sets anymore, destroy them
		m_llFreeCollisionModelInstances.Clear();
		DestroyCollisionSets();
	}
}

// $Barnyard: FUNCTION 005dfc20
AInstanceManager::Model::~Model()
{
	Reset();
}

void AInstanceManager::Model::Reset()
{
	if ( pSceneObject ) pSceneObject->Delete();
	if ( pManagedModel ) delete pManagedModel;
}

// T2SList:
// $Barnyard: FUNCTION 006c7e10
// $Barnyard: FUNCTION 006c7e60
// $Barnyard: FUNCTION 006c7e30
