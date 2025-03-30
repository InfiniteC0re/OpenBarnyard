#pragma once
#include "AToshiAnimationInterface.h"
#include "AAnimatableObjectType.h"
#include "AAnimControllerType.h"
#include "AAnimController.h"
#include "AModel.h"

#include <Toshi/T2DList.h>
#include <Toshi/T2SList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TPString8.h>
#include <Math/TVector2.h>
#include <Math/TQuaternion.h>

class AAnimatableObject
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimatableObject>::Node
    , public Toshi::T2SList<AAnimatableObject>::Node
{
public:
	TDECLARE_CLASS( AAnimatableObject, Toshi::TObject );

	struct AttachmentInfo
	{
		Toshi::TVector4    translation   = Toshi::TVector4::VEC_ZERO;
		Toshi::TQuaternion quaternion    = Toshi::TQuaternion::IDENTITY;
		AAnimatableObject* pParentObject = TNULL;
		TINT               Unk4          = 0;
		TINT8              iParentBone   = -1;
		TINT8              iBone         = -1;
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

public:
	AAnimatableObject();
	~AAnimatableObject();

	TBOOL Create( AAnimatableObjectType* a_pObjectType, void* a_Unk1, CREATE_FLAGS a_eFlags = CREATE_FLAGS_NONE );
	TBOOL Update( TFLOAT a_fDeltaTime );
	void  UpdateAnimControllers( TFLOAT a_fDeltaTime );

	// Starts animation immediately if possible, or schedules it to play later. Return TTRUE if started immediately.
	TBOOL PlayAnimation( const Toshi::TPString8& a_strName );

	void KillAllAnimations();

	void SetVisible( TBOOL a_bVisible );
	void SetSkeletonUpdating( TBOOL a_bUpdating, TBOOL a_bRecursive );

	const Toshi::T2Vector<Toshi::TPString8, 2>& GetQueuedAnimations() const { return m_vecQueuedAnims; }

	AModelInstanceRef GetModelInstance() { return m_pModelInstance; }
	TBOOL             IsAnimated() const { return !HASANYFLAG( m_eFlags, FLAGS_NO_ANIMATION ); }

private:
	// Immediately starts animation if possible. Returns TTRUE if started animation.
	TBOOL StartAnimation( const Toshi::TPString8& a_strName );

	// Tries to play queued animations
	void PlayQueuedAnimation();

private:
	AttachmentInfo                       m_oAttachmentInfo;
	void*                                m_pUnk1;
	AAnimatableObjectType*               m_pObjectType;
	AToshiAnimationInterface             m_oToshiAnimInterface;
	AModelInstanceRef                    m_pModelInstance;
	Toshi::T2DList<AAnimController>      m_llAnimControllers;
	Toshi::T2Vector<Toshi::TPString8, 2> m_vecQueuedAnims;
	Toshi::T2SList<AAnimatableObject>    m_llAttachedObjects;
	FLAGS                                m_eFlags;
};
