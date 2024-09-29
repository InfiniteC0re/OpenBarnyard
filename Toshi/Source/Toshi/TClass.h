#pragma once

TOSHI_NAMESPACE_START

class TObject;

class TClass
{
public:
	using CreateObject_t        = TObject* (*)();
	using CreateObjectInPlace_t = TObject* (*)( void* a_pPtr );
	using Initialise_t          = void ( * )();
	using Deinitialise_t        = void ( * )();

	using RecurseTreeCheck_t       = TBOOL ( * )( TClass*, void* );
	using RecurseTreeBaseBeginCb_t = void ( * )( TClass*, void* );
	using RecurseTreeBaseEndCb_t   = void ( * )( TClass*, void* );

public:
	TClass(
	    const TCHAR*          a_szName,
	    TClass*               a_pParentClass,
	    CreateObject_t        a_fnCreate,
	    CreateObjectInPlace_t a_fnCreateInPlace,
	    Initialise_t          a_fnInitialise,
	    Deinitialise_t        a_fnUnitialise,
	    TUINT16               a_uiVersionMajor,
	    TUINT16               a_uiVersionMinor,
	    TUINT32               a_uiClassSize,
	    TUINT32               a_uiClassAlignment
	);

	constexpr TClass(
	    const TCHAR*          a_szName,
	    CreateObject_t        a_fnCreate,
	    CreateObjectInPlace_t a_fnCreateInPlace,
	    Initialise_t          a_fnInitialise,
	    Deinitialise_t        a_fnUnitialise,
	    TUINT16               a_uiVersionMajor,
	    TUINT16               a_uiVersionMinor,
	    TUINT32               a_uiClassSize,
	    TUINT32               a_uiClassAlignment
	) :
	    m_szName( a_szName ),
	    m_pParent( TNULL ),
	    m_pPrevious( TNULL ),
	    m_pLastChildren( TNULL ),
	    m_fnCreate( a_fnCreate ),
	    m_fnCreateInPlace( a_fnCreateInPlace ),
	    m_fnInitialise( a_fnInitialise ),
	    m_fnDeinitialise( a_fnUnitialise ),
	    m_uiVersionMajor( a_uiVersionMajor ),
	    m_uiVersionMinor( a_uiVersionMinor ),
	    m_uiClassSize( a_uiClassSize ),
	    m_uiClassAlignment( a_uiClassAlignment ),
	    m_bInitialised( TFALSE )
	{}

	~TClass();

	void           Initialize();
	void           RecurseTree( RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom );
	void           RecurseTree2( RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom );
	class TObject* CreateObject() const;

	TBOOL        IsA( TClass* pClass );
	TBOOL        IsExactly( TClass* toCompare ) const { return this == toCompare; }
	TBOOL        IsInitialized() const { return m_bInitialised; }
	TClass*      GetParent() { return static_cast<TClass*>( m_pParent ); }
	const TCHAR* GetName() const { return m_szName; }
	TUINT16      GetVersionMajor() const { return m_uiVersionMajor; }
	TUINT16      GetVersionMinor() const { return m_uiVersionMinor; }
	TUINT32      GetMaxSizeOfDerivedClasses();
	TUINT32      GetMaxAlignmentOfDerivedClasses();

	// todo: move this function away from this class
	static TBOOL TryInitialize( TClass* tClass );

	// Looks for a class in parent
	static TClass* FindRecurse( const TCHAR* const& name, TClass* parent, TBOOL hasPrevious );
	static TClass* Find( const TCHAR* name, TClass* parent = TNULL );

public:
	inline TBOOL operator==( const TClass* other ) const { return this == other; }

private:
	const TCHAR*          m_szName;           // 0x00
	CreateObject_t        m_fnCreate;         // 0x04
	CreateObjectInPlace_t m_fnCreateInPlace;  // 0x08
	Initialise_t          m_fnInitialise;     // 0x0C
	Deinitialise_t        m_fnDeinitialise;   // 0x10
	TClass*               m_pParent;          // 0x14
	TClass*               m_pPrevious;        // 0x18
	TClass*               m_pLastChildren;    // 0x1C
	TUINT16               m_uiVersionMajor;   // 0x20
	TUINT16               m_uiVersionMinor;   // 0x22
	TUINT32               m_uiClassSize;      // 0x24
	TUINT32               m_uiClassAlignment; // 0x28
	TBOOL                 m_bInitialised;     // 0x2C
};

TOSHI_NAMESPACE_END
