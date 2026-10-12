#pragma once
#include "TQList.h"

TOSHI_NAMESPACE_START

template <class T>
class TGenericGlobalListener : public TQList<TGenericGlobalListener<T>>::TNode
{
public:
	using EventCallback = void ( * )( void*, const T& );

	template <class T>
	friend class TGlobalEmitter;

protected:
	TGenericGlobalListener()
	{
		m_pReceiver  = TNULL;
		m_fnCallback = TNULL;
	}

	void Execute( const T& a_rData )
	{
		m_fnCallback( m_pReceiver, a_rData );
	}

	void ConnectImpl( void* pReceiver, EventCallback fnCallback );
	void DisconnectImpl();

protected:
	void*         m_pReceiver;
	EventCallback m_fnCallback;
};

template <class T>
class TGlobalEmitter
{
public:
	class ListenerList : public Toshi::TQList<TGenericGlobalListener<T>>
	{
	public:
	};

public:
	// Instantiations: 00429630, <ATriggerGuiEvent> 005c26c0
	// $Barnyard: FUNCTION 00429630
	// TGlobalEmitter<AInteractionActivityEvent> 0059f9a0, TGlobalEmitter<AInteractionManagerEvent> 005a31b0
	// $Barnyard: FUNCTION 005a31b0
	// $Barnyard: FUNCTION 0059f9a0
	// $Barnyard: FUNCTION 005c26c0
	// <AGameStateControllerEvent>: 00429630, <AAnimalConversationEvent>: 00430410, <AUIDMapClearEvent>: 0040da70, <AUnitDestroyEvent>: 0040fe90
	// $Barnyard: FUNCTION 00430410
	// $Barnyard: FUNCTION 0040da70
	// $Barnyard: FUNCTION 0040fe90
	static void Throw( const T& a_rData )
	{
		// NOTE: the next listener is read before the callback, listeners can disconnect themselves
		for ( auto it = sm_oListeners.Begin(); it != sm_oListeners.End(); )
		{
			auto itNext = it;
			++itNext;

			it->Execute( a_rData );
			it = itNext;
		}
	}

public:
	inline static ListenerList sm_oListeners;
};

template <class ReceiverType, class EventType>
class TGlobalListener : public TGenericGlobalListener<EventType>
{
public:
	void Connect( ReceiverType* receiver, TGenericGlobalListener<EventType>::EventCallback callback )
	{
		TGenericGlobalListener<EventType>::ConnectImpl( receiver, callback );
	}

	void Disconnect()
	{
		TGenericGlobalListener<EventType>::DisconnectImpl();
	}
};

template <class T>
inline void TGenericGlobalListener<T>::ConnectImpl( void* pReceiver, EventCallback fnCallback )
{
	TASSERT( TNULL == m_pReceiver );
	m_pReceiver  = pReceiver;
	m_fnCallback = fnCallback;
	TGlobalEmitter<T>::sm_oListeners.Push( this );
}

template <class T>
inline void TGenericGlobalListener<T>::DisconnectImpl()
{
	TGlobalEmitter<T>::sm_oListeners.Remove( this );
	m_pReceiver  = TNULL;
	m_fnCallback = TNULL;
}

/**
	 * Used to catch all events that are thrown from TGlobalEvent
	 */
class TGenericGlobalEvent
{
public:
	TGenericGlobalEvent( const void* a_pData )
	    : m_pData( a_pData ) {}

public:
	const void* m_pData;
};

/**
	 * Use as a parent class for classes that are global events
	 */
template <class T>
class TGlobalEvent
{
public:
	// TGlobalEvent<AInteractionActivityEvent> 0059f9f0
	// $Barnyard: FUNCTION 0059f9f0
	// <AAnimalConversationEvent>: 004303f0
	// $Barnyard: FUNCTION 004303f0
	// <AGameStateControllerEvent>: 004296c0, <AInteractionManagerEvent>: 005a32a0
	// $Barnyard: FUNCTION 004296c0
	// $Barnyard: FUNCTION 005a32a0
	virtual void ThrowGlobal() const
	{
		TGlobalEmitter<T>::Throw( *TSTATICCAST( const T, this ) );
		TGlobalEmitter<TGenericGlobalEvent>::Throw( TGenericGlobalEvent( this ) );
	}
};

TOSHI_NAMESPACE_END
