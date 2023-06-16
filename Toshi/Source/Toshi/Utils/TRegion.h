#pragma once

namespace Toshi
{
	class TRegion
	{
	public:
		static int GetRegion() { return s_ToshiRegion; }
		static void SetRegion(int region) { TASSERT(!s_bRegionLocked); s_ToshiRegion = region; }
		static void LockRegion() { s_bRegionLocked = TTRUE; }
		static void UnlockRegion() { s_bRegionLocked = TFALSE; }

	private:
		static TBOOL s_bRegionLocked;
		static int s_ToshiRegion;
	};
}