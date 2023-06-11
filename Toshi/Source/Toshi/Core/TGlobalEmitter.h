#pragma once
#include "TDList.h"

namespace Toshi {

	template <class T>
	class TGenericGlobalListener : public TDList<TGenericGlobalListener<T>>::TNode
	{
	public:
		using EventCallback = void(*)(void*, T*);

	protected:
		TGenericGlobalListener()
		{
			m_pReceiver = TNULL;
			m_fnCallback = TNULL;
		}

		void ConnectImpl(void* pReceiver, EventCallback fnCallback);

	protected:
		void* m_pReceiver;
		EventCallback m_fnCallback;
	};

	template <class T>
	class TGlobalEmitter
	{
	public:
		// Important note: This should be TQList!
		inline static Toshi::TDList<TGenericGlobalListener<T>> sm_oListeners;
	};

	template <class ReceiverType, class EventType>
	class TGlobalListener : public TGenericGlobalListener<EventType>
	{
	public:
		void Connect(ReceiverType* receiver, TGenericGlobalListener<EventType>::EventCallback callback)
		{
			TGenericGlobalListener<EventType>::ConnectImpl(receiver, callback);
		}

		void Disconnect()
		{

		}
	};

	template<class T>
	inline void TGenericGlobalListener<T>::ConnectImpl(void* pReceiver, EventCallback fnCallback)
	{
		TASSERT(TNULL == m_pReceiver);
		m_pReceiver = pReceiver;
		m_fnCallback = fnCallback;
		TGlobalEmitter<T>::sm_oListeners.InsertHead(this);
	}

}