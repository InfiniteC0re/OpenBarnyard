#include "TObject.h"
#include <iostream>

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