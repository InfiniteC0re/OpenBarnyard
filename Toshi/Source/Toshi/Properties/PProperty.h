#pragma once

#include "Toshi/Strings/TPCString.h"

class PPropertyName
{

	PPropertyName()
	{
		m_pName.m_pPooledString = TNULL;
		m_pSubName.m_pPooledString = TNULL;
	}

	PPropertyName(const Toshi::TPCString& a_pName, const Toshi::TPCString& a_pSubName)
	{
		m_pName.m_pPooledString = TNULL;
		if (a_pName.m_pPooledString)
		{
			m_pName.m_pPooledString = a_pName.m_pPooledString;
			a_pName.m_pPooledString->m_iCount++;
		}
		m_pSubName.m_pPooledString = TNULL;
		if (a_pSubName.m_pPooledString)
		{
			m_pSubName.m_pPooledString = a_pSubName.m_pPooledString;
			a_pSubName.m_pPooledString->m_iCount++;
		}
	}

	PPropertyName(const PPropertyName& other)
	{
		m_pName.m_pPooledString = TNULL;
		if (other.m_pName.m_pPooledString)
		{
			m_pName.m_pPooledString = other.m_pName.m_pPooledString;
			other.m_pName.m_pPooledString->m_iCount++;
		}
		m_pSubName.m_pPooledString = TNULL;
		if (other.m_pSubName.m_pPooledString)
		{
			m_pSubName.m_pPooledString = other.m_pSubName.m_pPooledString;
			other.m_pSubName.m_pPooledString->m_iCount++;
		}
	}

protected:
	void Split(const Toshi::TPCString& a_pName);


	Toshi::TPCString m_pName;
	Toshi::TPCString m_pSubName;
};

class PPropertyBlock
{

};

class PProperty
{

};

