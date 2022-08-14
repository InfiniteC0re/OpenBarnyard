#include "TObject.h"

void __thiscall Toshi::TObject::Delete()
{
	if (this != 0)
	{
		this->DestructTObject(1);
	}
}

void Toshi::TObject::DestructTObject(uint32_t param_2)
{
	if ((param_2 & 2) == 0)
	{
		Toshi::TObject::~TObject();
		if ((param_2 & 1) != 0)
		{
			//tfree
		}
	}
	else
	{
		
	}
}

Toshi::TObject::~TObject()
{

}

Toshi::TClass* __thiscall Toshi::TObject::GetClass()
{
	return &m_sClass;
}