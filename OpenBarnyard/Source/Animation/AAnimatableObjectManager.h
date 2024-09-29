#pragma once
#include "AAnimatableObjectType.h"

#include <Toshi/TTask.h>
#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>
#include <File/TTRB.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class AAnimatableObjectManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AAnimatableObjectManager>
{
public:
	TDECLARE_CLASS( AAnimatableObjectManager, Toshi::TTask );

	struct PlaceholderNode : public Toshi::T2DList<PlaceholderNode>::Node
	{};

public:
	// constructors/destructor
	AAnimatableObjectManager();
	~AAnimatableObjectManager();

	void LoadTypesFromLibrary( const TCHAR* a_szLibName );

	void LoadAnimObjType( const TCHAR* a_szName, const PBProperties* a_pProperties, TBOOL a_bFlag );
	void LoadAnimObjType( const Toshi::TPString8& a_rcName, const PBProperties* a_pProperties, TBOOL a_bFlag );

	AAnimatableObjectType* FindType( const Toshi::TPString8& a_rcName );
	void DeleteType( const Toshi::TPString8& a_rcName );

	// TTask
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

private:
	void FUN_0057e3e0( AAnimatableObjectType* a_pObjectType );

private:
	Toshi::T2DList<PlaceholderNode>       m_UnkList;
	Toshi::T2DList<AAnimatableObjectType> m_ObjectTypes;
	Toshi::TTRB*                          m_pTRB;
	TINT                                  m_Unk;
};
