#include "pch.h"
#include "ASpeechCameraHelper.h"
#include "Animation/AModelRepos.h"
#include "Assets/AAssetLoader.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/ABikeManager.h"
#include "Input/AInputMap.h"
#include "Units/AUnitFacialComponent.h"
#include "Units/AVisibleUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASpeechCameraHelper );

// $Barnyard: FUNCTION 00466e30
ASpeechCameraHelper::ASpeechCameraHelper()
    : m_iCurrentModel( -1 )
    , m_pCurrentModel( TNULL )
    , m_pSkeletonInstance( TNULL )
    , m_bHasTransform( TFALSE )
    , m_iCameraBone( -1 )
    , m_iTargetBone( -1 )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_fYaw( 0.0f )
    , m_iSpeakerId( 0 )
    , m_iAngle( 0 )
    , m_uiSpeakerMask( 0 )
    , m_uiSavedHUDFlags( 0 )
{
	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
	{
		m_aSpeakers[ i ] = TNULL;

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail leaves the speaker spots uninitialised, UpdateSpeakers computes the directions
		// of all four spots (also of the ones the camera rig doesn't have) from them
		m_avSpeakerPositions[ i ]  = TVector4::VEC_ZERO;
		m_avSpeakerDirections[ i ] = TVector4::VEC_ZERO;
#endif
	}

	for ( TINT i = 0; i < NUM_MODELS; i++ )
		m_apModels[ i ] = TNULL;

	m_oTransform = TMatrix44::IDENTITY;
}

// $Barnyard: FUNCTION 004676e0
// $Barnyard: FUNCTION 00467b40
ASpeechCameraHelper::~ASpeechCameraHelper()
{
	Stop( TFALSE );
	DestroyModels();
	m_pHiddenUnit = TNULL;
}

// $Barnyard: FUNCTION 00467e10
void ASpeechCameraHelper::UpdateCurrentCamera()
{
	ACameraHelper::UpdateCurrentCamera();
	LoadModels();
	ResetModel();
}

// $Barnyard: FUNCTION 004668c0
TBOOL ASpeechCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_pCurrentModel && m_iCameraBone != -1 && m_iTargetBone != -1 )
	{
		TVector4  vecPosition;
		TVector4  vecTarget;
		TMatrix44 matBone;

		if ( m_bHasTransform )
		{
			// The bones of the camera rig placed with the locator's transform
			m_pCurrentModel->GetBoneTransform( m_iCameraBone, matBone );
			TVector4 vecTranslation = matBone.GetTranslation();
			matBone.Identity();
			matBone.SetTranslation( vecTranslation );
			matBone.Multiply( m_oTransform );
			vecPosition = matBone.GetTranslation();

			m_pCurrentModel->GetBoneTransform( m_iTargetBone, matBone );
			vecTranslation = matBone.GetTranslation();
			matBone.Identity();
			matBone.SetTranslation( vecTranslation );
			matBone.Multiply( m_oTransform );
			vecTarget = matBone.GetTranslation();
		}
		else
		{
			m_pCurrentModel->GetBoneTransform( m_iCameraBone, matBone );
			vecPosition = matBone.GetTranslation();

			m_pCurrentModel->GetBoneTransform( m_iTargetBone, matBone );
			vecTarget = matBone.GetTranslation();
		}

		LookAt( vecPosition, vecTarget );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00466870
TBOOL ASpeechCameraHelper::ProcessInputCommand( void* a_pCommand, void* a_pEvent )
{
	return AInputCommand( TUINTPTR( a_pCommand ) ) != AInputCommand_Interact;
}

// $Barnyard: FUNCTION 00466880
void ASpeechCameraHelper::OnAttach()
{
	m_fYaw      = 0.0f;
	m_vPosition = TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 00466b00
void ASpeechCameraHelper::ResetTarget()
{
	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
		m_aSpeakers[ i ] = TNULL;

	ACameraHelper::ResetTarget();
}

// $Barnyard: FUNCTION 00467480
void ASpeechCameraHelper::OnDetach()
{
	Stop( TFALSE );
}

// $Barnyard: FUNCTION 00466de0
void ASpeechCameraHelper::ShowHiddenUnit()
{
	if ( m_pHiddenUnit && m_pHiddenUnit->GetUnit() )
	{
		TSTATICCAST( AVisibleUnit, m_pHiddenUnit->GetUnit() )->SetVisible( TTRUE );
		m_pHiddenUnit = TNULL;
	}
}

// Loads and instantiates the model Data/Models/a_szName
// $Barnyard: FUNCTION 00467490
static AModelInstance* CreateCameraModel( const TCHAR* a_szName )
{
	// NOTE: retail formats the path into a global buffer (0x007ce238)
	TCHAR szPath[ 128 ];
	T2String8::Format( szPath, sizeof( szPath ), "data/models/%s", a_szName );

	TPString8 strPath( szPath );
	g_pModelRepos->LoadModel( strPath, AAssetLoader::GetAssetTRB( AAssetType_Startup ) );

	AModelInstance* pInstance = g_pModelRepos->InstantiateModel( g_pModelRepos->GetModel( strPath ) );
	pInstance->SetTranslation( TVector3( TVector4::VEC_ZERO.x, TVector4::VEC_ZERO.y, TVector4::VEC_ZERO.z ) );
	pInstance->GetTransform().SetEuler( TVector3( TMath::HALF_PI, 0.0f, 0.0f ) );

	return pInstance;
}

// $Barnyard: FUNCTION 00467580
void ASpeechCameraHelper::LoadModels()
{
	// The camera rig files of the conversation places (0x0071d0fc)
	static const TCHAR* s_aszModels[ NUM_MODELS ] = {
		"convpoint_01",
		"convpoint_01",
		"convpoint_pippost",
		"convpoint_nightbarn_jersy",
		"convpoint_sim_jersy",
		"convpoint_sim_madie",
		"sim_convpoint_pip",
		"convpoint_sim_gopher",
		"convpoint_pip_group",
	};

	for ( TINT i = 0; i < NUM_MODELS; i++ )
		m_apModels[ i ] = CreateCameraModel( s_aszModels[ i ] );
}

// $Barnyard: FUNCTION 00466c60
void ASpeechCameraHelper::DestroyModels()
{
	for ( TINT i = 0; i < NUM_MODELS; i++ )
	{
		if ( m_apModels[ i ] )
		{
			m_apModels[ i ]->GetSkeletonInstance()->RemoveAllAnimations();
			g_pModelRepos->DestroyModelInstance( m_apModels[ i ] );
		}
	}
}

// $Barnyard: FUNCTION 00467d60
void ASpeechCameraHelper::ResetModel()
{
	m_iCurrentModel = 0;
	m_pCurrentModel = m_apModels[ 0 ];

	if ( m_pCurrentModel )
	{
		m_pSkeletonInstance = m_pCurrentModel->GetSkeletonInstance();

		TMatrix44 matTransform;
		matTransform.Identity();
		matTransform.RotateX( TMath::HALF_PI );
		m_pCurrentModel->GetTransform().SetMatrix( matTransform );
	}

	m_iCameraBone = -1;
	m_iTargetBone = -1;
	m_iSpeakerId  = 0;
	m_iAngle      = 0;

	UpdateSpeakers();
}

// $Barnyard: FUNCTION 00467120
void ASpeechCameraHelper::Stop( TBOOL a_bFaceCentre )
{
	// Move the speakers back to where they were
	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
	{
		AUnit* pUnit = m_aSpeakers[ i ] ? m_aSpeakers[ i ]->GetUnit() : TNULL;

		if ( pUnit )
		{
			ASteer* pSteer = pUnit->GetSteer();
			pSteer->HaltAtPosition( TVector3( m_avSavedPositions[ i ].x, m_avSavedPositions[ i ].y, m_avSavedPositions[ i ].z ) );
			pSteer->HaltFacing( TVector3( m_avSavedDirections[ i ].x, m_avSavedDirections[ i ].y, m_avSavedDirections[ i ].z ) );

			AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, pUnit->FindComponentExact( &TGetClass( AUnitFacialComponent ) ) );

			if ( pFacial )
				pFacial->StopLook();

			pSteer->SetSpeechDialogMode( TFALSE );
		}
	}

	if ( a_bFaceCentre )
	{
		// Turn the speakers to the centre of the group
		// NOTE: retail doesn't initialise the sum (no caller passes TTRUE)
		TVector4 vecCentre = TVector4::VEC_ZERO;
		TINT     iNumUnits = 0;

		for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
		{
			if ( m_aSpeakers[ i ] && m_aSpeakers[ i ]->GetUnit() )
			{
				iNumUnits++;
				vecCentre.x += m_avSavedPositions[ i ].x;
				vecCentre.y += m_avSavedPositions[ i ].y;
				vecCentre.z += m_avSavedPositions[ i ].z;
			}
		}

		if ( iNumUnits > 0 )
		{
			TFLOAT fInvNum = 1.0f / TFLOAT( iNumUnits );
			vecCentre.x *= fInvNum;
			vecCentre.y *= fInvNum;
			vecCentre.z *= fInvNum;

			for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
			{
				AUnit* pUnit = m_aSpeakers[ i ] ? m_aSpeakers[ i ]->GetUnit() : TNULL;

				if ( pUnit )
				{
					TVector4 vecDirection( vecCentre.x - m_avSavedPositions[ i ].x, vecCentre.y - m_avSavedPositions[ i ].y, vecCentre.z - m_avSavedPositions[ i ].z, vecCentre.w );
					vecDirection.Normalise();

					pUnit->SetSteerDirection( vecDirection );
				}
			}
		}
	}

	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
		m_aSpeakers[ i ] = TNULL;

	if ( m_pHiddenUnit && m_pHiddenUnit->GetUnit() )
	{
		TSTATICCAST( AVisibleUnit, m_pHiddenUnit->GetUnit() )->SetVisible( TTRUE );
		m_pHiddenUnit = TNULL;
	}

	m_bHasTransform = TFALSE;
	m_oTransform    = TMatrix44::IDENTITY;
}

// $Barnyard: FUNCTION 00466f50
void ASpeechCameraHelper::SetSpeaker( TINT a_iAngle, TINT a_iCamera, TINT a_iUnitId, TBOOL a_bSwitchAngle )
{
	// NOTE: retail formats the names into a global buffer (0x007ce238)
	static TCHAR s_szBoneName[ 128 ];

	if ( a_bSwitchAngle )
	{
		if ( m_iSpeakerId == a_iUnitId )
			return;

		a_iAngle = ( m_iAngle == 0 );
	}

	T2String8::Format( s_szBoneName, sizeof( s_szBoneName ), "cam_c%02d_a%02d", a_iCamera, a_iAngle + 1 );
	m_iCameraBone = m_pCurrentModel->GetBoneIndex( TPString8( s_szBoneName ) );

	T2String8::Format( s_szBoneName, sizeof( s_szBoneName ), "targ_c%02d_a%02d", a_iCamera, a_iAngle + 1 );
	m_iTargetBone = m_pCurrentModel->GetBoneIndex( TPString8( s_szBoneName ) );

	m_iAngle     = a_iAngle;
	m_iSpeakerId = a_iUnitId;
}

// $Barnyard: FUNCTION 00467790
void ASpeechCameraHelper::ShowSpeaker( TINT a_iCamera, TINT a_iUnitId )
{
	// NOTE: retail formats the names into a global buffer (0x007ce238)
	static TCHAR s_szBoneName[ 128 ];

	TINT  iAngle = -1;
	TBOOL bFound = TFALSE;

	for ( TINT i = 0; i < MAX_SPEAKERS && !bFound; i++ )
	{
		// NOTE: like retail, the slots before the speaker must be filled
		if ( m_aSpeakers[ i ]->GetUnit()->GetId() == a_iUnitId )
		{
			iAngle = i;
			bFound = TTRUE;
		}
	}

	T2String8::Format( s_szBoneName, sizeof( s_szBoneName ), "cam_c%02d_a%02d", a_iCamera, iAngle + 1 );
	m_iCameraBone = m_pCurrentModel->GetBoneIndex( TPString8( s_szBoneName ) );

	T2String8::Format( s_szBoneName, sizeof( s_szBoneName ), "targ_c%02d_a%02d", a_iCamera, iAngle + 1 );
	m_iTargetBone = m_pCurrentModel->GetBoneIndex( TPString8( s_szBoneName ) );

	m_iAngle     = iAngle;
	m_iSpeakerId = a_iUnitId;
}

// $Barnyard: FUNCTION 00466c20
TINT ASpeechCameraHelper::FindModel( const TCHAR* a_szName )
{
	// The names of the camera rigs used by the speech sequences (0x0071d0f8)
	static const TCHAR* s_aszNames[ NUM_MODELS ] = {
		"convpoint_01",
		"convpoint_npc",
		"convpoint_pippost",
		"convpoint_nightbarn_jersy",
		"convpoint_sim_jersy",
		"convpoint_sim_madie",
		"sim_convpoint_pip",
		"convpoint_sim_gopher",
		"convpoint_pip_group",
	};

	for ( TINT i = 0; i < NUM_MODELS; i++ )
	{
		if ( TStringManager::String8CompareNoCase( a_szName, s_aszNames[ i ] ) == 0 )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00467d40
void ASpeechCameraHelper::SetModel( const TCHAR* a_szName )
{
	SetModel( TUINT( FindModel( a_szName ) ) );
}

// $Barnyard: FUNCTION 00467cb0
void ASpeechCameraHelper::SetModel( TUINT a_uiModel )
{
	m_iCurrentModel = TINT( a_uiModel );
	m_pCurrentModel = ( a_uiModel < NUM_MODELS ) ? m_apModels[ a_uiModel ] : TNULL;

	if ( m_pCurrentModel )
	{
		m_pSkeletonInstance = m_pCurrentModel->GetSkeletonInstance();

		TMatrix44 matTransform;
		matTransform.Identity();
		matTransform.RotateX( TMath::HALF_PI );
		m_pCurrentModel->GetTransform().SetMatrix( matTransform );
	}

	m_iCameraBone = -1;
	m_iTargetBone = -1;
	m_iSpeakerId  = 0;
	m_iAngle      = 0;

	UpdateSpeakers();
}

// $Barnyard: FUNCTION 00466a40
void ASpeechCameraHelper::ResetCamera()
{
	m_iCameraBone = -1;
	m_iTargetBone = -1;
	m_iSpeakerId  = 0;
	m_iAngle      = 0;
}

// $Barnyard: FUNCTION 00466bd0
void ASpeechCameraHelper::HideHUD()
{
	m_uiSavedHUDFlags = AGameStateController::GetSingleton()->GetCurrentState()->GetHUDParams().m_uiFlags;
	AGameStateController::GetSingleton()->GetCurrentState()->GetHUDParams().SetFlags( 0 );
}

// $Barnyard: FUNCTION 00466c00
void ASpeechCameraHelper::RestoreHUD()
{
	AGameStateController::GetSingleton()->GetCurrentState()->GetHUDParams().SetFlags( m_uiSavedHUDFlags );
}

// $Barnyard: FUNCTION 004673f0
void ASpeechCameraHelper::ReleaseSpeakers()
{
	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
	{
		if ( m_aSpeakers[ i ] && m_aSpeakers[ i ]->GetUnit() )
			m_aSpeakers[ i ]->GetUnit()->GetSteer()->SetSpeechDialogMode( TFALSE );

		m_aSpeakers[ i ] = TNULL;
	}

	if ( m_pHiddenUnit && m_pHiddenUnit->GetUnit() )
	{
		TSTATICCAST( AVisibleUnit, m_pHiddenUnit->GetUnit() )->SetVisible( TTRUE );
		m_pHiddenUnit = TNULL;
	}
}

// $Barnyard: FUNCTION 00466b40
void ASpeechCameraHelper::ClearSpeakers()
{
	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
	{
		m_aSpeakers[ i ]         = TNULL;
		m_avSavedPositions[ i ]  = TVector4::VEC_ZERO;
		m_avSavedDirections[ i ] = TVector4::VEC_ZERO;
	}

	m_uiSpeakerMask = 0;
}

// $Barnyard: FUNCTION 00466ca0
TBOOL ASpeechCameraHelper::AddSpeaker( AUnit* a_pUnit )
{
	TINT iSlot = 0;

	while ( m_aSpeakers[ iSlot ] )
	{
		if ( ++iSlot >= MAX_SPEAKERS )
			return TFALSE;
	}

	m_aSpeakers[ iSlot ] = a_pUnit->GetSelfProxy();

	ASteer* pSteer               = a_pUnit->GetSteer();
	m_avSavedPositions[ iSlot ]  = *pSteer->GetPosition();
	m_avSavedDirections[ iSlot ] = *pSteer->GetDirection();
	pSteer->SetSpeechDialogMode( TTRUE );

	return TTRUE;
}

// $Barnyard: FUNCTION 00466a60
TBOOL ASpeechCameraHelper::GetSpeakerPosition( TINT a_iSpeaker, TVector4& a_rOutPosition ) const
{
	if ( !( ( 1 << a_iSpeaker ) & m_uiSpeakerMask ) )
		return TFALSE;

	a_rOutPosition = m_avSpeakerPositions[ a_iSpeaker ];
	return TTRUE;
}

// $Barnyard: FUNCTION 00466ab0
TBOOL ASpeechCameraHelper::GetSpeakerDirection( TINT a_iSpeaker, TVector4& a_rOutDirection ) const
{
	if ( !( ( 1 << a_iSpeaker ) & m_uiSpeakerMask ) )
		return TFALSE;

	a_rOutDirection = m_avSpeakerDirections[ a_iSpeaker ];
	return TTRUE;
}

// $Barnyard: FUNCTION 00467b60
void ASpeechCameraHelper::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;

	if ( m_pCurrentModel )
	{
		TMatrix44 matTransform;
		matTransform.Identity();

		TVector4 vecTranslation;
		TMatrix44::TransformVector( vecTranslation, matTransform, m_vPosition );
		matTransform.SetTranslation( vecTranslation );

		matTransform.RotateY( m_fYaw - TMath::HALF_PI );
		matTransform.RotateX( TMath::HALF_PI );
		m_pCurrentModel->GetTransform().SetMatrix( matTransform );

		UpdateSpeakers();
	}
}

// $Barnyard: FUNCTION 00467c00
void ASpeechCameraHelper::SetYaw( TFLOAT a_fYaw )
{
	// Only the first two camera rigs can be turned
	if ( TUINT( m_iCurrentModel ) <= 1 )
	{
		m_fYaw = a_fYaw;

		if ( m_pCurrentModel )
		{
			TMatrix44 matTransform;
			matTransform.Identity();

			TVector4 vecTranslation;
			TMatrix44::TransformVector( vecTranslation, matTransform, m_vPosition );
			matTransform.SetTranslation( vecTranslation );

			matTransform.RotateY( m_fYaw - TMath::HALF_PI );
			matTransform.RotateX( TMath::HALF_PI );
			m_pCurrentModel->GetTransform().SetMatrix( matTransform );

			UpdateSpeakers();
		}
	}
}

// $Barnyard: FUNCTION 00467c90
void ASpeechCameraHelper::SetDirection( const TVector4& a_rcDirection )
{
	SetYaw( a_rcDirection.GetYaw() );
}

// $Barnyard: FUNCTION 00467de0
void ASpeechCameraHelper::SetTransform( const TMatrix44& a_rcTransform )
{
	m_oTransform    = a_rcTransform;
	m_bHasTransform = TTRUE;

	UpdateSpeakers();
}

// Normalises the vector on the XZ plane (y = 0), (0, 0, 1) if it's zero
// NOTE: a TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// $Barnyard: FUNCTION 00467960
void ASpeechCameraHelper::UpdateSpeakers()
{
	// NOTE: retail formats the names into a global buffer (0x007ce238)
	static TCHAR s_szBoneName[ 128 ];

	m_uiSpeakerMask = 0;

	if ( !m_pCurrentModel )
		return;

	TVector4 vecCentre    = TVector4::VEC_ZERO;
	TINT     iNumSpeakers = 0;

	// The speaker spots are the "char_XX" bones of the camera rig
	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
	{
		T2String8::Format( s_szBoneName, sizeof( s_szBoneName ), "char_%02d", i + 1 );
		TINT iBone = m_pCurrentModel->GetSkeletonInstance()->GetBoneID( s_szBoneName );

		if ( iBone > -1 )
		{
			TMatrix44 matBone;
			m_pCurrentModel->GetBoneTransform( iBone, matBone );

			if ( m_bHasTransform )
			{
				TMatrix44 matCopy = matBone;
				matBone.Multiply( m_oTransform, matCopy );
			}

			m_avSpeakerPositions[ i ] = matBone.GetTranslation();
			vecCentre.x += m_avSpeakerPositions[ i ].x;
			vecCentre.y += m_avSpeakerPositions[ i ].y;
			vecCentre.z += m_avSpeakerPositions[ i ].z;

			iNumSpeakers++;
			m_uiSpeakerMask |= 1 << i;
		}
	}

	if ( iNumSpeakers > 1 )
	{
		TFLOAT fInvNum = 1.0f / TFLOAT( iNumSpeakers );
		vecCentre.x *= fInvNum;
		vecCentre.y *= fInvNum;
		vecCentre.z *= fInvNum;
	}
	else if ( iNumSpeakers > 0 )
	{
		// A single speaker faces the camera rig
		vecCentre = m_bHasTransform ? m_oTransform.GetTranslation() : m_vPosition;
	}

	// The speakers face the centre
	for ( TINT i = 0; i < MAX_SPEAKERS; i++ )
	{
		m_avSpeakerDirections[ i ].x = vecCentre.x - m_avSpeakerPositions[ i ].x;
		m_avSpeakerDirections[ i ].y = vecCentre.y - m_avSpeakerPositions[ i ].y;
		m_avSpeakerDirections[ i ].z = vecCentre.z - m_avSpeakerPositions[ i ].z;
		m_avSpeakerDirections[ i ].w = vecCentre.w;
		NormaliseXZ( m_avSpeakerDirections[ i ] );
	}

	HideUnitInCamera();
}

// $Barnyard: FUNCTION 004675b0
void ASpeechCameraHelper::HideUnitInCamera()
{
	if ( m_pHiddenUnit )
		ShowHiddenUnit();

	TVector4 vecCamera = m_bHasTransform ? m_oTransform.GetTranslation() : m_vPosition;

	if ( ABikeManager::GetSingleton() )
	{
		// Hide the bike the camera would be inside of
		AUnit* pBike     = TNULL;
		TINT   iBikeType = -1;
		ABikeManager::GetSingleton()->FindClosestBike( vecCamera, pBike, iBikeType );

		if ( pBike && !ABikeManager::GetSingleton()->IsBikeParked( pBike ) )
		{
			TSTATICCAST( AVisibleUnit, pBike )->SetVisible( TFALSE );
			m_pHiddenUnit = pBike->GetSelfProxy();
		}
	}
}
