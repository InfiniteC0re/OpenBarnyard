#pragma once
#include "AFunctionTable.h"

#include <initializer_list>

//-----------------------------------------------------------------------------
// Placeholder of a script native of a class that isn't reconstructed yet.
// It has the signature of the retail native so the FSM scripts that use it can
// be parsed. When called it logs once and returns a neutral value.
// NOTE: not in retail.
//-----------------------------------------------------------------------------
class AFunctionStub
    : public AFunction
{
public:
	static constexpr TINT MAX_NUM_ARGS = 6;

public:
	AFunctionStub( const TCHAR* a_szName, const TCHAR* a_szTableName, const Toshi::TClass* a_pReturnType, std::initializer_list<const Toshi::TClass*> a_lstArgTypes );
	AFunctionStub( const AFunctionStub& a_rOther );
	~AFunctionStub() override;

	AFunction* Clone() const override;
	void       Call( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const override;
	void       GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const override;

	// Registers the function tables of the classes that aren't reconstructed yet
	static void RegisterStubTables();
	static void UnregisterStubTables();

private:
	const TCHAR*         m_szTableName;
	const Toshi::TClass* m_pReturnType;
	const Toshi::TClass* m_apArgTypes[ MAX_NUM_ARGS ];
	TINT                 m_iNumArgs;
	mutable TBOOL        m_bLogged;
};
