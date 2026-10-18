#include "ToshiPCH.h"
#include "TGUIManager.h"
#include "TGUIDeserialisable.h"
#include "TGUIFunction.h"
#include "TGUITextureFactory.h"
#include "TGUIFontFactory.h"
#include "TGUITextureBlock.h"
#include "TGUITextureSectionBlock.h"
#include "TGUITextureAnimationBlock.h"
#include "TGUIFontBlock.h"
#include "TGUIDialogBlock.h"
#include "TGUIStyleBlock.h"
#include "TGUIComponent.h"
#include "TGUIFrameComponent.h"
#include "TGUIHorizontalTripletComponent.h"
#include "TGUIVerticalTripletComponent.h"
#include "TGUIIconComponent.h"
#include "TGUITextComponent.h"
#include "TGUIStretch1x5Component.h"
#include "TGUIStretch5x1Component.h"
#include "TGUIConstantFunction.h"
#include "TGUISineFunction.h"
#include "TGUISawtoothFunction.h"
#include "TGUIWidget.h"
#include "TGUIWindow.h"
#include "TGUIScreen.h"
#include "TGUISoundFactory.h"
#include "TGUIAudioContext.h"
#include "TGUIInputContext.h"
#include "TGUIEffectManager.h"

#include <Plugins/PPropertyParser/PPropertyBlock.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUIManager );

TNodeList<TGUIWidgetInfo>    TGUIManager::ms_oWidgetClassList;
TNodeList<TGUIComponentInfo> TGUIManager::ms_oComponentClassList;
TNodeList<TGUILayoutInfo>    TGUIManager::ms_oLayoutClassList;

static TBOOL s_bStaticsInitialised = TFALSE;

// NOTE: not in the original. The classes the definition files create by name are referenced here (and initialised
// by InitialiseClass), the linker would strip them from the static library otherwise.
static TClass* s_apReferencedClasses[] = {
	&TGetClass( TGUIStyle ),
	&TGetClass( TGUIWidget ),
	&TGetClass( TGUIWindow ),
	&TGetClass( TGUIScreen ),
	&TGetClass( TGUILayoutManager ),
	&TGetClass( TGUIFrameComponent ),
	&TGetClass( TGUIHorizontalTripletComponent ),
	&TGetClass( TGUIVerticalTripletComponent ),
	&TGetClass( TGUIIconComponent ),
	&TGetClass( TGUITextComponent ),
	&TGetClass( TGUIStretch1x5Component ),
	&TGetClass( TGUIStretch5x1Component ),
	&TGetClass( TGUIConstantFunction ),
	&TGetClass( TGUISineFunction ),
	&TGetClass( TGUISawtoothFunction ),
};

static TBOOL InitialiseClassCallback( TClass* a_pClass, void* a_pUserData )
{
	TClass::TryInitialize( a_pClass );
	return TTRUE;
}

static TBOOL DeinitialiseClassCallback( TClass* a_pClass, void* a_pUserData )
{
	if ( a_pClass->IsInitialized() )
		a_pClass->Deinitialize();

	return TTRUE;
}

// Initialises the statics of the GUI classes
// NOTE: TGUIFunction is initialised too, its strings are created at static init time in Nicktoons
// $NicktoonsPS2: FUNCTION 00365d30
void TGUIManager::InitialiseClass()
{
	if ( s_bStaticsInitialised )
		return;

	for ( TClass* pClass : s_apReferencedClasses )
		TClass::TryInitialize( pClass );

	TGetClass( TGUIDeserialisable ).RecurseTree( TNULL, TNULL, InitialiseClassCallback, TNULL );
	TGetClass( TGUIComponent ).RecurseTree( TNULL, TNULL, InitialiseClassCallback, TNULL );
	TGetClass( TGUIFunction ).RecurseTree( TNULL, TNULL, InitialiseClassCallback, TNULL );

	s_bStaticsInitialised = TTRUE;
}

// $NicktoonsPS2: FUNCTION 00365df8
void TGUIManager::DeinitialiseClass()
{
	if ( !s_bStaticsInitialised )
		return;

	TGetClass( TGUIComponent ).RecurseTree( TNULL, TNULL, DeinitialiseClassCallback, TNULL );
	TGetClass( TGUIDeserialisable ).RecurseTree( TNULL, TNULL, DeinitialiseClassCallback, TNULL );
	TGetClass( TGUIFunction ).RecurseTree( TNULL, TNULL, DeinitialiseClassCallback, TNULL );

	s_bStaticsInitialised = TFALSE;
}

// NOTE: JPOG also creates a PPropertyWriter and registers TGUIFontBlock, which is registered here too
// $NicktoonsPS2: FUNCTION 00365418
// $JPOG: FUNCTION TGuiInterface.dll+00010150
TGUIManager::TGUIManager()
    : m_iUnknown( 0 )
    , m_pReader( TNULL )
    , m_pWriter( TNULL )
    , m_pTextureFactory( TNULL )
    , m_pSoundFactory( TNULL )
    , m_aSkus( 10, 0 )
    , m_oRuntimeErrorEmitter( this )
    , m_pFontFactory( TNULL )
    , m_pDefaultScreen( TNULL )
{
	m_pReader = new PPropertyReader();
	m_oReaderListener.Connect( *m_pReader->GetParseErrorEmitter(), this, OnReaderError, 0 );

	AddDefinitionBlock( TRef<PPropertyBlock>( new TGUITextureBlock() ) );
	AddDefinitionBlock( TRef<PPropertyBlock>( new TGUIFontBlock() ) );
	AddDefinitionBlock( TRef<PPropertyBlock>( new TGUITextureSectionBlock() ) );
	AddDefinitionBlock( TRef<PPropertyBlock>( new TGUITextureAnimationBlock() ) );
	AddDefinitionBlock( TRef<PPropertyBlock>( new TGUIDialogBlock() ) );
	AddDefinitionBlock( TRef<PPropertyBlock>( new TGUIStyleBlock() ) );

	// The actions of the doodads of the input devices: the navigation (North 0x200, East 0x400, South 0x800, West 0x1000),
	// the pointer moves (0x2000) and clicks (0x10000)...
	// NOTE: JPOG maps them in Create (with a few differences)
	m_oActionMap.AddActionMapping( 65538, 64 );
	m_oActionMap.AddActionMapping( 65540, 32 );
	m_oActionMap.AddActionMapping( 131189, 64 );
	m_oActionMap.AddActionMapping( 131188, 32 );
	m_oActionMap.AddActionMapping( 131108, 64 );
	m_oActionMap.AddActionMapping( 131106, 32 );
	m_oActionMap.AddActionMapping( 65537, 512 );
	m_oActionMap.AddActionMapping( 65538, 1024 );
	m_oActionMap.AddActionMapping( 65539, 2048 );
	m_oActionMap.AddActionMapping( 65540, 4096 );
	m_oActionMap.AddActionMapping( 65553, 512 );
	m_oActionMap.AddActionMapping( 65554, 1024 );
	m_oActionMap.AddActionMapping( 65555, 2048 );
	m_oActionMap.AddActionMapping( 65556, 4096 );
	m_oActionMap.AddActionMapping( 131186, 512 );
	m_oActionMap.AddActionMapping( 131189, 1024 );
	m_oActionMap.AddActionMapping( 131191, 2048 );
	m_oActionMap.AddActionMapping( 131188, 4096 );
	m_oActionMap.AddActionMapping( 131093, 512 );
	m_oActionMap.AddActionMapping( 131108, 1024 );
	m_oActionMap.AddActionMapping( 131107, 2048 );
	m_oActionMap.AddActionMapping( 131106, 4096 );
	m_oActionMap.AddActionMapping( 65545, 2 );
	m_oActionMap.AddActionMapping( 65547, 1 );
	m_oActionMap.AddActionMapping( 65547, 0x20000 );
	m_oActionMap.AddActionMapping( 196619, 0x2000 );
	m_oActionMap.AddActionMapping( 196609, 0x4000 );
	m_oActionMap.AddActionMapping( 196610, 0x8000 );
	m_oActionMap.AddActionMapping( 196609, 0x10000 );
	m_oActionMap.AddActionMapping( 196610, 0x10000 );
	m_oActionMap.AddActionMapping( 196611, 0x10000 );
	m_oActionMap.AddActionMapping( 131133, 1 );
	m_oActionMap.AddActionMapping( 131104, 1 );
	m_oActionMap.AddActionMapping( 131178, 1 );
	m_oActionMap.AddActionMapping( 131090, 2 );
	m_oActionMap.AddActionMapping( 131077, 2 );

	InitialiseClass();
}

// $NicktoonsPS2: FUNCTION 00365968
// $JPOG: FUNCTION TGuiInterface.dll+00010390
TGUIManager::~TGUIManager()
{
	// NOTE: Nicktoons destroys the listener at the end, the TEmitter destructor of the reader disconnects it there
	m_oReaderListener.Disconnect();

	if ( m_pReader )
		delete m_pReader;

	if ( m_pWriter )
		delete m_pWriter;

	DeleteAllInstances();
	DeleteAllStrings();

	DeinitialiseClass();

	m_oRuntimeErrorEmitter.Destroy();

	// NOTE: TArray doesn't destroy its elements
	for ( TINT i = 0; i < m_aSkus.Size(); i++ )
		m_aSkus[ i ].~TPString8();

	m_aSkus.FreeData();
	m_iUnknown = -1;
}

// NOTE: inlined in the destructor in JPOG
// $NicktoonsPS2: FUNCTION 003662e8
void TGUIManager::DeleteAllInstances()
{
	DeleteAllScreenInstances();
	DeleteAllTopLevelInstances();
	DeleteAllStyleInstances();
	DeleteAllTextureSections();
	DeleteAllFonts();
}

// $NicktoonsPS2: FUNCTION 00365e80
// $JPOG: FUNCTION TGuiInterface.dll+000107f0
void TGUIManager::AddDefinitionBlock( TRef<PPropertyBlock> a_pBlock )
{
	m_pReader->AddDefinitionBlock( a_pBlock );
}

// NOTE: stripped from Nicktoons (the error strings are still there)
// $JPOG: FUNCTION TGuiInterface.dll+00010860
TBOOL TGUIManager::LoadDefinitionFile( const TString8& a_rcFileName )
{
	if ( !m_pReader->Open( a_rcFileName, 2 ) )
	{
		Error( TString8::VarArgs( "TGUIManager::LoadDefinitionFile(\"%s\") Failed to open file!", a_rcFileName.GetString() ) );
		return TFALSE;
	}

	if ( !m_pReader->Load() )
	{
		Error( TString8::VarArgs( "TGUIManager::LoadDefinitionFile(\"%s\") Failed to load correctly!", a_rcFileName.GetString() ) );
		return TFALSE;
	}

	m_pReader->Close();
	return TTRUE;
}

// NOTE: stripped from Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+000108b0
TBOOL TGUIManager::LoadDefinitionFile( const TString8& a_rcFileName, const TArray<TPString8>& a_rcDefines )
{
	if ( !m_pReader->Open( a_rcFileName, 2 ) )
	{
		Error( TString8::VarArgs( "TGUIManager::LoadDefinitionFile(\"%s\") Failed to open file!", a_rcFileName.GetString() ) );
		return TFALSE;
	}

	for ( TINT i = 0; i < a_rcDefines.Size(); i++ )
		m_pReader->GetLexer()->Define( a_rcDefines[ i ].GetString() );

	if ( !m_pReader->Load() )
	{
		Error( TString8::VarArgs( "TGUIManager::LoadDefinitionFile(\"%s\") Failed to load correctly!", a_rcFileName.GetString() ) );
		return TFALSE;
	}

	m_pReader->Close();
	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00365fd8
// $JPOG: FUNCTION TGuiInterface.dll+00011de0
TINT16 TGUIManager::FindTexture( const TPString8& a_rcName )
{
	return m_pTextureFactory->GetTextureID( a_rcName );
}

// $NicktoonsPS2: FUNCTION 00366000
// $JPOG: FUNCTION TGuiInterface.dll+00010b90
TGUITextureSection* TGUIManager::FindTextureSection( const TPString8& a_rcName )
{
	for ( auto it = m_oTextureSections.Begin(); it != m_oTextureSections.End(); it++ )
	{
		if ( it->GetName() == a_rcName )
			return it;
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 00366280
// $JPOG: FUNCTION TGuiInterface.dll+00011d80
TGUITextureSection* TGUIManager::AddTextureSection( TGUITextureSection* a_pTextureSection )
{
	m_oTextureSections.InsertTail( a_pTextureSection );
	return a_pTextureSection;
}

// $JPOG: FUNCTION TGuiInterface.dll+00010f50
void TGUIManager::DeleteTextureSection( TGUITextureSection* a_pTextureSection )
{
	for ( auto it = m_oTextureSections.Begin(); it != m_oTextureSections.End(); it++ )
	{
		if ( it.Get() == a_pTextureSection )
		{
			m_oTextureSections.Delete( a_pTextureSection );
			return;
		}
	}
}

// $NicktoonsPS2: FUNCTION 00366550
// $JPOG: FUNCTION TGuiInterface.dll+00011c00
void TGUIManager::DeleteAllTextureSections()
{
	m_oTextureSections.DeleteAll();
}

// NOTE: Nicktoons looks the fonts up in its own font cache (fonts.trb, FUN_003c7218)
// $JPOG: FUNCTION TGuiInterface.dll+00010bc0
TGUIFont* TGUIManager::FindFont( const TPString8& a_rcName )
{
	for ( auto it = m_oFonts.Begin(); it != m_oFonts.End(); it++ )
	{
		if ( it->GetName() == a_rcName )
			return it;
	}

	return TNULL;
}

// NOTE: Barnyard route added on top (see SetFontFallback)
TGUIFont* TGUIManager::FindFallbackFont( const TPString8& a_rcName )
{
	TGUIFont* pFont = FindFont( a_rcName );

	if ( !pFont && ms_fnFontFallback )
		pFont = ms_fnFontFallback( this, a_rcName );

	return pFont;
}

// $JPOG: FUNCTION TGuiInterface.dll+00010d30
void TGUIManager::AddFont( TGUIFont* a_pFont )
{
	m_oFonts.InsertTail( a_pFont );
	a_pFont->SetFontID( m_pFontFactory->GetFontID( a_pFont->GetName() ) );
	a_pFont->SetManager( this );
}

// $JPOG: FUNCTION TGuiInterface.dll+00011d00
void TGUIManager::DeleteFont( TGUIFont* a_pFont )
{
	m_oFonts.Delete( a_pFont );
}

// NOTE: Nicktoons clears its own font cache instead (FUN_003c7288)
// $NicktoonsPS2: FUNCTION 003665e8
// $JPOG: FUNCTION TGuiInterface.dll+00011b80
void TGUIManager::DeleteAllFonts()
{
	m_oFonts.DeleteAll();
}

void TGUIManager::AddString( const TPString8& a_rcName, const TString16& a_rcValue )
{
	for ( auto it = m_oStrings.Begin(); it != m_oStrings.End(); it++ )
	{
		if ( it->strName == a_rcName )
		{
			it->strValue = a_rcValue;
			return;
		}
	}

	String* pString   = new String();
	pString->strName  = a_rcName;
	pString->strValue = a_rcValue;
	m_oStrings.InsertTail( pString );
}

const TGUIManager::String* TGUIManager::FindString( const TPString8& a_rcName ) const
{
	for ( auto it = m_oStrings.Begin(); it != m_oStrings.End(); it++ )
	{
		if ( it->strName == a_rcName )
			return it;
	}

	return TNULL;
}

void TGUIManager::DeleteAllStrings()
{
	m_oStrings.DeleteAll();
}

// NOTE: the title is a static string of Nicktoons that is empty
// $NicktoonsPS2: FUNCTION 00366b80
// $JPOG: FUNCTION TGuiInterface.dll+000114a0
void TGUIManager::Error( const TString8& a_rcMessage )
{
	TERROR( "Runtime error in TGUIManager:\n\t%s\n", a_rcMessage.GetString() );

	RuntimeError oError{ TString16( L"" ), TString16( a_rcMessage ) };
	GetSingleton()->m_oRuntimeErrorEmitter.Throw( &oError );
}

// $NicktoonsPS2: FUNCTION 00366a28
// $JPOG: FUNCTION TGuiInterface.dll+00011580
void TGUIManager::Error( const TString8& a_rcTitle, const TString8& a_rcMessage )
{
	TERROR( "Runtime error in TGUIManager:\n\t%s\n", a_rcMessage.GetString() );

	RuntimeError oError{ TString16( a_rcTitle ), TString16( a_rcMessage ) };
	GetSingleton()->m_oRuntimeErrorEmitter.Throw( &oError );
}

// $NicktoonsPS2: FUNCTION 00366cd8
// $JPOG: FUNCTION TGuiInterface.dll+000117c0
TBOOL __stdcall TGUIManager::OnReaderError( TGUIManager* a_pManager, PPropertyReader* a_pReader, PPropertyReaderParseError* a_pError )
{
	TString8 strMessage;
	strMessage.Format( "%s", a_pError->m_szMessage );

	RuntimeError oError{ TString16( L"PPropertyReader" ), TString16( strMessage ) };
	a_pManager->m_oRuntimeErrorEmitter.Throw( &oError );

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00365f70
// $JPOG: FUNCTION TGuiInterface.dll+00010ad0
TGUIStyle* TGUIManager::FindStyleInstance( const TPString8& a_rcName ) const
{
	for ( auto it = m_oStyles.Begin(); it != m_oStyles.End(); it++ )
	{
		if ( it->GetName() == a_rcName )
			return it;
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 003660a0
// $JPOG: FUNCTION TGuiInterface.dll+00011db0
void TGUIManager::AddStyleInstance( TGUIStyle* a_pStyle )
{
	m_oStyles.InsertTail( a_pStyle );
}

// $JPOG: FUNCTION TGuiInterface.dll+00011d40
void TGUIManager::DeleteStyleInstance( TGUIStyle* a_pStyle )
{
	m_oStyles.Delete( a_pStyle );
}

// $NicktoonsPS2: FUNCTION 00366330
void TGUIManager::DeleteAllStyleInstances()
{
	m_oStyles.DeleteAll();
}

// $NicktoonsPS2: FUNCTION 003669c8
// $JPOG: FUNCTION TGuiInterface.dll+00011410
void TGUIManager::AddWidgetClass( TGUIWidgetInfo* a_pWidgetInfo )
{
	ms_oWidgetClassList.InsertHead( a_pWidgetInfo );
}

// $NicktoonsPS2: FUNCTION 00365f00
// $JPOG: FUNCTION TGuiInterface.dll+00010a10
TGUIWidgetInfo* TGUIManager::FindWidgetClass( const TPString8& a_rcName )
{
	for ( auto it = ms_oWidgetClassList.Begin(); it != ms_oWidgetClassList.End(); it++ )
	{
		if ( it->GetWidgetName() == a_rcName )
			return it;
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 003669f8
// $JPOG: FUNCTION TGuiInterface.dll+00011470
void TGUIManager::AddComponentClass( TGUIComponentInfo* a_pComponentInfo )
{
	ms_oComponentClassList.InsertHead( a_pComponentInfo );
}

// $JPOG: FUNCTION TGuiInterface.dll+00010a90
TGUIComponentInfo* TGUIManager::FindComponentClass( const TPString8& a_rcName )
{
	for ( auto it = ms_oComponentClassList.Begin(); it != ms_oComponentClassList.End(); it++ )
	{
		if ( it->GetComponentName() == a_rcName )
			return it;
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+00011440
void TGUIManager::AddLayoutClass( TGUILayoutInfo* a_pLayoutInfo )
{
	ms_oLayoutClassList.InsertHead( a_pLayoutInfo );
}

// $JPOG: FUNCTION TGuiInterface.dll+00010a50
TGUILayoutInfo* TGUIManager::FindLayoutClass( const TPString8& a_rcName )
{
	for ( auto it = ms_oLayoutClassList.Begin(); it != ms_oLayoutClassList.End(); it++ )
	{
		if ( it->GetLayoutName() == a_rcName )
			return it;
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+00010bf0
TGUIStyle* TGUIManager::NewStyleInstance( const TGUIWidgetInfo* a_pWidgetInfo, const TPString8& a_rcName, const TClass* a_pClass )
{
	if ( FindStyleInstance( a_rcName ) )
		return TNULL;

	TObject* pObject = a_pClass->CreateObject();

	if ( !pObject )
		return TNULL;

	if ( !pObject->IsA( &TGetClass( TGUIStyle ) ) )
	{
		pObject->Delete();
		return TNULL;
	}

	TGUIStyle* pStyle = TSTATICCAST( TGUIStyle, pObject );
	pStyle->Create( a_pWidgetInfo, a_rcName );
	AddStyleInstance( pStyle );

	return pStyle;
}

// NOTE: JPOG also sets the interface to the widget
// $NicktoonsPS2: FUNCTION 003660d0
// $JPOG: FUNCTION TGuiInterface.dll+00010c70
void TGUIManager::AddTopLevelInstance( TGUIWidget* a_pWidget, TSumOfMinterms& a_rSkuFunction )
{
	TopLevelInfo* pInfo = new TopLevelInfo();
	pInfo->pWidget      = a_pWidget;
	pInfo->oSkuFunction = a_rSkuFunction;

	m_oTopLevelInstances.InsertTail( pInfo );
}

// $JPOG: FUNCTION TGuiInterface.dll+00010e20
void TGUIManager::DeleteTopLevelInstance( TGUIWidget* a_pWidget, const TSumOfMinterms& a_rcSkuFunction )
{
	for ( auto it = m_oTopLevelInstances.Begin(); it != m_oTopLevelInstances.End(); it++ )
	{
		if ( it->pWidget != a_pWidget || it->oSkuFunction.Size() != a_rcSkuFunction.Size() )
			continue;

		TBOOL bSame = TTRUE;

		for ( TINT i = 0; i < a_rcSkuFunction.Size(); i++ )
		{
			if ( !( it->oSkuFunction[ i ] == a_rcSkuFunction[ i ] ) )
			{
				bSame = TFALSE;
				break;
			}
		}

		if ( !bSame )
			continue;

		TopLevelInfo* pInfo = it;
		m_oTopLevelInstances.Remove( pInfo );

		if ( pInfo->pWidget )
			delete pInfo->pWidget;

		delete pInfo;
		return;
	}
}

// The widgets are deleted first, the instances then
// $NicktoonsPS2: FUNCTION 003663c8
// $JPOG: FUNCTION TGuiInterface.dll+00011050
void TGUIManager::DeleteAllTopLevelInstances()
{
	for ( auto it = m_oTopLevelInstances.Begin(); it != m_oTopLevelInstances.End(); it++ )
	{
		if ( it->pWidget )
			delete it->pWidget;
	}

	m_oTopLevelInstances.DeleteAll();
}

// The first top-level widget named a_rcName whose SKU condition is true for a_uiSkus
// $JPOG: FUNCTION TGuiInterface.dll+00010b00
TGUIWidget* TGUIManager::FindTopLevelInstance( const TPString8& a_rcName, TUINT a_uiSkus ) const
{
	for ( auto it = m_oTopLevelInstances.Begin(); it != m_oTopLevelInstances.End(); it++ )
	{
		if ( it->pWidget->GetName() == a_rcName && it->oSkuFunction.GetTruth( a_uiSkus ) )
			return it->pWidget;
	}

	return TNULL;
}

// The data of the callbacks of FindStyleReferences and FindTextureSectionReferences
struct FindReferencesData
{
	void*          pList;
	void*          pObject;
	TSumOfMinterms oSkuFunction;
};

// Adds the widgets using the style to the list, with the SKU condition of their top-level instance
// $NicktoonsPS2: FUNCTION 00366660
static TBOOL __stdcall FindStyleReferencesCallback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	FindReferencesData* pData = TSTATICCAST( FindReferencesData, a_pUserData );

	if ( a_pWidget->GetStyle() == pData->pObject )
	{
		TGUIManager::TopLevelInfo* pInfo = new TGUIManager::TopLevelInfo();
		pInfo->pWidget                   = a_pWidget;
		pInfo->oSkuFunction              = pData->oSkuFunction;

		TSTATICCAST( TNodeList<TGUIManager::TopLevelInfo>, pData->pList )->InsertTail( pInfo );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 003667b8
// $JPOG: FUNCTION TGuiInterface.dll+00011190
TNodeList<TGUIManager::TopLevelInfo>* TGUIManager::FindStyleReferences( TGUIStyle* a_pStyle )
{
	TNodeList<TopLevelInfo>* pList = new TNodeList<TopLevelInfo>();

	FindReferencesData oData;
	oData.pList   = pList;
	oData.pObject = a_pStyle;

	for ( auto it = m_oTopLevelInstances.Begin(); it != m_oTopLevelInstances.End(); it++ )
	{
		oData.oSkuFunction = it->oSkuFunction;
		it->pWidget->RecurseTree( TNULL, TNULL, FindStyleReferencesCallback, &oData );
	}

	return pList;
}

// Adds the widgets whose parameters reference the texture section to the list
// $JPOG: FUNCTION TGuiInterface.dll+00010000
static TBOOL __stdcall FindTextureSectionReferencesCallback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	FindReferencesData* pData = TSTATICCAST( FindReferencesData, a_pUserData );

	if ( a_pWidget->IsReferencingTextureSection( TSTATICCAST( TGUITextureSection, pData->pObject ) ) )
	{
		TGUIManager::ObjectReference* pReference = new TGUIManager::ObjectReference();
		pReference->pObject                      = a_pWidget;

		TSTATICCAST( TNodeList<TGUIManager::ObjectReference>, pData->pList )->InsertTail( pReference );
	}

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00011260
TNodeList<TGUIManager::ObjectReference>* TGUIManager::FindTextureSectionReferences( TGUITextureSection* a_pTextureSection )
{
	TNodeList<ObjectReference>* pList = new TNodeList<ObjectReference>();

	for ( auto it = m_oStyles.Begin(); it != m_oStyles.End(); it++ )
	{
		if ( it->IsReferencingTextureSection( a_pTextureSection ) == TTRUE )
		{
			ObjectReference* pReference = new ObjectReference();
			pReference->pObject         = it;
			pList->InsertTail( pReference );
		}
	}

	FindReferencesData oData;
	oData.pList   = pList;
	oData.pObject = a_pTextureSection;

	for ( auto it = m_oTopLevelInstances.Begin(); it != m_oTopLevelInstances.End(); it++ )
		it->pWidget->RecurseTree( TNULL, TNULL, FindTextureSectionReferencesCallback, &oData );

	return pList;
}

// $JPOG: FUNCTION TGuiInterface.dll+00011360
TNodeList<TGUIManager::ObjectReference>* TGUIManager::FindFontReferences( TGUIFont* a_pFont )
{
	TNodeList<ObjectReference>* pList = new TNodeList<ObjectReference>();

	for ( auto it = m_oStyles.Begin(); it != m_oStyles.End(); it++ )
	{
		if ( it->IsReferencingFont( a_pFont ) )
		{
			ObjectReference* pReference = new ObjectReference();
			pReference->pObject         = it;
			pList->InsertTail( pReference );
		}
	}

	return pList;
}

// NOTE: JPOG also sets the interface to the screen
// $NicktoonsPS2: FUNCTION 00366218
// $JPOG: FUNCTION TGuiInterface.dll+00010ce0
void TGUIManager::AddScreenInstance( TGUIScreen* a_pScreen )
{
	a_pScreen->SetState( TGUIWidget::State_Visible, TGUIWidget::State_Visible );
	m_oScreens.InsertTail( a_pScreen );
}

// NOTE: JPOG also clears the interface of the screen
// $JPOG: FUNCTION TGuiInterface.dll+00010ef0
void TGUIManager::DeleteScreenInstance( TGUIScreen* a_pScreen )
{
	for ( auto it = m_oScreens.Begin(); it != m_oScreens.End(); it++ )
	{
		if ( it.Get() == a_pScreen )
		{
			m_oScreens.Remove( a_pScreen );
			return;
		}
	}
}

// NOTE: JPOG clears the interfaces of the screens first
// $NicktoonsPS2: FUNCTION 003664b8
// $JPOG: FUNCTION TGuiInterface.dll+000110e0
void TGUIManager::DeleteAllScreenInstances()
{
	while ( !m_oScreens.IsEmpty() )
	{
		TGUIWidget* pScreen = m_oScreens.Head();
		m_oScreens.Remove( pScreen );

		if ( pScreen )
			delete pScreen;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+00010d80
void TGUIManager::AddSku( const TPString8& a_rcSku )
{
	for ( TINT i = 0; i < m_aSkus.Size(); i++ )
	{
		if ( m_aSkus[ i ] == a_rcSku )
			return;
	}

	m_aSkus.Push( a_rcSku );
}

// $JPOG: FUNCTION TGuiInterface.dll+00010fb0
void TGUIManager::DeleteSku( const TPString8& a_rcSku )
{
	TINT iIndex = -1;

	for ( TINT i = 0; i < m_aSkus.Size(); i++ )
	{
		if ( m_aSkus[ i ] == a_rcSku )
		{
			iIndex = i;
			break;
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// The original removes the SKU only if it isn't in the array, shifting the array from the index -1
	if ( iIndex != -1 )
	{
		for ( TINT j = iIndex; j < m_aSkus.Size() - 1; j++ )
			m_aSkus[ j ] = m_aSkus[ j + 1 ];

		m_aSkus[ m_aSkus.Size() - 1 ] = TPString8();
		m_aSkus.SetSize( m_aSkus.Size() - 1 );
	}
#else
	if ( iIndex == -1 )
	{
		for ( TINT j = iIndex; j < m_aSkus.Size() - 1; j++ )
			m_aSkus[ j ] = m_aSkus[ j + 1 ];

		m_aSkus.SetSize( m_aSkus.Size() - 1 );
	}
#endif
}

// $JPOG: FUNCTION TGuiInterface.dll+00011b60
void TGUIManager::DeleteAllSkus()
{
	for ( TINT i = 0; i < m_aSkus.Size(); i++ )
		m_aSkus[ i ].~TPString8();

	m_aSkus.FreeData();
}

// $JPOG: FUNCTION TGuiInterface.dll+000109a0
TBOOL TGUIManager::SaveDefinitionFile( const TString8& a_rcFileName )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// The writer is only created in JPOG
	if ( !m_pWriter )
		return TFALSE;
#endif

	if ( !m_pWriter->Open( a_rcFileName ) )
		return TFALSE;

	TBOOL bResult = m_pWriter->Save();
	m_pWriter->Close();

	return bResult;
}

TOSHI_NAMESPACE_END
