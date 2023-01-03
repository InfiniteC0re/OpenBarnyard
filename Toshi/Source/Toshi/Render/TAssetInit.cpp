#include "ToshiPCH.h"
#include "TAssetInit.h"

namespace Toshi
{
	bool TAssetInit::g_bCreateResources = true;
	bool TAssetInit::g_bAllowCrossTRBReferences;
	TTRB* TAssetInit::g_pCurrentTRB;
}