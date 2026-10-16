#pragma once
#include "AInputMap.h"

#include <Input/TInputInterface.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2Map.h>

struct AInputCommandArray
{
	static constexpr TUINT MAX_NUM_COMMANDS = 40;

	void Clear()
	{
		for ( TINT i = 0; i < iNumCommands; i++ )
		{
			aCommands[ i ] = AInputCommand_Empty;
		}

		iNumCommands = 0;
	}

	void AddCommand( AInputCommand a_eCommand )
	{
		if ( iNumCommands < MAX_NUM_COMMANDS )
		{
			aCommands[ iNumCommands++ ] = a_eCommand;
		}
	}

	TINT          iNumCommands = 0;
	AInputCommand aCommands[ MAX_NUM_COMMANDS ];
};

class AInputMapManager : public Toshi::TObject
    , public Toshi::TSingleton<AInputMapManager>
{
public:
	TDECLARE_CLASS( AInputMapManager, Toshi::TObject );

	using ActionId = TUINT32;

	using INPUTEVENT = TUINT32;
	enum INPUTEVENT_ : INPUTEVENT
	{
		INPUTEVENT_NONE      = 0,
		INPUTEVENT_GONE_DOWN = BITFLAG( Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN ),
		INPUTEVENT_GONE_UP   = BITFLAG( Toshi::TInputInterface::EVENT_TYPE_GONE_UP ),
		INPUTEVENT_REPEAT    = BITFLAG( Toshi::TInputInterface::EVENT_TYPE_REPEAT ),
		INPUTEVENT_VIRTUAL   = BITFLAG( Toshi::TInputInterface::EVENT_TYPE_VIRTUAL ),
		INPUTEVENT_MOVED     = BITFLAG( Toshi::TInputInterface::EVENT_TYPE_MOVED ),
	};

	struct Command
	{
		Command() = default;

		// $Barnyard: FUNCTION 00597110
		Command( const Toshi::TPString8& a_rcName, TINT32 a_iCode, TINT32 a_iEventType )
		    : Name( a_rcName ), uiCode( a_iCode ), uiEventType( a_iEventType )
		{}

		Toshi::TPString8 Name;
		TINT32           uiCode;
		TINT32           uiEventType;

		TBOOL IsEventTypeAllowed( Toshi::TInputInterface::EVENT_TYPE a_eEventType )
		{
			return HASANYFLAG( uiEventType, ( 1 << ( a_eEventType & 0x1F ) ) );
		}
	};

	static constexpr TUINT MAX_NUM_INPUT_MAPS = 32;

public:
	AInputMapManager();
	~AInputMapManager();

	TBOOL ReadControlsData();

	void       PushInputMap( AInputMap* a_pInputMap );
	AInputMap* PopInputMap();

	AInputMap* FindMap( const Toshi::TPString8& a_rMapName );
	TBOOL      SetMap( const Toshi::TPString8& a_rMapName );

	void GetEventCommands( const Toshi::TInputInterface::InputEvent* a_pEvent, AInputCommandArray& a_rCommandArray );

	AInputCommand GetCommandCode( const Toshi::TPString8& a_rCommandName );

	// Returns TFALSE if a_eCommand isn't registered, a_rwszOutName is TNULL if it has no localised name
	TBOOL GetCommandLocalisedName( AInputCommand a_eCommand, const TWCHAR*& a_rwszOutName );

	// Return TFALSE and set a_rwszOutName to TNULL if the doodad has no name
	TBOOL                       GetDoodadCustomName( Toshi::TInputDevice::Doodad a_iDoodad, const TWCHAR*& a_rwszOutName );
	TBOOL                       GetDoodadLocalisedName( Toshi::TInputDevice::Doodad a_iDoodad, const TWCHAR*& a_rwszOutName );
	Toshi::TInputDevice::Doodad GetDoodadFromKey( const Toshi::TPString8& a_rKey );

	TBOOL HasBindForDoodad( Toshi::TInputDevice::Doodad a_iDoodad );

private:
	void InitMouseDoodads();
	void InitKeyboardDoodads();
	void InitGamepadDoodads();
	void InitCommandMap();

	void DeinitMouseDoodads();
	void DeinitKeyboardDoodads();

	void BindDoodad( Toshi::TInputDevice::Doodad a_iDoodad, const Toshi::TPString8& a_ButtonName, ActionId a_uiAction );
	void UnbindDoodad( Toshi::TInputDevice::Doodad a_iDoodad );

private:
	Toshi::T2Vector<AInputMap*, MAX_NUM_INPUT_MAPS>             m_InputMaps;
	AInputMap*                                                  m_pActiveInputMap;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, Toshi::TPString8> m_oDoodadToNameMap;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, ActionId>         m_oKeyMap;
	Toshi::T2Map<AInputCommand, Command>                        m_oCommandMap;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, const TWCHAR*>    m_oDoodadToCustomNameMap; // 0xd4
	TINT                                                        m_iNumPushedInputMaps;
	// ...
	AInputMap* m_apPushedInputMaps[ MAX_NUM_INPUT_MAPS ];


#ifdef BARNYARD_COMMUNITY_PATCH
	TBOOL m_bIsUsingController  = TFALSE;
#endif // BARNYARD_COMMUNITY_PATCH
};
