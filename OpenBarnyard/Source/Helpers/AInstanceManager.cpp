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
    , m_uiModelsFlags( 0 )
    , m_iNumKeyLibraries( 0 )
    , m_flUnk1( 30.0f )
    , m_flUnk2( 85.0f )
    , m_flUnk3( 75.0f )
{
	m_iNumModels = NUM_MODELS;
	m_pModels    = new Model[ NUM_MODELS ];

	for ( TSIZE i = 0; i < TARRAYSIZE( m_aS1 ); i++ )
		m_llFreeS1.PushFront( &m_aS1[ i ] );

	m_pSomeBuffer = TMalloc( m_iNumModels << 3 );
}

AInstanceManager::~AInstanceManager()
{
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
		m_uiModelsFlags |= 4;
		m_uiModelsFlags |= 12;
	}
	else
	{
		m_uiModelsFlags &= ~4;
		m_uiModelsFlags &= ~12;
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

// $Barnyard: FUNCTION 005e00f0
void AInstanceManager::DestroyCollisionSets()
{
	while ( !m_vecCollisionSets.IsEmpty() )
	{
		ACollisionModelSet* pModelSet = *m_vecCollisionSets.Back();
		delete pModelSet;

		m_vecCollisionSets.PopBack();
	}

	m_uiModelsFlags &= ~2;
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

	a_rList.iUnknown = 0;
}

// $Barnyard: FUNCTION 005e0140
void AInstanceManager::SetupCollisions( TBOOL a_bCreateCollisionSets )
{
	const TBOOL bHasCollisionSets = ( ( m_uiModelsFlags & 2 ) == 2 );

	// Check if new flag value is different
	if ( bHasCollisionSets == a_bCreateCollisionSets ) return;

	// Update flags
	if ( bHasCollisionSets ) m_uiModelsFlags |= 2;
	else m_uiModelsFlags &= ~2;

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
