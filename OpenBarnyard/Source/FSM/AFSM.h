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

	void ResetArguments();

	TBOOL IsEmpty() const { return TTRUE; } // TODO: figure this out

private:
	Toshi::TPString8 m_strName;                   // 0x08

	TINT                 m_iNumArgs;
	const Toshi::TClass* m_apArgumentTypes[ MAX_NUM_ARGS ]; // 0x24
	// ...
	Toshi::TPString8 m_aSomeStrings[ MAX_NUM_ARGS ]; // 0x68
};
