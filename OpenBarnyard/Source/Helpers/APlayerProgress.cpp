#include "pch.h"
#include "APlayerProgress.h"

#include <Toshi/TStringManager.h>

#ifdef BARNYARD_LATEST
#  include "AOptions.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00651c30
APlayerProgress::APlayerProgress( TINT a_iSizeAlignment, TINT a_iAlignment, TINT a_iHeaderOffset, TINT a_iExtraSize )
{
	m_uiSaveSize     = a_iHeaderOffset + a_iExtraSize + sizeof( SaveData );
	m_iCurrentSlot   = 0;
	m_uiUnk24        = 0;
	m_eMiniGameFlags = UNLOCKFLAGS_UNK0 | UNLOCKFLAGS_UNK1 | UNLOCKFLAGS_UNK2 | UNLOCKFLAGS_UNK3 | UNLOCKFLAGS_UNK11 | UNLOCKFLAGS_UNK12 | UNLOCKFLAGS_ART0 | UNLOCKFLAGS_MOVIE0;
	m_uiBufferSize   = ~( a_iSizeAlignment - 1 ) & ( m_uiSaveSize + a_iSizeAlignment - 1 );
	m_pBuffer        = TSTATICCAST( TUINT8, TMemalign( a_iAlignment, m_uiBufferSize, TNULL ) );
	TUtil::MemClear( m_pBuffer, m_uiBufferSize );

	m_pSaveData = TREINTERPRETCAST( SaveData*, m_pBuffer + a_iHeaderOffset );
	TUtil::MemCopy( m_pSaveData->oHeader.szMagic, "BY", 2 );
	m_pSaveData->oHeader.uiSize = sizeof( SaveData );

	ClearAllSlots();
	TUtil::MemClear( m_pSaveData->oHeader.aOptions, sizeof( m_pSaveData->oHeader.aOptions ) );
	m_pSaveData->oHeader.uiUnk10         = m_uiUnk24;
	m_pSaveData->oHeader.uiUnlockedFlags = m_eMiniGameFlags;

	m_pUnk00 = TNULL;
	m_iUnk04 = 0;
	m_pUnk08 = TNULL;
	m_iUnk0C = 0;
}

// $Barnyard: FUNCTION 00651d10
APlayerProgress::~APlayerProgress()
{
	TFree( m_pBuffer );
	TFree( m_pUnk00 );
	TFree( m_pUnk08 );
}

// $Barnyard: FUNCTION 006519a0
void APlayerProgress::SetupHeader()
{
	TUtil::MemClear( &m_pSaveData->oHeader, 16 );
	TUtil::MemCopy( m_pSaveData->oHeader.szMagic, "BY", 2 );
	m_pSaveData->oHeader.uiVersion  = SAVE_VERSION;
	m_pSaveData->oHeader.uiDate     = SAVE_DATE;
	m_pSaveData->oHeader.uiSize     = sizeof( SaveData );
	m_pSaveData->oHeader.uiChecksum = 0;
}

// NOTE: inlined into ClearAllSlots (006519f0) and ClearCurrentSlot (00651a90)
static void ClearSlot( APlayerProgress::SaveSlot* a_pSlot )
{
	a_pSlot->bOccupied = 0;
	TUtil::MemClear( a_pSlot->aQuickSave, sizeof( a_pSlot->aQuickSave ) );
	TUtil::MemClear( a_pSlot->aHints, sizeof( a_pSlot->aHints ) );
	TUtil::MemClear( a_pSlot->aHighScores, sizeof( a_pSlot->aHighScores ) );
	TUtil::MemClear( &a_pSlot->iGender, 16 );
	a_pSlot->eFlags = APlayerProgress::SLOTFLAGS_NONE;
	a_pSlot->uiDate = 0;
	a_pSlot->uiTime = 0;
}

// $Barnyard: FUNCTION 006519f0
void APlayerProgress::ClearAllSlots()
{
	for ( TINT i = 0; i < NUM_SAVE_SLOTS; i++ )
		ClearSlot( &m_pSaveData->aSlots[ i ] );
}

// $Barnyard: FUNCTION 00651a90
void APlayerProgress::ClearCurrentSlot()
{
	ClearSlot( &m_pSaveData->aSlots[ m_iCurrentSlot ] );
}

// $Barnyard: FUNCTION 00651d70
TUINT32 APlayerProgress::UpdateChecksum()
{
	TUINT32 uiOldChecksum           = m_pSaveData->oHeader.uiChecksum;
	m_pSaveData->oHeader.uiChecksum = 0;

	TUINT32 uiChecksum              = TUtil::CRC32( m_pBuffer, m_uiSaveSize );
	m_pSaveData->oHeader.uiChecksum = uiOldChecksum;
	m_pSaveData->oHeader.uiChecksum = uiChecksum;

	return uiChecksum;
}

// $Barnyard: FUNCTION 00651da0
TBOOL APlayerProgress::Validate()
{
	if ( TStringManager::String8CompareNoCase( m_pSaveData->oHeader.szMagic, "BY", 2 ) == 0 && m_pSaveData->oHeader.uiSize == sizeof( SaveData ) )
	{
		TUINT32 uiChecksum              = m_pSaveData->oHeader.uiChecksum;
		m_pSaveData->oHeader.uiChecksum = 0;

		TUINT32 uiRealChecksum          = TUtil::CRC32( m_pBuffer, m_uiSaveSize );
		m_pSaveData->oHeader.uiChecksum = uiChecksum;

		if ( m_pSaveData->oHeader.uiChecksum == uiRealChecksum && m_pSaveData->oHeader.uiVersion == SAVE_VERSION && m_pSaveData->oHeader.uiDate == SAVE_DATE )
			return TTRUE;
	}

	ClearAllSlots();
	return TFALSE;
}

// $Barnyard: FUNCTION 00651b60
void APlayerProgress::UnlockQuestTierFlags( TINT a_iTier, TINT a_iQuest )
{
	if ( a_iTier > 1 || a_iQuest >= 6 )
		m_eMiniGameFlags |= UNLOCKFLAGS_MUD_JUMP;

	if ( a_iTier > 1 || a_iQuest >= 10 )
		m_eMiniGameFlags |= UNLOCKFLAGS_BARN_POOL;

	if ( a_iTier > 1 || a_iQuest >= 11 )
		m_eMiniGameFlags |= UNLOCKFLAGS_BIKE_RACE;

	if ( a_iTier > 1 || a_iQuest >= 13 )
		m_eMiniGameFlags |= UNLOCKFLAGS_CHASING_CHICKS;

	if ( a_iTier > 1 || a_iQuest >= 13 )
		m_eMiniGameFlags |= UNLOCKFLAGS_VEGGIE_PATCH;

	if ( a_iTier > 2 || ( a_iTier == 2 && a_iQuest >= 8 ) )
		m_eMiniGameFlags |= UNLOCKFLAGS_COW_TIPPING;

	if ( a_iTier > 2 || ( a_iTier == 2 && a_iQuest >= 19 ) )
		m_eMiniGameFlags |= UNLOCKFLAGS_BARNTENDER;
}

// $Barnyard: FUNCTION 00651be0
TBOOL APlayerProgress::UnlockLadderBeansFlags( TINT8 a_iNumGrown )
{
	UNLOCKFLAGS uiOldFlags = m_eMiniGameFlags;
	m_eMiniGameFlags       = uiOldFlags | ( UNLOCKFLAGS_ART0 << a_iNumGrown ) | ( UNLOCKFLAGS_MOVIE0 << a_iNumGrown );

	return uiOldFlags != m_eMiniGameFlags;
}

// $Barnyard: FUNCTION 00651e20
APlayerProgress::SaveSlot* APlayerProgress::SetCurrentSlotChampion()
{
	// NOTE: the original doesn't check for a missing slot here
	SaveSlot* pSlot = GetCurrentSlot();

	if ( !( pSlot->eFlags & SLOTFLAGS_CHAMPION ) )
		pSlot->eFlags |= SLOTFLAGS_NEWUNLOCK;

	pSlot->eFlags |= SLOTFLAGS_CHAMPION;
	return pSlot;
}

// $Barnyard: FUNCTION 00651e90
APlayerProgress::SaveSlot* APlayerProgress::SetCurrentSlotBarnDetail()
{
	// NOTE: the original doesn't check for a missing slot here
	SaveSlot* pSlot = GetCurrentSlot();

	if ( !( pSlot->eFlags & SLOTFLAGS_BARNDETAIL ) )
		pSlot->eFlags |= SLOTFLAGS_NEWUNLOCK;

	pSlot->eFlags |= SLOTFLAGS_BARNDETAIL;
	return pSlot;
}

// $Barnyard: FUNCTION 004232e0
TBOOL APlayerProgress::IsHero() const
{
	// NOTE: the original doesn't check for a missing slot here
	return GetCurrentSlot()->eFlags & SLOTFLAGS_HERO;
}

// $Barnyard: FUNCTION 004a7d00
APlayerProgress::SaveSlot* APlayerProgress::SetCurrentSlotHero()
{
	// NOTE: the original doesn't check for a missing slot here (it writes to address 0xd155 and returns TNULL)
	SaveSlot* pSlot = GetCurrentSlot();
	pSlot->eFlags |= SLOTFLAGS_HERO;

	return pSlot;
}

// $Barnyard: FUNCTION 0049ba20
TBOOL APlayerProgress::IsChampion() const
{
	// NOTE: the original doesn't check for a missing slot here
	return ( GetCurrentSlot()->eFlags & SLOTFLAGS_CHAMPION ) != 0;
}

// $Barnyard: FUNCTION 0049ba50
TBOOL APlayerProgress::HasCurrentSlotBarnDetail() const
{
	// NOTE: the original doesn't check for a missing slot here
	return ( GetCurrentSlot()->eFlags & SLOTFLAGS_BARNDETAIL ) != 0;
}

#ifdef BARNYARD_LATEST

// $BarnyardWiiJP: FUNCTION 80422900
TBOOL APlayerProgress::IsEasyDifficulty() const
{
	// NOTE: the latest version (Wii) returns TTRUE while an antic is played from the main menu (no save slot,
	// set by AFrontEndCharacterSelectState, cleared by AGameLoader::LoadSavedGame and ADifficultyOptionState),
	// otherwise SLOTFLAGS_EASY of the current slot. Here the difficulty is a setting of OpenBarnyard.cfg
	// (Options -> Difficulty, ADifficultyOptionState) that applies to the story and the antics alike, so the
	// save stays compatible with PC retail and Normal (the default) plays like PC retail everywhere.
	return AOptions::IsEasyDifficultyEnabled();
}

#endif // BARNYARD_LATEST
