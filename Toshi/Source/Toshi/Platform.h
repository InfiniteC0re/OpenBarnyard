#pragma once

TOSHI_NAMESPACE_START

class TString8;
class TString16;

enum class Platform
{
	Wii,
	Unk2,
	PC,
	Unk3
};

void ThreadSleep( TUINT dwMilliseconds );
void FixPathSlashes( TString8& a_rPath );
void FixPathSlashesBack( TString8& a_rPath );

TString8  Platform_UnicodeToUTF8( const TWCHAR* a_wszUnicodeString );
TString16 Platform_UTF8ToUnicode( const TCHAR* a_pchUTF8String );

TOSHI_NAMESPACE_END
