#include "ToshiPCH.h"
#include "TObject.h"

namespace Toshi
{
	constinit TClass TObject::s_Class = TClass("TObject", TMAKEVERSION(1, 0), TObject::CreateTObject, TObject::CreateTObjectInPlace, sizeof(TObject));
}