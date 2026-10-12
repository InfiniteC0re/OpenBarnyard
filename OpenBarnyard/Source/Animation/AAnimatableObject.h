#pragma once
#include "AToshiAnimationInterface.h"
#include "AAnimatableObjectType.h"
#include "AAnimControllerType.h"
#include "AAnimController.h"
#include "AModel.h"
#include "AAnimationBreakpointCallback.h"
#include "FSM/AFunctionTable.h"

#include <Toshi/T2DList.h>
#include <Toshi/T2SList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TPString8.h>
#include <Math/TVector2.h>
#include <Math/TQuaternion.h>

struct ATerrainLocator;

class AAnimatableObject
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimatableObject>::Node
    , public Toshi::T2SList<AAnimatableObject>::Node
{
public:
	TDECLARE_CLASS( AAnimatableObject, Toshi::TObject );

	// Script natives of the class, for the FSM
	// $Barnyard: FUNCTION 0057c620
	virtual AFunctionTable* GetFunctionTable() { return &ms_oFunctionTable; } // +0x0c

	static AFunctionTable ms_oFunctionTable;

	struct AttachmentInfo
	{
		// $Barnyard: FUNCTION 0057b580
		AttachmentInfo()
		    : translation( Toshi::TVector4::VEC_ZERO )
		    , quaternion( Toshi::TQuaternion::IDENTITY )
		    , pParentObject( TNULL )
		    , pLocator( TNULL )
		    , iParentBone( -1 )
		    , iBone( -1 )
		{}

		Toshi::TVector4        translation;
		Toshi::TQuaternion     quaternion;
		AAnimatableObject*     pParentObject;
		const ATerrainLocator* pLocator; // 0x34, the object follows this terrain locator when there's no parent
		TINT8                  iParentBone;
		TINT8                  iBone;
	};

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE         = 0,
		FLAGS_CREATED      = BITFLAG( 0 ),
		FLAGS_NO_ANIMATION = BITFLAG( 1 ),
	};

	using CREATE_FLAGS = TUINT;
	enum CREATE_FLAGS_ : CREATE_FLAGS
	{
		CREATE_FLAGS_NONE               = 0,
		CREATE_FLAGS_NO_ANIM_CONTROLLER = BITFLAG( 0 ),
	};

	friend class AAnimatableObjectManager;

public:
	AAnimatableObject();
	~AAnimatableObject();

	TBOOL Create( AAnimatableObjectType* a_pObjectType, void* a_pOwner, CREATE_FLAGS a_eFlags = CREATE_FLAGS_NONE );
	TBOOL Update( TFLOAT a_fDeltaTime );
	void  UpdateAnimControllers( TFLOAT a_fDeltaTime );

	// Starts animation immediately if possible, or schedules it to play later. Return TTRUE if started immediately.
	TBOOL PlayAnimation( const Toshi::TPString8& a_strName );

	void KillAllAnimations();

	// Returns the progress (0..1) of the main or overlay animation called a_strName, 0 if it's not playing
	TFLOAT GetAnimationProgress( const Toshi::TPString8& a_strName );

	// Returns the sequence duration of the main or overlay animation called a_strName, 0 if it's not playing
	TFLOAT GetAnimationDuration( const Toshi::TPString8& a_strName );

	// Sets the speed of every playing animation
	void SetAnimSpeed( TFLOAT a_fSpeed ) { m_oToshiAnimInterface.SetAllAnimSpeeds( a_fSpeed ); }

	// Sets the glow material flag of every mesh of the model and makes the instance render in the glow pass
	void SetGlow( TBOOL a_bGlow );

	// Changes the speed of the main or overlay animation called a_strName
	TBOOL SetAnimationSpeed( const Toshi::TPString8& a_strName, TFLOAT a_fSpeed );

	// Returns TTRUE if the animation is playing. With a_fMinWeight > 0, a main animation must also
	// be at least that blended in and not blending out
	TBOOL IsAnimPlaying( const Toshi::TPString8& a_strName, TFLOAT a_fMinWeight );

	// Stops the main or overlay animation called a_strName. Returns TFALSE if it's not playing
	TBOOL FinishAnimation( const Toshi::TPString8& a_strName );

	// Sets the weight of the main or overlay animation called a_strName. Returns TFALSE if it's not playing
	TBOOL SetAnimWeight( const Toshi::TPString8& a_strName, TFLOAT a_fWeight );

	// Returns the sequence time of the main or overlay animation called a_strName, 0 if it's not playing
	TFLOAT GetAnimationTime( const Toshi::TPString8& a_strName );

	// Moves the animation a_strName (normal or overlay) to a_fTime
	TBOOL SetAnimationTime( const Toshi::TPString8& a_strName, TFLOAT a_fTime );

	// Switches to the animation set of the object type called a_strName (or the default one), the animations are killed if it changes
	TBOOL SwitchToNamedAnimationSet( const Toshi::TPString8& a_strName );

	// Returns TTRUE if the model was visible the last time it was rendered
	TBOOL IsInCameraFrustum();

	// Returns TTRUE if the main or overlay animation called a_strName is playing and doesn't end
	// (or wrap around, for a looping one) within a_fDeltaTime
	TBOOL IsAnimContinuing( const Toshi::TPString8& a_strName, TFLOAT a_fDeltaTime );

	// Returns the speed of the main or overlay animation called a_strName, -1 if it's not playing
	TFLOAT GetAnimationSpeed( const Toshi::TPString8& a_strName );

	// Returns TTRUE if the animation is neither planned nor playing
	TBOOL IsAnimationFinished( const Toshi::TPString8& a_strName );

	// Returns TTRUE if the main animation will transit to a_strName or a_strName is queued
	TBOOL IsAnimationPlanned( const Toshi::TPString8& a_strName );

	// Finds the callback breakpoint called a_strName in the animations of the object type.
	// Listeners can be connected to its emitter (AAnimationBreakpointCallback::GetEmitter)
	AAnimationBreakpointCallback* GetAnimationEmitter( Toshi::TPString8 a_strName );

	void SetVisible( TBOOL a_bVisible );
	void SetSkeletonUpdating( TBOOL a_bUpdating, TBOOL a_bRecursive );
	void SetOpacity( TFLOAT a_fOpacity, TBOOL a_bRecursive );

	// Applies a_iSkin of the object type (texture and, optionally, skin lighting). Returns TFALSE if the type has no such skin.
	TBOOL ApplyMaterial( TINT a_iSkin, TBOOL a_bSkinLighting );

	// Returns the first attached object that is attached to the a_rcBone bone of this object
	AAnimatableObject* FindAttachedObject( const Toshi::TPString8& a_rcBone );

	// Returns the first attached object whose parent bone is a_iParentBone
	AAnimatableObject* FindAttachedObject( TINT a_iParentBone );

	// Returns the first attached object whose type is called a_rcTypeName
	AAnimatableObject* FindAttachedObjectOfType( const Toshi::TPString8& a_rcTypeName );

	// Calls SetSkeletonUpdating on every attached object whose type is called a_rcTypeName
	void SetAttachedSkeletonUpdatingOfType( const Toshi::TPString8& a_rcTypeName, TBOOL a_bUpdating, TBOOL a_bRecursive );

	// Calls SetSkeletonUpdating on every object attached to the a_rcBone bone of this object
	void SetAttachedSkeletonUpdatingOnBone( const Toshi::TPString8& a_rcBone, TBOOL a_bUpdating, TBOOL a_bRecursive );

	// Sets the offset from the parent's bone
	void SetAttachmentTransform( const Toshi::TVector4& a_rcTranslation, const Toshi::TQuaternion& a_rcRotation );

	void SetAnimControllersDisabled( TBOOL a_bDisabled );

	// Returns the animation controller whose class is exactly a_pClass
	AAnimController* FindAnimController( Toshi::TClass* a_pClass );

	void DestroyAnimControllers();

	// Sets transform of the model instance from the parent's bone and the attachment offset
	void UpdateAttachmentTransform();

	// Returns TTRUE if the animation set of the object has the animation called a_rcName
	TBOOL HasAnimation( const Toshi::TPString8& a_rcName );

	// Calls UpdateAttachmentTransform on every object attached to this one
	void UpdateAttachedObjectTransforms();

	// Places the model at m_oAttachmentInfo.pLocator (in the world space of the terrain locators)
	void UpdateLocatorTransform();

	// Makes the object follow the terrain locator a_szLocator
	void AttachToLocator( const TCHAR* a_szLocator );

	// $Barnyard: FUNCTION 0057b5e0
	void DetachFromLocator() { m_oAttachmentInfo.pLocator = TNULL; }

	const ATerrainLocator* GetLocator() const { return m_oAttachmentInfo.pLocator; }

	const Toshi::T2Vector<Toshi::TPString8, 2>& GetQueuedAnimations() const { return m_vecQueuedAnims; }

	// $Barnyard: FUNCTION 00416500
	AModelInstanceRef GetModelInstance() { return m_pModelInstance; }

	Toshi::T2SList<AAnimatableObject>& GetAttachedObjects() { return m_llAttachedObjects; }
	TBOOL                              IsAnimated() const { return !HASANYFLAG( m_eFlags, FLAGS_NO_ANIMATION ); }
	void                               AddFlags( FLAGS a_eFlags ) { m_eFlags |= a_eFlags; }

	AToshiAnimationInterface& GetToshiAnimInterface() { return m_oToshiAnimInterface; }
	AttachmentInfo&           GetAttachmentInfo() { return m_oAttachmentInfo; }
	AAnimatableObject*        GetParentObject() const { return m_oAttachmentInfo.pParentObject; }
	void*                     GetOwner() const { return m_pOwner; }
	AAnimatableObjectType*    GetObjectType() const { return m_pObjectType; }

private:
	// Immediately starts animation if possible. Returns TTRUE if started animation.
	TBOOL StartAnimation( const Toshi::TPString8& a_strName );

	// Tries to play queued animations
	void PlayQueuedAnimation();

private:
	AttachmentInfo                       m_oAttachmentInfo;     // 0x10
	void*                                m_pOwner;              // 0x3c, owner
	AAnimatableObjectType*               m_pObjectType;         // 0x40
	AToshiAnimationInterface             m_oToshiAnimInterface; // 0x44
	AModelInstanceRef                    m_pModelInstance;      // 0x7c
	Toshi::T2DList<AAnimController>      m_llAnimControllers;   // 0x80
	Toshi::T2Vector<Toshi::TPString8, 2> m_vecQueuedAnims;      // 0x88
	Toshi::T2SList<AAnimatableObject>    m_llAttachedObjects;   // 0x94
	FLAGS                                m_eFlags;
};
