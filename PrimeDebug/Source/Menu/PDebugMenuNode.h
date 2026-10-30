#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG_DEV
#  include "Util/PArray.h"

#  include <Toshi/TString8.h>

class PCVar;

//-----------------------------------------------------------------------------
// Purpose: the entries of the debug menu (Prime's geNode, geMenuNode,
// geLabelNode, geVarNode, geCommandLineNode): submenus, labels calling back
// the code that added them, console variables (changed with left/right or a
// number typed in) and console lines run when activated.
//-----------------------------------------------------------------------------
class PDebugNode
{
public:
	enum TYPE
	{
		TYPE_MENU        = 0,
		TYPE_LABEL       = 1,
		TYPE_VAR         = 2,
		TYPE_COMMANDLINE = 3,
	};

	static constexpr TINT    MAX_NAME_LENGTH = 0x5F;
	static constexpr TUINT32 COLOUR          = 0x808080;
	static constexpr TUINT32 SELECTED_COLOUR = 0xF5F5F5;

public:
	PDebugNode( TYPE a_eType, const TCHAR* a_szName );
	virtual ~PDebugNode(); // +0x00

	virtual TBOOL Find( TINT a_iId, PDebugNode** a_ppNode );                                                                 // +0x04
	virtual TBOOL SetTyped( const TCHAR* a_szText );                                                                         // +0x08, a value typed in
	virtual void  Clear();                                                                                                   // +0x0C
	virtual void  Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight ); // +0x10
	virtual void  Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime );                                    // +0x14, right/return (left: decrease) held

	TUINT32 GetColour( TINT a_iSelectedId, TFLOAT a_fAlpha );

	TYPE         GetType() const { return m_eType; }
	const TCHAR* GetName() const { return m_strName.GetString(); }
	TINT         GetNameLength() const { return m_iNameLength; }
	TINT         GetId() const { return m_iId; }

	// A bar behind a line of text at a_fX, a_fY
	static void DrawBar( TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TUINT32 a_uiColour );

protected:
	TYPE            m_eType;
	Toshi::TString8 m_strName;
	TINT            m_iNameLength; // at most MAX_NAME_LENGTH
	TINT            m_iId;         // unique

	static TINT ms_iLastNodeId;
};

class PDebugMenuNode : public PDebugNode
{
public:
	friend class PDebugMenuTree;

	PDebugMenuNode( const TCHAR* a_szName );
	virtual ~PDebugMenuNode();

	virtual TBOOL Find( TINT a_iId, PDebugNode** a_ppNode ) override;
	virtual void  Clear() override;
	virtual void  Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight ) override;

	// A path from this menu: names separated by '/'; a_ppParent gets the menu holding the node found
	PDebugNode* FindPath( const TCHAR* a_szPath, TINT a_iLength, PDebugMenuNode** a_ppParent );
	TBOOL       HasChild( const TCHAR* a_szName );
	TBOOL       FindChild( const TCHAR* a_szName, PDebugNode** a_ppChild );
	void        DrawChildren( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight );

	TINT        GetNumChildren() const { return m_oChildren.Size(); }
	PDebugNode* GetChild( TINT a_iIndex ) { return m_oChildren[ a_iIndex ]; }

private:
	PArray<PDebugNode*> m_oChildren;
};

// Called with the label's id when it is activated
typedef void ( *PDebugLabelCallback )( TINT a_iId, void* a_pUserData );

class PDebugLabelNode : public PDebugNode
{
public:
	PDebugLabelNode( const TCHAR* a_szName, PDebugLabelCallback a_fnCallback, void* a_pUserData );
	virtual ~PDebugLabelNode();

	virtual void Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime ) override;

	void* GetUserData() const { return m_pUserData; }

private:
	PDebugLabelCallback m_fnCallback;
	void*               m_pUserData;
};

// A console variable, changed with left/right or a number typed in
class PDebugVarNode : public PDebugNode
{
public:
	PDebugVarNode( const TCHAR* a_szName, PCVar* a_pVar );
	virtual ~PDebugVarNode();

	virtual TBOOL SetTyped( const TCHAR* a_szText ) override;
	virtual void  Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight ) override;
	virtual void  Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime ) override;

	TFLOAT GetHeldValue( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime );
	TBOOL  ApplyTyped();

private:
	PCVar*       m_pVar;
	TFLOAT       m_fValue; // while held: the value without the step rounding
	const TCHAR* m_szTyped;
};

// A console line executed when activated
class PDebugCommandLineNode : public PDebugNode
{
public:
	PDebugCommandLineNode( const TCHAR* a_szName, const TCHAR* a_szCommand );
	virtual ~PDebugCommandLineNode();

	virtual void Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight ) override;
	virtual void Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime ) override;

private:
	Toshi::TString8 m_strCommand;
};

#endif // PRIMEDEBUG_DEV
