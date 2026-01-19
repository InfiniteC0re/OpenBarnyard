#pragma once
#include <Toshi/TPString8.h>

#define VARTYPE_REGISTER( INTERNAL_NAME, NAME ) AVarType g_fsmType_##INTERNAL_NAME( NAME )
#define VARTYPE_DECLARE( INTERNAL_NAME )        extern AVarType g_fsmType_##INTERNAL_NAME
#define VARTYPE( INTERNAL_NAME )                ( g_fsmType_##INTERNAL_NAME )

class AVarType
{
public:
	AVarType( const TCHAR* a_pchName );
	~AVarType();

	TBOOL operator==( const Toshi::TPString8& a_strName ) const { return strPooled == a_strName; }

public:
	//-----------------------------------------------------------------------------
	// Static interface
	//-----------------------------------------------------------------------------
	static void Create( Toshi::TPString8Pool* a_pStringPool );
	static void Destroy();

private:
	inline static AVarType* ms_pHead        = TNULL;
	inline static AVarType* ms_pTail        = TNULL;
	inline static TINT      ms_bInitialised = 0;

private:
	Toshi::TPString8 strPooled;
	const TCHAR*     pchName;
	AVarType*        pNext;
};
