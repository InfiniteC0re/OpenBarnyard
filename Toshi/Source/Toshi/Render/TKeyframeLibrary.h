#pragma once
namespace Toshi
{
	class TKeyframeLibraryInstance
	{
		
		int m_iSCount; // 0x18
	public:
		TKeyframeLibrary* m_pKeyLibrary;
		inline TKeyframeLibrary* GetLibrary() { return m_pKeyLibrary; }

		inline int GetSCount() { return m_iSCount; }
		inline void SetSCount(int a_iSCount) { m_iSCount = a_iSCount; }
		TKeyframeLibraryInstance();
	};

	class TKeyframeLibrary
	{
	public:
		int m_iReferenceCount; // 0x18
	
		inline int GetReferenceCount() { return m_iReferenceCount; }
	};
}


