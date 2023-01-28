#pragma once
#include "rapidxml/rapidxml.hpp"

struct TTCTexture
{
	Toshi::TString8 Filename;
	Toshi::TString8 Format;

	TTCTexture(const char* filename, const char* format) : Filename(filename), Format(format)
	{
		
	}
};

class TTC
{
public:
	enum class Target
	{
		Win,
		PS2,
		XBOX,
		GC,
		PSP,
		DS,
	};

public:
	TTC() = default;

	static Target ParseTarget(Toshi::TString8 targetName);
	static TVersion ParseVersion(Toshi::TString8 versionString);

	void Parse(rapidxml::xml_node<char>* ttcNode);
	void Write();

private:
	TVersion m_Version;                // Version of the TTC XML file
	Target m_Target;                   // Name of target
	Toshi::TString8 m_LibraryName;     // Name of library
	Toshi::TString8 m_BaseTexturePath; // Base path of textures.  If not specified, the directory of this xml file will be the base.  All texture names will have their matching base path stripped if this is not empty.
	uint32_t m_AutoMipMap;             // Overrides the per-texture mip-map attribute. 0=no mip-map, otherwise how many.
	uint32_t m_MaxTextureWidth;
	// m_MaxTextureHeight?
	bool m_Compressed;
	std::vector<TTCTexture> m_Textures;
};