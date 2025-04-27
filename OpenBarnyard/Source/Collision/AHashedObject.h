#pragma once
#include "AObjectHash.h"

class AHashedObject
{
public:
	AHashedObject();
	virtual ~AHashedObject();

	virtual TFLOAT           Unknown1() = 0;
	virtual Toshi::TVector3* Unknown2() = 0;
	virtual TBOOL            Unknown3();
	virtual void             SetUnk2( void* a_pUnk );
	virtual void*            GetUnk2();

private:
	AObjectHash::Node* m_pHashNode;
	void* m_Unk2;
};
