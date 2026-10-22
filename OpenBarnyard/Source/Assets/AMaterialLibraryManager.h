#pragma once
#include "AMaterialLibrary.h"

#include <Toshi/TPString8.h>
#include <Plugins/PPropertyParser/PBProperties.h>
#include <Render/TTexture.h>
#include <Toshi/T2Map.h>
#include <Toshi/T2SList.h>

class AMaterialLibraryManager
    : public Toshi::TObject
    , public Toshi::TSingleton<AMaterialLibraryManager>
{
public:
	TDECLARE_CLASS( AMaterialLibraryManager, Toshi::TObject );

public:
	class List : public Toshi::TSingleton<List>
	{
	public:
		// $Barnyard: FUNCTION 006158e0
		List() {}

		AMaterialLibrary* CreateLibraryFromTRB( Toshi::TTRB* a_pTRB, const TCHAR* a_szFilePath );
		AMaterialLibrary* CreateLibraryFromAsset( const TCHAR* a_szFilePath, Toshi::TTRB* a_pTRB );
		void              DestroyLibrary( AMaterialLibrary* a_pLibrary );
		ATexture*         FindTexture( const TCHAR* a_szTextureName, AMaterialLibrary** a_ppMaterialLibrary, TINT* a_pTextureIndex );

	private:
		Toshi::T2DList<AMaterialLibrary> m_Libraries;
	};

	class TextureSlot : public Toshi::T2SList<TextureSlot>::Node
	{
	public:
		// $Barnyard: FUNCTION 00613b00
		TextureSlot() {}

		// $Barnyard: FUNCTION 00613ae0
		~TextureSlot() {}

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

#ifdef BARNYARD_COMMUNITY_PATCH
	// More slots are added in blocks when the 600 run out
	static constexpr TINT NUM_EXTRA_SLOTS_PER_BLOCK = 256;
	static constexpr TINT MAX_EXTRA_SLOT_BLOCKS     = 64;
#endif

public:
	AMaterialLibraryManager();
	~AMaterialLibraryManager();

	void LoadLibrariesFromProperties( const PBPropertyValue* a_pProperty, Toshi::TTRB* a_pTRB, TBOOL a_bUpdateGUIMaterials );
	void UnloadLibrariesFromProperties( const PBPropertyValue* a_pProperty, TBOOL a_bUpdateGUIMaterials );
	void LoadLibrary( const Toshi::TPString8& a_rLibName, Toshi::TTRB* a_pTRB, TBOOL a_bIsGUI );
	void UnrefLibrary( const Toshi::TPString8& a_rLibName, TBOOL a_bUnused );

	void CreateTextures( AMaterialLibrary* a_pMatLibrary );
	void OnLibraryLoaded( TBOOL a_bIsGUI );

	Toshi::TTexture* FindTexture( const TCHAR* a_szTextureName );
	TINT             GetNumUsedTextures() const { return m_iNumUsedTextures; }
	TINT             GetNumFreeTextures() const { return m_iNumFreeTextures; }

	void UnloadTexturesOfLibrary( AMaterialLibrary* a_pMaterialLibrary );

	// Destroys every loaded library and forgets their reference counts
	void DestroyAllLibraries( TBOOL a_bUpdateGUIMaterials );

private:
	void DestroyLibrary( LibrariesMap::Iterator& a_rcMaterialLibraryNode, TBOOL a_bUpdateGUIMaterials );

#ifdef BARNYARD_COMMUNITY_PATCH
	TBOOL AddTextureSlots();
#endif

private:
	Toshi::T2Map<Toshi::TPString8, int, Toshi::TPString8::Comparator> m_NumRefLibraries;
	LibrariesMap                                                      m_LoadedLibraries;
	TextureSlot                                                       m_aSlots[ MAX_NUM_TEXTURES ];
	Toshi::T2SList<TextureSlot>                                       m_UsedTextures;
	Toshi::T2SList<TextureSlot>                                       m_FreeTextures;
	TINT                                                              m_iNumUsedTextures;
	TINT                                                              m_iNumFreeTextures;

#ifdef BARNYARD_COMMUNITY_PATCH
	TextureSlot* m_apExtraSlotBlocks[ MAX_EXTRA_SLOT_BLOCKS ];
	TINT         m_iNumExtraSlotBlocks;
#endif
};
