#pragma once
#include "AModel.h"
#include "ANamedAnimationSet.h"
#include "AAnimControllerType.h"

#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

#include <Plugins/PPropertyParser/PBProperties.h>

class AAnimatableObject;
class AModelInstance;

class AAnimatableObjectType
    : public Toshi::T2DList<AAnimatableObjectType>::Node
{
public:
	constexpr static TSIZE MAX_ANIMATION_SETS = 6;

public:
	// constructors/destructor
	AAnimatableObjectType();
	virtual ~AAnimatableObjectType();

	virtual TBOOL CreateFromProperties( const PBProperties* a_pProperties, const Toshi::TPString8& a_rcName );
	virtual TBOOL Create( const Toshi::TPString8& a_rcName );
	virtual void  OnAnimatableObjectCreated( AAnimatableObject* a_pAnimatableObject );

	void Destroy();

	ANamedAnimationSetRef FindAnimationSet( const Toshi::TPString8& a_rcName ) const;
	ANamedAnimationSetRef GetAnimationSet( TINT a_iIndex ) const;
	ANamedAnimationSetRef GetDefaultAnimationSet() const;
	TINT                  GetNumAnimationSets() const;

	AModelInstance* Instantiate();

	const Toshi::TPString8& GetName() const { return m_strName; }
	AModel*                 GetModel() const { return m_pAModel; }

	Toshi::T2DList<AAnimControllerType>& GetAnimControllerTypes() { return m_llAnimControllerTypes; }

private:
	TBOOL LoadAnimationSet( const PBProperties* a_pProperties );
	TBOOL SetAnimController( const PBProperties* a_pProperties );
	TBOOL SetSkins( const PBProperties* a_pProperties );

private:
	Toshi::TPString8                                           m_strName;
	Toshi::T2Vector<ANamedAnimationSetRef, MAX_ANIMATION_SETS> m_vecAnimationSets;
	Toshi::T2DList<AAnimControllerType>                        m_llAnimControllerTypes;
	AModel*                                                    m_pAModel;
	void*                                                      m_pUnk;
};
