#pragma once
#include <Toshi/Core/TTask.h>
#include <Toshi/Strings/TString8.h>

class ADummyTask : public Toshi::TTask
{
public:
	TDECLARE_CLASS(Toshi::TTask);

public:
	ADummyTask() = default;

	void SetName(const char* a_szName)
	{
		m_Name = a_szName;
	}

private:
	Toshi::TString8 m_Name;
};
