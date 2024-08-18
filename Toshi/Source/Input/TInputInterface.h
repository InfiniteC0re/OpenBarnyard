#pragma once
#include "Toshi/TNodeList.h"
#include "Toshi/TArray.h"

TOSHI_NAMESPACE_START

class TInputDevice;

class TInputInterface :
    public TObject,
    public TSingleton<TInputInterface>
{
public:
	TDECLARE_CLASS( TInputInterface, TObject );

public:
	enum EVENT_TYPE
	{
		EVENT_TYPE_GONE_DOWN,
		EVENT_TYPE_GONE_UP,
		EVENT_TYPE_REPEAT,
		EVENT_TYPE_UNKNOWN,
		EVENT_TYPE_MOVED
	};

	class InputEvent
	{
	public:
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TWCHAR* a_wszString );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1 );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1, TINT a_iMagnitude2 );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1 );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1, TFLOAT a_fMagnitude2 );

		TINT GetMagnitudeInt( TINT a_iAxis );

		TFLOAT GetMagnitudeFloat( TINT a_iAxis );

		int8_t GetAxisCount() const
		{
			return m_iAxisCount;
		}

		TINT GetDoodad() const
		{
			return m_iDoodad;
		}

		EVENT_TYPE GetEventType() const
		{
			return m_eEventType;
		}

		TInputDevice* GetSource() const
		{
			return m_pSource;
		}

		TBOOL IsMagnitudeFloat()
		{
			return m_bIsMagnitudeFloat;
		}

		TBOOL IsMagnitudeInt()
		{
			return !m_bIsMagnitudeFloat;
		}

	public:
		TINT          m_iDoodad;           // 0x00
		EVENT_TYPE    m_eEventType;        // 0x04
		TBOOL         m_bIsMagnitudeFloat; // 0x08
		int8_t        m_iAxisCount;        // 0x09
		TInputDevice* m_pSource;           // 0x0C
		TWCHAR        m_wszString[ 4 ];    // 0x10

		union
		{
			TFLOAT Floats[ 2 ];
			TINT   Ints[ 2 ];
		} m_Magnitude; // 0x18 De blob 0x10 JPOG
	};

	using EventEmitter = TEmitter<TInputInterface, TInputInterface::InputEvent>;

public:
	TInputInterface() :
	    m_InputEventEmitter( this )
	{
		m_bIsExclusiveMode = TFALSE;
	}

	~TInputInterface()           = default;
	virtual TBOOL Initialise()   = 0;
	virtual TBOOL Deinitialise() = 0;
	virtual TBOOL AcquireAll();
	virtual TBOOL UnacquireAll();
	virtual TBOOL FlushAll();
	virtual void  SetExclusiveMode( TBOOL a_bIsExclusive );
	virtual TBOOL GetExclusiveMode() const;
	virtual TINT  ProcessEvents( TFLOAT a_fDeltaTime );
	virtual void  StopAllRepeats();

	void AddDevice( TInputDevice* device );
	void RemoveDevice( TInputDevice* device );

	void ReleaseAllDevices();

	TInputDevice* GetDeviceByIndex( TClass* a_pClass, TUINT a_uiIndex );

	EventEmitter& GetInputEventEmitter() { return m_InputEventEmitter; }

	template <class C>
	C* GetDeviceByIndex( TUINT a_uiIndex = 0 )
	{
		C* pDevice = TSTATICCAST( C, GetDeviceByIndex( &TGetClass( C ), a_uiIndex ) );
		TASSERT( pDevice == TNULL || pDevice->GetClass()->IsA( &TGetClass( C ) ) );
		return pDevice;
	}

private:
	TBOOL                   m_bIsExclusiveMode;  // 0x04
	TNodeList<TInputDevice> m_DeviceList;        // 0x08
	EventEmitter            m_InputEventEmitter; // 0x18
	TGenericEmitter         m_Emitter2;          // 0x24
};

class TInputDevice :
    public TObject,
    public TNodeList<TInputDevice>::TNode
{
public:
	TDECLARE_CLASS( TInputDevice, TObject );

public:
	static constexpr TINT INPUT_DEVICE_MOUSE_BUTTONS = 3;
	static constexpr TINT INPUT_DEVICE_MOUSE_WHEEL   = 4;

	using EventEmitter = TInputInterface::EventEmitter;
	using Doodad       = TINT;

	struct DoodadProperties
	{
		TINT  m_iUnk;
		TBOOL m_bFlag;
	};

	struct RepeatInfo
	{
		Doodad iDoodad;
		TFLOAT fLeftTime;
		TFLOAT fRepeatTime;
	};

public:
	TInputDevice();
	~TInputDevice() = default;

	virtual TBOOL        Acquire()                  = 0;
	virtual TBOOL        Unacquire()                = 0;
	virtual void         Release()                  = 0;
	virtual void         Update( TFLOAT deltaTime ) = 0;
	virtual TBOOL        Flush();
	virtual TINT         ProcessEvents( EventEmitter& emitter, TFLOAT deltaTime )                       = 0;
	virtual TINT         GetButtonCount() const                                                         = 0;
	virtual TINT         GetAxisCount() const                                                           = 0;
	virtual TBOOL        GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& a_rProperties ) const = 0;
	virtual TBOOL        StartRepeat( Doodad a_iDoodad, TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime );
	virtual void         StopRepeat( Doodad a_iDoodad );
	virtual void         StopAllRepeats();
	virtual TBOOL        IsForceFeedbackDevice();
	virtual const TCHAR* GetButtonFromDoodad( Doodad a_iDoodad ) const   = 0;
	virtual TBOOL        IsDown( Doodad a_iDoodad ) const                = 0;
	virtual TINT         GetAxisInt( Doodad a_iDoodad, TINT axis ) const = 0;
	virtual TINT         GetAxisInt2( Doodad a_iDoodad, TINT axis ) const { return GetAxisInt( a_iDoodad, axis ); }
	virtual TFLOAT       GetAxisFloat( Doodad a_iDoodad, TINT axis ) const = 0;
	virtual TFLOAT       GetAxisFloat2( Doodad a_iDoodad, TINT axis ) const { return GetAxisFloat( a_iDoodad, axis ); }
	virtual TBOOL        IsEnabled() const = 0;
	virtual void         ThrowRepeatEvent( EventEmitter& emitter, RepeatInfo* repeatInfo, TFLOAT deltaTime );

	TBOOL IsAcquired() const
	{
		return m_bIsAcquired;
	}

	void SetAcquired( TBOOL a_bAcquired )
	{
		m_bIsAcquired = a_bAcquired;
	}

	TInputInterface* GetInputInterface()
	{
		return m_pInputInterface;
	}

	void SetInputInterface( TInputInterface* a_pInterface )
	{
		m_pInputInterface = a_pInterface;
	}

protected:
	TINT ProcessRepeats( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime );

public:
	static TInputDevice** GetRegisteredDevices()
	{
		return s_aDevices;
	}

	static TUINT GetNumRegisteredDevices()
	{
		return s_uiDeviceCount;
	}

protected:
	static constexpr TUINT MAX_DEVICE_COUNT = 14;

	inline static TInputDevice* s_aDevices[ MAX_DEVICE_COUNT ];
	inline static TUINT         s_uiDeviceCount;

protected:
	TUINT              m_uiDeviceIndex;
	TArray<RepeatInfo> m_Repeats;
	TArray<void*>      m_Array2;
	TBOOL              m_bUnknown;
	TBOOL              m_bIsAcquired;
	TInputInterface*   m_pInputInterface;
};

TOSHI_NAMESPACE_END
