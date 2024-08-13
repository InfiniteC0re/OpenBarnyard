#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TString8.h>

class ADummyTask : public Toshi::TTask
{
public:
	TDECLARE_CLASS( ADummyTask, Toshi::TTask );

public:
	ADummyTask() = default;

	void SetName( const TCHAR* a_szName )
	{
		m_Name = a_szName;
	}

private:
	Toshi::TString8 m_Name;
};
