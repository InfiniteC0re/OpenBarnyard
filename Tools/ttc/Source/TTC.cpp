#include "pch.h"
#include "TTC.h"

TTC::Target TTC::ParseTarget(Toshi::TString8 targetName)
{
	if (targetName == "Win")
	{
		return TTC::Target::Win;
	}

	if (targetName == "PS2")
	{
		return TTC::Target::PS2;
	}

	if (targetName == "XBOX")
	{
		return TTC::Target::XBOX;
	}

	if (targetName == "GC")
	{
		return TTC::Target::GC;
	}

	if (targetName == "PSP")
	{
		return TTC::Target::PSP;
	}

	if (targetName == "DS")
	{
		return TTC::Target::DS;
	}

	return TTC::Target::Win;
}

TVersion TTC::ParseVersion(Toshi::TString8 versionString)
{
	TVersion version{ TMAKEVERSION(0, 0) };

	auto dotPos = versionString.Find(".");

	Toshi::TString8 majorStr;
	Toshi::TString8 minorStr;
	majorStr.Copy(versionString, dotPos);
	minorStr.Copy(versionString.GetString(dotPos + 1), versionString.Length() - dotPos - 1);

	version.Parts.Major = Toshi::T2String8::StringToInt(majorStr);
	version.Parts.Minor = Toshi::T2String8::StringToInt(minorStr);

	return version;
}

void TTC::Parse(rapidxml::xml_node<char>* ttcNode)
{
	m_Version = TTC::ParseVersion(ttcNode->first_attribute("Version")->value());
	m_Target = TTC::ParseTarget(ttcNode->first_attribute("Target")->value());
	m_LibraryName = ttcNode->first_attribute("LibraryName")->value();
	m_MaxTextureWidth = 512;
	m_Compressed = TFALSE;
	m_AutoMipMap = 0;

	if (ttcNode->first_attribute("BaseTexturePath") != TNULL)
	{
		m_BaseTexturePath = ttcNode->first_attribute("BaseTexturePath")->value();
	}

	if (ttcNode->first_attribute("AutoMipMap") != TNULL)
	{
		m_AutoMipMap = Toshi::T2String8::StringToInt(ttcNode->first_attribute("AutoMipMap")->value());
	}

	if (ttcNode->first_attribute("MaxTextureWidth") != TNULL)
	{
		m_MaxTextureWidth = Toshi::T2String8::StringToInt(ttcNode->first_attribute("MaxTextureWidth")->value());
	}

	if (ttcNode->first_attribute("Compressed") != TNULL)
	{
		m_Compressed = (bool)Toshi::T2String8::StringToInt(ttcNode->first_attribute("Compressed")->value());
	}

	// Textures
	for (auto texNode = ttcNode->first_node("Texture"); texNode != TNULL; texNode = texNode->next_sibling())
	{
		m_Textures.emplace_back(texNode->first_attribute("Filename")->value(), texNode->first_attribute("Format")->value());
	}
}

void TTC::Write()
{
	TIMPLEMENT();
}
