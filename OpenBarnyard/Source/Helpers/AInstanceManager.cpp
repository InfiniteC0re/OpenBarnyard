#include "pch.h"
#include "AInstanceManager.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "ALoadScreen.h"
#include "Render/AGlowViewport.h"
#include "Helpers/ARegrowthManager.h"
#include "Helpers/AInstanceLightingHelper.h"
#include "MiniGame/ABarnExploreMinigameState.h"

#include "Render/ABackend.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

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

static TMatrix44 s_mInstanceTransform = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

constexpr TFLOAT INSTANCE_RENDER_DISTANCE_MULTIPLIER = 1.2f;

// $Barnyard: FUNCTION 005dfd10
AInstanceManager::ACollisionObjectModelNode::ACollisionObjectModelNode()
{
}

// $Barnyard: FUNCTION 005dfac0
// $Barnyard: FUNCTION 005dfb60
// $Barnyard: FUNCTION 005dfd00
AInstanceManager::ACollisionObjectModelNode::~ACollisionObjectModelNode()
{
}

// $Barnyard: FUNCTION 005dfd60
AInstanceManager::AInstanceManager()
    : m_ppKeyLibs( TNULL )
    , m_pModels( TNULL )
    , m_pModelsRenderLists( TNULL )
    , m_iNumDynamicInstances( 0 )
    , m_pDynamicInstances( TNULL )
    , m_iInstanceLibIndex( -1 )
    , m_pCollObjectModelNodes( TNULL )
    , m_pCollisionModelInstances( TNULL )
    , m_uiFlags( FLAGS_NONE )
    , m_iNumKeyLibraries( 0 )
    , m_flLOD0Distance( 30.0f )
    , m_flVisibilityDistanceBlendEnd( 85.0f )
    , m_flVisibilityDistanceBlendStart( 75.0f )
{
	m_iNumModels = NUM_MODELS;
	m_pModels    = new Model[ NUM_MODELS ];

	for ( TSIZE i = 0; i < TARRAYSIZE( m_aInstances ); i++ )
		m_llFreeInstances.PushFront( &m_aInstances[ i ] );

	m_pModelsRenderLists = new RenderListEntry*[ m_iNumModels * MAX_NUM_LODS ];
}

// $Barnyard: FUNCTION 005e1d80
AInstanceManager::~AInstanceManager()
{
	m_llFreeInstances.Clear();
	Reset();
}

// $Barnyard: FUNCTION 005e1e80
TBOOL AInstanceManager::LoadModels( TINT a_iInstanceLibIndex, TINT a_iNumModels, TUINT* a_pModelIndices, TBOOL a_bCreateCollisionModelNodes, TBOOL a_bCreateCollisionModelInstances, TINT a_iNumCollisionModelNodes )
{
	TASSERT( a_iInstanceLibIndex > -1 );

	// Update flags
	if ( a_bCreateCollisionModelNodes )
	{
		m_uiFlags |= FLAGS_COLLISION_NODES;
		m_uiFlags |= FLAGS_COLLISION_NODES | FLAGS_UNK3;
	}
	else
	{
		m_uiFlags &= ~FLAGS_COLLISION_NODES;
		m_uiFlags &= ~( FLAGS_COLLISION_NODES | FLAGS_UNK3 );
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
		TSceneObject* pSceneObject              = pManagedModel->CreateSceneObject();
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
		for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
		{
			TModelLOD* pLOD = &pModel->GetLOD( i );

			for ( TINT k = 0; k < pLOD->iNumMeshes; k++ )
			{
				TMesh*     pMesh     = pLOD->ppMeshes[ k ];
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

// $Barnyard: FUNCTION 005e1a80
AInstanceManager::StaticInstanceEntry* AInstanceManager::FindInstance( TUINT8 a_uiModelIndex, TINT a_iIndex )
{
	for ( auto it = m_llUsedInstances.Begin(); it != m_llUsedInstances.End(); it++ )
	{
		if ( it->uiModelIndex == a_uiModelIndex && --a_iIndex < 0 )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005dfc60
void AInstanceManager::GetInstanceBoneTransform( TINT a_iModelIndex, StaticInstanceEntry* a_pInstance, TINT a_iBone, TMatrix44& a_rOutTransform )
{
	TMatrix44 mLocator;
	a_pInstance->pLocatorList->GetLocator( a_pInstance->uiLocatorIndex )->GetMatrix( mLocator );

	TMatrix44 mWorld;
	mWorld.Multiply( s_mInstanceTransform, mLocator );

	TSkeleton* pSkeleton = m_pModels[ a_iModelIndex ].pManagedModel->GetModel()->GetSkeleton();
	a_rOutTransform.Multiply( mWorld, pSkeleton->GetBone( a_iBone )->GetTransform() );
}

// $Barnyard: FUNCTION 005dff30
void AInstanceManager::CreateDynamicInstances( TINT a_iNumInstances )
{
	m_iNumDynamicInstances = a_iNumInstances;
	m_pDynamicInstances    = new DynamicInstanceEntry[ a_iNumInstances ];

	for ( TINT i = 0; i < a_iNumInstances; i++ )
		m_llFreeDynamicInstances.PushFront( &m_pDynamicInstances[ i ] );
}

// $Barnyard: FUNCTION 005dffe0
void AInstanceManager::DestroyDynamicInstances()
{
	m_llFreeDynamicInstances.Clear();
	m_llUsedDynamicInstances.Clear();

	delete[] m_pDynamicInstances;

	m_uiFlags &= ~FLAGS_UNK0;
	m_pDynamicInstances    = TNULL;
	m_iNumDynamicInstances = 0;
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
	g_pRender->BeginEndSceneHAL();

	// Destroy models
	for ( TINT i = 0; i < m_iNumModels; i++ )
		m_pModels[ i ].Reset();

	// Destroy key libs
	if ( m_ppKeyLibs )
	{
		for ( TINT i = 0; i < m_iNumKeyLibraries; i++ )
		{
			g_pRender->GetKeyframeLibraryManager().UnloadLibrary( m_ppKeyLibs[ i ] );
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

static AInstanceManager::RenderListEntry s_aRenderList[ AInstanceManager::MAX_RENDERED_INSTANCES ];

// $Barnyard: FUNCTION 005e17a0
TBOOL AInstanceManager::Render()
{
	TRenderContext* pRenderContext = g_pRender->GetCurrentContext();
	TMatrix44       matModelView   = pRenderContext->GetModelViewMatrix();

	// Reset render lists
	for ( TINT i = 0; i < m_iNumModels * MAX_NUM_LODS; i++ )
		m_pModelsRenderLists[ i ] = TNULL;

	const TFLOAT flVisibilityDistanceBlendStart = m_flVisibilityDistanceBlendStart;
	const TFLOAT flBlendFactor                  = 1.0f / ( m_flVisibilityDistanceBlendEnd - flVisibilityDistanceBlendStart );

	TINT iRenderListSize = 0;
	if ( m_uiFlags & FLAGS_INTERIOR )
	{
		// Don't check terrain section visibility

		// Static instances
		FillRenderList_All( m_llUsedInstances, matModelView, s_aRenderList, iRenderListSize, TFALSE );

		// Dynamic instances
		if ( m_pDynamicInstances ) FillRenderList_All( m_llUsedDynamicInstances, matModelView, s_aRenderList, iRenderListSize, TTRUE );
	}
	else
	{
		// Check terrain section visibility

		// Static instances
		FillRenderList_Visible( m_llUsedInstances, matModelView, s_aRenderList, iRenderListSize, TFALSE );

		// Dynamic instances
		if ( m_pDynamicInstances ) FillRenderList_Visible( m_llUsedDynamicInstances, matModelView, s_aRenderList, iRenderListSize, TTRUE );
	}

	const TUINT uiOldClipFlags = pRenderContext->GetClipFlags();

	// Calculate light direction matrices
	TMatrix44 matLightDir = g_pRender->GetLightDirection();

	TMatrix44 aLightMatrices[ 16 ];
	for ( TINT i = 0; i < TARRAYSIZE( aLightMatrices ); i++ )
	{
		aLightMatrices[ i ].ScaleAll( matLightDir, i * ( 1.0f / ( TARRAYSIZE( aLightMatrices ) - 1 ) ) );
	}

	const TFLOAT flOldShadeCoeff = pRenderContext->GetShadeCoeff();

	pRenderContext->SetShadeCoeff( AInstanceLightingHelper::GetSingleton()->GetShadeCoeff() );

	// Render everything
	for ( TINT i = 0; i < m_iNumModels * MAX_NUM_LODS; i++ )
	{
		if ( !m_pModelsRenderLists[ i ] ) continue;

		TModelInstance* pModelInstance = m_pModels[ i / MAX_NUM_LODS ].pSceneObject->GetInstance();
		TModel*         pModel         = pModelInstance->GetModel();

		const TINT iLODId = TMath::Min( i % MAX_NUM_LODS, pModel->GetNumLODs() - 1 );
		TModelLOD* pLOD   = &pModel->GetLOD( iLODId );

		pRenderContext->SetSkeletonInstance( pModelInstance->GetSkeletonInstance() );

		// Render every instance of this model
		RenderListEntry* pRenderList = m_pModelsRenderLists[ i ];
		while ( pRenderList )
		{
			// Set blending or don't draw instance that is completely blend out
			if ( pRenderList->matTransform.m_f43 >= flVisibilityDistanceBlendStart )
			{
				TFLOAT fBlend = ( pRenderList->matTransform.m_f43 - flVisibilityDistanceBlendStart ) * flBlendFactor;

				if ( fBlend < 1.0f )
				{
					pRenderContext->SetAlphaBlend( 1.0f - fBlend );
				}
				else
				{
					pRenderList = pRenderList->pNext;
					continue;
				}
			}
			else
			{
				pRenderContext->SetAlphaBlend( 1.0f );
			}

			pRenderContext->SetClipFlags( m_aInstanceVisMasks[ pRenderList - s_aRenderList ] );
			pRenderContext->SetModelViewMatrix( pRenderList->matTransform );

			for ( TINT k = 0; k < pLOD->iNumMeshes; k++ )
			{
				pLOD->ppMeshes[ k ]->Render();
			}

			pRenderList = pRenderList->pNext;
		}
	}

	// Restore context state
	pRenderContext->SetShadeCoeff( flOldShadeCoeff );
	pRenderContext->SetClipFlags( uiOldClipFlags );
	pRenderContext->SetAlphaBlend( 1.0f );
	pRenderContext->SetModelViewMatrix( matModelView );
	return TTRUE;
}

// $Barnyard: FUNCTION 005e0300
// $BarnyardWiiJP: FUNCTION 8037463c
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
	TINT iNumCollisionModels        = 0; // unique model slots needing a collision instance
	TINT iNumInstancesWithCollision = 0; // total instances queued (early-out guard)

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

		if ( iModelIndex == TUINT8( -1 ) ) continue;

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

		if ( ARegrowthManager::IsSingletonCreated() && HASANYFLAG( ARegrowthManager::GetSingleton()->GetFlags(), ARegrowthManager::FLAGS_IN_USE ) )
		{
			// NOTE: retail doesn't stop at the first match
			for ( TSIZE f = 0; f < TARRAYSIZE( s_apszRegrowthNames ); f++ )
			{
				TSIZE uLen = TStringManager::String8Length( s_apszRegrowthNames[ f ] );

				if ( TStringManager::String8CompareNoCase( s_apszRegrowthNames[ f ], pszName, uLen ) == 0 )
					bIsRegrowthObject = TTRUE;
			}
		}

		if ( bIsRegrowthObject ) continue;

		// ---- Validate prerequisites ----
		Model& rModel = m_pModels[ iModelIndex ];

		// FLAGS_UNK0: model must be loaded before an instance can be created
		if ( ( m_uiFlags & FLAGS_UNK0 ) && rModel.pManagedModel == TNULL ) continue;

		// Must have a free instance node and a valid model pointer
		if ( m_llFreeInstances.IsEmpty() || rModel.pManagedModel == TNULL ) continue;

		TModel* pModel = rModel.pManagedModel->GetModel();

		// ---- Setup collision type if needed ----
		if ( ( m_uiFlags & FLAGS_COLLISION_NODES ) && ( m_uiFlags & FLAGS_UNK3 ) )
		{
			const TBOOL bHasCollisionSet = ( m_uiFlags & FLAGS_COLLISION_SETS ) && ( apModelCollSets[ iModelIndex ] != TNULL ) && ( pModel->GetNumCollisionMeshes() > 0 );

			// "radius" bone presence is a secondary collision indicator
			const TINT iBoneRadius = pModel->GetSkeleton()->GetBoneID( "radius", 0 );

			// Names that always get a capsule collision
			const TBOOL bForceCapsule =
			    T2String8::FindString( pszName, "_coll" ) != TNULL ||
			    T2String8::FindString( pszName, "bale01" ) != TNULL ||
			    T2String8::FindString( pszName, "rock01" ) != TNULL;

			// Names that never get a collision
			const TBOOL bNoCollision =
			    T2String8::FindString( pszName, "FenceRBrown" ) != TNULL ||
			    T2String8::FindString( pszName, "FenceRGrey" ) != TNULL ||
			    T2String8::FindString( pszName, "roadbarrier" ) != TNULL ||
			    T2String8::FindString( pszName, "car_treewall" ) != TNULL
#ifdef BARNYARD_LATEST
			    // Latest version: the road signs lose their collision too
			    || T2String8::FindString( pszName, "sign_bump" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_left" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_right" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_s_bend1" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_s_bend2" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_cow" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_cross" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_stop" ) != TNULL ||
			    T2String8::FindString( pszName, "sign_speed" ) != TNULL
#endif
			    ;

			// NOTE: the instance itself is always created, this only decides which collision it gets
			if ( ( !bHasCollisionSet && iBoneRadius == -1 && !bForceCapsule ) || ( pLocator->iFlags2 < 0 ) || bNoCollision )
			{
				*( pCollisionFlagCursor++ ) = InstanceFlags_None;
			}
			else
			{
				if ( bHasCollisionSet )
				{
					*( pCollisionFlagCursor++ ) = InstanceFlags_WithCollision;

					// Only allocate one collision model instance per unique model slot
					if ( !abCollInstAllocated[ iModelIndex ] )
					{
						abCollInstAllocated[ iModelIndex ] = TTRUE;
						iNumCollisionModels++;
					}
				}
				else
				{
					// Capsule collision only
					*( pCollisionFlagCursor++ ) = InstanceFlags_RenderOnly;
				}

				iNumInstancesWithCollision++;
			}
		}

		// Re encode scale and rotation (truncating, like retail)
		const TUINT8 uiScale20     = TUINT8( ( pLocator->uiFlags1 & 0b1111 ) * 20.0 );
		const TUINT8 uiYaw6        = TUINT8( ( pLocator->uiFlags1 >> 4 ) * 6.6666665 );
		const TUINT8 uiScalePacked = TUINT8( TINT( TINT( uiScale20 ) * 0.050000001f ) );
		const TUINT8 uiYawPacked   = TUINT8( TINT( uiYaw6 * 0.14999999f ) );

		// Pop a free instance node
		StaticInstanceEntry* pNode = m_llFreeInstances.PopFront();

		// Pack instance metadata into the node
		pNode->pLocatorList     = a_pLocatorList;
		pNode->uiLocatorIndex   = iLocIdx;
		pNode->uiModelIndex     = iModelIndex;
		pNode->uiPackedScaleYaw = ( uiYawPacked << 4 ) | ( uiScalePacked != 0 ? uiScalePacked : 5 );

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

		// NOTE: the locator position is Z up, the Z is negated and becomes Y
		const TFLOAT fLocX = pLoc->vecPosition.x;
		const TFLOAT fLocY = -pLoc->vecPosition.z;
		const TFLOAT fLocZ = pLoc->vecPosition.y;

		const TUINT8 uiModelIndex = pNode->uiModelIndex;
		TModel*      pModel       = m_pModels[ uiModelIndex ].pManagedModel->GetModel();

		// ---- LOD0 bounding sphere ----
		TVector4 vOrigin( pModel->GetLOD( 0 ).BoundingSphere.GetOrigin() );

		const TFLOAT fScale = (TFLOAT)( pNode->uiPackedScaleYaw & 0b1111 ) * 0.2f;

		// ---- Capsule shape parameters ----
		TVector4 vCapsuleDir    = TVector4::VEC_NEGY;
		TFLOAT   fCapsuleLength = 0.0f;
		TFLOAT   fCapsuleRadius = TFLOAT( pNode->uiPackedScaleYaw & 0b1111 ) * pModel->GetLOD( 0 ).BoundingSphere.GetRadius() * 0.23f;
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
		// Only the WithCollision nodes use the collision mesh of the model (and need a transform),
		// the others use the capsule as the collision
		TTransformObject         transformForNode;
		TTransformObject*        pTransformForNode = TNULL;
		ACollisionModelInstance* pCollInstance     = TNULL;

		transformForNode.SetMatrix( TMatrix44::IDENTITY );
		transformForNode.SetScale( TVector3( 1.0f, 1.0f, 1.0f ) );

		iBone = pSkel->GetBoneID( "radius", 0 );
		if ( iBone != -1 && pSkel->GetBone( iBone ) != TNULL )
		{
			TMatrix44 mBone;
			mBone.Multiply( mWorld, pSkel->GetBone( iBone )->m_Transform );
			TVector4 vRadiusPt( mBone.m_f41, mBone.m_f42, mBone.m_f43, mBone.m_f44 );

			// Far end of the capsule axis
			TVector4 vScaledDir;
			vScaledDir.Multiply( vCapsuleDir, fCapsuleLength );

			TVector4 vCapsuleEnd( vScaledDir.x + vOrigin.x, vScaledDir.y + vOrigin.y, vScaledDir.z + vOrigin.z, vScaledDir.w );

			if ( bHasAxis )
			{
				if ( !vOrigin.IsEqual( vCapsuleEnd ) )
				{
					TFLOAT fDistSq = TMath::DistancePointToSegmentSq( vRadiusPt, vOrigin.AsVector3(), vCapsuleEnd.AsVector3() );

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
				}
			}
		}

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

		// NOTE: retail doesn't create the collision if there are no free nodes left
		if ( !m_llFreeCollisionModelNodes.IsEmpty() )
		{
			ACollisionObjectModelNode* pCollNode = m_llFreeCollisionModelNodes.PopBack();

			if ( pCollNode != TNULL )
			{
				TVALIDPTR( pTargetList );
				pTargetList->llCollModelNodes.PushBack( pCollNode );

				pCollNode->Create( vOrigin, fCapsuleRadius, fCapsuleLength, vCapsuleDir, ACollisionObject::TYPE_INSTANCE, pCollInstance, pTransformForNode );
			}
		}
	}
}

// $Barnyard: FUNCTION 005e14d0
void AInstanceManager::FillRenderList_All( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic )
{
	const TFLOAT flLOD0DistanceSq      = m_flLOD0Distance * m_flLOD0Distance;
	const TFLOAT flMaxRenderDistanceSq = m_flVisibilityDistanceBlendEnd * m_flVisibilityDistanceBlendEnd;

	TRenderContext*  pRenderContext = g_pRender->GetCurrentContext();
	const TMatrix44& matViewWorld   = pRenderContext->GetViewWorldMatrix();
	TVector4         vecCameraPos   = matViewWorld.GetTranslation();

	T2_FOREACH( a_rcInstanceList, it )
	{
		const TVector3* pInstancePosition = ( a_bDynamic ) ? &TSTATICCAST( DynamicInstanceEntry, it.Get() )->matTransform.GetTranslation3() : &it->pLocatorList->GetLocator( it->uiLocatorIndex )->vecPosition;

		// Check whether the instance is visible
		TSphere vecInstanceBounding(
		    pInstancePosition->x,
		    -pInstancePosition->z,
		    pInstancePosition->y,
		    INSTANCE_RENDER_DISTANCE_MULTIPLIER * m_pModels[ it->uiModelIndex ].pManagedModel->GetModel()->GetRenderDistance()
		);

		const TFLOAT flInstanceDistanceSq = TVector4::DistanceSq( vecInstanceBounding.AsVector4(), vecCameraPos );

		// Only the instances of the barn floor the player is on: the loft is above y = 3.246
		const TINT iBarnFloor = ABarnExploreMinigameState::ms_iFloor;

		if ( flInstanceDistanceSq < flMaxRenderDistanceSq &&
		     ( vecInstanceBounding.GetOrigin().y <= 3.246f || iBarnFloor == ABarnExploreMinigameState::FLOOR_LOFT ) &&
		     ( vecInstanceBounding.GetOrigin().y >= 3.246f || iBarnFloor == ABarnExploreMinigameState::FLOOR_GROUND || iBarnFloor == ABarnExploreMinigameState::FLOOR_BASEMENT ) )
		{
			TINT iLODIndex = flInstanceDistanceSq >= flLOD0DistanceSq ? 1 : 0;

			const TBOOL bVisibleInFrustum = pRenderContext->CullSphereToFrustumSimple( vecInstanceBounding, pRenderContext->GetWorldPlanes(), 6 );
			if ( bVisibleInFrustum )
			{
				TMatrix44 matLocatorLocalTransform;

				if ( a_bDynamic )
					matLocatorLocalTransform = TSTATICCAST( DynamicInstanceEntry, it.Get() )->matTransform;
				else
					it->pLocatorList->GetLocator( it->uiLocatorIndex )->GetMatrix( matLocatorLocalTransform );

				if ( a_rNumInstances >= MAX_RENDERED_INSTANCES ) return;

				// Collect light sources
				Toshi::TLightIDList oLightList;
				AGlowViewport::GetSingleton()->GetInfluencingLightIDs( vecInstanceBounding, oLightList );

				// Store data about the instance
				m_aInstanceVisMasks[ a_rNumInstances ] = 0x3F; // clip flags: all six frustum planes
				m_aInstanceFlags[ a_rNumInstances ]    = it->uiPackedScaleYaw >> 4;
				m_aInstanceLightIDs[ a_rNumInstances ] = oLightList[ 0 ];

				const TINT   iModelSlot     = ( it->uiModelIndex * MAX_NUM_LODS ) + iLODIndex;
				const TFLOAT flLocatorScale = ( it->uiPackedScaleYaw & 0b1111 ) * 0.2f;

				// Scale and apply global transform
				TMatrix44 matLocatorTransform;
				matLocatorTransform.Multiply( s_mInstanceTransform, matLocatorLocalTransform );
				matLocatorTransform.Scale( flLocatorScale, flLocatorScale, flLocatorScale );

				// Add to the render list, applying world transform
				RenderListEntry* pRenderListEntry = &a_pOutRenderData[ a_rNumInstances++ ];
				pRenderListEntry->matTransform.Multiply( a_rcWorldTransform, matLocatorTransform );
				pRenderListEntry->pNext = m_pModelsRenderLists[ iModelSlot ];

				m_pModelsRenderLists[ iModelSlot ] = pRenderListEntry;
			}
		}
	}
}

// $Barnyard: FUNCTION 005e10e0
void AInstanceManager::FillRenderList_Visible( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic )
{
	//-----------------------------------------------------------------------------
	// 1. Gather visible sections
	//-----------------------------------------------------------------------------

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

	//-----------------------------------------------------------------------------
	// 2. Fill the render list
	//-----------------------------------------------------------------------------

	const TFLOAT flLOD0DistanceSq      = m_flLOD0Distance * m_flLOD0Distance;
	const TFLOAT flMaxRenderDistanceSq = m_flVisibilityDistanceBlendEnd * m_flVisibilityDistanceBlendEnd;

	TRenderContext*  pRenderContext = g_pRender->GetCurrentContext();
	const TMatrix44& matViewWorld   = pRenderContext->GetViewWorldMatrix();
	TVector4         vecCameraPos   = matViewWorld.GetTranslation();

	T2_FOREACH( a_rcInstanceList, it )
	{
		const TUINT uiLocatorIndex = it->uiLocatorIndex;

		TUINT uiNumRenderedSections = 0;
		for ( TUINT i = 0; i < uiNumSectionsToRender; i++ )
		{
			const SectionRenderData& rSectionData = aSectionRenderData[ i ];

			if ( uiLocatorIndex >= rSectionData.uiStart && uiLocatorIndex < rSectionData.uiEnd )
			{
				// Skip all other sections if rendered the required number
				if ( uiNumRenderedSections >= uiNumSectionsToRender ) break;

				const TVector3* pInstancePosition = ( a_bDynamic ) ? &TSTATICCAST( DynamicInstanceEntry, it.Get() )->matTransform.GetTranslation3() : &it->pLocatorList->GetLocator( it->uiLocatorIndex )->vecPosition;

				// Check whether the instance is visible
				TSphere vecInstanceBounding(
				    pInstancePosition->x,
				    -pInstancePosition->z,
				    pInstancePosition->y,
				    INSTANCE_RENDER_DISTANCE_MULTIPLIER * m_pModels[ it->uiModelIndex ].pManagedModel->GetModel()->GetRenderDistance()
				);

				// Models 44 and 45 of the instance library get a bigger visibility radius
				if ( it->uiModelIndex == 44 || it->uiModelIndex == 45 )
					vecInstanceBounding.GetRadius() *= 1.75f;

				const TFLOAT flInstanceDistanceSq = TVector4::DistanceSq( vecInstanceBounding.AsVector4(), vecCameraPos );

				if ( flInstanceDistanceSq < flMaxRenderDistanceSq )
				{
					TINT iLODIndex = flInstanceDistanceSq >= flLOD0DistanceSq ? 1 : 0;

					const TBOOL bVisibleInFrustum = pRenderContext->CullSphereToFrustumSimple( vecInstanceBounding, pRenderContext->GetWorldPlanes(), 6 );
					if ( bVisibleInFrustum )
					{
						TMatrix44 matLocatorLocalTransform;

						if ( a_bDynamic )
							matLocatorLocalTransform = TSTATICCAST( DynamicInstanceEntry, it.Get() )->matTransform;
						else
							it->pLocatorList->GetLocator( it->uiLocatorIndex )->GetMatrix( matLocatorLocalTransform );

						if ( a_rNumInstances >= MAX_RENDERED_INSTANCES ) return;

						// Collect light sources
						Toshi::TLightIDList oLightList;
						AGlowViewport::GetSingleton()->GetInfluencingLightIDs( vecInstanceBounding, oLightList );

						// Store data about the instance
						m_aInstanceVisMasks[ a_rNumInstances ] = 0x3F; // clip flags: all six frustum planes
						m_aInstanceFlags[ a_rNumInstances ]    = it->uiPackedScaleYaw >> 4;
						m_aInstanceLightIDs[ a_rNumInstances ] = oLightList[ 0 ];

						const TINT   iModelSlot     = ( it->uiModelIndex * MAX_NUM_LODS ) + iLODIndex;
						const TFLOAT flLocatorScale = ( it->uiPackedScaleYaw & 0b1111 ) * 0.2f;

						// Scale and apply global transform
						TMatrix44 matLocatorTransform;
						matLocatorTransform.Multiply( s_mInstanceTransform, matLocatorLocalTransform );
						matLocatorTransform.Scale( flLocatorScale, flLocatorScale, flLocatorScale );

						// Add to the render list, applying world transform
						RenderListEntry* pRenderListEntry = &a_pOutRenderData[ a_rNumInstances++ ];
						pRenderListEntry->matTransform.Multiply( a_rcWorldTransform, matLocatorTransform );
						pRenderListEntry->pNext = m_pModelsRenderLists[ iModelSlot ];

						m_pModelsRenderLists[ iModelSlot ] = pRenderListEntry;
					}
				}
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

	m_uiFlags &= ~FLAGS_COLLISION_SETS;
}

// $Barnyard: FUNCTION 005e0f80
void AInstanceManager::DestroyInstances( ATerrainLocatorList* a_pLocatorList )
{
	// Collisions
	for ( TSIZE i = 0; i < TARRAYSIZE( m_aLists ); i++ )
	{
		if ( m_aLists[ i ].pLocators == a_pLocatorList )
		{
			ClearList( m_aLists[ i ] );
			break;
		}
	}

	// Instances
	for ( auto it = m_llUsedInstances.Begin(); it != m_llUsedInstances.End(); )
	{
		StaticInstanceEntry* pInstance = it;
		it                             = it->Next();

		if ( pInstance->pLocatorList == a_pLocatorList )
		{
			m_llUsedInstances.Erase( pInstance );
			m_llFreeInstances.PushFront( pInstance );
		}
	}

	// NOTE: the original also returns these to m_llFreeInstances
	for ( auto it = m_llOtherInstances_Unused.Begin(); it != m_llOtherInstances_Unused.End(); )
	{
		StaticInstanceEntry* pInstance = it;
		it                             = it->Next();

		if ( pInstance->pLocatorList == a_pLocatorList )
		{
			m_llOtherInstances_Unused.Erase( pInstance );
			m_llFreeInstances.PushFront( pInstance );
		}
	}

	// Dynamic instances
	for ( auto it = m_llUsedDynamicInstances.Begin(); it != m_llUsedDynamicInstances.End(); )
	{
		StaticInstanceEntry* pInstance = it;
		it                             = it->Next();

		if ( pInstance->pLocatorList == a_pLocatorList )
		{
			m_llUsedDynamicInstances.Erase( pInstance );
			m_llFreeDynamicInstances.PushFront( pInstance );
		}
	}
}

// $Barnyard: FUNCTION 005e0050
void AInstanceManager::ClearList( List& a_rList )
{
	while ( !a_rList.llCollModelNodes.IsEmpty() )
	{
		auto pLast = a_rList.llCollModelNodes.Back().Get();
		a_rList.llCollModelNodes.PopBack();

		ACollisionObjectModel* pModel = pLast;

		m_llFreeCollisionModelNodes.PushBack( pLast );
		if ( pModel->IsCollisionObjectLinked() )
			pModel->UnlinkCollisionObject();
	}

	while ( !a_rList.llCollModelInstances.IsEmpty() )
	{
		ACollisionModelInstanceNode* pLast = a_rList.llCollModelInstances.Back().Get();
		a_rList.llCollModelInstances.PopBack();

		m_llFreeCollisionModelInstances.PushBack( pLast );
	}

	a_rList.pLocators = TNULL;
}

// $Barnyard: FUNCTION 005e0140
void AInstanceManager::SetupCollisions( TBOOL a_bCreateCollisionSets )
{
	const TBOOL bHasCollisionSets = ( ( m_uiFlags & FLAGS_COLLISION_SETS ) == FLAGS_COLLISION_SETS );

	// Check if new flag value is different
	if ( bHasCollisionSets == a_bCreateCollisionSets ) return;

	// Update flags
	if ( a_bCreateCollisionSets ) m_uiFlags |= FLAGS_COLLISION_SETS;
	else m_uiFlags &= ~FLAGS_COLLISION_SETS;

	if ( a_bCreateCollisionSets )
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
			if ( m_pModels[ i ].pManagedModel == TNULL ) continue;

			TModel* pModel = m_pModels[ i ].pManagedModel->GetModel();

			if ( pModel->GetNumCollisionMeshes() > 0 )
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

// NOTE: inlined in AInstanceManager::Reset (005e1ae0)
void AInstanceManager::Model::Reset()
{
	if ( pSceneObject ) pSceneObject->Delete();
	if ( pManagedModel ) delete pManagedModel;

	// Reset can be called again (e.g. LoadModels resets after LoadMiniGameImpl already did), don't free twice
	pManagedModel = TNULL;
	pSceneObject  = TNULL;
}

// T2SList:
// $Barnyard: FUNCTION 006c7e10
// $Barnyard: FUNCTION 006c7e60
// $Barnyard: FUNCTION 006c7e30
