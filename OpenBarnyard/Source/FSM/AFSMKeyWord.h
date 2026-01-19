#pragma once
#include <Toshi/TPString8.h>

#define FSM_REGISTER_KEYWORD( INTERNAL_NAME, NAME ) AFSMKeyWord g_fsmType_##INTERNAL_NAME( NAME )
#define FSM_DECLARE_KEYWORD( INTERNAL_NAME )        extern AFSMKeyWord g_fsmType_##INTERNAL_NAME
#define FSM_KEYWORD( INTERNAL_NAME )                g_fsmType_##INTERNAL_NAME

class AFSMKeyWord
{
public:
	AFSMKeyWord( const TCHAR* a_pchName );
	~AFSMKeyWord();

private:
	inline static AFSMKeyWord* ms_pHead = TNULL;
	inline static AFSMKeyWord* ms_pTail = TNULL;

private:
	Toshi::TPString8 strUnknown;
	const TCHAR*     pchName;
	AFSMKeyWord*        pNext;
};
