#pragma once

namespace Toshi
{
	class TKeyframeLibrary
	{
	public:
		inline void SetReferenceCount(uint32_t value) { m_iReferenceCount = value; }
		inline uint32_t GetReferenceCount() { return m_iReferenceCount; }

	private:
		uint32_t m_iReferenceCount; // 0x18
	};

	class TKeyframeLibraryInstance
	{
	public:
		TKeyframeLibraryInstance();
		inline void SetLibrary(TKeyframeLibrary* value) { m_pKeyLibrary = value; }
		inline TKeyframeLibrary* GetLibrary() { return m_pKeyLibrary; }

		inline int GetSCount() { return m_iSCount; }
		inline void SetSCount(int a_iSCount) { m_iSCount = a_iSCount; }

	private:
		TKeyframeLibrary* m_pKeyLibrary;
		int m_iSCount; // 0x18
	};
}


