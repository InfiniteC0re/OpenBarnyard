#pragma once
#include "Render/AModel.h"
#include "ANamedAnimationSet.h"

#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

#include <Plugins/PPropertyParser/PBProperties.h>

class AAnimatableObjectType :
    public Toshi::T2DList<AAnimatableObjectType>::Node
{
public:
	constexpr static TSIZE MAX_ANIMATION_SETS = 6;

public:
	// constructors/destructor
	AAnimatableObjectType();
	virtual ~AAnimatableObjectType();

	virtual TBOOL CreateFromProperties( const PBProperties* a_pProperties, const Toshi::TPString8& a_rcName );
	virtual TBOOL Create( const Toshi::TPString8& a_rcName );
	virtual void  Unknown( void* );

	void Destroy();

	ANamedAnimationSetRef FindAnimationSet( const Toshi::TPString8& a_rcName );

private:
	TBOOL LoadAnimationSet( const PBProperties* a_pProperties );
	TBOOL SetAnimController( const PBProperties* a_pProperties );
	TBOOL SetSkins( const PBProperties* a_pProperties );

private:
	Toshi::TPString8                                           m_strName;
	Toshi::T2Vector<ANamedAnimationSetRef, MAX_ANIMATION_SETS> m_vecAnimationSets;
	Toshi::T2DList<AAnimatableObjectType>                      m_UnkList; // list type is a placeholder
	AModel*                                                    m_pAModel;
	void*                                                      m_pUnk;
};
