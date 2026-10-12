#pragma once
#include <Toshi/TPString8.h>
#include <Toshi/TString16.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

class PPropertyValue;

// Quick save of the running game state, kept in memory as a set of flat buffers.
// The buffers are copied as one block into the save slot (APlayerProgress) when the game is saved
// and copied back from it when a save is loaded.
class AHackedQuickSave
    : public Toshi::TObject
    , public Toshi::TSingleton<AHackedQuickSave>
{
public:
	TDECLARE_CLASS( AHackedQuickSave, Toshi::TObject );

	// Flat byte stream the quick save is written to / read from (0x14 bytes, vtable 0x007393dc)
	class Buffer
	{
	public:
		using FLAGS = TUINT32;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE  = 0,
			FLAGS_SAVED = BITFLAG( 0 ), // something has been saved into the buffer
		};

	public:
		// NOTE: inlined into AHackedQuickSave::AHackedQuickSave (0062f510)
		Buffer( TUINT a_uiSize );
		virtual ~Buffer();

		// Appends a_rcString's characters and a terminating '\0'
		void WriteString( const Toshi::TPString8& a_rcString );

		// Reads the next '\0'-terminated string into a_rOutString (pooled)
		void ReadString( Toshi::TPString8& a_rOutString );

		// Raw 4 bytes (the stream isn't aligned)
		void   WriteInt( TINT a_iValue );
		void   WriteFloat( TFLOAT a_fValue );
		void   ReadInt( TINT& a_rOutValue );
		void   ReadFloat( TFLOAT& a_rOutValue );
		void   ReadUInt( TUINT& a_rOutValue );
		void   WriteUInt8( TUINT8 a_uiValue );
		void   WriteUInt16( TUINT16 a_uiValue );
		void   ReadUInt8( TUINT8& a_rOutValue );
		void   ReadUInt16( TUINT16& a_rOutValue );
		TINT16 ReadInt16( TINT16& a_rOutValue );

		// One byte, 0 or 1
		void  WriteBool( const TBOOL& a_rcValue );
		TBOOL ReadBool( TBOOL& a_rOutValue );

		// '\0'-terminated C strings
		void WriteString( const TCHAR* a_szString );
		void ReadString( TCHAR* a_szOutString );

		// Copies the next string into a_szOutString without moving the cursor, TFALSE if it doesn't fit in a_iMaxLength
		TBOOL PeekString( TCHAR* a_szOutString, TINT a_iMaxLength );

		// Reads the next string and compares it with a_szString
		TBOOL CheckString( const TCHAR* a_szString );

		// Reads the next string and compares it with a_rcString
		TBOOL CheckString( const Toshi::TPString8& a_rcString, TBOOL a_bUnused );

		// Little endian 16-bit characters, terminated by a 0 character
		void WriteWString( const TWCHAR* a_wszString );
		void WriteWString( const Toshi::TString16& a_rcString );
		void ReadWString( Toshi::TString16& a_rOutString );

		// Type of the value (see GetPropertyTypeId) followed by the value
		void WritePropertyValue( const PPropertyValue& a_rcValue );
		void ReadPropertyValue( PPropertyValue& a_rOutValue );

		// Raw x, y, z, w (byte-wise, the stream isn't aligned)
		void WriteVector4( const Toshi::TVector4& a_rcVector );
		void ReadVector4( Toshi::TVector4& a_rOutVector );

		// Raw x, y, z (w isn't stored)
		void WriteVector3( const Toshi::TVector4& a_rcVector );
		void ReadVector3( Toshi::TVector4& a_rOutVector );

		// $Barnyard: FUNCTION 0062ed50
		void Reset()
		{
			m_eFlags &= ~FLAGS_SAVED;
			m_uiCursor = 0;
		}

		TBOOL IsSaved() const { return m_eFlags & FLAGS_SAVED; }
		void  SetSaved( TBOOL a_bSaved ) { a_bSaved ? ( m_eFlags |= FLAGS_SAVED ) : ( m_eFlags &= ~FLAGS_SAVED ); }

		TUINT  GetCursor() const { return m_uiCursor; }
		void   SetCursor( TUINT a_uiCursor ) { m_uiCursor = a_uiCursor; }
		TUINT  GetSize() const { return m_uiSize; }
		TCHAR* GetData() const { return m_pData; }

	private:
		friend AHackedQuickSave;

		TCHAR* m_pData;    // 0x04
		TUINT  m_uiSize;   // 0x08
		FLAGS  m_eFlags;   // 0x0c, stored in the save together with the cursor
		TUINT  m_uiCursor; // 0x10
	};

	using BUFFER = TINT;
	enum BUFFER_ : BUFFER
	{
		BUFFER_WORLD,     // regrowth, treasures, the player unit, the saveable units, replays and gates
		BUFFER_TIME,      // game time and animal spawn state
		BUFFER_PROFILE,   // player profile, quests, interaction events, inventory, persistent data and latches
		BUFFER_OFFERS,    // Joey's offers
		BUFFER_COOKING,   // the dishes made in the cooking minigame (ACookingMiniGame::Save/Load, ABarnExploreMinigameState::SpawnCookedFood)
		BUFFER_REGROWTH,  // regrowth state kept while in the barn
		BUFFER_REGROWTH2, // regrowth state kept while in the barn (second set)
		BUFFER_NUMOF
	};

	// Size of the block the buffers are copied to in the save slot (with each buffer's flags and cursor)
	static constexpr TUINT SAVE_DATA_SIZE = 0xb638;

public:
	AHackedQuickSave();
	~AHackedQuickSave();

	// Copies every buffer (flags, cursor and data) into a_pData. a_iSize has to be SAVE_DATA_SIZE.
	TBOOL SaveToMemory( void* a_pData, TINT a_iSize );

	// Copies every buffer (flags, cursor and data) from a_pData. a_iSize has to be SAVE_DATA_SIZE.
	TBOOL LoadFromMemory( const void* a_pData, TINT a_iSize );

	// Saves the game time and the animal spawn state into BUFFER_TIME
	TBOOL SaveTime();

	// Loads what SaveTime saved (if anything)
	TBOOL LoadTime();

	// Saves the player profile, quests, events, inventory, persistent data and latches into BUFFER_PROFILE
	TBOOL SaveProfile();

	// Loads what SaveProfile saved (if anything)
	TBOOL LoadProfile();

	// Saves the player unit (steer and components) into the current buffer
	TBOOL SavePlayer();

	// Loads what SavePlayer saved
	TBOOL LoadPlayer();

	// Saves the whole main game (every buffer but BUFFER_OFFERS, BUFFER_COOKING and the regrowth ones)
	void SaveGame();

	// Loads what SaveGame saved (if anything)
	void LoadGame();

	// Resets the world, time and profile buffers, and every other one with a_bAll
	void ResetBuffers( TBOOL a_bAll );

	// Saves / loads the regrowth state into BUFFER_REGROWTH2 (a_bSecond) or BUFFER_REGROWTH
	static TBOOL SaveRegrowth( TBOOL a_bSecond );
	static TBOOL LoadRegrowth( TBOOL a_bSecond );

	Buffer* GetBuffer( BUFFER a_eBuffer ) const { return m_apBuffers[ a_eBuffer ]; }

	// Offset of the data of buffer a_eBuffer in the block written by SaveToMemory
	static TUINT GetBufferDataOffset( BUFFER a_eBuffer );

	// Reads 4 bytes at a_pData (unaligned)
	static TINT ReadInt( const void* a_pData );

	// Reads at most a_iMaxLength 16-bit characters at a_pData into a_wszOutString, returns the number of characters read
	static TINT ReadWString( const void* a_pData, TWCHAR* a_wszOutString, TINT a_iMaxLength );

	// Day phase of the game time when SaveTime / SaveGame were last called (-1 if never)
	static TINT GetSavedDayPhase() { return ms_iSavedDayPhase; }

#ifdef BARNYARD_LATEST
	// Wii: the story changed the time of the day while the barn of the night was loaded (AQuestManager::SetQuestTier).
	// SaveTime keeps it, so AAssetLoader::DestroyAssetPackOfLibrary still destroys the night barn library ("-n").
	static constexpr TINT SAVEDDAYPHASE_NIGHT_BARN = -52;

	static void SetSavedDayPhase( TINT a_iDayPhase ) { ms_iSavedDayPhase = a_iDayPhase; }
#endif

private:
	Buffer* GetCurrentBuffer() const { return m_apBuffers[ m_iCurrentBuffer ]; }

public:
	inline static const TUINT ms_auiBufferSizes[ BUFFER_NUMOF ] = { 0x6400, 0x400, 0x3800, 0x400, 0x200, 0xa00, 0x600 }; // 0x007393c0

private:
	inline static TINT ms_iSavedDayPhase = -1; // 0x007810fc

private:
	Buffer* m_apBuffers[ BUFFER_NUMOF ]; // 0x04
	BUFFER  m_iCurrentBuffer;            // 0x20
};
