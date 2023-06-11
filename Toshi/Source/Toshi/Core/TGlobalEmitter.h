#pragma once
#include "TDList.h"

template <class T>
class TGlobalEmitter;

template <class T2>
class TGenericGlobalListener
{
public:
	typedef void(*EventCallback)(void*, T2*);

	void ConnectImpl(void* receiver, EventCallback* callback)
	{
		TASSERT(TNULL == m_pReceiver);
		m_pReceiver = receiver;
		m_pCallback = callback;
		m_oEmitter.sm_oListeners.InsertHead(this);
	}

	void DisconnectImpl()
	{

	}

protected:
	TGlobalEmitter<T2> m_oEmitter; // 0x0
	void* m_pReceiver;             // 0x8
	EventCallback* m_pCallback;
};

template <class T, class T2>
class TGlobalListener : public TGenericGlobalListener<T2>
{
public:

	void Connect(T* receiver, TGenericGlobalListener<T2>::EventCallback* callback)
	{
		TGenericGlobalListener<T2>::ConnectImpl(receiver, callback);
	}

	void Disconnect()
	{

	}
};

template <class T>
class TGlobalEmitter
{
public:
	// Important note: This should be TQList!
	Toshi::TDList<TGenericGlobalListener<T>> sm_oListeners;
};