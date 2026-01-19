#pragma once
#include <Toshi/TDList.h>
#include <Toshi/TPString8.h>

class AFSM
    : public Toshi::TDList<AFSM>::TNode
{
public:
	static constexpr TSIZE MAX_NUM_ARGS = 16;

public:
	AFSM( const Toshi::TPString8& a_strName );
	~AFSM();

	const Toshi::TPString8& GetName() const { return m_strName; }
	TBOOL                   IsEmpty() const { return TTRUE; } // TODO: figure this out

	void ResetArguments();

	TINT AddVariableName( const Toshi::TPString8& a_strName );
	TINT FindVariableNameIndex( const Toshi::TPString8& a_strName ) const;

	// Returns TTRUE if the type wasn't previously set, otherwise TFALSE
	TBOOL SetVariableType( TINT a_iIndex, const Toshi::TClass* a_pTypeClass );

private:
	Toshi::TPString8     m_strName;                   // 0x08
	TINT                 m_iNumVarTypes;
	const Toshi::TClass* m_aArgTypes[ MAX_NUM_ARGS ]; // 0x24
	TINT                 m_iNumVarNames;
	Toshi::TPString8     m_aVarNames[ MAX_NUM_ARGS ]; // 0x68
};
