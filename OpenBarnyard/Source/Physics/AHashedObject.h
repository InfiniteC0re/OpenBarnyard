#pragma once
#include "AObjectHash.h"

class AHashedObject
{
public:
	friend AObjectHash;

public:
	AHashedObject();
	virtual ~AHashedObject();

	virtual TFLOAT                 GetRadius()   = 0;
	virtual const Toshi::TVector4& GetPosition() = 0;
	virtual TBOOL                  Unknown3();
	virtual void                   SetUnk2( void* a_pUnk );
	virtual void*                  GetUnk2();

private:
	AObjectHash::CellObject* m_pHashNode;
	void* m_Unk2;
};
