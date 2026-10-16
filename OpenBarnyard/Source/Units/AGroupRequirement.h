#pragma once
#include "AInteractionRequirement.h"

#include <ToshiTools/T2DynamicVector.h>

//-----------------------------------------------------------------------------
// Combines the requirements "requirements" (names or a single name): all of them
// have to be satisfied ("boolean" is "and", the default) or any of them ("or").
// TClass 0x0078ac8c, vtable 0x0072a600, size 0x64.
//-----------------------------------------------------------------------------
class AGroupRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AGroupRequirement, AInteractionRequirement );

	using BOOLEAN = TINT;
	enum BOOLEAN_ : BOOLEAN
	{
		BOOLEAN_OR  = 0,
		BOOLEAN_AND = 1,
	};

public:
	AGroupRequirement();
	~AGroupRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

#ifdef BARNYARD_LATEST
	// NOTE: not in the original. Replaces the combined requirements (indices into AInteractionManager's requirements),
	// used to apply data changes of the latest version to the PC data (see AInteractionManager::LoadInteractions)
	void SetRequirements( const TINT* a_piRequirements, TINT a_iNumRequirements )
	{
		m_vecRequirements.Clear();

		for ( TINT i = 0; i < a_iNumRequirements; i++ )
			m_vecRequirements.PushBack( a_piRequirements[ i ] );
	}
#endif

private:
	Toshi::T2DynamicVector<TINT> m_vecRequirements; // 0x4c, indices into AInteractionManager's requirements
	BOOLEAN                      m_eBoolean;        // 0x60, "boolean"
};

TSTATICASSERT( sizeof( AGroupRequirement ) == 0x64 );
