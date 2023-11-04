#pragma once
#include <Toshi/Strings/TPString8.h>
#include <Plugins/PPropertyParser/PProperties.h>
#include <Toshi/File/TTRB.h>
#include <Toshi/Render/TTexture.h>
#include <Toshi2/T2Map.h>
#include <Toshi2/T2SList.h>
#include <Toshi2/T2DList.h>

class AMaterialLibrary :
	public Toshi::T2DList<AMaterialLibrary>::Node
{
public:
	AMaterialLibrary();

	void SetPath(const char* a_szFilePath)
	{
		m_Path = a_szFilePath;
	}

private:
	Toshi::TString8 m_Path;
	int* m_Unk1;
	int* m_Unk2;
	int m_iNumTextures;
	Toshi::TTRB m_TRB;
};

class AMaterialLibraryManager : 
	public Toshi::TGenericClassDerived<AMaterialLibraryManager, Toshi::TObject, "AMaterialLibraryManager", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AMaterialLibraryManager>
{
public:
	class List : 
		public Toshi::TSingleton<List>
	{
	public:
		AMaterialLibrary* CreateLibrary(const char* a_szFilePath, Toshi::TTRB* a_pTRB);

		TBOOL LoadTTLData(void* a_pTTLData);
		TBOOL LoadTTLFile(const char* a_szFilePath);

	private:
		Toshi::T2DList<AMaterialLibrary> m_Libraries;
	};

	class TextureSlot :
		public Toshi::T2SList<TextureSlot>::Node
	{
	public:
		TextureSlot() = default;

	public:
		Toshi::TTexture* m_pTexture;
		Toshi::TString8 m_Name;
		AMaterialLibrary* m_MaterialLibrary;
	};

	static constexpr TUINT NUM_SLOTS = 600;

public:
	AMaterialLibraryManager();

	void LoadFromProperties(const PPropertyValue* a_pProperty, Toshi::TTRB* a_pTRB, TBOOL a_bUpdateGUIMaterials);
	void LoadLibrary(const Toshi::TPString8& a_rLibName, Toshi::TTRB* a_pTRB, TBOOL a_bIsGUI);

	void CreateTextures(AMaterialLibrary* a_pMatLibrary);
	void OnLibraryLoaded(TBOOL a_bIsGUI);

private:
	Toshi::T2Map<Toshi::TPString8, int, Toshi::TPString8::Comparator> m_NumRefLibraries; // ?
	Toshi::T2Map<Toshi::TPString8, AMaterialLibrary*, Toshi::TPString8::Comparator> m_LoadedLibraries;
	TextureSlot m_aSlots[600];
	Toshi::T2SList<TextureSlot> m_UsedTextures;
	Toshi::T2SList<TextureSlot> m_FreeTextures;
	TINT m_iNumUsedTextures;
	TINT m_iNumFreeTextures;
};