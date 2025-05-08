#include "pch.h"
#include "AToshiClassReferenceHelper.h"

#ifdef TOSHI_SKU_WINDOWS
#  include <Platform/DX8/Input/TInputInterface_Win.h>
#  include <Platform/DX8/TTextureFactoryHAL_DX8.h>
#endif // TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#define REFERENCE_CLASS( CLASS_NAME ) static TANONYMOUSVAR( const TCHAR* ) = CLASS_NAME::GetClassStatic()->GetName()

AToshiClassReferenceHelper::AToshiClassReferenceHelper()
{
	REFERENCE_CLASS( TTextureFactoryHAL );
	REFERENCE_CLASS( TInputDXInterface );
	REFERENCE_CLASS( TInputInterface );
}
