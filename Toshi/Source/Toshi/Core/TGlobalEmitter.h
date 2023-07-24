#pragma once
#include "TDList.h"

namespace Toshi {

	template <class T>
	class TGenericGlobalListener : public TDList<TGenericGlobalListener<T>>::TNode
	{
	public:
		using EventCallback = void(*)(void*, const T&);

		template <class T>
		friend class TGlobalEmitter;

	protected:
		TGenericGlobalListener()
		{
			m_pReceiver = TNULL;
			m_fnCallback = TNULL;
		}

		void Execute(const T& a_rData)
		{
			m_fnCallback(m_pReceiver, a_rData);
		}

		void ConnectImpl(void* pReceiver, EventCallback fnCallback);
		void DisconnectImpl();

	protected:
		void* m_pReceiver;
		EventCallback m_fnCallback;
	};

	template <class T>
	class TGlobalEmitter
	{
	public:
		static void Throw(const T& a_rData)
		{
			for (auto it = sm_oListeners.Begin(); it != sm_oListeners.End(); it++)
			{
				it->Execute(a_rData);
			}
		}

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
			TGenericGlobalListener<EventType>::DisconnectImpl();
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

	template<class T>
	inline void Toshi::TGenericGlobalListener<T>::DisconnectImpl()
	{
		TGlobalEmitter<T>::sm_oListeners.RemoveHead();
		m_pReceiver = TNULL;
		m_fnCallback = TNULL;
	}
}