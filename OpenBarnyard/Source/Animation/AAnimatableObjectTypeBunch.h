#pragma once
#include "AAnimatableObjectType.h"

class AAnimatableObjectTypeBunch
    : public AAnimatableObjectType
{
public:
	enum TYPE : TINT
	{
		TYPE_BUNCH
	};

public:
	// constructors/destructor
	AAnimatableObjectTypeBunch();
	~AAnimatableObjectTypeBunch();

	// AAnimatableObjectType
	virtual TBOOL CreateFromProperties( const PBProperties* a_pProperties, const Toshi::TPString8& a_rcName ) OVERRIDE;
	virtual void  Unknown( AAnimatableObject* a_pAnimatableObject ) OVERRIDE;

private:
	TINT m_iBunchCount;
	TFLOAT m_fRange;
	Toshi::TPString8 m_strChildBone;
	Toshi::TPString8 m_strParentBone;
	TYPE             m_eType;
};
