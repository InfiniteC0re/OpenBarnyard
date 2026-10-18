#pragma once
#include "TGUITextureSection.h"
#include "TGUIFont.h"
#include "TGUIStyle.h"
#include "TGUIWidgetInfo.h"
#include "TGUIComponentInfo.h"
#include "TGUILayoutManager.h"
#include "TGUIActionMap.h"

#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TArray.h>
#include <Toshi/TEvent.h>
#include <Toshi/TString8.h>
#include <Toshi/TString16.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TSumOfMinterms.h>

class PPropertyBlock;
class PPropertyWriter;

TOSHI_NAMESPACE_START

class TGUITextureFactory;
class TGUIFontFactory;
class TGUISoundFactory;
class TGUIWidget;
class TGUIScreen;

//-----------------------------------------------------------------------------
// Purpose: owner of the GUI definitions: loads the definition files (.gui)
// with its PPropertyReader and the registered definition blocks, keeps the
// texture sections, fonts, styles and top-level widgets they declare and
// reports runtime errors with an emitter.
//
// The widget, component and layout classes register themselves to the class
// lists of the manager. The manager also keeps the screens and the action map
// of the input.
//
// NOTE: the TGUIInterface of JPOG (Nicktoons has no TGUIInterface, the
// widgets use the manager singleton).
//-----------------------------------------------------------------------------
class TGUIManager
    : public TObject
    , public TSingleton<TGUIManager>
{
public:
	TDECLARE_CLASS( TGUIManager, TObject );

	// Data of the runtime error emitter
	struct RuntimeError
	{
		// $JPOG: FUNCTION TGuiInterface.dll+00006420
		RuntimeError( const TString16& a_rcTitle, const TString16& a_rcMessage )
		    : strTitle( a_rcTitle ), strMessage( a_rcMessage )
		{}

		TString16 strTitle;   // 0x00
		TString16 strMessage; // 0x08
	};

	using Emitter = TEmitter<TGUIManager, RuntimeError>;

	// A top-level widget declared by a [dialog] block, with the SKU condition of the block ("skufunction")
	struct TopLevelInfo : public TNodeList<TopLevelInfo>::TNode
	{
		TGUIWidget*    pWidget;      // 0x0C
		TSumOfMinterms oSkuFunction; // 0x10
	};

	// A style or a widget that references a texture section or a font
	// NOTE: TNodeWrapper<TObject> and TNodeWrapper<TGUIStyle> in JPOG
	struct ObjectReference : public TNodeList<ObjectReference>::TNode
	{
		TObject* pObject; // 0x0C
	};

	// A string declared by a [strings] block (TGUIStringBlock)
	struct String : public TNodeList<String>::TNode
	{
		TPString8 strName;
		TString16 strValue;
	};

public:
	TGUIManager();
	~TGUIManager();

	void AddDefinitionBlock( TRef<PPropertyBlock> a_pBlock );

	// Loads a definition file (.gui) with the registered definition blocks
	TBOOL LoadDefinitionFile( const TString8& a_rcFileName );

	// Same, with symbols defined for the #ifdef directives of the file
	TBOOL LoadDefinitionFile( const TString8& a_rcFileName, const TArray<TPString8>& a_rcDefines );

	// Returns the ID of the texture named a_rcName (from the texture factory)
	TINT16 FindTexture( const TPString8& a_rcName );

	TGUITextureSection* FindTextureSection( const TPString8& a_rcName );
	TGUITextureSection* AddTextureSection( TGUITextureSection* a_pTextureSection );
	void                DeleteTextureSection( TGUITextureSection* a_pTextureSection );
	void                DeleteAllTextureSections();

	TGUIFont* FindFont( const TPString8& a_rcName );
	void      AddFont( TGUIFont* a_pFont );
	void      DeleteFont( TGUIFont* a_pFont );
	void      DeleteAllFonts();

	// NOTE: Barnyard route added on top: makes the fonts no [font] block defines (the labels look them up with
	// FindFallbackFont when FindFont fails), see PGUITRFontFactory::CreateFontFromT2GUIFont
	using FontFallback = TGUIFont* (*)( TGUIManager* a_pManager, const TPString8& a_rcName );
	static void SetFontFallback( FontFallback a_fnFallback ) { ms_fnFontFallback = a_fnFallback; }
	TGUIFont*   FindFallbackFont( const TPString8& a_rcName );

	TGUIStyle* FindStyleInstance( const TPString8& a_rcName ) const;
	void       AddStyleInstance( TGUIStyle* a_pStyle );
	void       DeleteStyleInstance( TGUIStyle* a_pStyle );
	void       DeleteAllStyleInstances();

	// $NicktoonsPS2: FUNCTION 00366608
	// $JPOG: FUNCTION TGuiInterface.dll+00011b20
	TNodeList<TGUIStyle>& GetStyleInstanceList() { return m_oStyles; }

	// Creates a style of the class a_pClass for a widget class
	TGUIStyle* NewStyleInstance( const TGUIWidgetInfo* a_pWidgetInfo, const TPString8& a_rcName, const TClass* a_pClass );

	// The top-level widgets (the [dialog] blocks)
	void        AddTopLevelInstance( TGUIWidget* a_pWidget, TSumOfMinterms& a_rSkuFunction );
	void        DeleteTopLevelInstance( TGUIWidget* a_pWidget, const TSumOfMinterms& a_rcSkuFunction );
	void        DeleteAllTopLevelInstances();
	TGUIWidget* FindTopLevelInstance( const TPString8& a_rcName, TUINT a_uiSkus ) const;

	// $NicktoonsPS2: FUNCTION 00366628
	// $JPOG: FUNCTION TGuiInterface.dll+00011b10
	TNodeList<TopLevelInfo>& GetTopLevelInstanceList() { return m_oTopLevelInstances; }

	// The top-level instances that use a style (a new list that the caller deletes)
	TNodeList<TopLevelInfo>* FindStyleReferences( TGUIStyle* a_pStyle );

	// The styles and widgets that use a texture section, the styles that use a font (new lists that the caller deletes)
	TNodeList<ObjectReference>* FindTextureSectionReferences( TGUITextureSection* a_pTextureSection );
	TNodeList<ObjectReference>* FindFontReferences( TGUIFont* a_pFont );

	// The screens
	void AddScreenInstance( TGUIScreen* a_pScreen );
	void DeleteScreenInstance( TGUIScreen* a_pScreen );
	void DeleteAllScreenInstances();

	// $JPOG: FUNCTION TGuiInterface.dll+00011b00
	TNodeList<TGUIWidget>& GetScreenInstanceList() { return m_oScreens; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006460
	void SetDefaultScreen( TGUIScreen* a_pScreen ) { m_pDefaultScreen = a_pScreen; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006470
	TGUIScreen* GetDefaultScreen() const { return m_pDefaultScreen; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006450
	TGUIActionMap& GetActionMap() { return m_oActionMap; }

	// The SKUs of the platforms (the variables of the SKU conditions)
	void AddSku( const TPString8& a_rcSku );
	void DeleteSku( const TPString8& a_rcSku );
	void DeleteAllSkus();

	// Saves the definitions to a file (needs the writer of JPOG)
	TBOOL SaveDefinitionFile( const TString8& a_rcFileName );

	static void            AddWidgetClass( TGUIWidgetInfo* a_pWidgetInfo );
	static TGUIWidgetInfo* FindWidgetClass( const TPString8& a_rcName );

	static void               AddComponentClass( TGUIComponentInfo* a_pComponentInfo );
	static TGUIComponentInfo* FindComponentClass( const TPString8& a_rcName );

	static void            AddLayoutClass( TGUILayoutInfo* a_pLayoutInfo );
	static TGUILayoutInfo* FindLayoutClass( const TPString8& a_rcName );

	// $JPOG: FUNCTION TGuiInterface.dll+00011b40
	static TNodeList<TGUILayoutInfo>& GetLayoutClassList() { return ms_oLayoutClassList; }

	// $JPOG: FUNCTION TGuiInterface.dll+00011b50
	static TNodeList<TGUIWidgetInfo>& GetWidgetClassList() { return ms_oWidgetClassList; }

	// $JPOG: FUNCTION TGuiInterface.dll+00011b30
	static TNodeList<TGUIComponentInfo>& GetComponentClassList() { return ms_oComponentClassList; }

	// NOTE: JPOG adds the strings to the TLocale of the system, TOSHI 2.0's T2Locale is read only
	void          AddString( const TPString8& a_rcName, const TString16& a_rcValue );
	const String* FindString( const TPString8& a_rcName ) const;
	void          DeleteAllStrings();

	// $NicktoonsPS2: FUNCTION 00366640
	// $JPOG: FUNCTION TGuiInterface.dll+00011ac0
	void SetTextureFactory( TGUITextureFactory* a_pFactory ) { m_pTextureFactory = a_pFactory; }

	// $NicktoonsPS2: FUNCTION 00366650
	// $JPOG: FUNCTION TGuiInterface.dll+00011a90
	TGUITextureFactory* GetTextureFactory() const { return m_pTextureFactory; }

	// $NicktoonsPS2: FUNCTION 00366648
	// $JPOG: FUNCTION TGuiInterface.dll+00011aa0
	void SetSoundFactory( TGUISoundFactory* a_pFactory ) { m_pSoundFactory = a_pFactory; }

	// $NicktoonsPS2: FUNCTION 00366658
	// $JPOG: FUNCTION TGuiInterface.dll+00011a70
	TGUISoundFactory* GetSoundFactory() const { return m_pSoundFactory; }

	// $JPOG: FUNCTION TGuiInterface.dll+00011ab0
	void SetFontFactory( TGUIFontFactory* a_pFactory ) { m_pFontFactory = a_pFactory; }

	// $JPOG: FUNCTION TGuiInterface.dll+00011a80
	TGUIFontFactory* GetFontFactory() const { return m_pFontFactory; }

	// $NicktoonsPS2: FUNCTION 00366630
	// $JPOG: FUNCTION TGuiInterface.dll+00011af0
	TNodeList<TGUITextureSection>& GetTextureSectionList() { return m_oTextureSections; }

	// $JPOG: FUNCTION TGuiInterface.dll+00011ae0
	TNodeList<TGUIFont>& GetFontList() { return m_oFonts; }

	// $NicktoonsPS2: FUNCTION 00366638
	// $JPOG: FUNCTION TGuiInterface.dll+00011ad0
	TArray<TPString8>& GetSkuArray() { return m_aSkus; }

	TNodeList<String>& GetStrings() { return m_oStrings; }

	PPropertyReader* GetReader() const { return m_pReader; }
	Emitter*         GetRuntimeErrorEmitter() { return &m_oRuntimeErrorEmitter; }

	// Report a runtime error: print it and throw it with the runtime error emitter of the manager
	static void Error( const TString8& a_rcMessage );
	static void Error( const TString8& a_rcTitle, const TString8& a_rcMessage );

private:
	// Deletes the screens, the top-level widgets, the styles, the texture sections and the fonts
	void DeleteAllInstances();

	static TBOOL __stdcall OnReaderError( TGUIManager* a_pManager, PPropertyReader* a_pReader, PPropertyReaderParseError* a_pError );

private:
	TINT                                                               m_iUnknown;             // 0x04 0, -1 once destroyed
	PPropertyReader*                                                   m_pReader;              // 0x0C
	PPropertyWriter*                                                   m_pWriter;              // 0x10 never created in Nicktoons
	TGUITextureFactory*                                                m_pTextureFactory;      // 0x14
	TGUISoundFactory*                                                  m_pSoundFactory;        // 0x18
	TNodeList<TGUIStyle>                                               m_oStyles;              // 0x18
	TNodeList<TGUITextureSection>                                      m_oTextureSections;     // 0x4C
	TArray<TPString8>                                                  m_aSkus;                // 0x5C
	TListener<PPropertyReader, PPropertyReaderParseError, TGUIManager> m_oReaderListener;      // 0x104
	Emitter                                                            m_oRuntimeErrorEmitter; // 0x118

	static TNodeList<TGUIWidgetInfo>    ms_oWidgetClassList;
	static TNodeList<TGUIComponentInfo> ms_oComponentClassList;
	static TNodeList<TGUILayoutInfo>    ms_oLayoutClassList;

	// NOTE: Barnyard route (see SetFontFallback)
	inline static FontFallback ms_fnFontFallback = TNULL;

	TNodeList<TopLevelInfo> m_oTopLevelInstances; // 0x2C
	TNodeList<TGUIWidget>   m_oScreens;           // 0x3C
	TGUIActionMap           m_oActionMap;         // 0x70

	// NOTE: only in JPOG
	TGUIScreen* m_pDefaultScreen;

	// NOTE: only in JPOG
	TGUIFontFactory*    m_pFontFactory;
	TNodeList<TGUIFont> m_oFonts;
	TNodeList<String>   m_oStrings;
};

TOSHI_NAMESPACE_END
