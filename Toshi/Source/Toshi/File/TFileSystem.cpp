#include "pch.h"
#include "TFileSystem.h"

Toshi::TFileSystem::TFileSystem(TFileSystem const& param_1)
{
	m_unk = param_1.m_unk;
	m_unk2 = param_1.m_unk2;
	m_name = TCString(param_1.GetName());
	m_prefix = TCString(param_1.GetPrefix());
}

Toshi::TFileSystem::TFileSystem(const char* a_name)
{
	//???
	// m_unk = m_unk
	m_unk = TNULL;
	m_unk2 = m_unk;

	m_name = TCString(a_name);
	m_prefix = TCString();
}

Toshi::TFileSystem::~TFileSystem()
{
	m_name.~TCString();
	m_prefix.~TCString();
}

void Toshi::TFileSystem::SetPrefix(TCString const& a_prefix)
{
	m_prefix = a_prefix;
	size_t len = m_prefix.Length();
	size_t i = 0;
	if (0 < len)
	{
		do
		{
			if (m_prefix[i] == '/')
			{
				m_prefix[i] = '\\';
			}
			i++;
		} while (i < len);
	}

}
