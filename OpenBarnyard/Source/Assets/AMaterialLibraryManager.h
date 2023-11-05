#pragma once
#include "AMaterialLibrary.h"

#include <Toshi/Strings/TPString8.h>
#include <Plugins/PPropertyParser/PProperties.h>
#include <Toshi/Render/TTexture.h>
#include <Toshi2/T2Map.h>
#include <Toshi2/T2SList.h>

class AMaterialLibraryManager : 
	public Toshi::TGenericClassDerived<AMaterialLibraryManager, Toshi::TObject, "AMaterialLibraryManager", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AMaterialLibraryManager>
{
public:
	class List : 
		public Toshi::TSingleton<List>
	{
	public:
		AMaterialLibrary* CreateLibraryFromAsset(const char* a_szFilePath, Toshi::TTRB* a_pTRB);
		ATexture* FindTexture(const char* a_szTextureName, AMaterialLibrary** a_ppMaterialLibrary, TINT* a_pTextureIndex);

	private:
		Toshi::T2DList<AMaterialLibrary> m_Libraries;
	};

	class TextureSlot :
		public Toshi::T2SList<TextureSlot>::Node
	{
	public:
		TextureSlot() = default;

		void SetTexture(Toshi::TTexture* a_pTexture)
		{
			m_pTexture = a_pTexture;
		}

		Toshi::TTexture* GetTexture() const
		{
			return m_pTexture;
		}

		void SetName(const Toshi::TString8& a_Name)
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

		void SetLibrary(AMaterialLibrary* a_pLibrary)
		{
			m_pMaterialLibrary = a_pLibrary;
		}

	private:
		Toshi::TTexture* m_pTexture;
		Toshi::TString8 m_Name;
		AMaterialLibrary* m_pMaterialLibrary;
	};

	using LibrariesMap = Toshi::T2Map<Toshi::TPString8, AMaterialLibrary*, Toshi::TPString8::Comparator>;

	static constexpr TUINT NUM_SLOTS = 600;

public:
	AMaterialLibraryManager();

	void LoadFromProperties(const PPropertyValue* a_pProperty, Toshi::TTRB* a_pTRB, TBOOL a_bUpdateGUIMaterials);
	void LoadLibrary(const Toshi::TPString8& a_rLibName, Toshi::TTRB* a_pTRB, TBOOL a_bIsGUI);
	void UnloadLibrary(const Toshi::TPString8& a_rLibName, TBOOL a_bUnused);

	void CreateTextures(AMaterialLibrary* a_pMatLibrary);
	void OnLibraryLoaded(TBOOL a_bIsGUI);

	Toshi::TTexture* FindTexture(const char* a_szTextureName);

private:
	void DestroyLibrary(LibrariesMap::Node*& a_rpMaterialLibraryNode, TBOOL a_bUnused);
	void UnloadTexturesOfLibrary(AMaterialLibrary* a_pMaterialLibrary);

private:
	Toshi::T2Map<Toshi::TPString8, int, Toshi::TPString8::Comparator> m_NumRefLibraries;
	LibrariesMap m_LoadedLibraries;
	TextureSlot m_aSlots[600];
	Toshi::T2SList<TextureSlot> m_UsedTextures;
	Toshi::T2SList<TextureSlot> m_FreeTextures;
	TINT m_iNumUsedTextures;
	TINT m_iNumFreeTextures;
};