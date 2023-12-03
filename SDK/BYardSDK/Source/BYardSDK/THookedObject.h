#pragma once

class THookedObject
{
public:
	virtual Toshi::TClass* GetClass() { }
	virtual void Delete() { }
	virtual ~THookedObject() { }

	TBOOL IsExactly(Toshi::TClass* pClass) { return GetClass() == pClass; }
	TBOOL IsA(Toshi::TClass* pClass) { return GetClass()->IsA(pClass); }
};
