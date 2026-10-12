#include "pch.h"
#include "ASteer.h"
#include "FSM/AMethodPointer.h"
#include "ASteerKinematics.h"
#include "ASteerTraits.h"
#include "Units/AUnit.h"
#include "FSM/AFunctionTable.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail registers RETURNS_ZERO (0054d8b0/006dc070) as the create functions.
TDEFINE_CLASS_INIT_NORUNTIME( ASteer );

// DAT_00789444
AFunctionTable ASteer::ms_oFunctionTable( &TGetClass( ASteer ), TNULL );

// FSM fallback: resolves the steer an ASteer native is called on from an FSM value (the unit's steer).
// $Barnyard: FUNCTION 005705d0
static TObject* __stdcall ASteerFunctionFallback( void* a_pUnk1, const AFunctionCaller* a_pCaller )
{
	if ( a_pCaller )
	{
		// Scripts call steer natives on the unit; the steer is resolved from it
		AUnit* pUnit = TSTATICCAST( AUnit, a_pCaller->GetObject() );

		if ( pUnit )
			return pUnit->GetSteer();
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00571c70
void ASteer::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ASteerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "IsCommandFinished", &ASteer::IsCommandFinished ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "Command_WanderPathfindingSpecific", &ASteer::Command_WanderPathfindingSpecific ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "Command_StandStill", &ASteer::Command_StandStill ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ASteer>::Create( "SetToCurrentFullDesiredSpeed", &ASteer::SetToCurrentFullDesiredSpeed ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<AUnit*, ASteer>::Create( "GetTargetedUnit", &ASteer::GetTargetedUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TBOOL, ASteer, AUnit*, TFLOAT>::Create( "Command_SeekUnit", &ASteer::Command_SeekUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, ASteer, TFLOAT>::Create( "IsFacingDesired", &ASteer::IsFacingDesired ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, ASteer, AUnit*>::Create( "Command_FaceUnit", &ASteer::Command_FaceUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ASteer>::Create( "Command_CancelCurrentCommand", &ASteer::Command_CancelCurrentCommand ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "IsCommandSuccess", &ASteer::IsCommandSuccess ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "IsCommandFailure", &ASteer::IsCommandFailure ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "HasArrived", &ASteer::HasArrived ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, ASteer>::Create( "GetSpeed", &ASteer::GetSpeed ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, ASteer>::Create( "GetPositionX", &ASteer::GetPositionX ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, ASteer>::Create( "GetPositionY", &ASteer::GetPositionY ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, ASteer>::Create( "GetPositionZ", &ASteer::GetPositionZ ) );
	ms_oFunctionTable.AddFunction( AMethodPointer3Args<ASteer, TFLOAT, TFLOAT, TFLOAT>::Create( "SetPositionXYZ", &ASteer::SetPositionXYZ ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "FaceCamera", &ASteer::FaceCamera ) );
	ms_oFunctionTable.AddFunction( AMethodPointer3Args<ASteer, TFLOAT, TFLOAT, TFLOAT>::Create( "SetFacingFromPartsXYZ", &ASteer::SetFacingFromPartsXYZ ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, ASteer>::Create( "SetRandomFacing", &ASteer::SetRandomFacing ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<ASteer, TBOOL>::Create( "SetSpeechDialogMode", &ASteer::SetSpeechDialogMode ) );
}

// $Barnyard: FUNCTION 00570610
void ASteer::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00571bd0
ASteer::ASteer()
    : m_fSpeedModifier( 1.0f )
    , m_pTraits( TNULL )
    , m_bUnk10( TFALSE )
    , m_eMovementType( MOVEMENT_STOPPED )
{
}

// $Barnyard: FUNCTION 00571eb0
// $Barnyard: FUNCTION 00571c00
ASteer::~ASteer()
{
	// NOTE: m_oOwner is released by TRef's destructor
}

// $Barnyard: FUNCTION 00555dc0
AFunctionTable* ASteer::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00570910
ASteer::Movement ASteer::GetMovementType()
{
	TFLOAT fSpeed = GetMeasuredSpeed();

	switch ( m_eMovementType )
	{
		case MOVEMENT_STOPPED:
			if ( fSpeed > 0.1f )
				m_eMovementType = MOVEMENT_WALK;

			break;
		case MOVEMENT_WALK:
			if ( m_pTraits->m_fMaxAesWalkSpeed > 0.0f && fSpeed > m_pTraits->m_fMaxAesWalkSpeed )
				m_eMovementType = MOVEMENT_RUN;
			else if ( fSpeed < 0.05f )
				m_eMovementType = MOVEMENT_STOPPED;

			break;
		case MOVEMENT_RUN:
			if ( m_pTraits->m_fMaxAesRunSpeed > 0.0f && fSpeed > m_pTraits->m_fMaxAesRunSpeed )
				m_eMovementType = MOVEMENT_SPRINT;
			else if ( fSpeed < m_pTraits->m_fMaxAesWalkSpeed * 0.9f )
				m_eMovementType = MOVEMENT_WALK;

			break;
		case MOVEMENT_SPRINT:
			if ( fSpeed < m_pTraits->m_fMaxAesRunSpeed * 0.9f )
				m_eMovementType = MOVEMENT_RUN;

			break;
	}

	return m_eMovementType;
}

// $Barnyard: FUNCTION 00570a10
void ASteer::Serialise( AHackedQuickSave::Buffer* a_pStream )
{
	TVector4 vecDirection;

	a_pStream->WriteVector4( *GetPosition() );
	CopyDirection( vecDirection );
	a_pStream->WriteVector4( vecDirection );
}

// $Barnyard: FUNCTION 00570a50
void ASteer::Deserialise( AHackedQuickSave::Buffer* a_pStream )
{
	TVector4 vecValue;

	a_pStream->ReadVector4( vecValue );
	SetPosition( vecValue );
	a_pStream->ReadVector4( vecValue );
	SetDirection( vecValue );
}

// $Barnyard: FUNCTION 00571f80
TBOOL ASteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ValidateLOD( a_iLOD );

	m_oOwner         = a_pOther->m_oOwner->GetUnit()->GetSelfProxy();
	m_fSpeedModifier = a_pOther->m_fSpeedModifier;
	m_pTraits        = a_pOther->m_oOwner->GetUnit()->GetSteerTraits();

	return TTRUE;
}

// $Barnyard: FUNCTION 00571ed0
TBOOL ASteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ValidateLOD( a_iLOD );

	m_oOwner  = a_pOwner->GetSelfProxy();
	m_pTraits = a_pOwner->GetSteerTraits();

	return TTRUE;
}

// $Barnyard: FUNCTION 00570aa0
TBOOL ASteer::IsFacingDesired( TFLOAT a_fTolerance )
{
	return GetKinematics()->IsFacingAtDesiredFacing( a_fTolerance );
}

// $Barnyard: FUNCTION 005710d0
TBOOL ASteer::IsCommandFinished()
{
	return HasArrived();
}

// $Barnyard: FUNCTION 00570b20
TBOOL ASteer::Command_FaceDirection( const TVector4& a_rcDirection )
{
	GetKinematics()->SetDesiredFacing( a_rcDirection );
	return TFALSE;
}

// $Barnyard: FUNCTION 00570ab0
TBOOL ASteer::Command_FaceUnit( AUnit* a_pUnit )
{
	TVector4 vecUnitPos;
	a_pUnit->GetSteer()->CopyPosition( vecUnitPos );

	TVector4 vecPos;
	CopyPosition( vecPos );

	// Flat direction towards the unit
	vecUnitPos.x = vecUnitPos.x - vecPos.x;
	vecUnitPos.y = 0.0f;
	vecUnitPos.z = vecUnitPos.z - vecPos.z;
	vecUnitPos.Normalise();

	return Command_FaceDirection( vecUnitPos );
}

// $Barnyard: FUNCTION 00570b40
void ASteer::SetLOD( TINT a_iOldLOD, TINT a_iNewLOD )
{
	ValidateLOD( a_iOldLOD );
	ValidateLOD( a_iNewLOD );

	if ( a_iNewLOD != a_iOldLOD )
		ChangedLOD( a_iNewLOD );
}

// $Barnyard: FUNCTION 00570b80
TFLOAT ASteer::GetCollisionRadius()
{
	return 0.01f;
}

// $Barnyard: FUNCTION 00570bd0
TBOOL ASteer::SetPosition( const TVector4& a_rcPosition )
{
	GetKinematics()->SetPosition( a_rcPosition );
	return TTRUE;
}

// $Barnyard: FUNCTION 00570b90
TBOOL ASteer::SetPosition3( const TVector3& a_rcPosition )
{
	GetKinematics()->SetPosition( TVector4( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f ) );
	return TTRUE;
}

// $Barnyard: FUNCTION 00570bf0
void ASteer::SetDirection( const TVector4& a_rcDirection )
{
	GetKinematics()->SetFacing( a_rcDirection );
}

// $Barnyard: FUNCTION 005710f0
TBOOL ASteer::SetRandomFacing()
{
	TVector4 vecDirection = ACamera::sm_vInitialLookDirection;
	vecDirection.AsVector3().RotateY( ARandom::GetSingleton()->m_oRandom.GetFloat() * TMath::TWO_PI );

	SetDirection( vecDirection );
	return TTRUE;
}

// $Barnyard: FUNCTION 005569f0
void ASteer::SetDesiredDirection( const TVector4& a_rcDirection )
{
	SetDirection( a_rcDirection );
}

// $Barnyard: FUNCTION 00570670
void ASteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = *GetKinematics()->GetPosition();
}

// $Barnyard: FUNCTION 00570640
void ASteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = GetKinematics()->GetPosition()->AsVector3();
}

// $Barnyard: FUNCTION 00570860
const TVector4* ASteer::GetPosition()
{
	return GetKinematics()->GetPosition();
}

// $Barnyard: FUNCTION 00555dd0
void ASteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = *GetDirection();
}

// $Barnyard: FUNCTION 00570710
void ASteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = GetKinematics()->GetFacingDir()->AsVector3();
}

// $Barnyard: FUNCTION 00570740
const TVector4* ASteer::GetDirection()
{
	return GetKinematics()->GetFacingDir();
}

// $Barnyard: FUNCTION 005707d0
void ASteer::CopyVelocity( TVector4& a_rOutVelocity )
{
	if ( GetKinematics() )
	{
		TVector4 vecVelocity = GetKinematics()->GetVelocity();
		a_rOutVelocity.Set( vecVelocity.x, vecVelocity.y, vecVelocity.z, 1.0f );
	}
	else
	{
		a_rOutVelocity = TVector4::VEC_ZERO;
	}
}

// $Barnyard: FUNCTION 00570870
TVector4 ASteer::GetVelocity()
{
	if ( GetKinematics() )
		return GetKinematics()->GetVelocity();

	return TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 00570750
TFLOAT ASteer::GetSpeed()
{
	return GetKinematics()->GetSpeed();
}

// $Barnyard: FUNCTION 00570760
TFLOAT ASteer::GetDesiredSpeed()
{
	return GetKinematics()->GetDesiredSpeed();
}

// $Barnyard: FUNCTION 00570790
TFLOAT ASteer::GetSpeedRatio()
{
	return GetKinematics()->GetSpeedRatio();
}

// $Barnyard: FUNCTION 005707b0
TFLOAT ASteer::GetMeasuredSpeedRatio()
{
	return GetKinematics()->GetMeasuredSpeedRatio();
}

// $Barnyard: FUNCTION 005707c0
TFLOAT ASteer::GetMeasuredSpeed()
{
	return GetKinematics()->GetMeasuredSpeed();
}

// $Barnyard: FUNCTION 005707a0
TFLOAT ASteer::GetTurnSpeed()
{
	return GetKinematics()->GetTurnSpeed();
}

// $Barnyard: FUNCTION 005708d0
TFLOAT ASteer::GetSpeedFromModifier( TFLOAT a_fModifier )
{
	return GetKinematics()->GetSpeedFromModifier( a_fModifier );
}

// $Barnyard: FUNCTION 005706a0
TFLOAT ASteer::GetGroundY()
{
	return GetKinematics()->GetGroundHeight();
}

// $Barnyard: FUNCTION 005706b0
TVector4 ASteer::GetGroundPosition()
{
	ASteerKinematics* pKinematics = GetKinematics();

	TVector4 vecPosition = *pKinematics->GetPosition();
	vecPosition.y        = pKinematics->GetGroundHeight();

	return vecPosition;
}

// $Barnyard: FUNCTION 00570850
TVector4& ASteer::PredictPosition( TINT a_iUnk, TVector4& a_rOut )
{
	return GetKinematics()->PredictPosition( a_iUnk, a_rOut );
}

// $Barnyard: FUNCTION 00570fc0
TBOOL ASteer::FaceTowards( const TVector4& a_rcTarget )
{
	TVector4 vecDirection = a_rcTarget;

	const TVector4* pPosition = GetPosition();
	vecDirection.x            = vecDirection.x - pPosition->x;
	vecDirection.z            = vecDirection.z - pPosition->z;
	vecDirection.y            = 0.0f;

	TFLOAT fMagSq = vecDirection.MagnitudeSqXZ();

	if ( fMagSq > 0.001f )
	{
		vecDirection.Divide( TMath::Sqrt( fMagSq ) );
		SetDesiredDirection( vecDirection );
		SetDirection( vecDirection );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00571190
TBOOL ASteer::FaceCamera()
{
	TVector4 vecCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	return FaceTowards( vecCameraPos );
}

// $Barnyard: FUNCTION 00570770
void ASteer::SetSpeed( TFLOAT a_fSpeed )
{
	GetKinematics()->SetSpeed( a_fSpeed );
}

// $Barnyard: FUNCTION 005708f0
void ASteer::SetDesiredSpeed( TFLOAT a_fSpeed )
{
	GetKinematics()->SetDesiredSpeed( a_fSpeed );
}

// $Barnyard: FUNCTION 00570c50
void ASteer::SetHasInteraction( TINT a_iValue )
{
	if ( a_iValue == 0 )
		SetInteracting( 0 );
}

// $Barnyard: FUNCTION 00570c70
TBOOL ASteer::Halt()
{
	ASteerKinematics* pKinematics = GetKinematics();
	pKinematics->Halt();
	pKinematics->SetSpeed( 0.0f );

	return TTRUE;
}

// $Barnyard: FUNCTION 00570ca0
TBOOL ASteer::HaltAtPosition( const TVector3& a_rcPosition )
{
	ASteerKinematics* pKinematics = GetKinematics();
	pKinematics->SetSpeed( 0.0f );
	pKinematics->SetPosition( TVector4( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 00570d00
TBOOL ASteer::HaltFacing( const TVector3& a_rcFacing )
{
	ASteerKinematics* pKinematics = GetKinematics();
	pKinematics->SetSpeed( 0.0f );
	pKinematics->SetFacing( TVector4( a_rcFacing.x, a_rcFacing.y, a_rcFacing.z, 1.0f ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 00570db0
TBOOL ASteer::Seek( const TVector4& a_rcTarget, TFLOAT a_fSeekOffset )
{
	ASteerKinematics* pKinematics = GetKinematics();

	if ( !IsIgnoringInvalidPosition() && !CanSeek( a_rcTarget ) )
		return TFALSE;

	pKinematics->SetSeekOffset( a_fSeekOffset );
	pKinematics->Seek( a_rcTarget );
	pKinematics->SetSeekSlowdown( TTRUE );
	SetToCurrentFullDesiredSpeed();

	return TTRUE;
}

// $Barnyard: FUNCTION 00570e30
TBOOL ASteer::SeekFacing( const TVector4& a_rcTarget, const TVector4& a_rcFacing )
{
	ASteerKinematics* pKinematics = GetKinematics();

	if ( !IsIgnoringInvalidPosition() && !CanSeek( a_rcTarget ) )
		return TFALSE;

	pKinematics->SetSeekOffset( 0.01f );
	pKinematics->SeekFacing( a_rcTarget, a_rcFacing, 1.0f );
	pKinematics->SetSeekSlowdown( TTRUE );
	SetToCurrentFullDesiredSpeed();

	return TTRUE;
}

// $Barnyard: FUNCTION 00570d60
TBOOL ASteer::HaltFacingDesired( const TVector3& a_rcFacing )
{
	ASteerKinematics* pKinematics = GetKinematics();
	pKinematics->SetSpeed( 0.0f );
	pKinematics->SetDesiredFacing( TVector4( a_rcFacing.x, a_rcFacing.y, a_rcFacing.z, 1.0f ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 00570f00
TBOOL ASteer::IsSeekingConstant()
{
	return GetKinematics()->IsSeekingConstant();
}

// $Barnyard: FUNCTION 00570f20
TBOOL ASteer::IsFacingAtDesiredFacing()
{
	return GetKinematics()->IsFacingAtDesiredFacing( 0.99f );
}

// $Barnyard: FUNCTION 00570ec0
TBOOL ASteer::GetSeekTarget( TVector3& a_rOutTarget )
{
	ASteerKinematics* pKinematics = GetKinematics();

	if ( pKinematics->IsSeekingConstant() )
	{
		a_rOutTarget = pKinematics->GetSeekTarget()->AsVector3();
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00570f40
TFLOAT ASteer::GetRadius()
{
	return m_pTraits->m_fRadius;
}

// $Barnyard: FUNCTION 005705f0
void ASteer::SetToCurrentFullDesiredSpeed()
{
	SetDesiredSpeed( GetSpeedFromModifier( m_fSpeedModifier ) );
}

// $Barnyard: FUNCTION 00570f50
TFLOAT ASteer::GetPositionX()
{
	return GetPosition()->x;
}

// $Barnyard: FUNCTION 00570f60
TFLOAT ASteer::GetPositionY()
{
	return GetPosition()->y;
}

// $Barnyard: FUNCTION 00570f70
TFLOAT ASteer::GetPositionZ()
{
	return GetPosition()->z;
}

// $Barnyard: FUNCTION 00570f80
void ASteer::SetPositionXYZ( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ )
{
	SetPosition( TVector4( a_fX, a_fY, a_fZ, 1.0f ) );
}

// $Barnyard: FUNCTION 00570c10
void ASteer::SetFacingFromPartsXYZ( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ )
{
	SetDirection( TVector4( a_fX, a_fY, a_fZ, 1.0f ) );
}

void ASteer::ValidateLOD( TINT& a_riLOD )
{
	// NOTE: inlined in retail (JPOG exports it as ASteer::ValidateLOD)
	TINT iMaxLOD = GetMaxLOD();

	if ( a_riLOD > iMaxLOD )
		a_riLOD = iMaxLOD;
}
