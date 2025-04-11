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

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	AAnimatableObject* CreateAnimatableObject( AAnimatableObjectType* a_pObjectType, void* a_Unk1 = TNULL, TUINT a_eFlags = 0 );

	// Loads all types specified in the library
	void  LoadTypesFromLibrary( const TCHAR* a_szLibName );
	void  LoadAnimObjType( const TCHAR* a_szName, const PBProperties* a_pProperties, TBOOL a_bNoBreakpoints );
	void  LoadAnimObjType( const Toshi::TPString8& a_rcName, const PBProperties* a_pProperties, TBOOL a_bNoBreakpoints );
	TBOOL LoadAnimSoundBreakpoints( const TCHAR* a_szFilePath );

	TINT                   FindNumAnimationSoundBreakpoints( const Toshi::TPString8& a_rcModelName, ANamedAnimation* a_pAnimation );
	AAnimatableObjectType* FindType( const Toshi::TPString8& a_rcName );

	void DetachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent );

	void DestroyType( const Toshi::TPString8& a_rcName );
	void DestroyAnimatableObject( AAnimatableObject* a_pAnimatableObject );
	void DestroyAttachedObjects( AAnimatableObject* a_pAnimatableObject );

private:
	void AttachSoundBreakpoints( AAnimatableObjectType* a_pObjectType );

private:
	Toshi::T2DList<AAnimatableObject>     m_llAnimatableObjects;
	Toshi::T2DList<AAnimatableObjectType> m_llObjectTypes;
	Toshi::TTRB*                          m_pAnimSoundBPsTRB;
	ASoundBreakpointsTable*               m_pSoundBreakpoints;
};
