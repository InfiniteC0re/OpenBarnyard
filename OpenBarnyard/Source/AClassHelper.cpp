#include "pch.h"
#include "AClassHelper.h"

#ifdef TOSHI_SKU_WINDOWS
#include <Platform/Windows/Input/TInputInterface_Win.h>
#include <Platform/DX8/TTextureFactoryHAL_DX8.h>
#endif // TOSHI_SKU_WINDOWS

TOSHI_NAMESPACE_USING

void AClassHelper::ReferenceTClasses()
{
	TTextureFactoryHAL::GetClassStatic();
	TInputDXInterface::GetClassStatic();
}
