#include "pch.h"
#include "AToshiClassReferenceHelper.h"

#ifdef TOSHI_SKU_WINDOWS
#include <Platform/Windows/Input/TInputInterface_Win.h>
#include <Platform/DX8/TTextureFactoryHAL_DX8.h>
#endif // TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AToshiClassReferenceHelper::AToshiClassReferenceHelper()
{
	TTextureFactoryHAL::GetClassStatic();
	TInputDXInterface::GetClassStatic();
}
