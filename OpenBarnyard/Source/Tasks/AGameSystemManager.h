#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>

// Creates and owns the world systems (managers and tasks) once the game starts.
class AGameSystemManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AGameSystemManager>
{
public:
	TDECLARE_CLASS( AGameSystemManager, Toshi::TTask );

	static constexpr TINT MAX_NUM_TASKS    = 30;
	static constexpr TINT MAX_NUM_MANAGERS = 25;

public:
	// NOTE: inlined in CreateObject (004018c0)
	AGameSystemManager();
	~AGameSystemManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	// $Barnyard: FUNCTION 00401a00
	virtual void OnActivate() override { ActivateSystemTasks( TTRUE ); }

	// $Barnyard: FUNCTION 00401a10
	virtual void OnDeactivate() override { ActivateSystemTasks( TFALSE ); }

	void SetUnk104( TINT a_iValue ) { m_iUnk104 = a_iValue; }

	// (De)activates every system task except AMiniGameManager, AMusicManager and AHudManager
	void ActivateSystemTasks( TBOOL a_bActivate );

private:
	// Creates a child task of a_pClass, calls Create on it and keeps track of it.
	// NOTE: inlined everywhere in retail
	Toshi::TTask* CreateSystemTask( Toshi::TClass* a_pClass );

	// Creates an object of a_pClass and keeps track of it, it's deleted in OnDestroy.
	// NOTE: inlined everywhere in retail
	Toshi::TObject* CreateManager( Toshi::TClass* a_pClass );

private:
	TINT            m_iNumTasks;                      // 0x20
	Toshi::TTask*   m_apTasks[ MAX_NUM_TASKS ];       // 0x24
	TINT            m_iNumManagers;                   // 0x9c
	Toshi::TObject* m_apManagers[ MAX_NUM_MANAGERS ]; // 0xa0
	TINT            m_iUnk104;                        // 0x104
};
