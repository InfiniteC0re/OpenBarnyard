#pragma once
#include <Toshi/Strings/TCString.h>

namespace Toshi
{
	class TFileSystem
	{
	public:

		TFileSystem(TFileSystem const&);
		TFileSystem(const char*);
		virtual ~TFileSystem();

		// Override these funcs in TNativeFileSystem
		virtual bool RemoveFile(TCString const&) { return true; }
		virtual bool GetFirstFile(TCString const&, TCString&, unsigned int) { return false; }
		virtual bool GetNextFile(TCString&, unsigned int) { return false; }
		virtual void SetPrefix(TCString const&);

		inline TCString const& GetName() const { return m_name; }
		inline TCString const& GetPrefix() const { return m_prefix; }

		void* m_unk; // 0x4
		void* m_unk2; // 0x8

		TCString m_name; // 0xC
		TCString m_prefix; // 0x14
		int m_unk3; // 0x1C
	};

}

