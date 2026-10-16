#pragma once
#include <Toshi/TTask.h>
#include <Input/TInputDeviceMouse.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TDList.h>
#include <Math/TVector4.h>
#include <Toshi/TSystem.h>

class AInputHandler : public Toshi::TTask
    , public Toshi::TSingleton<AInputHandler>
{
public:
	TDECLARE_CLASS( AInputHandler, Toshi::TTask );

	// Parameters of a controller rumble effect
	struct RumbleParams
	{
		RumbleParams() = default;

		// Lets the code fill it with { { 7 values }, iUnk } like an aggregate
		RumbleParams( const TFLOAT ( &a_rafValues )[ 7 ], TINT a_iUnk )
		    : iUnk( a_iUnk )
		{
			for ( TINT i = 0; i < 7; i++ )
				afUnk[ i ] = a_rafValues[ i ];
		}

		// $Barnyard: FUNCTION 0046ccd0
		RumbleParams( TFLOAT a_fValue0, TFLOAT a_fValue3, TFLOAT a_fValue5, TFLOAT a_fValue6, TINT a_iUnk, TFLOAT a_fValue1 )
		{
			afUnk[ 0 ] = a_fValue0;
			afUnk[ 1 ] = a_fValue1;
			afUnk[ 2 ] = 0.0f;
			afUnk[ 4 ] = 0.0f;
			afUnk[ 3 ] = a_fValue3;
			afUnk[ 5 ] = a_fValue5;
			afUnk[ 6 ] = a_fValue6;
			iUnk       = a_iUnk;
		}

		TFLOAT afUnk[ 7 ]; // 0x0
		TINT   iUnk;       // 0x1c
	};

	// A rumble effect that fades from m_fMaxStrength to m_fMinStrength over m_fDuration
	struct Rumble : public Toshi::TDList<Rumble>::TNode
	{
		enum FADE : TINT
		{
			FADE_NONE      = 0,
			FADE_LINEAR    = 1,
			FADE_QUADRATIC = 2,
		};

		// $Barnyard: FUNCTION 00595a50
		Rumble()
		{
			fMaxStrength = 0.0f;
			fMinStrength = 0.0f;
			fStrength    = 0.0f;
			fDuration    = 0.0f;
			fTime        = 0.0f;
			fUnk1        = 0.0f;
			fUnk2        = 0.0f;
			eFade        = FADE_LINEAR;
			pController  = TNULL;
		}

		// Updates the strength, returns TTRUE once the rumble is over
		TBOOL Update( TFLOAT a_fDeltaTime );

		TFLOAT                         fMaxStrength; // 0x08
		TFLOAT                         fMinStrength; // 0x0c
		TFLOAT                         fStrength;    // 0x10
		TFLOAT                         fDuration;    // 0x14, -1.0f = infinite
		TFLOAT                         fTime;        // 0x18
		TFLOAT                         fUnk1;        // 0x1c
		TFLOAT                         fUnk2;        // 0x20
		FADE                           eFade;        // 0x24
		Toshi::TInputDeviceController* pController;  // 0x28
	};

	// A rumble effect that comes from a world position, it's attenuated by the distance to m_vRumblePosition
	struct PositionalRumble : public Toshi::TDList<PositionalRumble>::TNode
	{
		// $Barnyard: FUNCTION 00595a80
		PositionalRumble()
		{
			fMaxStrength = 0.0f;
			fMinStrength = 0.0f;
			fStrength    = 0.0f;
			fDuration    = 0.0f;
			fTime        = 0.0f;
			fInnerRadius = 0.0f;
			fOuterRadius = 0.0f;
			eFade        = Rumble::FADE_LINEAR;
			vPosition    = Toshi::TVector4::VEC_ZERO;
			iUnk         = 0;
		}

		TFLOAT          fMaxStrength; // 0x08
		TFLOAT          fMinStrength; // 0x0c
		TFLOAT          fStrength;    // 0x10
		TFLOAT          fDuration;    // 0x14, -1.0f = infinite
		TFLOAT          fTime;        // 0x18
		TFLOAT          fInnerRadius; // 0x1c, full strength within it
		TFLOAT          fOuterRadius; // 0x20, no rumble outside of it
		Rumble::FADE    eFade;        // 0x24
		Toshi::TVector4 vPosition;    // 0x28
		TINT            iUnk;         // 0x38
	};

	// A controller that was disconnected while m_bRecordLostControllers is set
	struct LostController
	{
		TINT iUnk;   // 0x0, always -1
		TINT iIndex; // 0x4, TInputDeviceController::GetPortIndex
	};

	static constexpr TINT MAX_NUM_RUMBLES = 32;

public:
	AInputHandler();
	~AInputHandler();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	Toshi::TInputDeviceMouse*      GetMouseDevice() const { return m_pMouseDevice; }
	Toshi::TInputDeviceKeyboard*   GetKeyboardDevice() const { return m_pKeyboardDevice; }
	Toshi::TInputDeviceController* GetMainController() const { return m_pMainController; }

	TBOOL HadInputLastFrame() const { return m_bHadInputLastFrame; }

	TBOOL IsRecordingLostControllers() const { return m_bRecordLostControllers; }
	void  SetRecordLostControllers( TBOOL a_bValue ) { m_bRecordLostControllers = a_bValue; }

	// NOTE: not in PC retail, these are used by AFrontEndPlayersState (recovered from the original source)
	// Number of the controllers known to the input handler
	TINT GetNumControllers() const
	{
		TINT iNumControllers = 0;

		for ( TINT i = 0; i < m_Devices.Size(); i++ )
		{
			if ( m_Devices[ i ]->IsA( &TGetClass( Toshi::TInputDeviceController ) ) )
				iNumControllers++;
		}

		return iNumControllers;
	}

	// Returns the a_iIndex-th controller known to the input handler or TNULL
	Toshi::TInputDeviceController* GetController( TINT a_iIndex ) const
	{
		for ( TINT i = 0; i < m_Devices.Size(); i++ )
		{
			if ( m_Devices[ i ]->IsA( &TGetClass( Toshi::TInputDeviceController ) ) && a_iIndex-- == 0 )
				return TSTATICCAST( Toshi::TInputDeviceController, m_Devices[ i ] );
		}

		return TNULL;
	}

	// Starts a rumble effect on the controller. Does nothing on PC.
	// NOTE: shared folded stub (005cc080) that returns 0
	TBOOL SetRumble( Toshi::TInputDeviceController* a_pController, const RumbleParams* a_pParams ) { return TFALSE; }

	// Sets the world position the rumble effects come from (a_pController is unused)
	// $Barnyard: FUNCTION 005959f0
	void SetRumblePosition( Toshi::TInputDeviceController* a_pController, const Toshi::TVector4& a_rcPosition ) { m_vRumblePosition = a_rcPosition; }

	// Moves every positional rumble back to the free list
	void StopAllRumbles();

	// Forgets the controller the rumbles are played on if a_pController is specified
	void StopRumble( Toshi::TInputDeviceController* a_pController );

	// Stops the vibration of the controllers and pauses the rumble updates
	void SetRumblePaused( TBOOL a_bPaused );

	// Enables or disables the rumble (AOptions::IsRumbleEnabled), stops the vibration of the controllers when it's disabled
	void SetRumbleEnabled( TBOOL a_bEnabled );

	TBOOL IsValidDevice( Toshi::TInputDevice* a_pDevice ) const;

private:
	virtual void OnDestroy() override;

	// Picks the strongest rumble and applies it to m_pRumbleController
	void UpdateRumbles( TFLOAT a_fDeltaTime );

	TBOOL ProcessInputEvent( Toshi::TInputInterface* a_pInputInterface, Toshi::TInputInterface::InputEvent* a_pEvent );
	TBOOL OnDeviceStatusChanged( Toshi::TInputInterface* a_pInputInterface, Toshi::TInputInterface::DeviceStatusEvent* a_pEvent );
	TBOOL OnPause( Toshi::TSystemManager* a_pSystemManager, TBOOL* a_pPause );

private:
	Toshi::TInputDeviceMouse*                                                                          m_pMouseDevice;            // 0x20
	Toshi::TInputDeviceKeyboard*                                                                       m_pKeyboardDevice;         // 0x24
	Toshi::T2Vector<Toshi::TInputDevice*, 8>                                                           m_Devices;                 // 0x28, only controllers
	Toshi::TInputDeviceController*                                                                     m_pMainController;         // 0x4c
	Toshi::TInputDeviceController*                                                                     m_pLostMainController;     // 0x50
	Toshi::TListener<Toshi::TInputInterface, Toshi::TInputInterface::InputEvent, AInputHandler>        m_InputListener;           // 0x54
	Toshi::TListener<Toshi::TInputInterface, Toshi::TInputInterface::DeviceStatusEvent, AInputHandler> m_DeviceStatusListener;    // 0x68
	Toshi::TGenericListener                                                                            m_UnusedListener;          // 0x7c, only disconnected
	Toshi::TListener<Toshi::TSystemManager, TBOOL, AInputHandler>                                      m_PauseListener;           // 0x90
	TFLOAT                                                                                             m_fDeltaTime;              // 0xa4
	TBOOL                                                                                              m_bIsPaused;               // 0xa8
	TBOOL                                                                                              m_bHadInputLastFrame;      // 0xa9
	TBOOL                                                                                              m_bRecordLostControllers;  // 0xaa
	TINT                                                                                               m_iNumLostControllers;     // 0xac
	LostController                                                                                     m_aLostControllers[ 8 ];   // 0xb0
	TINT                                                                                               m_iUnkF0;                  // 0xf0
	TBOOL                                                                                              m_bUnkF4;                  // 0xf4
	TCHAR                                                                                              PADDING[ 0x2f6 - 0xf5 ];   // 0xf5, never accessed on PC
	TBOOL                                                                                              m_bRumblePaused;           // 0x2f6
	PositionalRumble*                                                                                  m_pPositionalRumbles;      // 0x2f8, pool of MAX_NUM_RUMBLES
	Toshi::TDList<PositionalRumble>                                                                    m_llFreePositionalRumbles; // 0x2fc
	Rumble*                                                                                            m_pRumbles;                // 0x304, pool of MAX_NUM_RUMBLES
	Toshi::TDList<Rumble>                                                                              m_llFreeRumbles;           // 0x308
	Toshi::TDList<PositionalRumble>                                                                    m_llPositionalRumbles;     // 0x310
	Toshi::TVector4                                                                                    m_vRumblePosition;         // 0x318, SetRumblePosition
	Toshi::TInputDeviceController*                                                                     m_pRumbleController;       // 0x328, never set on PC retail
	Toshi::TDList<Rumble>                                                                              m_llRumbles;               // 0x32c
	TBOOL                                                                                              m_bRumbleEnabled;          // 0x334
};

TSTATICASSERT( sizeof( AInputHandler ) == 0x338 );
