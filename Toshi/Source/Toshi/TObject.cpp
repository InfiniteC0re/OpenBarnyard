#include "pch.h"
#include "TObject.h"

namespace Toshi
{
	TObject::~TObject()
	{
		std::cout << "Called ~TObject" << std::endl;
	}

	void TObject::Destroy()
	{
		delete this;
	}
}