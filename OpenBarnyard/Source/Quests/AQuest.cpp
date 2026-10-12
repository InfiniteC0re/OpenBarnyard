#include "pch.h"
#include "AQuest.h"
#include "AQuestElement.h"
#include "Units/AAnimalPopulationManager.h"

#include <Toshi/TStringManager.h>
#include <Toshi/TString8.h>
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c9bf0, CreateObject 005cb220, CreateObjectInPlace 005cb280
TDEFINE_CLASS( AQuest );

// $Barnyard: FUNCTION 005caba0
AQuest::AQuest()
    : m_iTitle( -1 )
    , m_vecElements( 10, 0 )
    , m_fTimeLimit( 0.0f )
    , m_iDataSize( 0 )
    , m_bEndSeqBlocks( TTRUE )
    , m_uiFlags( FLAGS_NONE )
    , m_iSummary( -1 )
{
	m_oUnitNames.iUnk = 10;
}

// $Barnyard: FUNCTION 005cac20
// $Barnyard: FUNCTION 005cb2c0
AQuest::~AQuest()
{
	for ( TINT i = 0; i < m_vecElements.Size(); i++ )
	{
		if ( m_vecElements[ i ] )
			delete m_vecElements[ i ];
	}

	m_vecElements.Resize( 0 );
}

// $Barnyard: FUNCTION 005cada0
TBOOL AQuest::Create( const PBProperties* a_pProperties )
{
	const PBProperties* pElements = TNULL;

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const TCHAR*           szName = it->GetName().GetString();
		const PBPropertyValue* pValue = it->GetValue();

		switch ( szName[ 0 ] )
		{
			case 'e':
				if ( TStringManager::String8Compare( szName, "elements" ) == 0 )
					pElements = pValue->GetProperties();
				else if ( TStringManager::String8Compare( szName, "endseq" ) == 0 )
					m_strEndSeq = pValue->GetTPString8();
				else if ( TStringManager::String8Compare( szName, "endseqblocks" ) == 0 )
					m_bEndSeqBlocks = pValue->GetRaw() != TNULL;
				break;
			case 'n':
				// "name"
				m_strName = pValue->GetTPString8();
				break;
			case 'q':
				// "quest_giver"
				m_strQuestGiver = pValue->GetTPString8();
				break;
			case 's':
				// "startseq"
				m_strStartSeq = pValue->GetTPString8();
				break;
			case 't':
				if ( szName[ 2 ] == 't' )
				{
					// "title"
					m_iTitle = TINT( pValue->GetLocaleString().uiIndex );
				}
				else if ( szName[ 2 ] == 'm' )
				{
					// "timelimit"
					m_fTimeLimit = pValue->GetFloat();
				}
				break;
			case 'x':
				// "xsummary"
				m_iSummary = TINT( pValue->GetLocaleString().uiIndex );
				break;
		}
	}

	if ( pElements )
		CreateElements( m_vecElements, &m_iDataSize, pElements );

	// When the quest can be done depends on the spawn of the quest giver
	AAnimalPopulationManager::QuestTierSpawn* pSpawn = AAnimalPopulationManager::GetSingleton()->FindQuestTierSpawn( TNULL, m_strQuestGiver.GetString(), m_strName.GetString() );

	if ( pSpawn )
	{
		if ( pSpawn->oSpawn.uiFlags & AAnimalPopulationManager::SPAWNFLAGS_DAY )
			m_uiFlags |= FLAGS_DAY;
		else if ( pSpawn->oSpawn.uiFlags & AAnimalPopulationManager::SPAWNFLAGS_NIGHT )
			m_uiFlags |= FLAGS_NIGHT;

		if ( m_strName == TPString8( "Miles Welcome" ) )
			m_uiFlags = FLAGS_DAY | FLAGS_NIGHT;

		if ( pSpawn->oSpawn.uiFlags & AAnimalPopulationManager::SPAWNFLAGS_UNK2 )
			m_uiFlags |= FLAGS_UNK3;
		else if ( pSpawn->oSpawn.uiFlags & AAnimalPopulationManager::SPAWNFLAGS_UNK3 )
			m_uiFlags |= FLAGS_UNK2;
	}

	AAnimalPopulationManager::GetSingleton()->GetQuestUnitNames( m_strName.GetString(), m_oUnitNames );

	return TTRUE;
}

// $Barnyard: FUNCTION 005ca720
TBOOL AQuest::CreateElements( TArray<AQuestElement*>& a_rElements, TINT* a_pDataSize, const PBProperties* a_pProperties )
{
	for ( TUINT32 i = 0; i < a_pProperties->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pProperty = a_pProperties->GetPropertyByIndex( i );
		const PBProperties*             pElement  = pProperty->GetValue()->GetProperties();

		// The class of the element is named after the property ("dummy" -> ADummyQuestElement)
		TString8 strClassName;
		strClassName.Format( "a%squestelement", pProperty->GetName().GetString() );

		const TClass* pClass = TClass::Find( strClassName, &TGetClass( AQuestElement ) );

		if ( !pClass )
			return TFALSE;

		AQuestElement* pQuestElement = TSTATICCAST( AQuestElement, pClass->CreateObject() );

		if ( pQuestElement->Create( pElement, a_pDataSize ) )
			a_rElements.Push( pQuestElement );
		else
			delete pQuestElement;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c9c10
const TWCHAR* AQuest::GetTitle() const
{
	if ( m_iTitle == -1 )
		return TNULL;

	return T2Locale::GetSingleton()->GetString( m_iTitle );
}

// $Barnyard: FUNCTION 005ca220
TBOOL AQuest::HasUnit( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_oUnitNames.iSize; i++ )
	{
		if ( m_oUnitNames.pData[ i ] == a_rcName )
			return TTRUE;
	}

	return TFALSE;
}
