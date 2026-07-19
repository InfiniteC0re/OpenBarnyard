#pragma once
#include <Toshi/TSingleton.h>

#include <BYardSDK/AMaterialLibraryManager.h>

class AMaterialLibraryManager2
    : public Toshi::TObject
    , public Toshi::TSingleton<AMaterialLibraryManager2>
{
public:
	TDECLARE_CLASS( AMaterialLibraryManager2, Toshi::TObject );

public:
	class TextureSlot : public Toshi::T2SList<TextureSlot>::Node
	{
	public:
		TextureSlot() = default;

		void SetTexture( Toshi::TTexture* a_pTexture )
		{
			m_pTexture = a_pTexture;
		}

		Toshi::TTexture* GetTexture() const
		{
			return m_pTexture;
		}

		void SetName( const Toshi::TString8& a_Name )
		{
			m_Name = a_Name;
			m_Name.MakeLower();
		}

		void ResetName()
		{
			m_Name = "";
		}

		const Toshi::TString8& GetName()
		{
			return m_Name;
		}

		AMaterialLibrary* GetLibrary() const
		{
			return m_pMaterialLibrary;
		}

		void SetLibrary( AMaterialLibrary* a_pLibrary )
		{
			m_pMaterialLibrary = a_pLibrary;
		}

	private:
		Toshi::TTexture*  m_pTexture;
		Toshi::TString8   m_Name;
		AMaterialLibrary* m_pMaterialLibrary;
	};

	using LibrariesMap = Toshi::T2Map<Toshi::TPString8, AMaterialLibrary*, Toshi::TPString8::Comparator>;

	static constexpr TUINT MAX_NUM_TEXTURES = 2000;

public:
	AMaterialLibraryManager2();
	~AMaterialLibraryManager2() = default;

	void             CreateTextures( AMaterialLibrary* a_pMatLibrary );
	void             UnloadTextures( AMaterialLibrary* a_pMaterialLibrary );
	Toshi::TTexture* FindTexture( const TCHAR* a_szTextureName );

	TINT GetNumUsedTextures() const { return m_iNumUsedTextures; }
	TINT GetNumFreeTextures() const { return m_iNumFreeTextures; }

private:
	TextureSlot                                                       m_aSlots[ MAX_NUM_TEXTURES ];
	Toshi::T2SList<TextureSlot>                                       m_UsedTextures;
	Toshi::T2SList<TextureSlot>                                       m_FreeTextures;
	TINT                                                              m_iNumUsedTextures;
	TINT                                                              m_iNumFreeTextures;
};
