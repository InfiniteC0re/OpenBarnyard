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
	virtual TBOOL                  IsNoQuery();
	virtual void                   SetQueryStamp( void* a_pUnk );
	virtual void*                  GetQueryStamp();

private:
	AObjectHash::CellObject* m_pHashNode;
	void*                    m_pQueryStamp;
};
