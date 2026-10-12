#pragma once
#ifdef BARNYARD_LATEST

#  include <Toshi/T2DList.h>
#  include <Toshi/T2Map.h>
#  include <Toshi/TString8.h>
#  include <Toshi/TPString8.h>
#  include <File/TTRB.h>

class AGUI2XUIResource;
class AGUI2XUIElement;
class AGUI2XUIScene;
class AGUI2XUIVisual;

//-----------------------------------------------------------------------------
// The XUI resources, scenes, material libraries and fonts the game loaded.
// It lives inside AGUI2 (at 0xC4C in the Wii build).
// NOTE: there's no RTTI for this class, the name follows TXUI of newer TOSHI.
//-----------------------------------------------------------------------------
class AGUI2XUI
{
public:
	struct SceneNode
	    : public Toshi::T2DList<SceneNode>::Node
	{
		AGUI2XUIScene* pScene; // 0x08
	};

	struct ResourceNode
	    : public Toshi::T2DList<ResourceNode>::Node
	{
		AGUI2XUIResource* pResource; // 0x08
		Toshi::TString8   strName;   // 0x0C
	};

	struct NameNode
	    : public Toshi::T2DList<NameNode>::Node
	{
		Toshi::TPString8 strName; // 0x08
	};

public:
	AGUI2XUI();
	~AGUI2XUI();

	void           AddScene( AGUI2XUIScene* a_pScene );
	void           DeleteScene( AGUI2XUIScene* a_pScene );
	AGUI2XUIScene* FindScene( const TWCHAR* a_wszId );
	void           DeleteScenes();

	AGUI2XUIVisual* CreateVisual( const TWCHAR* a_wszName );

	void              SetMainResource( AGUI2XUIResource* a_pResource );
	void              AddResource( const TCHAR* a_szName, AGUI2XUIResource* a_pResource );
	AGUI2XUIResource* FindResource( const TCHAR* a_szName );
	void              DeleteResources();

	void AddMaterialLibrary( const Toshi::TPString8& a_rcName );
	void UnloadMaterialLibraries();
	void AddFont( const Toshi::TPString8& a_rcName );
	void CloseFonts();

	// NOTE: in the original this is called on an empty object made on the stack.
	// It loads Data/XUI/<lang>/<name>.trb with its string table and gives it to a_pXUI.
	static AGUI2XUIResource* LoadResource( const TCHAR* a_szName, AGUI2XUI* a_pXUI, TBOOL a_bMainResource, TBOOL a_bUnused, Toshi::TTRB* a_pTRB );

private:
	Toshi::T2DList<SceneNode> m_llScenes; // 0x00
	// NOTE: nothing in the recovered code inserts into this map, the key type is unknown
	Toshi::T2Map<TUINT32, AGUI2XUIElement*> m_mapUnk;         // 0x08
	Toshi::T2DList<ResourceNode>            m_llResources;    // 0x20
	AGUI2XUIResource*                       m_pMainResource;  // 0x28
	Toshi::T2DList<NameNode>                m_llMaterialLibs; // 0x2C
	Toshi::T2DList<NameNode>                m_llFonts;        // 0x34
};

#endif // BARNYARD_LATEST
