#pragma once
#include "TDList.h"

TOSHI_NAMESPACE_START

class TGenericEmitter;

class TGenericListener : public TPriList<TGenericListener>::TNode
{
public:
	using t_Callback = TBOOL( __stdcall* )( void* a_pCaller, void* a_pOwner, void* a_pData );
	friend class TGenericEmitter;

public:
	TGenericListener() = default;
	~TGenericListener();

	void Connect( TGenericEmitter& a_rEmitter, void* a_pCaller, t_Callback a_pCallback, TINT a_iPriority );
	void Disconnect();

private:
	void Execute( void* pOwner, void* pData );

private:
	void*      m_pCaller   = TNULL;
	t_Callback m_pCallback = TNULL;
};

class TGenericEmitter
{
public:
	TGenericEmitter();
	TGenericEmitter( void* a_pOwner );
	~TGenericEmitter() = default;

	void Create( void* a_pOwner );
	void Destroy();

	void Throw( void* a_pData );

	TPriList<TGenericListener>&       GetListeners() { return m_Listeners; }
	const TPriList<TGenericListener>& GetListeners() const { return m_Listeners; }

public:
	TPriList<TGenericListener> m_Listeners;
	void*                      m_pOwner;
};

template <typename Owner, typename Data = Owner>
class TEmitter : public TGenericEmitter
{
public:
	TEmitter( Owner* owner )
	    : TGenericEmitter( owner ) {}

	void Throw( Data* pData ) { TGenericEmitter::Throw( pData ); }
	void Throw( Data&& data ) { TGenericEmitter::Throw( &data ); }

	void Create( Owner* owner ) { TGenericEmitter::Create( owner ); }
	void Destroy() { TGenericEmitter::Destroy(); }
};

template <typename Owner, typename Data, typename Caller>
class TListener : public TGenericListener
{
public:
	using t_CallerCallback = TBOOL( __stdcall* )( Caller* a_pCaller, Owner* a_pOwner, Data* a_pData );

public:
	void Connect( TGenericEmitter& a_rEmitter, Caller* a_pCaller, t_CallerCallback a_pCallback, TINT a_iPriority )
	{
		TGenericListener::Connect(
		    a_rEmitter,
		    a_pCaller,
		    TREINTERPRETCAST( t_Callback, a_pCallback ),
		    a_iPriority
		);
	}
};

TOSHI_NAMESPACE_END
