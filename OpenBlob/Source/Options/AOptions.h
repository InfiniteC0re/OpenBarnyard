#pragma once
class AOptions : Toshi::TObject, public Toshi::TSingleton<AOptions>
{
public:
	AOptions()
	{
		TTODO("Actually read it from g_oTheApp, aka safefile");
		m_Options->unk = 2;
		m_Options->unk2 = 8;
		m_Options->m_iVolume = 7;
	}

	typedef uint32_t OPTIONS;
	enum OPTIONS_ : OPTIONS
	{
		OPTIONS_MAXSIZE = 28
	};

	struct Options
	{
		int unk;
		int unk2;
		int m_iVolume;
	};

	Options* m_Options; // 0x4

	void SoundThings(TBOOL unk, float fUnk, float fUnk2);

	bool GetOptions(void* a_Options, uint32_t a_iMaxSize)
	{
		TASSERT(OPTIONS_MAXSIZE <= a_iMaxSize);
		Toshi::TUtil::MemCopy(a_Options, m_Options, OPTIONS_MAXSIZE);
	}
};

