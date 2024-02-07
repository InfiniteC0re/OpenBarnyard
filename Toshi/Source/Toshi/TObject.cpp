#include "ToshiPCH.h"
#include "TObject.h"

namespace Toshi {

	TDEFINE_CLASS_COMPILETIME(TObject, 0, 2);

	void TObject::Delete()
	{
		delete this;
	}

	TObject::~TObject()
	{

	}

}

