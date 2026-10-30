#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG

class PCVar;

typedef void ( *PCVarChangedCallback )( PCVar* a_pVar );

// Value, range and step of int and float variables (4 components share the space for vectors)
union PCVarValue
{
	struct
	{
		TINT iValue;
		TINT iMin;
		TINT iMax;
		TINT iStep;
	};

	struct
	{
		TFLOAT fValue;
		TFLOAT fMin;
		TFLOAT fMax;
		TFLOAT fStep;
	};

	TCHAR szString[ 16 ];
};

//-----------------------------------------------------------------------------
// Purpose: a console variable (Prime's crCVar), kept by PConsole in a list
// sorted by name. The value lives in the variable or, for the Ref ones, in
// the caller's storage. Names are dot-separated levels ("debug.text_size"),
// the debug menu and the auto-completion walk them level by level.
//-----------------------------------------------------------------------------
class PCVar
{
public:
	friend class PConsole;

	enum TYPE : TUINT16
	{
		TYPE_NONE       = 0,
		TYPE_INT        = 1,
		TYPE_FLOAT      = 2,
		TYPE_VECTOR4    = 3,
		TYPE_FLOATSPLAT = 4, // one float written to all four components of a vector
		TYPE_MATRIX     = 5,
		TYPE_STRING     = 6,
	};

	enum FLAGS_ : TUINT16
	{
		FLAGS_NONE          = 0,
		FLAGS_READONLY      = BITFLAG( 0 ),
		FLAGS_VALIDATERANGE = BITFLAG( 1 ),
		FLAGS_BOOL          = BITFLAG( 3 ), // needs FLAGS_VALIDATERANGE and TYPE_INT
		FLAGS_ENUM          = BITFLAG( 4 ), // value names in the description: "(0=name,1=name,...)"
		FLAGS_HEX           = BITFLAG( 5 ),
		FLAGS_HASH          = BITFLAG( 7 ), // set from a string: the value is its hash
	};

public:
	PCVar()
	    : m_fnOnChanged( TNULL )
	    , m_szName( TNULL )
	    , m_szDescription( TNULL )
	    , m_uiFlags( 0 )
	    , m_uiType( TYPE_NONE )
	    , m_pUserData( TNULL )
	    , m_pValue( TNULL )
	    , m_pNext( TNULL )
	    , m_pPrev( TNULL )
	{
		memset( &m_oValue, 0, sizeof( m_oValue ) );
	}

	// a_pStorage: TNULL to keep the value in the variable. The strings must outlive the variable
	TBOOL Create( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT a_iType, const PCVarValue* a_pDefault, void* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData );
	void  Remove();

	void CreateInt( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT a_iValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData );
	void CreateIntRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT a_iValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TINT a_iMin, TINT a_iMax, TINT a_iStep );
	void CreateIntRef( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData );
	void CreateIntRefRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TINT a_iMin, TINT a_iMax, TINT a_iStep );
	void CreateFloat( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT a_fValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData );
	void CreateFloatRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT a_fValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep );
	void CreateFloatRef( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData );
	void CreateFloatRefRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep );
	void CreateFloatSplatRef( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData );
	void CreateFloatSplatRefRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep );

	// Setters return TFALSE for read-only variables or values outside the range (floats within one step of it are clamped)
	TBOOL SetInt( TINT a_iValue, TINT* a_pOldValue = TNULL );
	TBOOL SetFloat( TFLOAT a_fValue, TFLOAT* a_pOldValue = TNULL );
	TBOOL SetFloatSplat( TFLOAT a_fValue, TFLOAT* a_pOldValue = TNULL );
	TBOOL SetVector4( const TFLOAT* a_pValue, TFLOAT* a_pOldValue = TNULL );
	TBOOL SetMatrix( const TFLOAT* a_pValue, TFLOAT* a_pOldValue = TNULL );
	TBOOL SetString( const TCHAR* a_szValue );

	// One step up or down
	void StepInt( TBOOL a_bUp, TINT* a_pOldValue = TNULL );
	void StepFloat( TBOOL a_bUp, TFLOAT* a_pOldValue = TNULL );
	void StepFloatSplat( TBOOL a_bUp, TFLOAT* a_pOldValue = TNULL );

	TBOOL GetMinInt( TINT* a_pMin );
	TBOOL GetMaxInt( TINT* a_pMax );
	TBOOL GetMinFloat( TFLOAT* a_pMin );
	TBOOL GetMaxFloat( TFLOAT* a_pMax );
	void  GetVector4( TFLOAT* a_pOut );

	// The value for display ("1 (name)", "1 (true)", ...) / for a config file
	void FormatValue( TCHAR* a_szBuffer, TINT a_iBufferSize );
	void FormatValueRaw( TCHAR* a_szBuffer, TINT a_iBufferSize );

	// Splits the "(0=name,1=name,...)" list of an enum variable's description into a_ppNames; returns the count
	TINT GetEnumNames( TCHAR* a_szBuffer, TUINT a_uiBufferSize, TCHAR** a_ppNames, TINT a_iMaxNames );

	TINT         GetInt() const { return *(const TINT*)m_pValue; }
	TFLOAT       GetFloat() const { return *(const TFLOAT*)m_pValue; }
	TBOOL        GetBool() const { return GetInt() != 0; }
	const TCHAR* GetString() const { return (const TCHAR*)m_pValue; }

	const TCHAR*      GetName() const { return m_szName; }
	const TCHAR*      GetDescription() const { return m_szDescription; }
	TUINT16           GetFlags() const { return m_uiFlags; }
	TYPE              GetType() const { return TYPE( m_uiType ); }
	void*             GetUserData() const { return m_pUserData; }
	const PCVarValue& GetRange() const { return m_oValue; }
	PCVar*            GetNext() const { return m_pNext; }

private:
	// Outside the range: rejected beyond one step, clamped within it
	TBOOL ClampFloat( TFLOAT& a_rValue );

private:
	PCVarValue           m_oValue;        // 0x00, also holds strings of up to 16 bytes
	PCVarChangedCallback m_fnOnChanged;   // 0x10
	const TCHAR*         m_szName;        // 0x14
	const TCHAR*         m_szDescription; // 0x18
	TUINT16              m_uiFlags;       // 0x1C
	TUINT16              m_uiType;        // 0x1E
	void*                m_pUserData;     // 0x20
	void*                m_pValue;        // 0x24, m_oValue or external storage
	PCVar*               m_pNext;         // 0x28
	PCVar*               m_pPrev;         // 0x2C
};

#endif // PRIMEDEBUG
