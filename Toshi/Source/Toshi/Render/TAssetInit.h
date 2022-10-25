#pragma once
#include "Toshi/File/TTRB.h"

namespace Toshi
{
	class TAssetInit
	{
	public:
		static bool g_bCreateResources;
		static bool g_bAllowCrossTRBReferences;
		static TTRB* g_pCurrentTRB;
	};
}