#include "pch.h"
#include "ARecipeDisplayState.h"
#include "AGameStateController.h"
#include "ALoadScreen.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Assets/AAssetLoader.h"
#include "GUI/AGUI2.h"
#include "Helpers/AHintManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Tasks/ARootTask.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AObjectTraits.h"
#include "Units/AUnitFactory.h"
#include "Units/AUnitManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Render/TCameraObject.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0044a0c0, CreateObject 0044b780, CreateObjectInPlace 0044b7e0 (the constructor is inlined in both)
TDEFINE_CLASS( ARecipeDisplayState );

TPSTRING8_DECLARE( DialogInputMap );

// cooking.AllIngredients
struct IngredientData
{
	const TCHAR* szName;   // 0x00, the unit type
	TINT         iCentred; // 0x04, 1 if the model is centred on its bounding sphere
	TFLOAT       fScale;   // 0x08
	TINT         iUnk24;   // 0x0c
	TINT         aUnk[ 3 ];
};

// cooking.AllRecipes
struct RecipeData
{
	const TCHAR* szName;
	const TCHAR* aszIngredients[ ACookingRecipe::MAX_INGREDIENTS ];
};

// RecipeEntryData (cooking.AllRecipes) is in MiniGame/ACookingRecipe.h

TSTATICASSERT( sizeof( IngredientData ) == 0x1c );

// $Barnyard: FUNCTION 0044a5e0
ARecipeDisplayState::ARecipeDisplayState()
    : m_eInput( INPUT_NONE )
    , m_iUnk2268( 0 )
    , m_iUnk226C( 0 )
    , m_eState( STATE_LOADING )
    , m_fLoadTime( 0.0f )
{
	SetInputMap( TPS8( DialogInputMap ) );
	m_bCocktails     = TFALSE;
	m_pRecipesFound  = TNULL;
	m_fTime          = 0.0f;
	m_eInput         = INPUT_NONE;
	m_iRecipe        = -1;
	m_bCollectRecipe = TTRUE;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: CreateGUI measures the page number before it's formatted
	m_wszPageNumber[ 0 ] = L'\0';
#endif
}

// $Barnyard: FUNCTION 0044a880
ARecipeDisplayState::ARecipeDisplayState( TBOOL a_bCocktails )
    : m_eInput( INPUT_NONE )
    , m_iUnk2268( 0 )
    , m_iUnk226C( 0 )
    , m_eState( STATE_LOADING )
    , m_fLoadTime( 0.0f )
{
	SetInputMap( TPS8( DialogInputMap ) );
	m_bCocktails     = a_bCocktails;
	m_bCollectRecipe = TFALSE;
	m_pRecipesFound  = TNULL;
	m_fTime          = 0.0f;
	m_eInput         = INPUT_NONE;
	m_iRecipe        = -1;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: CreateGUI measures the page number before it's formatted
	m_wszPageNumber[ 0 ] = L'\0';
#endif
}

// $Barnyard: FUNCTION 0044a110
// $Barnyard: FUNCTION 0044a860
ARecipeDisplayState::~ARecipeDisplayState()
{
}

// $Barnyard: FUNCTION 0044a320
TBOOL ARecipeDisplayState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	// Eat the input while loading
	if ( m_eState != STATE_SHOWING )
		return TTRUE;

	if ( a_eInputCommand == AInputCommand_Move )
	{
		// The analog stick turns the pages
		if ( m_fTime > 1.0f )
		{
			TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
			TFLOAT fY = a_pInputEvent->GetMagnitudeFloat( 1 );

			if ( fX < -0.5f )
				m_eInput |= INPUT_PREVIOUS;
			else if ( fX > 0.5f )
				m_eInput |= INPUT_NEXT;

			if ( fY < -0.5f )
				m_eInput |= INPUT_UP;
			else if ( fY > 0.5f )
				m_eInput |= INPUT_DOWN;

			if ( fX < -0.5f || fX > 0.5f || fY > 0.5f || fY < -0.5f )
			{
				m_fTime = 0.0f;
				return TTRUE;
			}
		}

		return TFALSE;
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN && a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_REPEAT )
		return TFALSE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_OK:
		case AInputCommand_Shoot:
			m_eInput |= INPUT_OK;
			return TTRUE;
		case AInputCommand_Cancel:
			m_eInput |= INPUT_CANCEL;
			return TTRUE;
		case AInputCommand_Up:
		case AInputCommand_Forward:
			m_eInput |= INPUT_UP;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Down:
		case AInputCommand_Backward:
			m_eInput |= INPUT_DOWN;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Left:
			m_eInput |= INPUT_PREVIOUS;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Right:
			m_eInput |= INPUT_NEXT;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		default:
			return TFALSE;
	}
}

// $Barnyard: FUNCTION 0044c500
TBOOL ARecipeDisplayState::OnUpdate( TFLOAT a_fDeltaTime )
{
	switch ( m_eState )
	{
		case STATE_START:
			g_oLoadScreen.Reset();
			g_oLoadScreen.StartLoading( 0, TTRUE );
			m_eState = STATE_LOADING;
			return TTRUE;
		case STATE_LOADING:
			m_fLoadTime += a_fDeltaTime;

			if ( ATerrainInterface::GetSingleton()->IsLoaded() && m_fLoadTime > 1.0f )
				m_eState = STATE_LOADED;
			else
				g_oLoadScreen.Update( 1.0f, TTRUE );

			return TTRUE;
		case STATE_LOADED:
			AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );
			g_oLoadScreen.Reset();
			// fallthrough
		case STATE_CREATE:
			m_eState = STATE_SHOWING;
			Create();
			m_oBackground.Show();
			return TTRUE;
	}

	m_fTime += a_fDeltaTime;

	if ( m_eInput & INPUT_PREVIOUS )
	{
		if ( !m_bCollectRecipe )
		{
			// The page before the first recipe is the progress page
			m_iRecipe = ( m_iRecipe > -1 ) ? GetPreviousRecipe() : GetLastRecipe();
			UpdatePage();
		}
	}
	else if ( m_eInput & INPUT_NEXT )
	{
		if ( !m_bCollectRecipe )
		{
			m_iRecipe = ( m_iRecipe > -1 ) ? GetNextRecipe() : GetFirstRecipe();
			UpdatePage();
		}
	}
	else if ( m_eInput & ( INPUT_OK | INPUT_CANCEL ) )
	{
		Remove();
		return TTRUE;
	}

	m_eInput = INPUT_NONE;
	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0044a310
void ARecipeDisplayState::OnInsertion()
{
	m_eState = STATE_CREATE;
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0044bf60
void ARecipeDisplayState::OnRemoval()
{
	m_oFade.RemoveSelf();

	for ( TINT i = ACookingRecipe::MAX_INGREDIENTS - 1; i >= 0; i-- )
	{
		m_aIngredientModels[ i ].RemoveAllObjects();
		m_aIngredientModels[ i ].RemoveSelf();
		m_aIngredientNames[ i ].RemoveSelf();
		m_aPlusSigns[ i ].RemoveSelf();
		m_aIngredientAreas[ i ].RemoveSelf();
	}

	m_oRecipeName.RemoveSelf();
	m_oTitle.RemoveSelf();
	m_oPrompt.RemoveSelf();
	m_oRightArrow.RemoveSelf();
	m_oPageLabel.RemoveSelf();
	m_oPageNumber.RemoveSelf();
	m_oPageNumberArea.RemoveSelf();
	m_oLeftArrow.RemoveSelf();
	m_oProgressTitle.RemoveSelf();
	m_oProgress.RemoveSelf();
	m_oProgressArea.RemoveSelf();
	m_oNavigation.RemoveSelf();
	m_oRecipeNameArea.RemoveSelf();
	m_oIngredientsArea.RemoveSelf();
	m_oPage.RemoveSelf();
	m_oBackground.RemoveSelf();

	for ( TINT i = 0; i < m_oRecipes.Size(); i++ )
		m_oRecipes[ i ].Destroy();

	m_oRecipes.Clear();

	for ( TINT i = 0; i < m_oIngredients.Size(); i++ )
		m_oIngredients[ i ].Destroy();

	m_oIngredients.Clear();

	ARootTask::GetSingleton()->SetPaused( TFALSE );
	AGameState::OnRemoval();
}

// NOTE: 004490f0, shared with AHintDisplayState::OnActivate
void ARecipeDisplayState::OnActivate()
{
	m_oBackground.T2GUIElement::Show();
	SetInputMap( TPS8( DialogInputMap ) );
	AGameState::OnActivate();
}

// NOTE: 00449110, shared with AHintDisplayState::OnDeactivate
void ARecipeDisplayState::OnDeactivate()
{
	m_oBackground.T2GUIElement::Hide();
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 0044c410
void ARecipeDisplayState::Create()
{
	// Darken the screen
	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	T2GUIElement* pRoot = AGUI2::GetRootElement();
	m_oFade.SetDimensions( fWidth, fHeight );
	m_oFade.SetColour( 0x7F000000 );
	pRoot->AddChildTail( m_oFade );

	LoadRecipes();
	m_pRecipesFound = AHintManager::GetRecipes();

	if ( m_bCollectRecipe )
	{
		m_iRecipe = AHintManager::GetSingleton()->AddRecipe();

		if ( m_iRecipe < 0 || m_iRecipe >= m_oRecipes.Size() )
			m_bCollectRecipe = TFALSE;
		else
			m_bCocktails = m_oRecipes[ m_iRecipe ].IsCocktail();
	}
	else
	{
		m_iRecipe = GetFirstRecipe();
		ARootTask::GetSingleton()->SetPaused( TTRUE );
	}

	CreateGUI();

	if ( m_bCollectRecipe )
		m_oTitle.T2GUIElement::Hide();

	UpdatePage();
}

// $Barnyard: FUNCTION 0044c180
void ARecipeDisplayState::LoadRecipes()
{
	AAssetLoader::GetSymbolAddress( "cooking", "MaxIngredients", AAssetType_AssetPack );
	TINT*           pNumIngredients = TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "cooking", "NumberOfIngredients", AAssetType_AssetPack ) );
	IngredientData* pIngredients    = TSTATICCAST( IngredientData, AAssetLoader::GetSymbolAddress( "cooking", "AllIngredients", AAssetType_AssetPack ) );

	for ( TINT i = 0; i < *pNumIngredients; i++ )
	{
		ACookingIngredient oIngredient;
		oIngredient.Create( pIngredients[ i ].szName, pIngredients[ i ].fScale, pIngredients[ i ].iCentred == 1 );
		oIngredient.SetUnk24( pIngredients[ i ].iUnk24 );
		AUnitManager::GetSingleton()->GetUnitFactory()->Find( oIngredient.GetName() );

		m_oIngredients.PushBack( oIngredient );
	}

	TINT*            pNumRecipes = TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "cooking", "NumberOfRecipes", AAssetType_AssetPack ) );
	RecipeEntryData* pRecipes    = TSTATICCAST( RecipeEntryData, AAssetLoader::GetSymbolAddress( "cooking", "AllRecipes", AAssetType_AssetPack ) );

	for ( TINT i = 0; i < *pNumRecipes; i++ )
	{
		ACookingRecipe oRecipe;
		oRecipe.Create( pRecipes[ i ].iNameString, pRecipes[ i ].szName );
		oRecipe.SetPrice( pRecipes[ i ].iPrice );
		oRecipe.SetQuest( pRecipes[ i ].szQuest );
		oRecipe.SetType( pRecipes[ i ].iType );

		for ( TINT k = 0; k < ACookingRecipe::MAX_INGREDIENTS; k++ )
		{
			if ( !pRecipes[ i ].aszIngredients[ k ] )
				break;

			oRecipe.AddIngredient( FindIngredient( pRecipes[ i ].aszIngredients[ k ] ) );
		}

		m_oRecipes.PushBack( oRecipe );
	}
}

// $Barnyard: FUNCTION 0044b4e0
ACookingIngredient* ARecipeDisplayState::FindIngredient( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_oIngredients.Size(); i++ )
	{
		if ( m_oIngredients[ i ].GetName().GetString8().Compare( a_szName ) == 0 )
			return &m_oIngredients[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0044b820
void ARecipeDisplayState::PlaceIngredient( ACookingIngredient* a_pIngredient, TBOOL a_bSetPosition, TVector4 a_vPosition, TVector4 a_vRotation, TFLOAT a_fScale )
{
	AAnimatableObject* pObject = a_pIngredient->GetObject();
	AObjectTraits*     pTraits = a_pIngredient->GetTraits();

	TMatrix44 matTransform = TMatrix44::IDENTITY;
	matTransform.RotateX( TMath::HALF_PI );
	matTransform.Rotate( a_vRotation.x, a_vRotation.y, a_vRotation.z );
	matTransform.Rotate( pTraits->m_vecHoldRotate.x, pTraits->m_vecHoldRotate.y, pTraits->m_vecHoldRotate.z );

	TFLOAT fScale = pTraits->m_fItemScale * a_pIngredient->GetScale() * a_fScale;
	matTransform.Scale( fScale, fScale, fScale );

	if ( a_bSetPosition )
		matTransform.GetTranslation() = a_vPosition;

	if ( a_pIngredient->IsCentred() )
	{
		// Move the centre of the model to the position
		const TVector3& rcOrigin = pObject->GetModelInstance()->GetSceneObject()->GetInstance()->GetModel()->m_LODs[ 0 ].BoundingSphere.GetOrigin();
		TVector3        vecCentre( -rcOrigin.x, -rcOrigin.y, -rcOrigin.z );

		TMatrix44::TransformVector( matTransform.GetTranslation3(), matTransform, vecCentre );
	}

	pObject->GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 0044b550
TINT ARecipeDisplayState::GetNumFoundRecipes()
{
	TINT iNumFound = 0;

	for ( TINT i = m_oRecipes.Size(); i != 0; i-- )
	{
		if ( IsShownAndFound( i - 1 ) )
			iNumFound++;
	}

	return iNumFound;
}

// $Barnyard: FUNCTION 0044b5a0
TINT ARecipeDisplayState::GetNumRecipes()
{
	TINT iNumRecipes = 0;

	for ( TINT i = m_oRecipes.Size(); i != 0; i-- )
	{
		if ( IsShownType( i - 1 ) )
			iNumRecipes++;
	}

	return iNumRecipes;
}

// $Barnyard: FUNCTION 0044b5e0
TINT ARecipeDisplayState::GetNextRecipe()
{
	for ( TINT i = m_iRecipe + 1; i < m_oRecipes.Size(); i++ )
	{
		if ( IsShownAndFound( i ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 0044b640
TINT ARecipeDisplayState::GetPreviousRecipe()
{
	for ( TINT i = m_iRecipe - 1; i >= 0; i-- )
	{
		if ( IsShownAndFound( i ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 0044b690
TINT ARecipeDisplayState::GetLastRecipe()
{
	for ( TINT i = m_oRecipes.Size() - 1; i >= 0; i-- )
	{
		if ( IsShownAndFound( i ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 0044b6e0
TINT ARecipeDisplayState::GetFirstRecipe()
{
	for ( TINT i = 0; i < m_oRecipes.Size(); i++ )
	{
		if ( IsShownAndFound( i ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 0044b730
TINT ARecipeDisplayState::GetPageNumber()
{
	TINT iPage = 0;

	for ( TINT i = 0; i <= m_iRecipe && i < m_oRecipes.Size(); i++ )
	{
		if ( IsShownType( i ) )
			iPage++;
	}

	return iPage;
}

// $Barnyard: FUNCTION 0044ab00
void ARecipeDisplayState::CreateGUI()
{
	T2GUIElement* pRoot         = AGUI2::GetRootElement();
	TFLOAT        fScreenWidth  = pRoot->GetWidth() * 0.85f;
	TFLOAT        fScreenHeight = pRoot->GetHeight() * 0.85f;

	m_oBackground.SetDimensions( fScreenWidth, fScreenHeight );
	m_oBackground.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	// The title at the top
	TFLOAT fTextWidth = fScreenWidth * 0.95f;
	m_oTitle.Create( T2GUIFontManager::FindFont( "Rekord26" ), fTextWidth );
	m_oTitle.SetText( ALocaleManager::Interface()->GetString( m_bCocktails ? 1782 : 1775 ) );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitle.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oTitle.SetScale( 1.5f );
	m_oBackground.AddChildTail( m_oTitle );

	// What to press at the bottom
	T2GUIFont* pFont18 = T2GUIFontManager::FindFont( "Rekord18" );
	ALocale::FormatInputTags( m_wszPrompt, TARRAYSIZE( m_wszPrompt ), ALocaleManager::Interface()->GetString( 1776 ) );
	m_oPrompt.Create( pFont18, fTextWidth );
	m_oPrompt.SetText( m_wszPrompt );
	m_oPrompt.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oPrompt.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oBackground.AddChildTail( m_oPrompt );

	// The page of the cookbook in the middle
	T2GUITextureSection* pPage = T2GUITextureSectionManager::GetTextureSection( "Recipie_Page" );
	m_oPage.SetDimensions( fScreenWidth * 0.75f, fScreenHeight * 0.75f );
	m_oPage.SetTextureSection( pPage );
	m_oPage.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oPage.SetAlpha( 1.0f );
	m_oBackground.AddChildTail( m_oPage );

	// The ingredients: "A + B + C" with their models
	pFont18                 = T2GUIFontManager::FindFont( "Rekord18" );
	T2GUIFont* pFont26      = T2GUIFontManager::FindFont( "Rekord26" );
	TFLOAT     fPlusWidth   = pFont26->GetTextWidth( L" + ", 1.0f );
	TFLOAT     fAreaWidth   = fScreenWidth * ( 0.75f * 0.9f );
	TFLOAT     fItemWidth   = ( fAreaWidth - ( fPlusWidth + fPlusWidth ) ) * ( 1.0f / 3.0f );
	TFLOAT     fItemHeight  = pFont18->GetLineHeight() + fItemWidth + 6.0f;
	TFLOAT     fItemSpacing = fItemWidth + fPlusWidth;

	m_oIngredientsArea.SetDimensions( fAreaWidth, fItemHeight );
	m_oIngredientsArea.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );

	for ( TINT i = 0; i < ACookingRecipe::MAX_INGREDIENTS; i++ )
	{
		m_aIngredientAreas[ i ].SetDimensions( fItemWidth, fItemHeight );

		m_aIngredientModels[ i ].Create( fItemWidth, fItemWidth, TFALSE, TTRUE );
		TCameraObject* pCamera = m_aIngredientModels[ i ].GetCamera();
		pCamera->SetFOV( TMath::PI / 3.0f );
		pCamera->SetNear( 0.001f );
		pCamera->SetFar( 10.0f );

		TMatrix44 matCamera = TMatrix44::IDENTITY;
		matCamera.GetTranslation().Set( 0.0f, 0.0f, -0.75f, 1.0f );
		m_aIngredientModels[ i ].SetCameraMatrix( matCamera );

		m_aIngredientNames[ i ].Create( pFont18, fItemSpacing );
		m_aIngredientNames[ i ].SetText( L"? ? ?" );
		m_aIngredientNames[ i ].SetColour( 0xFFFFFFDD );

		m_aPlusSigns[ i ].Create( pFont26, fPlusWidth );
		m_aPlusSigns[ i ].SetText( L"+" );
		m_aPlusSigns[ i ].SetTextAlign( T2GUIFont::TextAlign_Center );
	}

	for ( TINT i = 0; i < ACookingRecipe::MAX_INGREDIENTS; i++ )
	{
		m_aIngredientAreas[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_aIngredientModels[ i ].SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
		m_aIngredientNames[ i ].SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		m_aIngredientAreas[ i ].AddChildTail( m_aIngredientModels[ i ] );
		m_aIngredientAreas[ i ].AddChildTail( m_aIngredientNames[ i ] );
		m_aIngredientAreas[ i ].SetColour( 0x66777722 );
		m_aIngredientAreas[ i ].SetAlpha( 0.0f );

		TFLOAT fX = i * fItemSpacing;
		m_aIngredientAreas[ i ].SetTransform( fX, 0.0f, 0.0f );
		m_oIngredientsArea.AddChildTail( m_aIngredientAreas[ i ] );

		m_aPlusSigns[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_aPlusSigns[ i ].SetTransform( fX + fItemWidth, fItemWidth * 0.5f, 0.0f );
		m_oIngredientsArea.AddChildTail( m_aPlusSigns[ i ] );
	}

	// No "+" after the last ingredient
	m_aPlusSigns[ ACookingRecipe::MAX_INGREDIENTS - 1 ].SetAlpha( 0.0f );

	m_oIngredientsArea.SetTransform( 0.0f, -24.0f, 0.0f );
	m_oIngredientsArea.SetAlpha( 0.0f );
	m_oPage.AddChildTail( m_oIngredientsArea );

	// The navigation: "< Recipe n/m >"
	pFont26                     = T2GUIFontManager::FindFont( "Rekord26" );
	T2GUITextureSection* pArrow = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );

	m_oLeftArrow.SetTextureSection( *pArrow );
	m_oLeftArrow.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

	m_oPageLabel.Create( pFont26, pFont26->GetTextWidth( m_wszPageNumber, 1.0f ) );
	m_oPageLabel.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( m_bCocktails ? 1781 : 1780 ) );
	m_oPageLabel.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oPageLabel.SetColour( 0xFFFFFFDD );

	TStringManager::String16Format( m_wszPageNumber, TARRAYSIZE( m_wszPageNumber ), L"%i/%i", 99, 999 );
	m_oPageNumber.Create( pFont26, pFont26->GetTextWidth( m_wszPageNumber, 1.0f ) );
	m_oPageNumber.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oPageNumber.SetText( m_wszPageNumber );
	m_oPageNumber.SetColour( 0xFFFFC800 );

	m_oRightArrow.SetTextureSection( *pArrow );
	m_oRightArrow.FlipU();
	m_oRightArrow.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );

	TFLOAT fArrowHeight = pArrow->GetHeight();
	TFLOAT fHeight      = ( pFont26->GetLineHeight() >= fArrowHeight ) ? pFont26->GetLineHeight() : fArrowHeight;

	TFLOAT fPageNumberWidth = m_oPageNumber.GetWidth();
	m_oPageNumberArea.SetDimensions( m_oPageLabel.GetWidth() + fPageNumberWidth, fHeight );
	m_oPageNumberArea.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oPageNumberArea.AddChildTail( m_oPageLabel );
	m_oPageNumberArea.AddChildTail( m_oPageNumber );
	m_oPageNumberArea.SetTranslation( 0.0f, 9.0f );
	m_oPageNumberArea.SetAlpha( 0.0f );

	m_oNavigation.SetDimensions( m_oPageNumberArea.GetWidth() + pArrow->GetWidth() * 4.0f, fHeight );
	m_oNavigation.AddChildTail( m_oLeftArrow );
	m_oNavigation.AddChildTail( m_oPageNumberArea );
	m_oNavigation.AddChildTail( m_oRightArrow );
	m_oNavigation.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oNavigation.SetTransform( 0.0f, 24.0f, 0.0f );
	m_oNavigation.SetAlpha( 0.0f );
	m_oPage.AddChildTail( m_oNavigation );

	// The name of the recipe
	pFont26 = T2GUIFontManager::FindFont( "Rekord26" );

	m_oRecipeNameArea.SetDimensions( fTextWidth, fScreenHeight * 0.1875f );
	m_oRecipeNameArea.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );

	m_oRecipeName.Create( pFont26, fTextWidth );
	m_oRecipeName.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oRecipeName.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oRecipeNameArea.AddChildTail( m_oRecipeName );
	m_oRecipeNameArea.SetTransform( 0.0f, 6.0f, 0.0f );
	m_oRecipeNameArea.SetAlpha( 0.0f );
	m_oNavigation.AddChildTail( m_oRecipeNameArea );

	// The progress page
	T2GUIFontManager::FindFont( "Rekord18" );
	pFont26 = T2GUIFontManager::FindFont( "Rekord26" );

	TStringManager::String16Format( m_wszUnk222E, TARRAYSIZE( m_wszUnk222E ), L"%i", 9999 );
	TStringManager::String16Format( m_wszUnk223E, TARRAYSIZE( m_wszUnk223E ), L"%i", 9999 );
	TStringManager::String16Format( m_wszProgress, TARRAYSIZE( m_wszProgress ), L"%i%%", 1000 );

	m_oProgressArea.SetDimensions( fScreenWidth * 0.6f, ( m_oPage.GetHeight() - m_oNavigation.GetHeight() ) * 0.75f );
	m_oProgressArea.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oProgressArea.SetTransform( 0.0f, -24.0f, 0.0f );
	m_oProgressArea.SetAlpha( 0.0f );
	m_oPage.AddChildTail( m_oProgressArea );

	m_oProgressTitle.Create( pFont26, m_oProgressArea.GetWidth() );
	m_oProgressTitle.SetText( ALocaleManager::Interface()->GetString( 1778 ) );
	m_oProgressTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oProgressTitle.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oProgressTitle.SetColour( 0xFFFFD800 );
	m_oProgressTitle.SetShadowEnabled( TTRUE );
	m_oProgressTitle.SetTranslation( 0.0f, 10.0f );
	m_oProgressArea.AddChildTail( m_oProgressTitle );

	m_oProgress.Create( pFont26, m_oProgressArea.GetWidth() );
	m_oProgress.SetText( m_wszProgress );
	m_oProgress.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oProgress.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oProgress.SetColour( 0xFFFFFFFF );
	m_oProgressArea.AddChildTail( m_oProgress );

	m_oBackground.SetAlpha( 0.0f );

	// The cocktails have a pink page
	if ( m_bCocktails )
		m_oPage.SetColour( 0xFFC7B6D8 );

	AGUI2::GetRootElement()->AddChildTail( m_oBackground );

	for ( TINT i = 0; i < ACookingRecipe::MAX_INGREDIENTS; i++ )
		m_aIngredientModels[ i ].UpdateViewport();

	m_oBackground.T2GUIElement::Hide();
}

// $Barnyard: FUNCTION 0044ba00
void ARecipeDisplayState::UpdatePage()
{
	TStringManager::String16Format( m_wszPageNumber, TARRAYSIZE( m_wszPageNumber ), L"%i/%i", GetPageNumber(), GetNumRecipes() );

	// No paging while collecting a recipe
	m_oLeftArrow.SetVisible( !m_bCollectRecipe );
	m_oRightArrow.SetVisible( !m_bCollectRecipe );

	m_oPageNumber.T2GUIElement::Show();
	m_oPageLabel.T2GUIElement::Show();

	if ( GetNumFoundRecipes() == 0 )
	{
		m_oLeftArrow.T2GUIElement::Hide();
		m_oRightArrow.T2GUIElement::Hide();
	}

	if ( m_iRecipe > -1 )
	{
		// Show the recipe and its ingredients
		m_oProgressArea.T2GUIElement::Hide();
		m_oIngredientsArea.T2GUIElement::Show();
		m_oRecipeNameArea.T2GUIElement::Show();

		for ( TINT i = ACookingRecipe::MAX_INGREDIENTS - 1; i >= 0; i-- )
			m_aIngredientModels[ i ].RemoveAllObjects();

		// The name, scaled down if it doesn't fit
		T2GUIFontManager::FindFont( "Rekord18" );
		T2GUIFont* pFont26 = T2GUIFontManager::FindFont( "Rekord26" );

		ACookingRecipe& rRecipe    = m_oRecipes[ m_iRecipe ];
		TFLOAT          fNameWidth = pFont26->GetTextWidth( ALocaleManager::Interface()->GetString( rRecipe.GetNameString() ), 1.0f );
		TFLOAT          fMaxWidth  = m_oPage.GetWidth() * 0.75f;

		if ( fNameWidth > fMaxWidth && fNameWidth > 0.0f )
		{
			m_oRecipeName.Create( pFont26, fMaxWidth );
			m_oRecipeName.SetScale( fMaxWidth / fNameWidth );
		}
		else
		{
			m_oRecipeName.SetScale( 1.0f );
			m_oRecipeName.Create( pFont26, fMaxWidth );
		}

		m_oRecipeName.SetText( ALocaleManager::Interface()->GetString( m_oRecipes[ m_iRecipe ].GetNameString() ) );

		TINT iNumIngredients = m_oRecipes[ m_iRecipe ].GetNumIngredients();

		for ( TINT i = 0; i < ACookingRecipe::MAX_INGREDIENTS; i++ )
		{
			if ( i < iNumIngredients )
			{
				ACookingIngredient* pIngredient = m_oRecipes[ m_iRecipe ].GetIngredient( i );
				PlaceIngredient( pIngredient, TFALSE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );

				m_aIngredientNames[ i ].SetText( pIngredient->GetScreenName() );
				m_aIngredientModels[ i ].AddObject( pIngredient->GetObject(), TNULL, TNULL );
			}
			else
			{
				m_aIngredientNames[ i ].SetText( L"- - -" );
			}
		}
	}
	else
	{
		// Show how many recipes were found
		TINT iPercentage = 0;

		if ( GetNumRecipes() > 0 )
			iPercentage = TINT( TFLOAT( GetNumFoundRecipes() ) * 100.0f / TFLOAT( GetNumRecipes() ) );

		TStringManager::String16Format( m_wszProgress, TARRAYSIZE( m_wszProgress ), L"%i", iPercentage );
		TStringManager::String16Copy( m_wszProgress + TStringManager::String16Length( m_wszProgress ), L"%" );

		TFLOAT fAreaWidth  = m_oProgressArea.GetWidth();
		TFLOAT fAreaHeight = m_oProgressArea.GetHeight() - m_oProgressTitle.GetHeight() - 15.0f;
		TFLOAT fMaxSize    = ( fAreaWidth >= fAreaHeight ) ? fAreaHeight : fAreaWidth;

		m_oProgress.SetScale( 1.0f );
		m_oProgress.SetTextAndFitWidth( m_wszProgress );

		TFLOAT fSize = m_oProgress.GetWidth();
		if ( m_oProgress.GetHeight() > fSize ) fSize = m_oProgress.GetHeight();

		if ( fSize > 0.0f )
		{
			// Make the number as big as the area
			TFLOAT fScale = fMaxSize / fSize;
			TMath::Clip( fScale, 0.01f, 1.5f );

			m_oProgress.SetScale( fScale );
			m_oProgress.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_MIDDLECENTER );
			m_oProgress.SetTranslation( 0.0f, fAreaHeight * -0.5f );
		}

		m_oProgressArea.T2GUIElement::Show();
		m_oIngredientsArea.T2GUIElement::Hide();
		m_oRecipeNameArea.T2GUIElement::Hide();
	}
}
