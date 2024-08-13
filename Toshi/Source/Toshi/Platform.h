#pragma once

TOSHI_NAMESPACE_START

class TString8;

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

TOSHI_NAMESPACE_END
