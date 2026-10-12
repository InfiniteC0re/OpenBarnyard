#include "pch.h"
#include "ARegrowthManager.h"
#include "ASimPersistData.h"
#include "AGameTimeManager.h"
#include "AInstanceLightingHelper.h"
#include "ALoadScreen.h"
#include "ARandom.h"
#include "AWeightedRandom.h"
#include "FSM/AMethodPointer.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "Render/ASkinShader/ASkinMaterial.h"
#include "Render/ARenderer.h"
#include "Physics/ACollisionModelSet.h"
#include "Physics/ACollisionManager.h"
#include "MiniGame/ABarnExploreMinigameState.h"

#include <Toshi/T2String8.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderContext.h>
#include <Render/TModel.h>
#include <Render/TSkeleton.h>
#include <Math/TSphere.h>

#ifdef TRENDERINTERFACE_DX8
#  include "Platform/DX8/ASkinShader/ASkinMaterial_DX8.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail converts through a 64-bit integer and keeps the low 32 bits. Objects grown from the start have a
// stage far beyond the TINT range, which a plain TINT conversion would turn into 0x80000000 (not grown)
static TINT StageToInt( TFLOAT a_fStage )
{
	return TINT( TINT64( a_fStage ) );
}

TPSTRING8_DECLARE( fruit );
TPSTRING8_DECLARE( crate );
TPSTRING8_DECLARE( light );
TPSTRING8_DECLARE( heavy );
TPSTRING8_DECLARE( milk );
TPSTRING8_DECLARE( dayrespawn );
TPSTRING8_DECLARE( non_trees );
TPSTRING8_DECLARE( defaulttree );

// NOTE: GetClass 005e22c0, CreateObject 005e49d0, CreateObjectInPlace 005e4a30
TDEFINE_CLASS_INIT( ARegrowthManager, 1, 0 );

// DAT_00792280
AFunctionTable ARegrowthManager::ms_oFunctionTable( &TGetClass( ARegrowthManager ), TNULL );

// DAT_007962e8
ARegrowthManager::Types* ARegrowthManager::ms_pTypes = TNULL;

// "ModelNames" of the TRB (DAT_00792278)
static const TCHAR** s_ppModelNames;

// The objects that get a collision while the objects of a group are created (DAT_0079227c, 8000 bytes)
static ARegrowthManager::CollisionEntry* s_pCollisionEntries;

// DAT_007922e8
static ARegrowthManager::RenderData s_aRenderData[ ARegrowthManager::MAX_NUM_RENDERED_OBJECTS ];

// The clip flags of the rendered objects, not used (DAT_00791eb8)
static TUINT s_auiRenderClipFlags[ ARegrowthManager::MAX_NUM_RENDERED_OBJECTS ];

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 005e2740
static TObject* __stdcall ARegrowthManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return ARegrowthManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005e6040
void ARegrowthManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ARegrowthManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer3ArgsRT<TINT, ARegrowthManager, TINT, TINT, TFLOAT>::Create( "GrowFruits", &ARegrowthManager::GrowFruits ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TINT, ARegrowthManager, TINT>::Create( "FreeFruitSlotCount", &ARegrowthManager::CountEmpty ) );
}

// $Barnyard: FUNCTION 005e2330
void ARegrowthManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005e7710
ARegrowthManager::Types::Types()
{
	ms_pTypes     = this;
	m_ppEnumNames = TNULL;
	m_ppBoneNames = TNULL;
	m_iNumEnums   = 0;
}

// $Barnyard: FUNCTION 005e7650
ARegrowthManager::Types::~Types()
{
	m_ppEnumNames = TNULL;
	m_ppBoneNames = TNULL;
	m_iNumEnums   = 0;
}

// $Barnyard: FUNCTION 005e7660
void ARegrowthManager::Types::Load( TTRB* a_pTRB )
{
	m_ppEnumNames = TNULL;
	m_ppBoneNames = TNULL;
	m_iNumEnums   = 0;

	m_ppBoneNames = a_pTRB->CastSymbol<const TCHAR*>( "BoneNames" );
	m_ppEnumNames = a_pTRB->CastSymbol<const TCHAR*>( "EnumNames" );

	if ( m_ppEnumNames )
	{
		const TCHAR** ppName = m_ppEnumNames;

		if ( *ppName )
		{
			while ( ppName[ 1 ] )
				ppName++;

			ppName++;
		}

		m_iNumEnums = ppName - m_ppEnumNames;
	}
}

// $Barnyard: FUNCTION 005e76c0
TINT ARegrowthManager::Types::FindEnum( const TCHAR* a_szName ) const
{
	if ( !m_ppEnumNames || !*m_ppEnumNames )
		return -1;

	const TCHAR** ppName = m_ppEnumNames;

	while ( T2String8::CompareNoCase( a_szName, *ppName, T2String8::Length( a_szName ) ) != 0 )
	{
		if ( !ppName[ 1 ] )
			return -1;

		ppName++;
	}

	return ppName - m_ppEnumNames;
}

//-----------------------------------------------------------------------------
// TypeMask
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005e7750
ARegrowthManager::TypeMask::TypeMask( const TypeMask& a_rcOther )
{
	auiBits[ 0 ] = 0;
	auiBits[ 1 ] = 0;

	for ( TINT i = 0; i < 2; i++ )
		auiBits[ i ] |= a_rcOther.auiBits[ i ];
}

// $Barnyard: FUNCTION 005e7730
ARegrowthManager::TypeMask& ARegrowthManager::TypeMask::operator|=( const TypeMask& a_rcOther )
{
	auiBits[ 0 ] |= a_rcOther.auiBits[ 0 ];
	auiBits[ 1 ] |= a_rcOther.auiBits[ 1 ];

	return *this;
}

// $Barnyard: FUNCTION 005e7780
ARegrowthManager::TypeMask ARegrowthManager::TypeMask::operator&( const TypeMask& a_rcOther ) const
{
	TypeMask oResult( a_rcOther );

	for ( TINT i = 0; i < 2; i++ )
		oResult.auiBits[ i ] &= auiBits[ i ];

	return oResult;
}

// $Barnyard: FUNCTION 005e7830
ARegrowthManager::TypeMask ARegrowthManager::TypeMask::FromType( TUINT a_iType )
{
	TypeMask oMask;

	if ( a_iType < 56 )
		oMask.auiBits[ ( a_iType + 8 ) >> 5 ] |= 1 << ( ( a_iType + 8 ) & 0x1f );

	return oMask;
}

// $Barnyard: FUNCTION 005e78a0
ARegrowthManager::TypeMask ARegrowthManager::TypeMask::FromTypeWithFlags( TUINT a_iType )
{
	GrowthMapping* pMapping = &ARegrowthManager::GetSingleton()->m_pGrowthMapping[ a_iType ];

	TypeMask oMask = FromType( a_iType );
	oMask.auiBits[ 0 ] |= pMapping->uiFlags;

	return oMask;
}

// $Barnyard: FUNCTION 005e79c0
ARegrowthManager::TypeMask ARegrowthManager::TypeMask::WithoutFlag( TUINT a_uiFlag )
{
	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		TypeMask oFlag;
		oFlag.auiBits[ a_uiFlag >> 5 ] |= 1 << ( a_uiFlag & 0x1f );

		// NOTE: retail sets the bit of the type if it doesn't have the flag, but the mask it builds is reset every iteration and never used
		TypeMatches( i, oFlag );
	}

	TypeMask oResult;
	oResult.auiBits[ 0 ] = ~a_uiFlag;
	oResult.auiBits[ 1 ] = 0;

	return oResult;
}

// $Barnyard: FUNCTION 005e7aa0
ARegrowthManager::TypeMask ARegrowthManager::TypeMask::NonTrees()
{
	return WithoutFlag( 1 );
}

// $Barnyard: FUNCTION 005e7810
ARegrowthManager::TypeMask ARegrowthManager::TypeMask::Crates()
{
	TypeMask oMask;
	oMask.auiBits[ 0 ] |= BITFLAG( TYPEFLAG_CRATE );

	return oMask;
}

// $Barnyard: FUNCTION 005e7ad0
ARegrowthManager::TypeMask ARegrowthManager::TypeMask::FromName( const TPString8& a_rcName )
{
	TypeMask oMask;

	if ( a_rcName == TPS8( fruit ) )
		oMask.auiBits[ 0 ] |= BITFLAG( TYPEFLAG_FRUIT );
	else if ( a_rcName == TPS8( crate ) )
		oMask.auiBits[ 0 ] |= BITFLAG( TYPEFLAG_CRATE );
	else if ( a_rcName == TPS8( light ) )
		oMask.auiBits[ 0 ] |= BITFLAG( TYPEFLAG_LIGHT );
	else if ( a_rcName == TPS8( heavy ) )
		oMask.auiBits[ 0 ] |= BITFLAG( TYPEFLAG_HEAVY );
	else if ( a_rcName == TPS8( milk ) )
		oMask.auiBits[ 0 ] |= BITFLAG( TYPEFLAG_MILK );
	else if ( a_rcName == TPS8( dayrespawn ) )
		oMask.auiBits[ 0 ] |= BITFLAG( TYPEFLAG_DAYRESPAWN );
	else if ( a_rcName == TPS8( non_trees ) )
		oMask = NonTrees();
	else
		oMask = FromType( ARegrowthManager::GetSingleton()->GetTypeIndex( a_rcName ) );

	return oMask;
}

// $Barnyard: FUNCTION 005e7970
TBOOL ARegrowthManager::HasTypeFlag( TUINT a_uiType, TYPEFLAG a_eFlag )
{
	return a_uiType <= 126 &&
	    a_uiType != TUINT( ms_pTypes->GetNumEnums() ) &&
	    ( ( 1 << a_eFlag ) & ARegrowthManager::GetSingleton()->m_pGrowthMapping[ a_uiType ].uiFlags );
}

// $Barnyard: FUNCTION 005e7910
TBOOL ARegrowthManager::TypeMatches( TUINT a_uiType, const TypeMask& a_rcMask )
{
	if ( a_uiType > 126 || a_uiType == TUINT( ms_pTypes->GetNumEnums() ) )
		return TFALSE;

	return !( TypeMask::FromTypeWithFlags( a_uiType ) & a_rcMask ).IsEmpty();
}

//-----------------------------------------------------------------------------
// GrowthObject
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005e2260
TFLOAT ARegrowthManager::GrowthObject::GetScale() const
{
	return uiScale * ( 1.0f / 255.0f ) * 0.4f + 0.8f;
}

// $Barnyard: FUNCTION 005e2600
void ARegrowthManager::GrowthObject::SetScale( TFLOAT a_fScale )
{
	if ( a_fScale < 0.8f || a_fScale > 1.2f )
		a_fScale = 1.0f;

	uiScale = TUINT8( ( a_fScale - 0.8f ) * 255.0f );
}

// $Barnyard: FUNCTION 005e2520
void ARegrowthManager::GrowthObject::GetMatrix( TMatrix44& a_rOutMatrix ) const
{
	a_rOutMatrix.Identity();
	a_rOutMatrix.SetFromQuaternion( quatRotation );
	a_rOutMatrix.SetTranslation( vecPosition );
}

// $Barnyard: FUNCTION 005e72e0
ARegrowthManager::CrateSchema::Content* ARegrowthManager::GrowthObject::PickCrateContent()
{
	CrateSchema* pSchemas = ARegrowthManager::GetSingleton()->m_pCrateSchemas;
	CrateSchema* pSchema  = pSchemas;

	if ( pSchema->szName )
	{
		TINT iSchema = uiCrate >> 3;

		for ( TINT i = 0; iSchema != i; i++ )
		{
			if ( !pSchema[ 1 ].szName )
			{
				pSchema = pSchemas;
				break;
			}

			pSchema++;
		}
	}

	return pSchema ? pSchema->PickRandom() : TNULL;
}

// $Barnyard: FUNCTION 005e6da0
ARegrowthManager::CrateSchema::Content* ARegrowthManager::CrateSchema::PickRandom()
{
	AWeightedRandom<TINT, 100> oPicker( &ARandom::GetSingleton()->m_oRandom );

	TFLOAT   fChances = 0.0f;
	TINT     iContent = 0;
	Content* pContent = pContents;

	if ( pContent->szName )
	{
		do
		{
			oPicker.Add( iContent, pContent->fChance );
			fChances += pContent->fChance;
			iContent++;
			pContent++;
		} while ( pContent->szName );

		if ( fChances >= 1.0f )
			goto PICK;
	}

	// The chance of an empty crate
	oPicker.Add( iContent, 1.0f - fChances );

PICK:
	AWeightedRandom<TINT, 100>::Entry* pPicked = oPicker.Pick();

	// NOTE: retail doesn't check if an entry was picked
	TINT iPicked = pPicked->oValue;

	Content* pResult = pContents;

	if ( !pResult->szName )
		return TNULL;

	for ( TINT i = 0; iPicked != i; i++ )
	{
		if ( !pResult[ 1 ].szName )
			return TNULL;

		pResult++;
	}

	return pResult;
}

//-----------------------------------------------------------------------------
// TypeLists
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005e4970
void ARegrowthManager::TypeLists::SetGrowing( GrowthObject* a_pObject, TFLOAT a_fGrowTime )
{
	if ( a_pObject->fGrowTime == TIME_EMPTY )
	{
		a_pObject->fGrowTime = a_fGrowTime;
		llEmpty.Transfer( a_pObject, llGrowing );
	}
}

// $Barnyard: FUNCTION 005e48f0
void ARegrowthManager::TypeLists::SetGrowing( GrowthObject* a_pObject, TFLOAT a_fGrowTime, GrowthObject*& a_rpNext )
{
	if ( a_pObject->fGrowTime == TIME_EMPTY )
	{
		a_pObject->fGrowTime = a_fGrowTime;
		a_rpNext             = llEmpty.Transfer( a_pObject, llGrowing );
	}
}

// $Barnyard: FUNCTION 005e49a0
void ARegrowthManager::TypeLists::SetEmpty( GrowthObject* a_pObject )
{
	if ( a_pObject->fGrowTime != TIME_EMPTY )
	{
		a_pObject->fGrowTime = TIME_EMPTY;
		llGrowing.Transfer( a_pObject, llEmpty );
	}
}

// $Barnyard: FUNCTION 005e4930
void ARegrowthManager::TypeLists::SetEmpty( GrowthObject* a_pObject, GrowthObject*& a_rpNext )
{
	if ( a_pObject->fGrowTime != TIME_EMPTY )
	{
		a_pObject->fGrowTime = TIME_EMPTY;
		a_rpNext             = llGrowing.Transfer( a_pObject, llEmpty );
	}
}

//-----------------------------------------------------------------------------
// Nodes
//-----------------------------------------------------------------------------

ARegrowthManager::Model::Model()
    : pModel( TNULL ), pSceneObject( TNULL )
{
}

ARegrowthManager::Model::~Model()
{
	// NOTE: 005dfc20, shared with AInstanceManager::Model
	if ( pSceneObject )
		pSceneObject->Delete();

	if ( pModel )
		delete pModel;
}

// $Barnyard: FUNCTION 005e2710
ARegrowthManager::ACollisionObjectModelNode::ACollisionObjectModelNode()
{
}

// $Barnyard: FUNCTION 005e2280
// $Barnyard: FUNCTION 005e2350
// $Barnyard: FUNCTION 005e2700
ARegrowthManager::ACollisionObjectModelNode::~ACollisionObjectModelNode()
{
}

ARegrowthManager::ModelInstanceNode::ModelInstanceNode()
{
	// NOTE: 005dfd40, shared with AInstanceManager
}

// $Barnyard: FUNCTION 005e22a0
ARegrowthManager::ModelInstanceNode::~ModelInstanceNode()
{
}

//-----------------------------------------------------------------------------
// ARegrowthManager
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005e2e40
ARegrowthManager::ARegrowthManager()
{
	m_iNumModels             = 0;
	m_pModels                = TNULL;
	m_ppRenderLists          = TNULL;
	m_pCollisionNodes        = TNULL;
	m_pModelInstanceNodes    = TNULL;
	m_iNumCollisionModelSets = 0;
	m_pCrateSchemas          = TNULL;
	m_fLODDistance           = 30.0f;
	m_fRenderDistance        = 60.0f;
	m_fFadeDistance          = 50.0f;
	m_pRenderData            = TNULL;
	m_eFlags                 = FLAGS_NONE;
	m_iNumRespawned          = 0;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail never initialises it
	m_fFadeScale = 1.0f / ( m_fRenderDistance - m_fFadeDistance );
#endif

	for ( TINT i = 0; i < TARRAYSIZE( m_auiUsedGroupIds ); i++ )
		m_auiUsedGroupIds[ i ] = 0;

	s_pCollisionEntries = TSTATICCAST( CollisionEntry, TMalloc( MAX_NUM_COLLISION_ENTRIES * sizeof( CollisionEntry ) ) );
	new Types();

	LoadTRB();
	m_fCollisionUpdateTimer = 5.0f;

	for ( TINT i = 0; i < TARRAYSIZE( m_auiUsedGroupIds ); i++ )
		m_auiUsedGroupIds[ i ] = 0;

	m_aGroups[ 0 ].uiGroupId = 0xff;
}

// $Barnyard: FUNCTION 005e7470
// $Barnyard: FUNCTION 005e7630
ARegrowthManager::~ARegrowthManager()
{
	ms_pTypes->GetNumEnums();

	ClearGroup( &m_aGroups[ 0 ] );
	m_llFreeCollisionNodes.Clear();
	m_llFreeModelInstances.Clear();

	if ( m_pCollisionNodes )
		delete[] m_pCollisionNodes;

	m_pCollisionNodes = TNULL;

	if ( m_pModelInstanceNodes )
		delete[] m_pModelInstanceNodes;

	m_pModelInstanceNodes = TNULL;

	m_llFreeObjects.Clear();
	Deactivate();

	// NOTE: retail destroys the types here and frees them after closing the TRB
	ms_pTypes->~Types();
	m_oTRB.Close();

	TFree( s_pCollisionEntries );
	s_pCollisionEntries = TNULL;

	if ( ms_pTypes )
	{
		Types* pTypes = ms_pTypes;
		ms_pTypes     = TNULL;
		TFree( pTypes );
	}
}

// $Barnyard: FUNCTION 005e3020
AFunctionTable* ARegrowthManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 005e28f0
TBOOL ARegrowthManager::LoadTRB()
{
	if ( m_oTRB.Load( "Data\\treeregrowth\\treeregrowth.trb" ) != TTRB::ERROR_OK )
		return TFALSE;

	m_pGrowthMapping = m_oTRB.CastSymbol<GrowthMapping>( "GrowthMapping" );
	s_ppModelNames   = m_oTRB.CastSymbol<const TCHAR*>( "ModelNames" );
	m_iNumModels     = *m_oTRB.CastSymbol<TINT>( "NumberModels" );
	m_pCrateSchemas  = m_oTRB.CastSymbol<CrateSchema>( "CrateSchema" );

	ms_pTypes->Load( &m_oTRB );
	m_oTRB.DeleteSymbolTable();

	for ( TINT i = 0; i < NUM_LIGHT_SLOTS; i++ )
	{
		m_aiLightIds[ i ]  = -1;
		m_aiLightUnk1[ i ] = 0;
		m_aiLightUnk2[ i ] = 0;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005e2890
TBOOL ARegrowthManager::ResetGrowthObject( TBOOL a_bAllowCollision )
{
	if ( a_bAllowCollision )
		m_eFlags |= ( FLAGS_ALLOW_COLL | FLAGS_ALLOW_COLL2 );
	else
		m_eFlags &= ~( FLAGS_ALLOW_COLL | FLAGS_ALLOW_COLL2 );

	m_eFlags &= ~FLAGS_ACTIVE;

	for ( TINT i = 0; i < MAX_NUM_OBJECTS; i++ )
		m_llFreeObjects.PushFront( &m_aObjects[ i ] );

	return TTRUE;
}

// $Barnyard: FUNCTION 005e4a70
void ARegrowthManager::Activate( TBOOL a_bCollision )
{
	TINT iNumModels = m_iNumModels;
	m_eFlags |= FLAGS_IN_USE;

	m_pModels       = new Model[ iNumModels ];
	m_ppRenderLists = TSTATICCAST( RenderData*, TMalloc( m_iNumModels * 2 * sizeof( RenderData* ) ) );
	m_eFlags |= FLAGS_ACTIVE;

	for ( TINT i = 0; i < NUM_LIGHT_SLOTS; i++ )
	{
		m_aiLightUnk2[ i ] = 0;
		m_aiLightUnk1[ i ] = 0;
		m_aiLightIds[ i ]  = -1;

		for ( TINT k = 0; k < 5; k++ )
			m_aiLightUnk3[ k ][ i ] = 0;
	}

	TTRB* pAssetTRB = AAssetLoader::GetAssetTRB( AAssetType_AssetPack );
	AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( TPString8( "miscobj" ), pAssetTRB );
	AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( TPString8( "veggie_gen" ), pAssetTRB );
	AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( TPString8( "instances" ), pAssetTRB );

	g_oLoadScreen.Update( 1.0f, TTRUE );

	if ( a_bCollision == TTRUE )
		SetCollisionEnabled( TTRUE );
	else
		SetCollisionEnabled( TFALSE );

	if ( HASANYFLAG( m_eFlags, FLAGS_ALLOW_COLL ) && m_llFreeCollisionNodes.IsEmpty() )
	{
		if ( m_pCollisionNodes )
			delete[] m_pCollisionNodes;

		m_pCollisionNodes = new ACollisionObjectModelNode[ MAX_NUM_COLLISION_NODES ];

		for ( TINT i = 0; i < MAX_NUM_COLLISION_NODES; i++ )
			m_llFreeCollisionNodes.PushFront( &m_pCollisionNodes[ i ] );
	}
}

// $Barnyard: FUNCTION 005e7330
void ARegrowthManager::Deactivate()
{
	if ( HASANYFLAG( m_eFlags, FLAGS_ACTIVE ) )
	{
		SetCollisionEnabled( TFALSE );
		ClearAllObjects();
		UnloadModels();

		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPString8( "miscobj" ) );
		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPString8( "veggie_gen" ) );
		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( TPString8( "instances" ) );

		m_eFlags &= ~FLAGS_ACTIVE;
	}

	m_eFlags &= ~FLAGS_IN_USE;
}

// $Barnyard: FUNCTION 005e3030
void ARegrowthManager::LoadModels()
{
	m_eFlags |= FLAGS_MODELS_LOADED;

	TTRB* pAssetTRB = AAssetLoader::GetAssetTRB( AAssetType_AssetPack );

	for ( TINT i = 0; i < m_iNumModels; i++ )
	{
		if ( ( i & 0xf ) == 0 )
			g_oLoadScreen.Update( 1.0f, TTRUE );

		Model* pModel = &m_pModels[ i ];

		pModel->pModel = new TManagedModel();
		pModel->pModel->Create( s_ppModelNames[ i ], pAssetTRB );
		pModel->pSceneObject = pModel->pModel->CreateSceneObject();

		TTransformObject& rTransform = pModel->pSceneObject->GetTransform();
		rTransform.SetMatrix( TMatrix44::IDENTITY );
		rTransform.SetEuler( TVector3( TMath::DegToRad( 90.0f ), 0.0f, 0.0f ) );
		rTransform.SetTranslate( TVector3::VEC_ZERO );

		// Bake the transform into the matrix
		TMatrix44 matTransform;
		rTransform.GetLocalMatrixImp( matTransform );
		rTransform.SetMatrix( matTransform );

		pModel->pSceneObject->DisableSkeletonUpdate();

		// No HD lighting
		TModel* pTModel = pModel->pModel->GetModel();

		for ( TINT k = 0; k < pTModel->GetNumLODs(); k++ )
		{
			TModelLOD& rLOD = pTModel->GetLOD( k );

			for ( TINT j = 0; j < rLOD.iNumMeshes; j++ )
			{
#ifdef TRENDERINTERFACE_DX8
				TSTATICCAST( ASkinMaterialHAL, rLOD.ppMeshes[ j ]->GetMaterial() )->SetHDLighting( TFALSE );
#endif
			}
		}
	}

	TBOOL bCollision = HASANYFLAG( m_eFlags, FLAGS_COLLISION );

	if ( bCollision )
	{
		// Recreate the collision model sets of the new models
		SetCollisionEnabled( TFALSE );
		SetCollisionEnabled( bCollision );
	}
}

// $Barnyard: FUNCTION 005e6f30
void ARegrowthManager::UnloadModels()
{
	if ( m_pModels )
		delete[] m_pModels;

	m_pModels = TNULL;

	TFree( m_ppRenderLists );
	m_ppRenderLists = TNULL;

	ClearGroup( &m_aGroups[ 0 ] );

	if ( m_pCollisionNodes )
		delete[] m_pCollisionNodes;

	m_pCollisionNodes = TNULL;

	if ( m_pModelInstanceNodes )
		delete[] m_pModelInstanceNodes;

	m_pModelInstanceNodes = TNULL;

	m_llFreeCollisionNodes.Clear();
	m_llFreeModelInstances.Clear();

	m_eFlags &= ~FLAGS_MODELS_LOADED;
}

// $Barnyard: FUNCTION 005e2a30
void ARegrowthManager::SetCollisionEnabled( TBOOL a_bEnabled )
{
	if ( HASALLFLAGS( m_eFlags, FLAGS_COLLISION ) == a_bEnabled )
		return;

	if ( a_bEnabled )
		m_eFlags |= FLAGS_COLLISION;
	else
		m_eFlags &= ~FLAGS_COLLISION;

	if ( HASANYFLAG( m_eFlags, FLAGS_COLLISION ) )
	{
		if ( m_llFreeModelInstances.IsEmpty() )
		{
			if ( m_pModelInstanceNodes )
				delete[] m_pModelInstanceNodes;

			m_pModelInstanceNodes = new ModelInstanceNode[ MAX_NUM_MODEL_INSTANCES ];

			for ( TINT i = 0; i < MAX_NUM_MODEL_INSTANCES; i++ )
				m_llFreeModelInstances.PushFront( &m_pModelInstanceNodes[ i ] );
		}

		for ( TINT i = 0; i < m_iNumModels; i++ )
		{
			Model* pModel = &m_pModels[ i ];

			if ( pModel->pModel && pModel->pModel->GetModel()->GetNumCollisionMeshes() > 0 )
			{
				ACollisionModelSet* pModelSet = new ACollisionModelSet();
				pModelSet->Create( pModel->pModel->GetModel() );

				m_apCollisionModelSets[ m_iNumCollisionModelSets++ ] = pModelSet;
			}
		}
	}
	else
	{
		m_llFreeModelInstances.Clear();
		DestroyCollisionModelSets();
	}
}

// $Barnyard: FUNCTION 005e29e0
void ARegrowthManager::DestroyCollisionModelSets()
{
	while ( m_iNumCollisionModelSets != 0 )
	{
		ACollisionModelSet* pModelSet                        = m_apCollisionModelSets[ m_iNumCollisionModelSets - 1 ];
		m_apCollisionModelSets[ --m_iNumCollisionModelSets ] = TNULL;

		if ( pModelSet )
			delete pModelSet;
	}

	m_eFlags &= ~FLAGS_COLLISION;
}

// $Barnyard: FUNCTION 005e31e0
void ARegrowthManager::GetModelCollisionSets( TBOOL* a_pbUsedModels, ACollisionModelSet** a_ppOutModelSets )
{
	TUtil::MemSet( a_pbUsedModels, 0, MAX_NUM_COLLISION_SETS );

	for ( TINT i = 0; i < m_iNumModels; i++ )
	{
		TManagedModel* pModel = m_pModels[ i ].pModel;

		if ( !pModel )
		{
			a_ppOutModelSets[ i ] = TNULL;
			continue;
		}

		a_ppOutModelSets[ i ] = TNULL;

		for ( TINT k = 0; k < m_iNumCollisionModelSets; k++ )
		{
			if ( m_apCollisionModelSets[ k ]->GetModel() == pModel->GetModel() )
			{
				a_ppOutModelSets[ i ] = m_apCollisionModelSets[ k ];
				break;
			}
		}
	}
}

// $Barnyard: FUNCTION 005e2560
ARegrowthManager::Group* ARegrowthManager::FindGroup( TUINT a_uiGroup )
{
	for ( TINT i = 0; i < MAX_NUM_GROUPS; i++ )
	{
		if ( m_aGroups[ i ].uiGroupId == a_uiGroup )
			return &m_aGroups[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005e25a0
ARegrowthManager::Group* ARegrowthManager::FindOrCreateGroup( TUINT a_uiGroup )
{
	Group* pGroup = FindGroup( a_uiGroup );

	if ( pGroup )
		return pGroup;

	// A free group
	return FindGroup( 0xff );
}

// $Barnyard: FUNCTION 005e59a0
void ARegrowthManager::ClearGroup( Group* a_pGroup )
{
	while ( !a_pGroup->llCollisionNodes.IsEmpty() )
	{
		ACollisionObjectModelNode* pNode = a_pGroup->llCollisionNodes.Back();
		a_pGroup->llCollisionNodes.PopBack();
		m_llFreeCollisionNodes.PushBack( pNode );

		if ( pNode->IsCollisionObjectLinked() )
			pNode->UnlinkCollisionObject();
	}

	while ( !a_pGroup->llModelInstances.IsEmpty() )
	{
		ModelInstanceNode* pNode = a_pGroup->llModelInstances.Back();
		a_pGroup->llModelInstances.PopBack();
		m_llFreeModelInstances.PushBack( pNode );
	}

	ReleaseGroupCollisionNodes( a_pGroup->uiGroupId );
	a_pGroup->uiGroupId = 0xff;
}

// $Barnyard: FUNCTION 005e4380
void ARegrowthManager::ReleaseGroupCollisionNodes( TUINT a_uiGroup )
{
	for ( auto it = m_llPendingCollisionNodes.Begin(); it != m_llPendingCollisionNodes.End(); ++it )
	{
		ACollisionObjectModelNode* pNode = it;

		if ( pNode->m_pObject->uiGroup == a_uiGroup )
			it = m_llPendingCollisionNodes.Transfer( pNode, m_llFreeCollisionNodes );
	}
}

// $Barnyard: FUNCTION 005e7230
void ARegrowthManager::ClearAllObjects()
{
	TINT iNumTypes = ms_pTypes->GetNumEnums();

	// NOTE: retail clears one list more than there are types
	for ( TINT i = 0; i <= iNumTypes; i++ )
	{
		m_aTypeLists[ i ].llEmpty.Clear();
		m_aTypeLists[ i ].llGrowing.Clear();
	}

	m_llFreeObjects.Clear();

	for ( TINT i = 0; i < MAX_NUM_OBJECTS; i++ )
		m_llFreeObjects.PushFront( &m_aObjects[ i ] );

	for ( TINT i = 0; i < TARRAYSIZE( m_auiUsedGroupIds ); i++ )
		m_auiUsedGroupIds[ i ] = 0;

	ClearGroup( &m_aGroups[ 0 ] );
}

// $Barnyard: FUNCTION 005e4070
TUINT8 ARegrowthManager::AllocateGroupId()
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_ACTIVE ) )
		return 0;

	TUINT uiId = 254;

	while ( m_auiUsedGroupIds[ uiId >> 5 ] & ( 1 << ( uiId & 0x1f ) ) )
	{
		if ( TINT( --uiId ) < 1 )
			return 0xff;
	}

	m_auiUsedGroupIds[ uiId >> 5 ] |= 1 << ( uiId & 0x1f );
	return TUINT8( uiId );
}

// $Barnyard: FUNCTION 005e70c0
void ARegrowthManager::DestroyGroup( TUINT a_uiGroup )
{
	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		TypeLists* pLists = &m_aTypeLists[ i ];

		for ( auto it = pLists->llEmpty.Begin(); it != pLists->llEmpty.End(); )
		{
			GrowthObject* pObject = it;
			it                    = it->Next();

			if ( pObject->uiGroup == a_uiGroup )
			{
				pLists->llEmpty.Erase( pObject );
				m_llFreeObjects.PushFront( pObject );
			}
		}

		for ( auto it = pLists->llGrowing.Begin(); it != pLists->llGrowing.End(); )
		{
			GrowthObject* pObject = it;
			it                    = it->Next();

			if ( pObject->uiGroup == a_uiGroup )
			{
				pLists->llGrowing.Erase( pObject );
				m_llFreeObjects.PushFront( pObject );
			}
		}
	}

	m_auiUsedGroupIds[ a_uiGroup >> 5 ] &= ~( 1 << ( a_uiGroup & 0x1f ) );

	Group* pGroup = FindGroup( a_uiGroup );

	if ( pGroup )
		ClearGroup( pGroup );
}

// $Barnyard: FUNCTION 005e2650
TINT ARegrowthManager::FindCrateSchema( const TCHAR* a_szName )
{
	CrateSchema* pSchema = m_pCrateSchemas;
	TUINT8       uiIndex = 0;

	if ( !pSchema->szName )
		return 0;

	while ( !T2String8::FindString( a_szName, pSchema->szName ) )
	{
		pSchema++;
		uiIndex++;

		if ( !pSchema->szName )
			return 0;
	}

	return uiIndex;
}

// $Barnyard: FUNCTION 005e2690
TINT ARegrowthManager::FindTypeByPrefix( const TCHAR* a_szName, const TCHAR** a_ppNames, TINT a_iNumNames )
{
	if ( a_iNumNames <= 0 )
		return -1;

	for ( TUINT8 i = 0; i < a_iNumNames; i++ )
	{
		const TCHAR* szName = a_ppNames[ i ];

		if ( szName && *szName && T2String8::CompareNoCase( szName, a_szName, T2String8::Length( szName ) ) == 0 )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005e29a0
TINT ARegrowthManager::GetTypeIndex( const TPString8& a_rcName )
{
	return ms_pTypes->FindEnum( a_rcName.GetString() );
}

// $Barnyard: FUNCTION 005e24c0
TINT ARegrowthManager::GetScreenNameId( TINT a_iType ) const
{
	return m_pGrowthMapping[ a_iType ].iScreenNameId;
}

// $Barnyard: FUNCTION 005e24e0
TINT ARegrowthManager::GetNumFruits( TINT a_iType ) const
{
	return m_pGrowthMapping[ a_iType ].uiNumFruits;
}

// $Barnyard: FUNCTION 005e2500
TINT ARegrowthManager::GetNumRegrow( TINT a_iType ) const
{
	return m_pGrowthMapping[ a_iType ].uiNumRegrow;
}

// $Barnyard: FUNCTION 005e5890
TPString8 ARegrowthManager::GetUnitTypeIfInteraction( TINT a_iType )
{
	GrowthMapping* pMapping = &m_pGrowthMapping[ a_iType ];

	if ( pMapping->szInteraction )
		return TPString8( pMapping->szUnitType );

	return TPString8();
}

// $Barnyard: FUNCTION 005e58e0
TPString8 ARegrowthManager::GetInteraction( TINT a_iType )
{
	const TCHAR* szInteraction = m_pGrowthMapping[ a_iType ].szInteraction;

	if ( szInteraction )
		return TPString8( szInteraction );

	return TPS8( defaulttree );
}

// $Barnyard: FUNCTION 005e5940
TPString8 ARegrowthManager::GetUnitType( TINT a_iType )
{
	if ( m_pGrowthMapping[ a_iType ].szUnitType )
		return TPString8( m_pGrowthMapping[ a_iType ].szUnitType );

	return TPString8();
}

// $Barnyard: FUNCTION 005e22d0
TFLOAT ARegrowthManager::DecodeGrowTime( TUINT8 a_uiValue )
{
	if ( a_uiValue == 0x80 )
		return 0.0f;

	if ( a_uiValue == 0 )
		return TIME_EMPTY;

	if ( a_uiValue < 0x80 )
		return -( a_uiValue * 0.0078125f * ( a_uiValue * 0.0078125f ) * 100000000.0f );

	TFLOAT fValue = ( a_uiValue - 128 ) * 0.0078125f * ( ( a_uiValue - 128 ) * 0.0078125f ) * 25.0f;
	return fValue * fValue;
}

// $Barnyard: FUNCTION 005e2750
TUINT8 ARegrowthManager::EncodeGrowTime( TFLOAT a_fGrowTime )
{
	if ( a_fGrowTime == 0.0f )
		return 0x80;

	if ( a_fGrowTime == TIME_EMPTY )
		return 0;

	if ( a_fGrowTime < 0.0f )
	{
		TFLOAT fMin   = 0.001f;
		TFLOAT fValue = -a_fGrowTime;

		TFLOAT* pValue = ( fValue <= 0.001f ) ? &fMin : &fValue;
		fValue         = TMath::Sqrt( *pValue * 0.0001f ) * 128.0f;
		TMath::Clip( fValue, 1.0f, 127.0f );

		return TUINT8( fValue );
	}

	TFLOAT fValue = TFLOAT( TUINT8( ( TMath::Sqrt( a_fGrowTime * 0.2f ) + 1.0f ) * 128.0f ) );
	TMath::Clip( fValue, 129.0f, 255.0f );

	return TUINT8( fValue );
}

// $Barnyard: FUNCTION 005e4320
ARegrowthManager::ACollisionObjectModelNode* ARegrowthManager::FindCollisionNode( GrowthObject* a_pObject, T2SList<ACollisionObjectModelNode>* a_pList )
{
	T2_FOREACH( *a_pList, it )
	{
		if ( it->m_pObject == a_pObject )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005e4540
ARegrowthManager::GrowthObject* ARegrowthManager::FindObjectBySaveKey( TFLOAT a_fKey, T2SList<GrowthObject>* a_pList )
{
	T2_FOREACH( *a_pList, it )
	{
		if ( a_fKey == it->GetSaveKey() )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005e3e60
TINT ARegrowthManager::CountEmptyInType( TUINT a_uiGroup, TINT a_iType )
{
	TINT iCount = 0;

	T2_FOREACH( m_aTypeLists[ a_iType ].llEmpty, it )
	{
		if ( a_uiGroup == it->uiGroup && it->fGrowTime <= TIME_EMPTY )
			iCount++;
	}

	return iCount;
}

// $Barnyard: FUNCTION 005e3ec0
TINT ARegrowthManager::CountEmpty( TINT a_uiGroup )
{
	TINT iCount = 0;

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
		iCount += CountEmptyInType( a_uiGroup, i );

	return iCount;
}

// $Barnyard: FUNCTION 005e3f00
TINT ARegrowthManager::CountInType( TUINT a_uiGroup, TINT a_iType )
{
	TINT iCount = 0;

	T2_FOREACH( m_aTypeLists[ a_iType ].llGrowing, it )
	{
		if ( a_uiGroup == it->uiGroup )
			iCount++;
	}

	return iCount;
}

// $Barnyard: FUNCTION 005e3f50
TBOOL ARegrowthManager::HasGroupInType( TUINT a_uiGroup, TINT a_iType )
{
	T2_FOREACH( m_aTypeLists[ a_iType ].llGrowing, it )
	{
		if ( a_uiGroup == it->uiGroup )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005e3fa0
TBOOL ARegrowthManager::IsGrowingInType( GrowthObject* a_pObject, TINT a_iType )
{
	T2_FOREACH( m_aTypeLists[ a_iType ].llGrowing, it )
	{
		if ( a_pObject == it )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005e3ff0
TBOOL ARegrowthManager::IsInType( GrowthObject* a_pObject, TINT a_iType )
{
	T2_FOREACH( m_aTypeLists[ a_iType ].llGrowing, it )
	{
		if ( a_pObject == it )
			return TTRUE;
	}

	T2_FOREACH( m_aTypeLists[ a_iType ].llEmpty, it )
	{
		if ( a_pObject == it )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005e4220
TINT ARegrowthManager::FindTypeOfGroup( TUINT a_uiGroup )
{
	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		if ( HasGroupInType( a_uiGroup, i ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005e4270
TINT ARegrowthManager::FindTypeOfObject( GrowthObject* a_pObject )
{
	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		if ( IsGrowingInType( a_pObject, i ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005e42c0
TINT ARegrowthManager::CountInArea( AInteriorArea* a_pArea, TINT a_iType )
{
	TINT iCount = 0;

	T2_FOREACH( m_aTypeLists[ a_iType ].llGrowing, it )
	{
		if ( a_pArea->CheckPositionInsideArea( it->vecPosition ) )
			iCount++;
	}

	return iCount;
}

// $Barnyard: FUNCTION 005e3b80
TINT ARegrowthManager::CountObjectsAtStage( TUINT a_uiGroup, TINT a_iStage )
{
	TFLOAT fTime  = AGameTimeManager::GetSingleton()->GetTotalTime();
	TINT   iCount = 0;

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

		T2_FOREACH( m_aTypeLists[ i ].llGrowing, it )
		{
			if ( a_uiGroup != it->uiGroup || it->fGrowTime <= TIME_EMPTY )
				continue;

			TINT iStage = StageToInt( ( fTime - it->fGrowTime ) * pMapping->fStagesPerSecond );

			if ( iStage < pMapping->iNumStages - 1 )
			{
				if ( a_iStage != iStage )
					continue;
			}
			else if ( a_iStage <= 100 )
			{
				continue;
			}

			iCount++;
		}
	}

	return iCount;
}

// $Barnyard: FUNCTION 005e3c70
TINT ARegrowthManager::CountTypeObjectsAtStage( TINT a_iStage, TINT a_iType )
{
	GrowthMapping* pMapping = &m_pGrowthMapping[ a_iType ];
	TFLOAT         fTime    = AGameTimeManager::GetSingleton()->GetTotalTime();
	TINT           iCount   = 0;

	T2_FOREACH( m_aTypeLists[ a_iType ].llGrowing, it )
	{
		if ( it->fGrowTime <= TIME_EMPTY )
			continue;

		TINT iStage = StageToInt( ( fTime - it->fGrowTime ) * pMapping->fStagesPerSecond );

		if ( iStage < pMapping->iNumStages - 1 )
		{
			if ( a_iStage == iStage )
				iCount++;
		}
		else if ( a_iStage > 100 )
		{
			iCount++;
		}
	}

	return iCount;
}

// $Barnyard: FUNCTION 005e3d10
ARegrowthManager::GrowthObject* ARegrowthManager::FindRandomObjectAtStage( TUINT a_uiGroup, TINT a_iStage, TINT& a_rOutType )
{
	TINT iNumObjects = CountObjectsAtStage( a_uiGroup, a_iStage );

	if ( iNumObjects <= 0 )
		return TNULL;

	TINT   iObject = ( iNumObjects == 1 ) ? 1 : ARandom::GetSingleton()->m_oRandom.GetInt( iNumObjects - 1 ) + 1;
	TFLOAT fTime   = AGameTimeManager::GetSingleton()->GetTotalTime();

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

		T2_FOREACH( m_aTypeLists[ i ].llGrowing, it )
		{
			if ( a_uiGroup != it->uiGroup || it->fGrowTime <= TIME_EMPTY )
				continue;

			TINT iStage = StageToInt( ( fTime - it->fGrowTime ) * pMapping->fStagesPerSecond );

			if ( iStage < pMapping->iNumStages - 1 )
			{
				if ( a_iStage != iStage )
					continue;
			}
			else if ( a_iStage <= 100 )
			{
				continue;
			}

			if ( --iObject == 0 )
			{
				a_rOutType = i;
				return it;
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005e40d0
ARegrowthManager::GrowthObject* ARegrowthManager::FindClosestVisible( const TVector4& a_rcPosition, const TypeMask& a_rcMask, TFLOAT a_fMaxDistance )
{
	TFLOAT        fTime    = AGameTimeManager::GetSingleton()->GetTotalTime();
	TFLOAT        fClosest = a_fMaxDistance;
	GrowthObject* pClosest = TNULL;

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		if ( !TypeMatches( i, TypeMask( a_rcMask ) ) )
			continue;

		GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

		T2_FOREACH( m_aTypeLists[ i ].llGrowing, it )
		{
			// Only the grown objects
			if ( pMapping->uiFirstModel == 0xff || StageToInt( ( fTime - it->fGrowTime ) * pMapping->fStagesPerSecond ) < pMapping->iNumStages - 1 )
				continue;

			TFLOAT fDistance = TVector4::Distance( it->vecPosition, a_rcPosition );

			// The closest one on the screen
			TVector2 vecScreen;

			if ( fDistance < fClosest && ARenderer::GetSingleton()->GetScreenPosition( it->vecPosition, vecScreen ) )
			{
				fClosest = fDistance;
				pClosest = it;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 005e5450
ARegrowthManager::GrowthObject* ARegrowthManager::FindObjectInFront( const TVector4& a_rcPosition, const TVector2& a_rcDirection, const TypeMask& a_rcMask, TINT& a_rOutType, TFLOAT a_fMaxDistance, TFLOAT a_fMinDot )
{
	TFLOAT        fTime    = AGameTimeManager::GetSingleton()->GetTotalTime();
	TFLOAT        fClosest = a_fMaxDistance;
	GrowthObject* pFound   = TNULL;

	// The rays to the objects start a bit lower
	TVector4 vecRayStart = a_rcPosition;
	vecRayStart.y        = vecRayStart.y - 0.4f;

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		if ( !TypeMatches( i, TypeMask( a_rcMask ) ) )
			continue;

		GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

		T2_FOREACH( m_aTypeLists[ i ].llGrowing, it )
		{
			// Only the grown objects
			if ( pMapping->uiFirstModel == 0xff || StageToInt( ( fTime - it->fGrowTime ) * pMapping->fStagesPerSecond ) < pMapping->iNumStages - 1 )
				continue;

			const TSphere& rcSphere = m_pModels[ pMapping->uiFirstModel ].pModel->GetModel()->GetLOD( 0 ).BoundingSphere;
			TFLOAT         fRadius  = rcSphere.GetRadius();
			TFLOAT         fObjectY = it->vecPosition.y;

			// The height has to match
			if ( !( fRadius + fObjectY + 0.1f > a_rcPosition.y ) || !( fRadius + 0.1f > TMath::Abs( a_rcPosition.y - fObjectY - fRadius * 0.5f ) ) )
				continue;

			TVector2 vecToObject( it->vecPosition.x - a_rcPosition.x, it->vecPosition.z - a_rcPosition.z );
			TFLOAT   fDistance = vecToObject.NormalizeMag() - fRadius;

			if ( fDistance > fClosest )
				continue;

			TFLOAT fDot    = vecToObject.y * a_rcDirection.y + vecToObject.x * a_rcDirection.x;
			TFLOAT fMinDot = fDistance;
			TMath::Clip( fMinDot, 0.4f, 1.0f );

			if ( !( fMinDot * a_fMinDot < fDot ) )
				continue;

			// Nothing between the player and the object?
			TVector4 vecDirection( it->vecPosition.x - a_rcPosition.x, 0.0f, it->vecPosition.z - a_rcPosition.z, it->vecPosition.w );
			TFLOAT   fRayLength = vecDirection.NormaliseMag() * 1.5f - fRadius;

			if ( fRayLength >= 0.0f && ATerrainInterface::GetSingleton()->CollideRay( vecRayStart, vecDirection, fRayLength, 0, TFALSE ) )
				continue;

			Group* pGroup = FindGroup( it->uiGroup );

			if ( pGroup )
			{
				ACollisionObjectModelNode* pNode = FindCollisionNode( it, &pGroup->llCollisionNodes );

				if ( pNode )
				{
					// Sweep a sphere from the player to the object, nothing else may be hit before its collision
					TVector4 vecSweepDirection( a_rcDirection.x, 0.0f, a_rcDirection.y, 1.0f );
					vecSweepDirection.Normalise();

					TVector4 vecNormal;
					TFLOAT   fSweepDistance = fRadius + a_fMaxDistance;

					ACollisionObject* pHit = ACollisionManager::GetSingleton()->CollideSweptSphereObjects( vecRayStart, vecSweepDirection, 0.1f, fSweepDistance, vecNormal, ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_UNIT, 0.0f, TNULL, 0 );

					if ( pHit != pNode )
					{
						// Try again with a bigger sphere, nothing hit is also fine
						fSweepDistance = a_fMaxDistance * 0.9f + fRadius;
						pHit           = ACollisionManager::GetSingleton()->CollideSweptSphereObjects( vecRayStart, vecSweepDirection, 0.3f, fSweepDistance, vecNormal, ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_UNIT, 0.0f, TNULL, 0 );

						if ( pHit != TNULL && pHit != pNode )
							continue;
					}
				}
			}

			fClosest   = fDistance;
			a_rOutType = i;
			pFound     = it;
		}
	}

	return pFound;
}

// $Barnyard: FUNCTION 005e4fa0
TBOOL ARegrowthManager::TakeRandomObject( TUINT a_uiGroup, TVector4& a_rOutPosition )
{
	TINT          iType;
	GrowthObject* pObject = FindRandomObjectAtStage( a_uiGroup, 10000, iType );

	if ( !pObject )
		return TFALSE;

	a_rOutPosition = pObject->vecPosition;
	m_aTypeLists[ iType ].SetEmpty( pObject );

	return TTRUE;
}

// $Barnyard: FUNCTION 005e6c30
TBOOL ARegrowthManager::TakeObject( TINT a_iType, GrowthObject* a_pObject )
{
	if ( !a_pObject )
		return TFALSE;

	m_aTypeLists[ a_iType ].SetEmpty( a_pObject );
	DeactivateObjectCollision( a_pObject );

	return TTRUE;
}

// $Barnyard: FUNCTION 005e5000
TINT ARegrowthManager::EmptyGroup( TUINT a_uiGroup )
{
	TINT iCount = 0;

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		TypeLists* pLists = &m_aTypeLists[ i ];

		for ( auto it = pLists->llGrowing.Begin(); it != pLists->llGrowing.End(); it = it->Next() )
		{
			GrowthObject* pObject = it;

			if ( a_uiGroup == pObject->uiGroup )
			{
				iCount++;

				GrowthObject* pNext = pObject;
				pLists->SetEmpty( pObject, pNext );
				it = pNext;
			}
		}
	}

	return iCount;
}

// $Barnyard: FUNCTION 005e5090
TBOOL ARegrowthManager::StartGrowing( TINT a_iType, GrowthObject* a_pObject, TFLOAT a_fAge )
{
	if ( !a_pObject )
		return TFALSE;

	TFLOAT     fTime  = AGameTimeManager::GetSingleton()->GetTotalTime();
	TypeLists* pLists = &m_aTypeLists[ a_iType ];

	if ( HasTypeFlag( a_iType, TYPEFLAG_DAYRESPAWN ) || HasTypeFlag( a_iType, TYPEFLAG_INSTANT ) )
	{
		// Grown already
		pLists->SetGrowing( a_pObject, fTime - a_fAge - 1000000000.0f );
		return TTRUE;
	}

	TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, 10.0f );
	pLists->SetGrowing( a_pObject, fRandom + fTime - a_fAge );
	a_pObject->uiCrate = TUINT8( ARandom::GetSingleton()->m_oRandom.GetInt( 255 ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 005e51b0
TINT ARegrowthManager::GrowObjects( TUINT a_uiGroup, TINT a_iCount, TFLOAT a_fAge, TINT a_iType )
{
	TypeLists* pLists    = &m_aTypeLists[ a_iType ];
	TINT       iNumEmpty = CountEmptyInType( a_uiGroup, a_iType );
	TINT       iNumLeft  = ( iNumEmpty < a_iCount ) ? iNumEmpty : a_iCount;
	TFLOAT     fTime     = AGameTimeManager::GetSingleton()->GetTotalTime();

	while ( iNumLeft > 0 )
	{
	PICK:
		// Grow a random empty object
		TINT iObject = ( iNumEmpty == 1 ) ? 1 : ARandom::GetSingleton()->m_oRandom.GetInt( iNumEmpty - 1 ) + 1;

		for ( auto it = pLists->llEmpty.Begin(); it != pLists->llEmpty.End(); it = it->Next() )
		{
			GrowthObject* pObject = it;

			if ( a_uiGroup == pObject->uiGroup && pObject->fGrowTime <= TIME_EMPTY && --iObject == 0 )
			{
				iNumLeft--;
				iNumEmpty--;

				// NOTE: retail inlines ARandom's RandRaw, it's a random number between 0 and 10
				TFLOAT fRandom = ( ARandom::GetSingleton()->m_oRandom.RandRaw() >> 1 ) * 4.6566129e-9f;

				GrowthObject* pNext = pObject;
				pLists->SetGrowing( pObject, fRandom + fTime - a_fAge, pNext );

				if ( iNumLeft > 0 )
					goto PICK;

				it = pNext;
			}
		}
	}

	// NOTE: retail returns this rather than the number of grown objects
	return iNumEmpty - iNumLeft;
}

// $Barnyard: FUNCTION 005e52f0
TINT ARegrowthManager::GrowFruits( TINT a_uiGroup, TINT a_iCount, TFLOAT a_fAge )
{
	TINT iNumLeft = a_iCount;

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		iNumLeft -= GrowObjects( a_uiGroup, iNumLeft, a_fAge, i );

		if ( iNumLeft <= 0 )
			break;
	}

	return a_iCount - iNumLeft;
}

// $Barnyard: FUNCTION 005e5340
TINT ARegrowthManager::GrowGroup( TUINT a_uiGroup )
{
	TINT iResult = 0;

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		iResult = HasTypeFlag( i, TYPEFLAG_FRUIT );

		if ( iResult )
		{
			GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

			iResult = GrowObjects( a_uiGroup, pMapping->uiNumFruits, TFLOAT( pMapping->iNumStages ) / pMapping->fStagesPerSecond, i );

			if ( iResult != 0 )
				break;
		}
	}

	return iResult;
}

// $Barnyard: FUNCTION 005e53a0
void ARegrowthManager::GrowAllInType( TINT a_iType, TINT a_iCount )
{
	TBOOL abGroups[ 256 ];
	TUtil::MemSet( abGroups, 0, sizeof( abGroups ) );

	T2_FOREACH( m_aTypeLists[ a_iType ].llEmpty, it )
	{
		abGroups[ it->uiGroup ] = TTRUE;
	}

	GrowthMapping* pMapping = &m_pGrowthMapping[ a_iType ];

	for ( TINT i = 0; i < 256; i++ )
	{
		if ( abGroups[ i ] )
			GrowObjects( i, a_iCount, TFLOAT( pMapping->iNumStages ) / pMapping->fStagesPerSecond, a_iType );
	}
}

// $Barnyard: FUNCTION 005e5ea0
void ARegrowthManager::RespawnInstant()
{
	TINT iNumTypes = ms_pTypes->GetNumEnums();

	for ( TINT i = 0; i <= iNumTypes; i++ )
	{
		if ( !HasTypeFlag( i, TYPEFLAG_INSTANT ) )
			continue;

		TypeLists* pLists = &m_aTypeLists[ i ];

		for ( auto it = pLists->llEmpty.Begin(); it != pLists->llEmpty.End(); )
		{
			GrowthObject* pObject = it;
			it                    = it->Next();

			StartGrowing( i, pObject, 0.0f );
		}
	}

	m_iNumRespawned = 1;
}

// $Barnyard: FUNCTION 005e5f60
void ARegrowthManager::RespawnDayRespawn()
{
	TINT iNumTypes = ms_pTypes->GetNumEnums();

	for ( TINT i = 0; i <= iNumTypes; i++ )
	{
		TBOOL bInstant = HasTypeFlag( i, TYPEFLAG_INSTANT );

		if ( !HasTypeFlag( i, TYPEFLAG_DAYRESPAWN ) )
			continue;

		TypeLists* pLists = &m_aTypeLists[ i ];

		for ( auto it = pLists->llEmpty.Begin(); it != pLists->llEmpty.End(); )
		{
			GrowthObject* pObject = it;
			it                    = it->Next();

			if ( bInstant || ( pObject->uiCrate & 7 ) == 7 )
				StartGrowing( i, pObject, 0.0f );
		}
	}

	m_iNumRespawned = 1;
}

// $Barnyard: FUNCTION 005e6c60
void ARegrowthManager::NewDay( TBOOL a_bSkipSimPersist, TBOOL a_bAllDayRespawn )
{
	TINT iNumTypes = ms_pTypes->GetNumEnums();

	for ( TINT i = 0; i < iNumTypes; i++ )
	{
		if ( HasTypeFlag( i, TYPEFLAG_FRUIT ) )
			GrowAllInType( i, m_pGrowthMapping[ i ].uiNumFruits );
	}

	if ( a_bAllDayRespawn )
		RespawnInstant();
	else
		RespawnDayRespawn();

	if ( !a_bSkipSimPersist )
	{
		ASimPersistData::GetSingleton()->SetFlags( ASimPersistData::FLAGS_REGROWTH_NEWDAY );
		ASimPersistData::GetSingleton()->SetFlags( ASimPersistData::FLAGS_REGROWTH_NEWDAY2 );
	}
}

// $Barnyard: FUNCTION 005e6ce0
void ARegrowthManager::NewDayAll()
{
	NewDay( TFALSE, TTRUE );
}

// $Barnyard: FUNCTION 005e6cf0
void ARegrowthManager::NewDayFruits( TBOOL a_bUnused )
{
	TINT iNumTypes = ms_pTypes->GetNumEnums();

	for ( TINT i = 0; i < iNumTypes; i++ )
	{
		if ( HasTypeFlag( i, TYPEFLAG_FRUIT ) )
			GrowAllInType( i, m_pGrowthMapping[ i ].uiNumFruits );
	}

	RespawnInstant();
}

// $Barnyard: FUNCTION 005e6d50
void ARegrowthManager::NewDayFruits2()
{
	TINT iNumTypes = ms_pTypes->GetNumEnums();

	for ( TINT i = 0; i < iNumTypes; i++ )
	{
		if ( HasTypeFlag( i, TYPEFLAG_FRUIT ) )
			GrowAllInType( i, m_pGrowthMapping[ i ].uiNumFruits );
	}

	RespawnInstant();
}

// $Barnyard: FUNCTION 005e5140
void ARegrowthManager::DeactivateObjectCollision( GrowthObject* a_pObject )
{
	Group* pGroup = FindGroup( a_pObject->uiGroup );

	if ( !pGroup )
		return;

	ACollisionObjectModelNode* pNode = FindCollisionNode( a_pObject, &pGroup->llCollisionNodes );

	if ( pNode )
	{
		pGroup->llCollisionNodes.Transfer( pNode, m_llPendingCollisionNodes );
		pNode->UnlinkCollisionObject();
	}
}

// $Barnyard: FUNCTION 005e4410
void ARegrowthManager::Update( TFLOAT a_fDeltaTime )
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_ACTIVE ) )
		return;

	m_fCollisionUpdateTimer -= a_fDeltaTime;

	if ( m_fCollisionUpdateTimer >= 0.0f )
		return;

	m_fCollisionUpdateTimer = 0.5f;
	TFLOAT fTime            = AGameTimeManager::GetSingleton()->GetTotalTime();


	// Activate the collision of the objects that grew
	for ( auto it = m_llPendingCollisionNodes.Begin(); it != m_llPendingCollisionNodes.End(); ++it )
	{
		ACollisionObjectModelNode* pNode   = it;
		GrowthObject*              pObject = pNode->m_pObject;

		if ( pObject->fGrowTime == TIME_EMPTY )
			continue;

		TFLOAT         fAge     = fTime - pObject->fGrowTime;
		GrowthMapping* pMapping = &m_pGrowthMapping[ FindTypeOfObject( pObject ) ];

		if ( StageToInt( fAge * pMapping->fStagesPerSecond ) >= pMapping->iNumStages - 1 )
		{
			pNode->Activate();

			// NOTE: retail doesn't check if the group exists
			Group* pGroup = FindGroup( pObject->uiGroup );
			it            = m_llPendingCollisionNodes.Transfer( pNode, pGroup->llCollisionNodes );
		}
	}
}

// $Barnyard: FUNCTION 005e45a0
TBOOL ARegrowthManager::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	TUINT32 auiGrownGroups[ 8 ];
	TUtil::MemSet( auiGrownGroups, 0, sizeof( auiGrownGroups ) );

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		TypeLists* pLists = &m_aTypeLists[ i ];

		if ( HasTypeFlag( i, TYPEFLAG_FRUIT ) )
		{
			// Only which trees have grown fruits is saved, GrowGroup grows them again
			GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

			T2_FOREACH( pLists->llGrowing, it )
			{
				if ( it->fGrowTime > TIME_EMPTY && StageToInt( ( AGameTimeManager::GetSingleton()->GetTotalTime() - it->fGrowTime ) * pMapping->fStagesPerSecond ) >= pMapping->iNumStages - 1 )
					auiGrownGroups[ it->uiGroup >> 5 ] |= 1 << ( it->uiGroup & 0x1f );
			}
		}
		else
		{
			a_pBuffer->WriteInt( pLists->llGrowing.Size() );

			T2_FOREACH( pLists->llGrowing, it )
			{
				TFLOAT fKey = it->GetSaveKey();
				a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &fKey ) );
				a_pBuffer->WriteUInt8( EncodeGrowTime( it->fGrowTime ) );
			}

			a_pBuffer->WriteInt( pLists->llEmpty.Size() );

			T2_FOREACH( pLists->llEmpty, it )
			{
				TFLOAT fKey = it->GetSaveKey();
				a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &fKey ) );
				a_pBuffer->WriteUInt8( EncodeGrowTime( it->fGrowTime ) );
			}
		}
	}

	for ( TINT i = 0; i < 8; i++ )
		a_pBuffer->WriteInt( auiGrownGroups[ i ] );

	// The collisions waiting for their object to grow
	a_pBuffer->WriteInt( m_llPendingCollisionNodes.Size() );

	T2_FOREACH( m_llPendingCollisionNodes, it )
	{
		GrowthObject* pObject = it->m_pObject;

		TFLOAT fKey = pObject->GetSaveKey();
		a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &fKey ) );

		TINT iType = ms_pTypes->GetNumEnums() - 1;

		while ( iType >= 0 && !IsInType( pObject, iType ) )
			iType--;

		a_pBuffer->WriteUInt8( TUINT8( iType ) );
	}

	TINT iNumGroupNodes = m_aGroups[ 0 ].llCollisionNodes.Size();
	a_pBuffer->WriteInt( iNumGroupNodes );

	for ( TINT i = iNumGroupNodes; i > 0; i-- )
	{
		a_pBuffer->WriteInt( 0 );
		a_pBuffer->WriteUInt8( 0 );
	}

	for ( TINT i = 0; i < NUM_LIGHT_SLOTS; i++ )
	{
		a_pBuffer->WriteInt( m_aiLightIds[ i ] );
		a_pBuffer->WriteInt( m_aiLightUnk1[ i ] );
		a_pBuffer->WriteInt( m_aiLightUnk2[ i ] );
	}

	a_pBuffer->WriteInt( m_iNumRespawned );
	a_pBuffer->WriteString( "ARManEnd" );

	return TTRUE;
}

// $Barnyard: FUNCTION 005e5a50
TBOOL ARegrowthManager::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		TypeLists* pLists = &m_aTypeLists[ i ];

		// Everything is empty until the save says otherwise
		while ( !pLists->llGrowing.IsEmpty() )
		{
			GrowthObject* pObject = pLists->llGrowing.PopBack();
			pLists->llEmpty.PushBack( pObject );
		}

		if ( HasTypeFlag( i, TYPEFLAG_FRUIT ) )
			continue;

		TINT iNumGrowing;
		a_pBuffer->ReadInt( iNumGrowing );

		for ( TINT k = 0; k < iNumGrowing; k++ )
		{
			TFLOAT fKey;
			a_pBuffer->ReadFloat( fKey );

			GrowthObject* pObject = FindObjectBySaveKey( fKey, &pLists->llEmpty );

			if ( pObject )
			{
				pLists->llEmpty.Transfer( pObject, pLists->llGrowing );

				TUINT8 uiGrowTime;
				a_pBuffer->ReadUInt8( uiGrowTime );
				pObject->fGrowTime = DecodeGrowTime( uiGrowTime );
			}
			else
			{
				TUINT8 uiUnused;
				a_pBuffer->ReadUInt8( uiUnused );
			}
		}

		TINT iNumEmpty;
		a_pBuffer->ReadInt( iNumEmpty );

		for ( TINT k = 0; k < iNumEmpty; k++ )
		{
			TFLOAT fKey;
			a_pBuffer->ReadFloat( fKey );

			GrowthObject* pObject = FindObjectBySaveKey( fKey, &pLists->llEmpty );

			if ( pObject )
			{
				TUINT8 uiGrowTime;
				a_pBuffer->ReadUInt8( uiGrowTime );

				// NOTE: DecodeGrowTime is inlined here
				pObject->fGrowTime = DecodeGrowTime( uiGrowTime );
			}
			else
			{
				TUINT8 uiUnused;
				a_pBuffer->ReadUInt8( uiUnused );
			}
		}
	}

	// Grow the fruits of the trees that had them
	TUINT32 auiGrownGroups[ 8 ];
	TUtil::MemSet( auiGrownGroups, 0, sizeof( auiGrownGroups ) );

	for ( TINT i = 0; i < 8; i++ )
	{
		TUINT uiBits;
		a_pBuffer->ReadUInt( uiBits );
		auiGrownGroups[ i ] = uiBits;
	}

	for ( TINT i = 0; i < 255; i++ )
	{
		if ( auiGrownGroups[ i >> 5 ] & ( 1 << ( i & 0x1f ) ) )
			GrowGroup( i );
	}

	// Activate the pending collisions
	while ( !m_llPendingCollisionNodes.IsEmpty() )
	{
		ACollisionObjectModelNode* pNode   = m_llPendingCollisionNodes.Begin();
		GrowthObject*              pObject = pNode->m_pObject;

		// NOTE: retail looks for the type of the object, but doesn't use it
		for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
		{
			if ( IsInType( pObject, i ) )
				break;
		}

		pNode->Activate();
		m_llPendingCollisionNodes.Transfer( pNode, FindGroup( pObject->uiGroup )->llCollisionNodes );
	}

	TINT iNumPending;
	a_pBuffer->ReadInt( iNumPending );

	for ( TINT i = 0; i < iNumPending; i++ )
	{
		TFLOAT fKey;
		a_pBuffer->ReadFloat( fKey );

		TUINT8 uiType;
		a_pBuffer->ReadUInt8( uiType );

		GrowthObject* pObject = FindObjectBySaveKey( fKey, &m_aTypeLists[ uiType ].llEmpty );

		if ( pObject || ( pObject = FindObjectBySaveKey( fKey, &m_aTypeLists[ uiType ].llGrowing ) ) )
			DeactivateObjectCollision( pObject );
	}

	TINT iNumGroupNodes;
	a_pBuffer->ReadInt( iNumGroupNodes );

	for ( TINT i = 0; i < iNumGroupNodes; i++ )
	{
		TFLOAT fUnused;
		a_pBuffer->ReadFloat( fUnused );

		TUINT8 uiUnused;
		a_pBuffer->ReadUInt8( uiUnused );
	}

	for ( TINT i = 0; i < NUM_LIGHT_SLOTS; i++ )
	{
		a_pBuffer->ReadInt( m_aiLightIds[ i ] );
		a_pBuffer->ReadInt( m_aiLightUnk1[ i ] );
		a_pBuffer->ReadInt( m_aiLightUnk2[ i ] );
	}

	a_pBuffer->ReadInt( m_iNumRespawned );
	a_pBuffer->CheckString( "ARManEnd" );

	return TTRUE;
}

// $Barnyard: FUNCTION 005e4d00
TINT ARegrowthManager::AddObject( const TMatrix44& a_rcMatrix, const TMatrix44& a_rcObjectMatrix, const TCHAR* a_szName, TUINT8 a_uiGroup, TINT a_iType, TBOOL a_bGrown, TINT& a_rOutType, GrowthObject*& a_rpOutObject, TINT a_iScale )
{
	TINT  iNumInGroup = 0;
	TINT  iMaxInGroup = 1000000;
	TINT  iNumTypes   = ms_pTypes->GetNumEnums();
	TBOOL bFruit      = TFALSE;
	TINT  iType;

	a_rpOutObject = TNULL;
	a_rOutType    = a_iType;

	if ( !HasTypeFlag( a_iType, TYPEFLAG_FRUIT ) )
	{
		// The type is in the name of the locator / bone
		iType = FindTypeByPrefix( a_szName, ms_pTypes->GetBoneNames(), iNumTypes );

		// The fruits only grow on the trees
		if ( HasTypeFlag( iType, TYPEFLAG_FRUIT ) )
			return -1;
	}
	else
	{
		iNumInGroup = CountInType( a_uiGroup, a_iType );
		iMaxInGroup = m_pGrowthMapping[ a_iType ].uiNumFruits;
		bFruit      = TTRUE;

		// Only the "Fruit" bones of the tree
		if ( T2String8::CompareNoCase( "Fruit", a_szName, T2String8::Length( "Fruit" ) ) != 0 )
			return -1;

		iType = a_iType;
	}

	if ( iType == -1 || iType == iNumTypes || m_llFreeObjects.IsEmpty() )
		return -1;

	GrowthObject* pObject = m_llFreeObjects.PopFront();
	a_rpOutObject         = pObject;

	TMatrix44 matObject;

	if ( bFruit )
	{
		// NOTE: retail copies the matrix first
		TMatrix44 matCopy = a_rcObjectMatrix;
		matObject.Multiply( a_rcMatrix, matCopy );
	}
	else
	{
		matObject.Multiply( a_rcMatrix, a_rcObjectMatrix );
	}

	TMatrix44::MatToQuat( pObject->quatRotation, matObject );
	pObject->vecPosition = matObject.GetTranslation();

	if ( HasTypeFlag( iType, TYPEFLAG_DAYRESPAWN ) )
	{
		pObject->uiCrate = 7;
		pObject->uiCrate = TUINT8( ( FindCrateSchema( a_szName ) << 3 ) | ( pObject->uiCrate & 7 ) );
	}

	pObject->SetScale( a_iScale * 0.01f );
	pObject->uiGroup   = a_uiGroup;
	pObject->fGrowTime = TIME_EMPTY;

	TBOOL bGroupFull = bFruit ? ( iNumInGroup + 1 > iMaxInGroup ) : TFALSE;

	if ( HasTypeFlag( iType, TYPEFLAG_DAYRESPAWN ) ||
	     HasTypeFlag( iType, TYPEFLAG_INSTANT ) ||
	     bGroupFull ||
	     ( !a_bGrown && m_pGrowthMapping[ iType ].iNumStages > 1 ) )
	{
		pObject->fGrowTime = TIME_EMPTY;
		m_aTypeLists[ iType ].llEmpty.PushFront( pObject );
	}
	else
	{
		pObject->fGrowTime = AGameTimeManager::GetSingleton()->GetTotalTime() - 100000.0f;
		m_aTypeLists[ iType ].llGrowing.PushFront( pObject );
	}

	a_rOutType = iType;

	// The model of the grown object
	return m_pGrowthMapping[ iType ].uiFirstModel + m_pGrowthMapping[ iType ].iNumStages - 2;
}

// $Barnyard: FUNCTION 005e23b0
void ARegrowthManager::AddCollisionEntry( GrowthObject* a_pObject, TINT* a_piNumEntries, TINT* a_piNumMeshes, TINT* a_piNumCollisions, const TCHAR* a_szName, TINT a_iType, TINT a_iModel, ACollisionModelSet** a_ppModelSets, CollisionEntry* a_pEntries, TBOOL* a_pbUsedModels )
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_ALLOW_COLL ) || !HASANYFLAG( m_eFlags, FLAGS_ALLOW_COLL2 ) )
		return;

	TManagedModel* pModel = m_pModels[ a_iModel ].pModel;

	if ( !pModel )
		return;

	TBOOL bMesh   = HASANYFLAG( m_eFlags, FLAGS_COLLISION ) && a_ppModelSets[ a_iModel ] && pModel->GetModel()->GetNumCollisionMeshes() > 0;
	TBOOL bRadius = pModel->GetModel()->GetSkeleton()->GetBoneID( "radius" ) != -1;

	if ( ( bMesh || bRadius || T2String8::FindString( a_szName, "_coll" ) || HasTypeFlag( a_iType, TYPEFLAG_COLLISION ) ) && !T2String8::FindString( a_szName, "_nc" ) )
	{
		a_pEntries[ *a_piNumEntries ].uiType  = TUINT8( a_iType );
		a_pEntries[ *a_piNumEntries ].pObject = a_pObject;

		if ( bMesh )
		{
			a_pEntries[ *a_piNumEntries ].eKind = CollisionEntry::KIND_MESH;
			( *a_piNumEntries )++;

			if ( !a_pbUsedModels[ a_iModel ] )
			{
				a_pbUsedModels[ a_iModel ] = TTRUE;
				( *a_piNumMeshes )++;
				( *a_piNumCollisions )++;
				return;
			}
		}
		else
		{
			a_pEntries[ *a_piNumEntries ].eKind = CollisionEntry::KIND_SIMPLE;
			( *a_piNumEntries )++;
		}

		( *a_piNumCollisions )++;
	}
}

// $Barnyard: FUNCTION 005e6090
void ARegrowthManager::CreateCollisions( TINT a_iNumCollisions, TINT a_iNumMeshes, TUINT8 a_uiGroup, TBOOL* a_pbUsedModels, ACollisionModelSet** a_ppModelSets, CollisionEntry* a_pEntries, const TMatrix44& a_rcMatrix )
{
	if ( a_iNumCollisions <= 0 )
		return;

	Group* pGroup     = FindOrCreateGroup( a_uiGroup );
	pGroup->uiGroupId = a_uiGroup;

	// The collision model instances of the models that use the collision meshes
	ModelInstanceNode* apModelInstances[ MAX_NUM_COLLISION_SETS ];
	TUtil::MemSet( apModelInstances, 0, sizeof( apModelInstances ) );

	if ( a_iNumMeshes > 0 )
	{
		for ( TINT i = 0; i < MAX_NUM_COLLISION_SETS; i++ )
		{
			if ( a_pbUsedModels[ i ] )
			{
				ACollisionModelSet* pModelSet = a_ppModelSets[ i ];
				ModelInstanceNode*  pNode     = m_llFreeModelInstances.PopBack();

				pGroup->llModelInstances.PushBack( pNode );
				pNode->oInstance.Create( pModelSet, m_pModels[ i ].pSceneObject );

				apModelInstances[ i ] = pNode;
			}
		}
	}

	for ( TINT i = 0; i < a_iNumCollisions; i++ )
	{
		CollisionEntry* pEntry = &a_pEntries[ i ];

		if ( pEntry->eKind == CollisionEntry::KIND_NONE )
			continue;

		GrowthObject*  pObject  = pEntry->pObject;
		TINT           iType    = pEntry->uiType;
		GrowthMapping* pMapping = &m_pGrowthMapping[ iType ];

		// A capsule from the centre of the model...
		const TSphere& rcSphere = m_pModels[ pMapping->uiFirstModel ].pModel->GetModel()->GetLOD( 0 ).BoundingSphere;

		TVector4 vecDirection = TVector4::VEC_NEGY;
		TFLOAT   fRadius      = rcSphere.GetRadius();
		TFLOAT   fLength      = 0.0f;
		TVector4 vecCentre( rcSphere.GetOrigin().x, rcSphere.GetOrigin().y, rcSphere.GetOrigin().z, 1.0f );

		TTransformObject oTransform;
		oTransform.SetMatrix( TMatrix44::IDENTITY );
		oTransform.SetScale( TVector3( 1.0f, 1.0f, 1.0f ) );

		// Only the objects using the collision mesh of the model have these
		ACollisionModelInstance* pModelInstance = TNULL;
		TTransformObject*        pTransform     = TNULL;

		TMatrix44 matObject;
		matObject.Identity();
		matObject.SetFromQuaternion( pObject->quatRotation );
		matObject.SetTranslation( pObject->vecPosition );

		TMatrix44::TransformVector( vecCentre, matObject, vecCentre );

		// ...or the bones of the model of the grown object
		TSkeleton* pSkeleton = m_pModels[ pMapping->uiFirstModel + pMapping->iNumStages - 2 ].pModel->GetModel()->GetSkeleton();
		TBOOL      bHasEnd   = TFALSE;

		if ( pSkeleton->GetBoneID( "rock01_01" ) != -1 )
		{
			vecCentre = pObject->vecPosition;
			fRadius   = 0.6f;
			fLength   = 0.2f;
		}

		TINT iBone = pSkeleton->GetBoneID( "centre" );

		if ( iBone != -1 )
		{
			TSkeletonBone* pBone = pSkeleton->GetBone( iBone );

			if ( pBone )
			{
				TMatrix44 matBone;
				matBone.Multiply( matObject, pBone->GetTransform() );
				vecCentre = matBone.GetTranslation();
			}
		}

		iBone = pSkeleton->GetBoneID( "end" );

		if ( iBone != -1 )
		{
			TSkeletonBone* pBone = pSkeleton->GetBone( iBone );

			if ( pBone )
			{
				TMatrix44 matBone;
				matBone.Multiply( matObject, pBone->GetTransform() );

				TVector4 vecEnd = matBone.GetTranslation();

				if ( !vecEnd.IsEqual( vecCentre ) )
				{
					vecDirection.x = vecEnd.x - vecCentre.x;
					vecDirection.y = vecEnd.y - vecCentre.y;
					vecDirection.z = vecEnd.z - vecCentre.z;
					vecDirection.w = vecEnd.w;
					fLength        = vecDirection.NormaliseMag();
					bHasEnd        = TTRUE;
				}
			}
		}

		iBone = pSkeleton->GetBoneID( "radius" );

		if ( iBone != -1 )
		{
			TSkeletonBone* pBone = pSkeleton->GetBone( iBone );

			if ( pBone )
			{
				TMatrix44 matBone;
				matBone.Multiply( matObject, pBone->GetTransform() );

				TVector4 vecRadius = matBone.GetTranslation();

				TVector4 vecOffset;
				vecOffset.Multiply( vecDirection, fLength );

				TVector4 vecEnd( vecOffset.x + vecCentre.x, vecOffset.y + vecCentre.y, vecOffset.z + vecCentre.z, vecOffset.w );

				if ( bHasEnd )
				{
					if ( !vecCentre.IsEqual( vecEnd ) )
					{
						TFLOAT fDistanceSq = TMath::DistancePointToSegmentSq( vecRadius, vecCentre.AsVector3(), vecEnd.AsVector3() );

						if ( fDistanceSq > 0.0001f )
						{
							fRadius = TMath::Sqrt( fDistanceSq );

							if ( pEntry->eKind != CollisionEntry::KIND_MESH )
								goto NO_MESH;

							fRadius *= 1.2f;
						}
					}
				}
				else
				{
					TFLOAT fDistanceSq = TVector4::DistanceSq( vecRadius, vecCentre );

					if ( fDistanceSq > 0.0001f )
					{
						fLength = 0.0f;
						fRadius = TMath::Sqrt( fDistanceSq ) * 1.2f;

						if ( pEntry->eKind != CollisionEntry::KIND_MESH )
							goto NO_MESH;

						fRadius *= 1.2f;
					}
				}
			}
		}

		if ( pEntry->eKind == CollisionEntry::KIND_MESH )
		{
			// The collision mesh of the model
			ModelInstanceNode* pInstanceNode = apModelInstances[ pMapping->uiFirstModel ];
			pModelInstance                   = pInstanceNode ? &pInstanceNode->oInstance : TNULL;

			oTransform.SetMatrix( matObject );
			oTransform.SetScale( TVector3( 1.0f, 1.0f, 1.0f ) );
			pTransform = &oTransform;
		}

	NO_MESH:
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail doesn't check if there are free nodes left
		if ( m_llFreeCollisionNodes.IsEmpty() )
			continue;
#endif

		ACollisionObjectModelNode* pNode = m_llFreeCollisionNodes.PopBack();
		pGroup->llCollisionNodes.PushBack( pNode );
		pNode->m_pObject = pObject;

		// NOTE: these are ACollisionObject::TYPE_ flags, crates also get TYPE_MOVABLE
		TUINT eTypeFlags = ACollisionObject::TYPE_INSTANCE;

		if ( HasTypeFlag( iType, TYPEFLAG_CRATE ) )
			eTypeFlags = ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_INSTANCE;

		pNode->Create( vecCentre, fRadius, fLength, vecDirection, eTypeFlags, pModelInstance, pTransform );

		// The collision of the taken objects is activated once they grow again
		if ( pObject->fGrowTime == TIME_EMPTY )
			DeactivateObjectCollision( pObject );
	}
}

// $Barnyard: FUNCTION 005e6810
void ARegrowthManager::CreateFromSkeleton( TSkeleton* a_pSkeleton, const TMatrix44& a_rcMatrix, TUINT8 a_uiGroup, TUINT a_uiType, TBOOL a_bGrown )
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_ACTIVE ) )
		return;

	TINT                iNumEntries    = 0;
	TINT                iNumMeshes     = 0;
	TINT                iNumCollisions = 0;
	TBOOL               abUsedModels[ MAX_NUM_COLLISION_SETS + 4 ];
	ACollisionModelSet* apModelSets[ MAX_NUM_COLLISION_SETS ];

	GetModelCollisionSets( abUsedModels, apModelSets );

	TINT iNumAdded = 0;

	for ( TINT i = 0; i < a_pSkeleton->GetBoneCount(); i++ )
	{
		TSkeletonBone* pBone = a_pSkeleton->GetBone( i );

		// "_sNN": the scale in percents
		TFLOAT       fScale;
		const TCHAR* pScale = T2String8::FindString( pBone->GetName(), "_s" );

		if ( pScale )
			fScale = T2String8::StringToFloat( pScale + 2 );
		else
			fScale = 100.0f;

		TINT          iType;
		GrowthObject* pObject;
		TINT          iModel = AddObject( a_rcMatrix, pBone->GetTransform(), pBone->GetName(), a_uiGroup, a_uiType, a_bGrown, iType, pObject, TINT( fScale ) );

		if ( iModel >= 0 )
		{
			AddCollisionEntry( pObject, &iNumEntries, &iNumMeshes, &iNumCollisions, pBone->GetName(), iType, iModel, apModelSets, s_pCollisionEntries, abUsedModels );
			iNumAdded++;
		}
	}

	if ( iNumAdded == 0 && m_llFreeObjects.Size() > 0 && a_uiType != TUINT( -1 ) && a_uiType < TUINT( ms_pTypes->GetNumEnums() ) )
	{
		// The tree has no objects, add one at its position
		GrowthObject* pObject = m_llFreeObjects.PopFront();
		pObject->uiGroup      = a_uiGroup;
		pObject->vecPosition  = a_rcMatrix.GetTranslation();
		pObject->quatRotation = TQuaternion::IDENTITY;

		m_aTypeLists[ a_uiType ].llGrowing.PushFront( pObject );
	}

	CreateCollisions( iNumCollisions, iNumMeshes, a_uiGroup, abUsedModels, apModelSets, s_pCollisionEntries, a_rcMatrix );
}

// $Barnyard: FUNCTION 005e6a30
void ARegrowthManager::CreateFromLocatorList( ATerrainLocatorList* a_pLocatorList, const TMatrix44& a_rcMatrix, TUINT8 a_uiGroup, TUINT a_uiType, TBOOL a_bGrown )
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_ACTIVE ) )
		return;

	TINT                iNumEntries    = 0;
	TINT                iNumMeshes     = 0;
	TINT                iNumCollisions = 0;
	TBOOL               abUsedModels[ MAX_NUM_COLLISION_SETS + 4 ];
	ACollisionModelSet* apModelSets[ MAX_NUM_COLLISION_SETS ];

	GetModelCollisionSets( abUsedModels, apModelSets );

	TINT iNumAdded = 0;

	for ( TINT i = 0; i < a_pLocatorList->GetNumLocators(); i++ )
	{
		const ATerrainLocator* pLocator = a_pLocatorList->GetLocator( i );

		TMatrix44 matLocator;
		pLocator->GetMatrix( matLocator );

		// The scale of the locator (in percents)
		TINT iScale = pLocator->uiFlags1 & 0xf;

		TINT          iType;
		GrowthObject* pObject;
		TUINT         uiModel = AddObject( a_rcMatrix, matLocator, a_pLocatorList->GetLocatorName( i ), a_uiGroup, a_uiType, a_bGrown, iType, pObject, TINT( iScale * 20.0 ) );

		if ( uiModel < 200 )
		{
			AddCollisionEntry( pObject, &iNumEntries, &iNumMeshes, &iNumCollisions, a_pLocatorList->GetLocatorName( i ), iType, uiModel, apModelSets, s_pCollisionEntries, abUsedModels );
			iNumAdded++;
		}
	}

	if ( iNumAdded == 0 && a_uiType != TUINT( -1 ) && a_uiType < TUINT( ms_pTypes->GetNumEnums() ) )
	{
		// NOTE: retail doesn't check if the free list is empty here
		GrowthObject* pObject = m_llFreeObjects.PopFront();
		pObject->uiGroup      = a_uiGroup;
		pObject->vecPosition  = a_rcMatrix.GetTranslation();

		m_aTypeLists[ a_uiType ].llGrowing.PushFront( pObject );
	}

	CreateCollisions( iNumCollisions, iNumMeshes, a_uiGroup, abUsedModels, apModelSets, s_pCollisionEntries, a_rcMatrix );
}

// $Barnyard: FUNCTION 005e6ff0
TUINT8 ARegrowthManager::CreateFromLocators( ATerrainLocatorList* a_pLocatorList )
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_ACTIVE ) )
		return 0;

	// The locators are Z up
	TMatrix44 matTransform;
	matTransform.m_f11 = 1.0f;
	matTransform.m_f12 = 0.0f;
	matTransform.m_f13 = 0.0f;
	matTransform.m_f14 = 0.0f;
	matTransform.m_f21 = 0.0f;
	matTransform.m_f22 = 0.0f;
	matTransform.m_f23 = 1.0f;
	matTransform.m_f24 = 0.0f;
	matTransform.m_f31 = 0.0f;
	matTransform.m_f32 = -1.0f;
	matTransform.m_f33 = 0.0f;
	matTransform.m_f34 = 0.0f;
	matTransform.m_f41 = 0.0f;
	matTransform.m_f42 = 0.0f;
	matTransform.m_f43 = 0.0f;
	matTransform.m_f44 = 1.0f;

	TUINT8 uiGroup = AllocateGroupId();
	CreateFromLocatorList( a_pLocatorList, matTransform, uiGroup, TUINT( -1 ), TTRUE );

	return uiGroup;
}

// $Barnyard: FUNCTION 005e3270
void ARegrowthManager::BuildRenderList()
{
	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	TINT   iNumRendered      = 0;
	TFLOAT fRenderDistanceSq = m_fRenderDistance * m_fRenderDistance;
	TFLOAT fLODDistanceSq    = m_fLODDistance * m_fLODDistance;

	TVector4 vecCameraPos = pRenderContext->GetViewWorldMatrix().GetTranslation();
	pRenderContext->GetViewWorldMatrix();

	TFLOAT fTime = AGameTimeManager::GetSingleton()->GetTotalTime();

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

		if ( pMapping->uiFirstModel == 0xff )
			continue;

		GROWTHSTYLE eStyle = pMapping->eGrowthStyle;

		T2_FOREACH( m_aTypeLists[ i ].llGrowing, it )
		{
			GrowthObject* pObject = it;
			TFLOAT        fStage  = ( fTime - pObject->fGrowTime ) * pMapping->fStagesPerSecond;
			TINT          iStage  = StageToInt( fStage );

			if ( eStyle == GROWTHSTYLE_SCALE )
			{
				if ( fStage <= 0.001f )
					continue;

				if ( iStage < 1 )
					iStage = 1;

				fStage = ( fStage - 0.001f ) * 1.001f;
			}
			else if ( iStage <= 0 )
			{
				continue;
			}

			TSphere oSphere( pObject->vecPosition.x, pObject->vecPosition.y, pObject->vecPosition.z, m_pModels[ pMapping->uiFirstModel ].pModel->GetModel()->GetRenderDistance() * 1.2f );
			TFLOAT  fDistanceSq = TVector4::DistanceSq( pObject->vecPosition, vecCameraPos );

			if ( fDistanceSq >= fRenderDistanceSq )
				continue;

			TBOOL bFar = fDistanceSq >= fLODDistanceSq;

			if ( !TRenderContext::CullSphereToFrustumSimple( oSphere, pRenderContext->GetWorldPlanes(), 6 ) )
				continue;

			if ( iNumRendered >= MAX_NUM_RENDERED_OBJECTS )
				break;

			if ( iStage >= pMapping->iNumStages )
				iStage = pMapping->iNumStages - 1;

			s_auiRenderClipFlags[ iNumRendered ] = TRenderContext::CLIPFLAGS_ALL;
			RenderData* pRenderData              = &m_pRenderData[ iNumRendered++ ];

			TMatrix44 matObject;
			pObject->GetMatrix( matObject );
			pRenderData->oMatrix.Multiply( m_oModelView, matObject );

			if ( eStyle == GROWTHSTYLE_SCALE )
			{
				TINT   iNumStages = pMapping->iNumStages;
				TFLOAT fGrowth    = ( fStage < TFLOAT( iNumStages - 1 ) ) ? fStage / TFLOAT( iNumStages - 1 ) : 1.0f;

				// The last stage pops in
				if ( iStage == iNumStages - 2 && fGrowth >= 0.997f )
				{
					iStage++;
					fGrowth = ( fGrowth - 0.997f ) * 333.331f;
				}

				TFLOAT fScale = pObject->GetScale() * fGrowth;
				pRenderData->oMatrix.Scale( fScale, fScale, fScale );
			}

			TINT iList = 2 * ( iStage + pMapping->uiFirstModel ) - 2;

			if ( bFar )
				iList++;

			pRenderData->pNext       = m_ppRenderLists[ iList ];
			m_ppRenderLists[ iList ] = pRenderData;
		}
	}
}

// $Barnyard: FUNCTION 005e35d0
void ARegrowthManager::BuildRenderListInterior()
{
	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	TINT   iNumRendered      = 0;
	TFLOAT fRenderDistanceSq = m_fRenderDistance * m_fRenderDistance;
	TFLOAT fLODDistanceSq    = m_fLODDistance * m_fLODDistance;

	TVector4 vecCameraPos = pRenderContext->GetViewWorldMatrix().GetTranslation();
	pRenderContext->GetViewWorldMatrix();

	TFLOAT fTime = AGameTimeManager::GetSingleton()->GetTotalTime();

	for ( TINT i = ms_pTypes->GetNumEnums() - 1; i >= 0; i-- )
	{
		GrowthMapping* pMapping = &m_pGrowthMapping[ i ];

		if ( pMapping->uiFirstModel == 0xff )
			continue;

		GROWTHSTYLE eStyle = pMapping->eGrowthStyle;

		T2_FOREACH( m_aTypeLists[ i ].llGrowing, it )
		{
			GrowthObject* pObject = it;
			TFLOAT        fStage  = ( fTime - pObject->fGrowTime ) * pMapping->fStagesPerSecond;
			TINT          iStage  = StageToInt( fStage );

			if ( eStyle == GROWTHSTYLE_SCALE )
			{
				if ( fStage <= 0.001f )
					continue;

				if ( iStage < 1 )
					iStage = 1;

				fStage = ( fStage - 0.001f ) * 1.001f;
			}
			else if ( iStage <= 0 )
			{
				continue;
			}

			TSphere oSphere( pObject->vecPosition.x, pObject->vecPosition.y, pObject->vecPosition.z, m_pModels[ pMapping->uiFirstModel ].pModel->GetModel()->GetRenderDistance() * 1.2f );
			TFLOAT  fDistanceSq = TVector4::DistanceSq( pObject->vecPosition, vecCameraPos );

			if ( fDistanceSq >= fRenderDistanceSq )
				continue;

			// The objects of the other floor of the barn aren't rendered
			// Only the objects of the barn floor the player is on (ABarnExploreMinigameState::ms_iFloor)
			const TINT iBarnFloor = ABarnExploreMinigameState::ms_iFloor;

			if ( !( oSphere.GetOrigin().y <= 3.246f || iBarnFloor == ABarnExploreMinigameState::FLOOR_LOFT ) ||
			     !( oSphere.GetOrigin().y >= 3.246f || iBarnFloor == ABarnExploreMinigameState::FLOOR_GROUND || iBarnFloor == ABarnExploreMinigameState::FLOOR_BASEMENT ) )
				continue;

			TBOOL bFar = fDistanceSq >= fLODDistanceSq;

			if ( !TRenderContext::CullSphereToFrustumSimple( oSphere, pRenderContext->GetWorldPlanes(), 6 ) )
				continue;

			if ( iNumRendered >= MAX_NUM_RENDERED_OBJECTS )
				break;

			if ( iStage >= pMapping->iNumStages )
				iStage = pMapping->iNumStages - 1;

			s_auiRenderClipFlags[ iNumRendered ] = TRenderContext::CLIPFLAGS_ALL;
			RenderData* pRenderData              = &m_pRenderData[ iNumRendered++ ];

			TMatrix44 matObject;
			pObject->GetMatrix( matObject );
			pRenderData->oMatrix.Multiply( m_oModelView, matObject );

			if ( eStyle == GROWTHSTYLE_SCALE )
			{
				TINT   iNumStages = pMapping->iNumStages;
				TFLOAT fGrowth    = ( fStage < TFLOAT( iNumStages - 1 ) ) ? fStage / TFLOAT( iNumStages - 1 ) : 1.0f;

				// The last stage pops in
				if ( iStage == iNumStages - 2 && fGrowth >= 0.997f )
				{
					iStage++;
					fGrowth = ( fGrowth - 0.997f ) * 333.331f;
				}

				TFLOAT fScale = ( HasTypeFlag( i, TYPEFLAG_DAYRESPAWN ) ? 1.0f : pObject->GetScale() ) * fGrowth;
				pRenderData->oMatrix.Scale( fScale, fScale, fScale );
			}

			TINT iList = 2 * ( iStage + pMapping->uiFirstModel ) - 2;

			if ( bFar )
				iList++;

			pRenderData->pNext       = m_ppRenderLists[ iList ];
			m_ppRenderLists[ iList ] = pRenderData;
		}
	}
}

// $Barnyard: FUNCTION 005e3990
TBOOL ARegrowthManager::Render()
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_ACTIVE ) )
		return TTRUE;

	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	m_oModelView = pRenderContext->GetModelViewMatrix();
	pRenderContext->ClearLightIDs();

	for ( TINT i = 0; i < 2 * m_iNumModels; i++ )
		m_ppRenderLists[ i ] = TNULL;

	m_pRenderData = s_aRenderData;

	if ( HASANYFLAG( m_eFlags, FLAGS_INTERIOR ) )
		BuildRenderListInterior();
	else
		BuildRenderList();

	TFLOAT fOldShadeCoeff = pRenderContext->GetShadeCoeff();
	pRenderContext->SetShadeCoeff( AInstanceLightingHelper::GetSingleton()->GetShadeCoeff() );

	TUINT uiOldClipFlags = pRenderContext->GetClipFlags();

	for ( TINT i = 0; i < 2 * m_iNumModels; i++ )
	{
		if ( !m_ppRenderLists[ i ] )
			continue;

		TModelInstance* pModelInstance = m_pModels[ i >> 1 ].pSceneObject->GetInstance();
		TModel*         pModel         = pModelInstance->GetModel();

		TINT iLOD = TMath::Min( i & 1, pModel->GetNumLODs() - 1 );

		TModelLOD& rLOD = pModel->GetLOD( iLOD );
		pRenderContext->SetSkeletonInstance( pModelInstance->GetSkeletonInstance() );

		for ( RenderData* pRenderData = m_ppRenderLists[ i ]; pRenderData; pRenderData = pRenderData->pNext )
		{
			// Fade out
			if ( pRenderData->oMatrix.m_f43 >= m_fFadeDistance )
			{
				TFLOAT fAlpha = 1.0f - ( pRenderData->oMatrix.m_f43 - m_fFadeDistance ) * m_fFadeScale;

				if ( fAlpha <= 0.0f )
					continue;

				pRenderContext->SetAlphaBlend( fAlpha );
			}
			else
			{
				pRenderContext->SetAlphaBlend( 1.0f );
			}

			pRenderContext->SetModelViewMatrix( pRenderData->oMatrix );

			for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
				rLOD.ppMeshes[ k ]->Render();
		}
	}

	pRenderContext->SetClipFlags( uiOldClipFlags );
	pRenderContext->SetShadeCoeff( fOldShadeCoeff );
	pRenderContext->SetAlphaBlend( 1.0f );
	pRenderContext->SetModelViewMatrix( m_oModelView );

	return TTRUE;
}
