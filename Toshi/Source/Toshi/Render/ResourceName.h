#pragma once
namespace Toshi
{
	class ResourceName
	{
		char m_cNameLength; // 0x0
		char* m_sName; // 0x1

	public:
		const char* GetName() { return m_sName; }
		int GetNameLength() { return m_cNameLength; }
	};
}


