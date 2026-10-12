#pragma once
#include "AAnimationBreakpoint.h"

#include <Toshi/TPString8.h>
#include <Math/TVector4.h>
#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Creates, destroys, detaches, hides, shows or moves attached objects of the
// object. The attached objects are found by "ModelName" (type of the object)
// or, if it's empty, by "BoneName" (bone of the parent).
//-----------------------------------------------------------------------------
class AAnimationBreakpointAttachment
    : public AAnimationBreakpoint
{
public:
	TDECLARE_CLASS( AAnimationBreakpointAttachment, AAnimationBreakpoint );

	// "Mode", order of the name table at 0x00775098
	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_CREATE_CHILD,         // "CreateChild"
		MODE_DESTROY_CHILD,        // "DestroyChild"
		MODE_DETACH_CHILD,         // "DetachChild"
		MODE_HIDE_CHILD,           // "HideChild"
		MODE_SHOW_CHILD,           // "ShowChild"
		MODE_SET_CHILD_TRANSFORM,  // "SetChildTransform"
		MODE_SET_PARENT_TRANSFORM, // "SetParentTransform"
		MODE_NUMOF,
	};

public:
	AAnimationBreakpointAttachment();
	~AAnimationBreakpointAttachment();

	//-----------------------------------------------------------------------------
	// AAnimationBreakpoint
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;             // +0x10
	virtual void  Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) OVERRIDE; // +0x14

	const Toshi::TPString8& GetModelName() const { return m_strModelName; }
	MODE                    GetMode() const { return m_eMode; }
	void                    SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }

private:
	// Returns the first attached object of a_pObject called m_strModelName, or attached to m_strBoneName
	AAnimatableObject* FindChild( AAnimatableObject* a_pObject );

	// Sets skeleton updating of the attached objects called m_strModelName, or attached to m_strBoneName
	void SetChildSkeletonUpdating( AAnimatableObject* a_pObject, TBOOL a_bUpdating );

private:
	Toshi::TVector4    m_vecTranslation;    // 0x08, "Translate"
	Toshi::TQuaternion m_quatRotation;      // 0x18, "Rotation" or "RotationAxis" and "RotationAngle" (degrees)
	Toshi::TPString8   m_strModelName;      // 0x28, "ModelName"
	Toshi::TPString8   m_strBoneName;       // 0x2c, "BoneName"
	Toshi::TPString8   m_strChildBoneName;  // 0x30, "ChildBoneName", only with MODE_CREATE_CHILD
	Toshi::TPString8   m_strParentBoneName; // 0x34, "ParentBoneName", only with MODE_CREATE_CHILD
	MODE               m_eMode;             // 0x38
	TBOOL              m_bEnabled;          // 0x3c, cleared by AHoneyCollectingGame for its "honeyglob"
};

TSTATICASSERT( sizeof( AAnimationBreakpointAttachment ) == 0x40 );
