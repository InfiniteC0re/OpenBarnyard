#include "pch.h"
#include "TObject.h"

namespace Toshi
{
	TObject::~TObject()
	{
		TOSHI_CORE_INFO("Called ~TObject");
	}

	void TObject::Destroy()
	{
		delete this;
	}
}