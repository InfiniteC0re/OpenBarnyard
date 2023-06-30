#pragma once
class AOptions : Toshi::TObject, public Toshi::TSingleton<AOptions>
{
public:
	AOptions()
	{
		TTODO("Actually read it from g_oTheApp, aka safefile");
		m_Options->m_iUnk = 2;
		m_Options->m_iPitch = 8;
		m_Options->m_iVolume = 7;
	}

	typedef uint32_t OPTIONS;
	enum OPTIONS_ : OPTIONS
	{
		OPTIONS_MAXSIZE = 28
	};

	// This is a part of the savefile!
	
	struct Options
	{
		int m_iUnk;
		int m_iPitch;
		int m_iVolume;
		float m_fUnk;
		int m_iUnk2;
		int m_iUnk3;
		int m_iUnk4;
	};
	static_assert(sizeof(Options) == OPTIONS_MAXSIZE);

	Options* m_Options; // 0x4

	void SoundThings(TBOOL unk, float fUnk, float fUnk2);

	bool GetOptions(void* a_Options, uint32_t a_iMaxSize)
	{
		TASSERT(OPTIONS_MAXSIZE <= a_iMaxSize);
		Toshi::TUtil::MemCopy(a_Options, m_Options, OPTIONS_MAXSIZE);
	}
};

