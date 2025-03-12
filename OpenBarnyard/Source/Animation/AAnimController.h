#pragma once
#include <Toshi/T2DList.h>

class AAnimatableObject;
class AAnimControllerType;

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
	virtual TBOOL Unknown4( void* );

	TBOOL Create( void* a_pUnk, AAnimatableObject* a_pAnimatableObject, AAnimControllerType* a_pControllerType );

protected:
	void*                m_pUnk1;
	AAnimatableObject*   m_pAnimatableObject;
	AAnimControllerType* m_pControllerType;
	TBOOL                m_bUnk4;
};
