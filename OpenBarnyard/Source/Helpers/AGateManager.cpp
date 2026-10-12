#include "pch.h"
#include "AGateManager.h"
#include "AGameLoader.h"
#include "AInstanceManager.h"
#include "FSM/AMethodPointer.h"
#include "Assets/AAssetLoader.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Player/APlayerManager.h"
#include "Quests/AQuestManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Toshi/T2String8.h>
#include <Toshi/THashMap.h>
#include <Toshi/TStringManager.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderContext.h>
#include <Render/TModel.h>
#include <Math/TSphere.h>

#include <ctype.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( tomato );

// NOTE: GetClass 005dd540, CreateObject 005dfa20, CreateObjectInPlace 005dfa80
// NOTE: GetClass 005dd540, CreateObject 005dfa20, CreateObjectInPlace 005dfa80
TDEFINE_CLASS_INIT( AGateManager, 1, 0 );

// DAT_0078de4c
AFunctionTable AGateManager::ms_oFunctionTable( &TGetClass( AGateManager ), TNULL );

// The prefix of the gate locators (PTR_s_wgate__00778690)
static const TCHAR* s_szGateLocatorPrefix = "wgate_";

// DAT_0078deac, initialised by a static initialiser (0070df70)
static TINT s_iGateLocatorPrefixLength = T2String8::Length( s_szGateLocatorPrefix );

// PTR_s_whitegate_00778694
static const TCHAR* s_apszModelNames[ AGateManager::MODEL_NUMOF ] = {
	"whitegate",
	"browngate",
	"junkgate",
	"golfgate",
	"barndoor",
	"padlock",
};

// PTR_s_barn_007786ac
static const TCHAR* s_apszGateTypes[ AGateManager::GATETYPE_NUMOF ] = {
	"barn",
	"southpath",
	"woods",
	"golf",
	"honey",
	"junkyard",
	"oldmill",
	"gulch",
	"nightbarn",
};

// Time since the nearby padlock was looked for (DAT_0078de48)
static TFLOAT s_fNearbyPadlockTimer;

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 005dda90
static TObject* __stdcall AGateManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return AGateManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005df8c0
void AGateManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AGateManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AGateManager>::Create( "OpenClosestGate", &AGateManager::OpenClosestGate ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AGateManager, TPString8>::Create( "CloseGateType", &AGateManager::CloseGateType ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AGateManager, TPString8>::Create( "IsGateOpen", &AGateManager::IsGateOpen ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AGateManager, TPString8, TBOOL>::Create( "SetGate", &AGateManager::SetGate ) );
}

// $Barnyard: FUNCTION 005dd550
void AGateManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 005dd7c0
AGateManager::Gate::Gate()
{
}

// $Barnyard: FUNCTION 005dd570
AGateManager::Gate::~Gate()
{
}

// $Barnyard: FUNCTION 005dd730
void AGateManager::Gate::Close()
{
	if ( !bOpen )
		return;

	if ( eModel == MODEL_PADLOCK )
	{
		SetFlags( GetFlags() | 1 );
		bOpen = TFALSE;
	}
	else
	{
		// Swing it back
		TMatrix44 matTransform;
		GetTransform().GetLocalMatrixImp( matTransform );
		matTransform.RotateZ( fAngle * -TMath::DegToRad( 1.0f ) );
		GetTransform().SetMatrix( matTransform );

		bOpen = TFALSE;
		oCollision.Activate();
	}
}

// $Barnyard: FUNCTION 005df950
AGateManager::AGateManager()
    : m_pGates( TNULL )
    , m_iNumGates( 0 )
    , m_pModels( TNULL )
    , m_pNearbyPadlock( TNULL )
    , m_pTomatoTarp( TNULL )
    , m_pTomatoFence1( TNULL )
    , m_pTomatoFence2( TNULL )
    , m_bTomatoGateOpen( TFALSE )
{
	m_oMirrorMatrix = TMatrix44::IDENTITY;
	m_oMirrorMatrix.Scale( -1.0f, 1.0f, 1.0f );

	Create();
}

// $Barnyard: FUNCTION 005de000
// $Barnyard: FUNCTION 005de570
AGateManager::~AGateManager()
{
	if ( m_pGates )
		delete[] m_pGates;

	if ( m_pModels )
		delete[] m_pModels;

	if ( m_pTomatoTarp )
	{
		// NOTE: retail calls 0061c680 (ACollisionObject::UnlinkCollisionObject), it removes the hash of the object (through a virtual) and unlinks it
		m_oTomatoCollision.UnlinkCollisionObject();

		AModelRepos::GetSingleton()->DestroyModelInstance( m_pTomatoTarp );
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pTomatoFence1 );
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pTomatoFence2 );
	}
}

// $Barnyard: FUNCTION 005de100
AFunctionTable* AGateManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 005deee0
void AGateManager::Create()
{
	ATerrainLocatorList* pLocatorList = ATerrainInterface::GetSingleton()->GetLocatorList();

	// NOTE: retail has room for 40 gate locators on the stack
	const ATerrainLocator* apLocators[ 40 ];
	TINT                   iNumGates = 0;

	for ( TINT i = 0; i < pLocatorList->GetNumLocators(); i++ )
	{
		if ( T2String8::Compare( pLocatorList->GetLocatorName( i ), s_szGateLocatorPrefix, s_iGateLocatorPrefixLength ) == 0 )
			apLocators[ iNumGates++ ] = pLocatorList->GetLocator( i );
	}

	m_iNumGates = iNumGates;
	m_pGates    = new Gate[ iNumGates ];
	m_pModels   = new TManagedModel[ NUM_LOADED_MODELS ];

	// The models of the gate models
	THashMap<MODEL, TManagedModel*> oModels( 17 );
	TINT                            iNumLoaded = 0;

	for ( TINT i = 0; i < MODEL_NUMOF; i++ )
	{
		if ( i == MODEL_BROWNGATE )
		{
			oModels.Insert( i, &m_pModels[ 0 ] );
		}
		else
		{
			TCHAR szPath[ 64 ];
			TStringManager::String8Format( szPath, sizeof( szPath ), "data\\models\\%s.trb", s_apszModelNames[ i ] );

			m_pModels[ iNumLoaded ].Create( szPath, AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) );
			oModels.Insert( i, &m_pModels[ iNumLoaded ] );
			iNumLoaded++;
		}
	}

	for ( TINT i = 0; i < iNumGates; i++ )
	{
		const ATerrainLocator* pLocator = apLocators[ i ];
		const TCHAR*           szName   = pLocatorList->GetHeader()->ppNames[ pLocator->uiNameId ];
		Gate*                  pGate    = &m_pGates[ i ];

		pGate->szName = szName;

		// The model
		TINT iModel = 0;

		while ( !T2String8::FindString( szName, s_apszModelNames[ iModel ] ) )
		{
			if ( ++iModel >= MODEL_NUMOF )
				break;
		}

		if ( iModel >= MODEL_NUMOF )
			continue;

		pGate->eModel = iModel;

		// The type
		TINT iType = 0;

		for ( ; iType < GATETYPE_NUMOF; iType++ )
		{
			TCHAR szType[ 64 ];
			TStringManager::String8Format( szType, sizeof( szType ), "%s%s%s", s_szGateLocatorPrefix, s_apszGateTypes[ iType ], "_" );

			if ( T2String8::FindString( szName, szType ) )
				break;
		}

		if ( iType >= GATETYPE_NUMOF )
			continue;

		pGate->eType = iType;
		pGate->bOpen = TFALSE;

		THashMap<MODEL, TManagedModel*>::Node* pModelNode = oModels.Find( pGate->eModel );
		pGate->Create( pModelNode ? pModelNode->oValue : TNULL );

		TMatrix44 matWorld;
		pLocatorList->GetWorldTransform().GetLocalMatrixImp( matWorld );

		TMatrix44 matLocator;
		pLocator->GetMatrix( matLocator );

		TBOOL bInverted = TFALSE;

		// The index (first two digits of the name)
		const TCHAR* pIndex = szName;

		while ( !isdigit( *pIndex ) )
			pIndex++;

		TCHAR szIndex[ 4 ];
		T2String8::Copy( szIndex, pIndex, 2 );
		szIndex[ 2 ]  = '\0';
		pGate->iIndex = TUINT8( T2String8::StringToInt( szIndex ) );

		// The angle (next number of the name, it can be negative)
		// NOTE: retail doesn't stop at the end of the name if there's no other number
		const TCHAR* pAngle = pIndex + 2;

		while ( !isdigit( *pAngle ) )
			pAngle++;

		if ( *pAngle )
		{
			TBOOL bNegative = pAngle[ -1 ] == '-';

			TCHAR szAngle[ 4 ];
			T2String8::Copy( szAngle, pAngle, 3 );
			szAngle[ 3 ] = '\0';

			pGate->fAngle = T2String8::StringToFloat( szAngle );

			if ( bNegative )
				pGate->fAngle = -pGate->fAngle;
		}

		if ( T2String8::FindString( szName, "inverted" ) )
		{
			pGate->fAngle = -pGate->fAngle;

			TMatrix44 matCopy = matLocator;
			matLocator.Multiply( matCopy, m_oMirrorMatrix );
			bInverted = TTRUE;
		}

		TMatrix44 matTransform;
		matTransform.Multiply( matWorld, matLocator );
		pGate->GetTransform().SetMatrix( matTransform );

		if ( pGate->GetInstance()->GetModel()->GetNumCollisionMeshes() > 0 )
		{
			pGate->oCollision.Create( pGate );
			pGate->oCollision.SetType( pGate->oCollision.GetType() | ACollisionObject::TYPE_GATE );

			if ( bInverted )
			{
				// NOTE: the result isn't used
				TMatrix44 matCopy = matTransform;
				matTransform.Multiply( matCopy, m_oMirrorMatrix );

				pGate->oCollision.GetModelInstance()->SetTransformObject( &pGate->GetTransform() );
			}
		}

		pGate->EnableDistanceCulling();
		pGate->GetFadeDistances().x = 80.0f;
		pGate->GetFadeDistances().y = 90.0f;
		pGate->GetFadeParams().x    = 0.1f;
		pGate->bInverted            = bInverted;
	}

	if ( AGameLoader::IsMainGameLoaded() || AGameLoader::IsChangingTerrain() )
	{
		// The fence of the tomato launcher
		m_pTomatoTarp   = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data/models/launchertarp" ), TNULL );
		m_pTomatoFence1 = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data/models/fncborder01" ), TNULL );
		m_pTomatoFence2 = AModelRepos::GetSingleton()->InstantiateNewModel( TPString8( "data/models/fncborder02" ), TNULL );

		TFLOAT fLOD0Distance, fBlendEnd, fBlendStart;
		AInstanceManager::GetSingleton()->GetVisibilityDistances( fLOD0Distance, fBlendEnd, fBlendStart );
		m_pTomatoFence1->GetSceneObject()->GetInstance()->GetModel()->m_aLODDistances[ 0 ] = fLOD0Distance;

		ATerrainLocatorList* pLocators = ATerrainInterface::GetSingleton()->GetLocatorList();

		// The locators are Z up
		TMatrix44 matZUp;
		matZUp.m_f11 = 1.0f;
		matZUp.m_f12 = 0.0f;
		matZUp.m_f13 = 0.0f;
		matZUp.m_f14 = 0.0f;
		matZUp.m_f21 = 0.0f;
		matZUp.m_f22 = 0.0f;
		matZUp.m_f23 = 1.0f;
		matZUp.m_f24 = 0.0f;
		matZUp.m_f31 = 0.0f;
		matZUp.m_f32 = -1.0f;
		matZUp.m_f33 = 0.0f;
		matZUp.m_f34 = 0.0f;
		matZUp.m_f41 = 0.0f;
		matZUp.m_f42 = 0.0f;
		matZUp.m_f43 = 0.0f;
		matZUp.m_f44 = 1.0f;

		ATerrainLocator* pFence1 = pLocators->FindLocator( "tomatofence1" );

		if ( pFence1 )
		{
			TMatrix44 matLocator;
			pFence1->GetMatrix( matLocator );

			TMatrix44 matTransform;
			matTransform.Multiply( matZUp, matLocator );

			m_pTomatoTarp->SetTransform( matTransform );
			m_pTomatoFence1->SetTransform( matTransform );
		}

		m_oTomatoCollision.Create( m_pTomatoTarp->GetSceneObject() );

		ATerrainLocator* pFence2 = pLocators->FindLocator( "tomatofence2" );

		if ( pFence2 )
		{
			TMatrix44 matLocator;
			pFence2->GetMatrix( matLocator );

			TMatrix44 matTransform;
			matTransform.Multiply( matZUp, matLocator );

			m_pTomatoFence2->SetTransform( matTransform );
		}

		m_bTomatoGateOpen = TFALSE;
		m_pTomatoTarp->SetSkeletonUpdating( TFALSE );
		m_pTomatoFence1->SetSkeletonUpdating( !m_bTomatoGateOpen );
		m_pTomatoFence2->SetSkeletonUpdating( !m_bTomatoGateOpen );
	}
}

// $Barnyard: FUNCTION 005dd940
void AGateManager::SetTomatoGateOpen( TBOOL a_bOpen )
{
	m_bTomatoGateOpen = a_bOpen;
	m_pTomatoTarp->SetSkeletonUpdating( a_bOpen );
	m_pTomatoFence1->SetSkeletonUpdating( !m_bTomatoGateOpen );
	m_pTomatoFence2->SetSkeletonUpdating( !m_bTomatoGateOpen );
}

// $Barnyard: FUNCTION 005dec70
void AGateManager::Update( TFLOAT a_fDeltaTime )
{
	s_fNearbyPadlockTimer += a_fDeltaTime;

	if ( s_fNearbyPadlockTimer > 0.5f )
	{
		m_pNearbyPadlock      = FindNearbyPadlock( 3.6f );
		s_fNearbyPadlockTimer = 0.0f;
	}

	TBOOL bPadlockOpening = TFALSE;

	for ( auto it = m_vecOpeningGates.Begin(); it != m_vecOpeningGates.End(); )
	{
		OpeningGate* pOpening = &it.Value();
		Gate*        pGate    = pOpening->pGate;

		if ( pGate->eModel == MODEL_PADLOCK )
		{
			bPadlockOpening = TTRUE;
			pOpening->fTime += a_fDeltaTime;

			if ( pOpening->fTime > 2.0f )
			{
				// Gone
				pGate->SetFlags( ( pGate->GetFlags() & ~1 ) | 4 );
				pGate->GetFadeParams().y = 1.0f;
				pGate->bOpen             = TTRUE;
			}
			else
			{
				// Blink
				if ( TMath::Abs( TMath::Sin( pOpening->fTime * 3.1415927f * 8.0f ) ) >= 0.5f )
					pGate->SetFlags( pGate->GetFlags() | 1 );
				else
					pGate->SetFlags( pGate->GetFlags() & ~1 );
			}
		}
		else if ( !bPadlockOpening )
		{
			// The gates swing open after the padlocks disappeared
			TFLOAT    fAngle   = pGate->fAngle * TMath::DegToRad( 1.0f );
			TMatrix44 matStart = pOpening->matStart;
			TFLOAT    fSpeed   = TMath::LERP( 2.0f, 0.1f, pOpening->fTime * 0.4f );

			pOpening->fTime += fSpeed * a_fDeltaTime;

			TFLOAT fRotation = TMath::LERP( 0.0f, fAngle, pOpening->fTime * 0.4f );
			matStart.RotateZ( fRotation );
			pGate->GetTransform().SetMatrix( matStart );

			if ( TMath::Abs( fAngle ) < TMath::Abs( fRotation ) )
			{
				pGate->bOpen = TTRUE;
				pGate->oCollision.UnlinkCollisionObject();
			}
		}

		if ( pGate->bOpen )
			m_vecOpeningGates.Erase( it );
		else
			it++;
	}
}

// $Barnyard: FUNCTION 005dd5c0
void AGateManager::Render()
{
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();
	TRenderContext*   pRenderContext   = pRenderInterface->GetCurrentContext();
	TUINT             uiOldClipFlags   = pRenderContext->GetClipFlags();
	const TPlane*     pWorldPlanes     = pRenderContext->GetWorldPlanes();

	// The light of the mirrored gates is mirrored too
	TMatrix44 matLightDir      = pRenderInterface->GetLightDirection();
	TMatrix44 matMirroredLight = pRenderInterface->GetLightDirection();
	TMatrix44 matLightDirCopy  = pRenderInterface->GetLightDirection();
	matMirroredLight.Multiply( matLightDirCopy, m_oMirrorMatrix );

	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		Gate* pGate = &m_pGates[ i ];

		const TVector3& rcPosition = pGate->GetPosition();
		TSphere         oSphere( rcPosition.x, rcPosition.y, rcPosition.z, pGate->GetModelRef()->GetModel()->GetRenderDistance() );

		TINT iClipFlags = TRenderContext::CullSphereToFrustum( oSphere, pWorldPlanes, uiOldClipFlags, -1 );

		if ( iClipFlags >= 0 )
		{
			pRenderContext->SetClipFlags( iClipFlags );

			if ( pGate->bInverted )
			{
				pRenderInterface->SetLightDirectionMatrix( matMirroredLight );
				pGate->RenderIfVisible();
				pRenderInterface->SetLightDirectionMatrix( matLightDir );
			}
			else
			{
				pGate->RenderIfVisible();
			}
		}
	}

	pRenderContext->SetClipFlags( uiOldClipFlags );
}

// $Barnyard: FUNCTION 005de110
void AGateManager::OpenGate( Gate* a_pGate, TBOOL a_bAnimate )
{
	if ( a_bAnimate )
	{
		OpeningGate oOpening;
		a_pGate->GetTransform().GetLocalMatrixImp( oOpening.matStart );
		oOpening.pGate = a_pGate;
		oOpening.fTime = 0.0f;

		// The padlocks go first
		if ( a_pGate->eModel == MODEL_PADLOCK )
			m_vecOpeningGates.InsertBefore( m_vecOpeningGates.Begin(), oOpening, TFALSE );
		else
			m_vecOpeningGates.PushBack( oOpening );
	}
	else if ( a_pGate->eModel == MODEL_PADLOCK )
	{
		a_pGate->SetFlags( a_pGate->GetFlags() & ~1 );
		a_pGate->bOpen = TTRUE;
	}
	else
	{
		TMatrix44 matTransform;
		a_pGate->GetTransform().GetLocalMatrixImp( matTransform );
		matTransform.RotateZ( a_pGate->fAngle * TMath::DegToRad( 1.0f ) );
		a_pGate->GetTransform().SetMatrix( matTransform );

		a_pGate->bOpen = TTRUE;
		a_pGate->oCollision.UnlinkCollisionObject();
	}
}

// $Barnyard: FUNCTION 005de670
void AGateManager::OpenGatesOfType( GATETYPE a_eType, TBOOL a_bAnimate )
{
	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		Gate* pGate = &m_pGates[ i ];

		if ( pGate->eType == a_eType && !pGate->bOpen )
			OpenGate( pGate, a_bAnimate );
	}
}

// $Barnyard: FUNCTION 005ddaa0
void AGateManager::CloseGatesOfType( GATETYPE a_eType )
{
	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		Gate* pGate = &m_pGates[ i ];

		if ( pGate->eType == a_eType && !pGate->bOpen )
			pGate->Close();
	}
}

// $Barnyard: FUNCTION 005de210
TBOOL AGateManager::IsGateOpening( Gate* a_pGate )
{
	if ( !a_pGate )
		return TFALSE;

	T2_FOREACH( m_vecOpeningGates, it )
	{
		if ( it->pGate == a_pGate )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005dd810
AGateManager::Gate* AGateManager::FindGate( GATETYPE a_eType, TINT a_iIndex )
{
	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		if ( m_pGates[ i ].eType == a_eType && a_iIndex == m_pGates[ i ].iIndex )
			return &m_pGates[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005dd850
AGateManager::Gate* AGateManager::FindGateOfType( GATETYPE a_eType )
{
	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		if ( m_pGates[ i ].eType == a_eType )
			return &m_pGates[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005dd880
AGateManager::Gate* AGateManager::FindClosestPadlock( GATETYPE a_eType, const TVector4& a_rcPosition, TFLOAT& a_rfDistanceSq, TBOOL a_bOpen )
{
	if ( !AGameLoader::IsMainGameLoaded() )
		return TNULL;

	Gate* pClosest = TNULL;

	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		Gate* pGate = &m_pGates[ i ];

		if ( pGate->eModel == MODEL_PADLOCK && a_eType == pGate->eType && pGate->bOpen != a_bOpen )
		{
			const TVector3& rcPosition = pGate->GetPosition();

			TFLOAT fDX         = rcPosition.x - a_rcPosition.x;
			TFLOAT fDZ         = rcPosition.z - a_rcPosition.z;
			TFLOAT fDistanceSq = fDZ * fDZ + fDX * fDX;

			if ( fDistanceSq < a_rfDistanceSq )
			{
				a_rfDistanceSq = fDistanceSq;
				pClosest       = pGate;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 005de590
AGateManager::Gate* AGateManager::FindNearbyPadlock( TFLOAT a_fMaxDistanceSq )
{
	if ( !AGameLoader::IsMainGameLoaded() )
		return TNULL;

	const TVector4& rcPlayerPos = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();

	// NOTE: the first padlock in range, not the closest one
	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		Gate* pGate = &m_pGates[ i ];

		if ( pGate->eModel != MODEL_PADLOCK )
			continue;

		const TVector3& rcPosition = pGate->GetPosition();
		TVector4        vecPosition( rcPosition.x, rcPosition.y, rcPosition.z, 1.0f );

		if ( TVector4::DistanceSq( rcPlayerPos, vecPosition ) < a_fMaxDistanceSq )
		{
			if ( pGate->bOpen || IsGateOpening( pGate ) )
				return TNULL;

			return pGate;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005dee90
void AGateManager::OpenClosestGate()
{
	if ( m_pNearbyPadlock || ( m_pNearbyPadlock = FindNearbyPadlock( 100.0f ) ) )
	{
		OpenGatesOfType( m_pNearbyPadlock->eType, TTRUE );
		m_pNearbyPadlock = TNULL;

		AQuestManager::GetSingleton()->PlayQuestCompleteMusic();
	}
}

// The index of the gate type called a_rcName, GATETYPE_NUMOF if there's none
// NOTE: inlined in 005de6d0, 005de830 and 005de910
AGateManager::GATETYPE AGateManager::FindGateType( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < GATETYPE_NUMOF; i++ )
	{
		if ( T2String8::Compare( a_rcName.GetString(), s_apszGateTypes[ i ] ) == 0 )
			return i;
	}

	return GATETYPE_NUMOF;
}

// $Barnyard: FUNCTION 005de6d0
void AGateManager::OpenGateType( TPString8 a_strType )
{
	if ( a_strType == TPS8( tomato ) )
	{
		SetTomatoGateOpen( TTRUE );
		return;
	}

	GATETYPE eType = FindGateType( a_strType );

	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		Gate* pGate = &m_pGates[ i ];

		if ( pGate->eType != eType )
			continue;

		// NOTE: OpenGate( pGate, TFALSE ) is inlined here, without checking if the gate is open
		if ( pGate->eModel == MODEL_PADLOCK )
		{
			pGate->bOpen = TTRUE;
			pGate->SetFlags( pGate->GetFlags() & ~1 );
		}
		else
		{
			TMatrix44 matTransform;
			pGate->GetTransform().GetLocalMatrixImp( matTransform );
			matTransform.RotateZ( pGate->fAngle * TMath::DegToRad( 1.0f ) );
			pGate->GetTransform().SetMatrix( matTransform );

			pGate->bOpen = TTRUE;
			pGate->oCollision.UnlinkCollisionObject();
		}
	}
}

// $Barnyard: FUNCTION 005de830
void AGateManager::CloseGateType( TPString8 a_strType )
{
	if ( a_strType == TPS8( tomato ) )
	{
		SetTomatoGateOpen( TFALSE );
		return;
	}

	GATETYPE eType = FindGateType( a_strType );

	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		Gate* pGate = &m_pGates[ i ];

		if ( pGate->eType == eType && pGate->bOpen == TTRUE )
			pGate->Close();
	}
}

// $Barnyard: FUNCTION 005de910
TBOOL AGateManager::IsGateOpen( TPString8 a_strType )
{
	if ( a_strType == TPS8( tomato ) )
		return m_bTomatoGateOpen;

	// NOTE: retail doesn't check if there's a gate of the type
	return FindGateOfType( FindGateType( a_strType ) )->bOpen;
}

// $Barnyard: FUNCTION 005de9e0
void AGateManager::SetGate( TPString8 a_strType, TBOOL a_bOpen )
{
	if ( a_strType == TPS8( tomato ) )
	{
		SetTomatoGateOpen( a_bOpen );
	}
	else if ( a_bOpen )
	{
		OpenGateType( a_strType );
	}
	else
	{
		CloseGateType( a_strType );
	}
}

// $Barnyard: FUNCTION 005de290
TBOOL AGateManager::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	// The gates that are opening are saved open
	T2_FOREACH( m_vecOpeningGates, it )
	{
		it->pGate->bOpen = TTRUE;
	}

	a_pBuffer->WriteString( "AGateManager" );

	TINT   iBit   = 0;
	TUINT8 uiBits = 0;

	for ( TINT i = 0; i < m_iNumGates; i++ )
	{
		if ( m_pGates[ i ].bOpen )
			uiBits |= 1 << iBit;

		if ( ++iBit == 8 )
		{
			a_pBuffer->WriteUInt8( uiBits );
			uiBits = 0;
			iBit   = 0;
		}
	}

	if ( m_bTomatoGateOpen )
		uiBits |= 1 << iBit;

	a_pBuffer->WriteUInt8( uiBits );

	return TTRUE;
}

// $Barnyard: FUNCTION 005de380
TBOOL AGateManager::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->CheckString( "AGateManager" );

	TINT   iNumGates = m_iNumGates;
	TUINT8 uiBits    = 0;
	TINT   iBit      = 0;
	TINT   iNumBytes = iNumGates / 8 + ( iNumGates % 8 != 0 );
	TINT   iGate     = 0;

	if ( iNumBytes > 0 )
	{
		for ( TINT iByte = 0; iGate < iNumGates; )
		{
			a_pBuffer->ReadUInt8( uiBits );
			iBit = 0;

			do
			{
				Gate* pGate = &m_pGates[ iGate++ ];
				TBOOL bOpen = ( uiBits & ( 1 << iBit++ ) ) != 0;

				pGate->bOpen = bOpen;

				if ( bOpen )
					OpenGate( pGate, TFALSE );

				iNumGates = m_iNumGates;
			} while ( iGate != iNumGates && iBit < 8 );

			if ( ++iByte >= iNumBytes )
				break;
		}

		if ( iBit >= 8 )
		{
			a_pBuffer->ReadUInt8( uiBits );
			iBit = 0;
		}
	}

	// NOTE: SetTomatoGateOpen is inlined here
	SetTomatoGateOpen( ( ( 1 << iBit ) & uiBits ) != 0 );

	return TTRUE;
}
