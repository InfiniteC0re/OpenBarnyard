#pragma once

namespace Toshi
{
	class TRegion
	{
	public:
		static bool GetRegion() { return s_bRegionLocked; }
		static void SetRegion(bool locked) { s_bRegionLocked = locked; }
		static void LockRegion() { s_bRegionLocked = true; }
		static void UnlockRegion() { s_bRegionLocked = false; }

	private:
		static bool s_bRegionLocked;
	};
}