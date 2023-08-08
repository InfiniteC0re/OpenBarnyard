#pragma once
#include "Toshi/Strings/TPString8.h"

class PPropertyName
{
public:
	PPropertyName()
	{
		
	}

	PPropertyName(const PPropertyName& other)
	{
		m_Name = other.m_Name;
		m_SubName = other.m_SubName;
	}

	PPropertyName(const Toshi::TPString8& a_oName, const Toshi::TPString8& a_oSubName)
	{
		m_Name = a_oName;
		m_SubName = a_oSubName;
	}

protected:
	void Split(const Toshi::TPString8& a_pName);

protected:
	Toshi::TPString8 m_Name;
	Toshi::TPString8 m_SubName;
};
