#pragma once
#include "AMaterialLibrary.h"

#include <Toshi/TPString8.h>
#include <Plugins/PPropertyParser/PBProperties.h>
#include <Render/TTexture.h>
#include <Toshi/T2Map.h>
#include <Toshi/T2SList.h>

class AMaterialLibraryManager :
    public Toshi::TObject,
    public Toshi::TSingleton<AMaterialLibraryManager>
{
public:
	TDECLARE_CLASS( AMaterialLibraryManager, Toshi::TObject );

public:
	class List :
	    public Toshi::TSingleton<List>
	{
	public:
		AMaterialLibrary* CreateLibraryFromTRB( Toshi::TTRB* a_pTRB, const TCHAR* a_szFilePath );
		AMaterialLibrary* CreateLibraryFromAsset( const TCHAR* a_szFilePath, Toshi::TTRB* a_pTRB );
		ATexture*         FindTexture( const TCHAR* a_szTextureName, AMaterialLibrary** a_ppMaterialLibrary, TINT* a_pTextureIndex );

	private:
		Toshi::T2DList<AMaterialLibrary> m_Libraries;
	};

	class TextureSlot :
	    public Toshi::T2SList<TextureSlot>::Node
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

	static constexpr TUINT MAX_NUM_TEXTURES = 600;

public:
	AMaterialLibraryManager();

	void LoadLibrariesFromProperties( const PBPropertyValue* a_pProperty, Toshi::TTRB* a_pTRB, TBOOL a_bUpdateGUIMaterials );
	void LoadLibrary( const Toshi::TPString8& a_rLibName, Toshi::TTRB* a_pTRB, TBOOL a_bIsGUI );
	void UnloadLibrary( const Toshi::TPString8& a_rLibName, TBOOL a_bUnused );

	void CreateTextures( AMaterialLibrary* a_pMatLibrary );
	void OnLibraryLoaded( TBOOL a_bIsGUI );

	Toshi::TTexture* FindTexture( const TCHAR* a_szTextureName );
	TINT             GetNumUsedTextures() const { return m_iNumUsedTextures; }
	TINT             GetNumFreeTextures() const { return m_iNumFreeTextures; }

	void UnloadTexturesOfLibrary( AMaterialLibrary* a_pMaterialLibrary );

private:
	void DestroyLibrary( LibrariesMap::Iterator& a_rcMaterialLibraryNode, TBOOL a_bUpdateGUIMaterials );

private:
	Toshi::T2Map<Toshi::TPString8, int, Toshi::TPString8::Comparator> m_NumRefLibraries;
	LibrariesMap                                                      m_LoadedLibraries;
	TextureSlot                                                       m_aSlots[ MAX_NUM_TEXTURES ];
	Toshi::T2SList<TextureSlot>                                       m_UsedTextures;
	Toshi::T2SList<TextureSlot>                                       m_FreeTextures;
	TINT                                                              m_iNumUsedTextures;
	TINT                                                              m_iNumFreeTextures;
};
