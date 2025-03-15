#pragma once
#include <Toshi/T2DList.h>

class AAnimatableObject;
class AAnimControllerType;
class ANamedAnimation;

class AAnimController
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimController>::Node
{
public:
	TDECLARE_CLASS( AAnimController, Toshi::TObject );

public:
	AAnimController();
	virtual ~AAnimController();

	virtual TBOOL OnCreate();
	virtual void  Unknown2();
	virtual TBOOL Unknown3( void* );
	virtual TBOOL IsAnimationAllowed( ANamedAnimation* a_pNamedAnimation );

	TBOOL Create( void* a_pUnk, AAnimatableObject* a_pAnimatableObject, AAnimControllerType* a_pControllerType );

	TBOOL IsEnabled() const { return !m_bDisabled; }

protected:
	void*                m_pUnk1;
	AAnimatableObject*   m_pAnimatableObject;
	AAnimControllerType* m_pControllerType;
	TBOOL                m_bDisabled;
};
