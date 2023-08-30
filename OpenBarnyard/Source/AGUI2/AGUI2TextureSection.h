#pragma once
#include "AGUI2Material.h"

#include <Toshi/Math/TVector4.h>
#include <Toshi/File/TTRB.h>

class AGUI2TextureSection
{
public:
	const char* GetName() const
	{
		return m_szName;
	}

	AGUI2Material* GetMaterial()
	{
		return m_pMaterial;
	}

	const Toshi::TVector4& GetUVPoint1() const
	{
		return m_UV1;
	}

	const Toshi::TVector4& GetUVPoint2() const
	{
		return m_UV2;
	}

private:
	const char* m_szName;
	int m_Unk1;
	int m_iPoint1X;
	int m_iPoint1Y;
	int m_iPoint2X;
	int m_iPoint2Y;
	AGUI2Material* m_pMaterial;
	Toshi::TVector4 m_UV1;
	Toshi::TVector4 m_UV2;
};

class AGUI2TextureSectionManager
{
public:
	struct TRBHeader
	{
		AGUI2TextureSection* m_pSections;
		uint32_t m_uiNumSections;
	};

	inline static TBOOL g_providedtrb = TFALSE;
	inline static Toshi::TTRB* g_trb = TNULL;
	inline static TRBHeader* g_defblock = TNULL;
	inline static AGUI2TextureSection* g_sections = TNULL;

public:
	static TBOOL Open(const char* a_szFileName, Toshi::TTRB* a_pTRB);
	static void Close();

	static void CreateMaterials();
	static void DestroyMaterials();
};
