#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TString8.h>
#include <Toshi/TArray.h>
#include <Toshi/THPTimer.h>

// NOTE: the profiling methods (ProfileBegin, ProfileEnd, DumpToConsole...) are cut from the Windows build, only the
// constructor and the destructor are left. Nothing creates it but its TClass.
class ALoadTimeProfiler
    : public Toshi::TObject
    , public Toshi::TSingleton<ALoadTimeProfiler>
{
public:
	TDECLARE_CLASS( ALoadTimeProfiler, Toshi::TObject );

public:
	ALoadTimeProfiler();
	~ALoadTimeProfiler();

private:
	Toshi::THPTimer*               m_pTimer;        // 0x04
	TINT                           m_iUnk1;         // 0x08
	TINT                           m_iUnk2;         // 0x0C
	TCHAR                          m_aUnk3[ 0x80 ]; // 0x10
	Toshi::TArray<Toshi::TString8> m_aProfiles;     // 0x90
	TBOOL                          m_bUnk4;         // 0xA0
};
