#include "TObject.h"

// commented this because it's just a virtual destructor that is being created automatically
// see this for more info:
// https://stackoverflow.com/questions/8052146/whats-this-extra-parameter-passed-into-virtual-destructor

//void __thiscall Toshi::TObject::Delete()
//{
//	if (this != 0)
//	{
//		this->DestructTObject(1);
//	}
//}

// commented this because it's just a virtual constructor that is being created automatically
//void Toshi::TObject::DestructTObject(uint32_t param_2)
//{
//	if ((param_2 & 2) == 0)
//	{
//		Toshi::TObject::~TObject();
//		if ((param_2 & 1) != 0)
//		{
//			//tfree
//		}
//	}
//	else
//	{
//		
//	}
//}

//Toshi::TClass* __thiscall Toshi::TObject::GetClass()
//{
//	return &m_sClass;
//}