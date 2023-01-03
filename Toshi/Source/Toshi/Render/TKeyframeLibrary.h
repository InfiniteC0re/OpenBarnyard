#pragma once

#undef LoadLibrary

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

		inline uint32_t GetSCount() { return m_iSCount; }
		inline void SetSCount(uint32_t a_iSCount) { m_iSCount = a_iSCount; }

	private:
		TKeyframeLibrary* m_pKeyLibrary;
		uint32_t m_iSCount; // 0x18
	};

	class TKeyframeLibraryManager
	{
	public:
		TKeyframeLibraryManager()
		{
			TIMPLEMENT();
		}
		
		~TKeyframeLibraryManager()
		{
			TIMPLEMENT();
		}

		TKeyframeLibrary* GetLibrary(const char* name)
		{
			TIMPLEMENT();
			return TNULL;
		}

		TKeyframeLibrary* LoadLibrary(class TRBHeader* trbheader)
		{
			TIMPLEMENT();
			return TNULL;
		}

		void UnloadLibrary(TKeyframeLibrary* library)
		{
			TIMPLEMENT();
		}
	};
}


