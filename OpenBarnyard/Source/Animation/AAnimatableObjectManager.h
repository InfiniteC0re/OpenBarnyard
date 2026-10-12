#pragma once
#include "AAnimatableObjectType.h"
#include "AToshiAnimationInterface.h"
#include "AAnimatableObject.h"
#include "ASoundBreakpoint.h"

#include <Toshi/TTask.h>
#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2ObjectPool.h>
#include <File/TTRB.h>
#include <Plugins/PPropertyParser/PBProperties.h>

extern Toshi::T2ObjectPool<AToshiAnimationRef, 100> g_oAnimationRefPool;

class AAnimatableObjectManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AAnimatableObjectManager>
{
public:
	TDECLARE_CLASS( AAnimatableObjectManager, Toshi::TTask );

public:
	// constructors/destructor
	AAnimatableObjectManager();
	~AAnimatableObjectManager();

	//-----------------------------------------------------------------------------
	// TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	// Called by ARenderer::RenderMainScene once the cameras are set up: updates the skeletons of
	// objects that have something attached, moves attached objects and updates the particle effects
	void UpdateAttachedObjects( TFLOAT a_fDeltaTime );

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	AAnimatableObject* CreateAnimatableObject( AAnimatableObjectType* a_pObjectType, void* a_pOwner = TNULL, TUINT a_eFlags = 0 );

	// Loads all types specified in the library
	void LoadTypesFromLibrary( const TCHAR* a_szLibName );
	void UnloadTypesFromProperties( const PBPropertyValue* a_pArray );

	void  LoadAnimObjType( const TCHAR* a_szName, const PBProperties* a_pProperties, TBOOL a_bNoBreakpoints );
	void  LoadAnimObjType( const Toshi::TPString8& a_rcName, const PBProperties* a_pProperties, TBOOL a_bNoBreakpoints );
	TBOOL LoadAnimSoundBreakpoints( const TCHAR* a_szFilePath );

	TINT                   FindNumAnimationSoundBreakpoints( const Toshi::TPString8& a_rcModelName, ANamedAnimation* a_pAnimation );
	AAnimatableObjectType* FindType( const Toshi::TPString8& a_rcName );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: not in retail. TRUE if a loaded type holds a_pModel (the type unloads it in AAnimatableObjectType::Destroy)
	TBOOL IsModelUsedByType( const AModel* a_pModel );
#endif

	// Attaches bone a_iBone (-1: the origin) of a_pObject to bone a_iParentBone of a_pParent
	void AttachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent, TINT8 a_iBone, TINT8 a_iParentBone, const Toshi::TVector4& a_rcTranslation, const Toshi::TQuaternion& a_rcRotation, TFLOAT a_fScale );

	// Same as above but looks the bones up by name (empty a_rcBone: the origin)
	void AttachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent, const Toshi::TPString8& a_rcBone, const Toshi::TPString8& a_rcParentBone, const Toshi::TVector4& a_rcTranslation, const Toshi::TQuaternion& a_rcRotation, TFLOAT a_fScale );

	void DetachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent );

	// Returns the bone of a_pObject that is attached to its parent or -1
	TINT GetAttachmentBone( AAnimatableObject* a_pObject );

	// Returns the a_iIndex'th object attached to a_iBone of a_pObject
	AAnimatableObject* GetAttachedObject( AAnimatableObject* a_pObject, TINT a_iBone, TINT a_iIndex );
	AAnimatableObject* GetAttachedObject( AAnimatableObject* a_pObject, const Toshi::TPString8& a_rcBone, TINT a_iIndex );

	// NOTE: retail has two identical copies of GetAttachedObject by bone name (0057e350 and 0057e120)
	AAnimatableObject* GetAttachedObjectByBoneName( AAnimatableObject* a_pObject, const Toshi::TPString8& a_rcBone, TINT a_iIndex );

	// Returns the a_iIndex'th object of the type a_rcTypeName attached to a_iBone of a_pObject
	// NOTE: retail has these as __stdcall functions
	static AAnimatableObject* GetAttachedObjectOfType( AAnimatableObject* a_pObject, TINT a_iBone, const Toshi::TPString8& a_rcTypeName, TINT a_iIndex );
	static AAnimatableObject* GetAttachedObjectOfType( AAnimatableObject* a_pObject, const Toshi::TPString8& a_rcBone, const Toshi::TPString8& a_rcTypeName, TINT a_iIndex );

	// Returns number of objects attached to a_iBone of a_pObject
	TINT GetNumAttachedObjects( AAnimatableObject* a_pObject, TINT a_iBone );
	TINT GetNumAttachedObjects( AAnimatableObject* a_pObject, const Toshi::TPString8& a_rcBone );

	// Same as above but also counts what is attached to the root bone of the attached objects (recursively)
	TINT GetNumAttachedObjectsRecursive( AAnimatableObject* a_pObject, TINT a_iBone );
	TINT GetNumAttachedObjectsRecursive( AAnimatableObject* a_pObject, const Toshi::TPString8& a_rcBone );

	void DestroyType( const Toshi::TPString8& a_rcName );
	void DestroyType( const TCHAR* a_szName );
	void DestroyAnimatableObject( AAnimatableObject* a_pAnimatableObject );

	// Destroys every object of the type a_pType
	void DestroyAnimatableObjectsOfType( AAnimatableObjectType* a_pType );
	void DestroyAttachedObjects( AAnimatableObject* a_pAnimatableObject );

private:
	void AttachSoundBreakpoints( AAnimatableObjectType* a_pObjectType );

	// Moves a_pObject and everything attached to it to the end of the update list, so parents are updated before children
	void RegisterAnimatableObject( AAnimatableObject* a_pObject );

private:
	Toshi::T2DList<AAnimatableObject>     m_llAnimatableObjects;
	Toshi::T2DList<AAnimatableObjectType> m_llObjectTypes;
	Toshi::TTRB*                          m_pAnimSoundBPsTRB;
	ASoundBreakpointsTable*               m_pSoundBreakpoints;
};
