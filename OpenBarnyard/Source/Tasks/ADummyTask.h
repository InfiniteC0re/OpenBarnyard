#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TString8.h>

class ADummyTask : public Toshi::TTask
{
public:
	TDECLARE_CLASS( ADummyTask, Toshi::TTask );

public:
	// $Barnyard: FUNCTION 00404030
	ADummyTask() {}

	// $Barnyard: FUNCTION 00404430
	// $Barnyard: FUNCTION 00404410
	~ADummyTask() {}

	virtual TBOOL OnCreate() override { return TTRUE; } // +0x18, RETURNS_TRUE 0046cde0
	// $Barnyard: FUNCTION 00404080
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override { return TTRUE; } // +0x1C, jumps to RETURNS_TRUE 00413920

	void SetName( const TCHAR* a_szName )
	{
		m_Name = a_szName;
	}

private:
	Toshi::TString8 m_Name;
};
