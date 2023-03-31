#pragma once
#include <Toshi/Locale/T2Locale.h>

class ALocaleManager : public Toshi::T2Locale, public Toshi::TSingleton<ALocaleManager>
{
public:
	typedef int Platform;
	enum Platform_ : Platform
	{
		Platform_Wii,
		Platform_Unk,
		Platform_PC,
	};

	struct LocaleFormat
	{

	};


public:
	ALocaleManager();
	ALocaleManager(const ALocaleManager&) = delete;
	ALocaleManager(const ALocaleManager&&) = delete;

	virtual ~ALocaleManager() { };
	virtual const char* GetLanguageFilename(int32_t langid) override;

	void SetLanguage(Lang langid);

	static void Create();
	static void Destroy();

	static const char* GetLocaleCode(int code);
	inline const char* GetCurLocaleCode() { return GetLocaleCode(m_code); }
	static const char* GetVOLocaleCode(int code);

	static int GetSoundChannel(int code);
	inline int GetCurSoundChannel() { return GetSoundChannel(m_code); }

	static int GetOSLanguage() { return 0; }
	static int FixStringIdPlatform(int stringid);
	static ALocaleManager& Instance() { return *static_cast<ALocaleManager*>(s_Singleton); }

private:
	static constinit Platform s_Platform; // 00981a20

private:
	int m_code;							  // 0x24
	void* m_LocaleBuffer;                 // 0x2C
	Toshi::TTRB m_Format;                 // 0x30
	LocaleFormat* m_LocaleFormat;         // 0x44
};