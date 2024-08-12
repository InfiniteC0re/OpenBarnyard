#pragma once
#include "AOptions.h"
#include "Render/ARenderer.h"

#include <Toshi/TTask.h>
#include <Toshi/TString8.h>

class ARootTask :
    public Toshi::TTask,
    public Toshi::TSingleton< ARootTask >
{
public:
    TDECLARE_CLASS( ARootTask, Toshi::TTask );

public:
    ARootTask();

    virtual TBOOL OnCreate() override;
    virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
    virtual void  OnChildDied( Toshi::TClass* a_pClass, Toshi::TTask* a_pDeletedTask ) override;
    virtual void  OnActivate() override;
    virtual void  OnDeactivate() override;

    void CreateStartupGameStates();
    void LoadStartupData();

    TBOOL IsPaused() const;
    TBOOL IsGameSystemCreated() const { return m_bGameSystemCreated; }

    void  SetRenderWorld( TBOOL a_bReady ) { m_bRenderWorld = a_bReady; }
    TBOOL IsRenderWorld() { return m_bRenderWorld; }

    TBOOL ShouldRenderMainScene() const { return !m_bStopRenderingScene; }

private:
    void        CreateGameSystem();
    static void LoadFrontEnd();

private:
    AOptions*  m_pOptions;             // 0x20
    TTask*     m_pGUISystem;           // 0x24
    TTask*     m_pGUI2;                // 0x28
    TTask*     m_pInputHandler;        // 0x2C
    ARenderer* m_pRenderer;            // 0x30
    TTask*     m_pGameStateController; // 0x34
    TTask*     m_pMoviePlayer;         // 0x3C
    TTask*     m_pGameSystemManager;   // 0x40
    TTask*     m_pSoundManager;        // 0x44
    TBOOL      m_bRenderWorld;         // 0x54
    TBOOL      m_bPaused;              // 0x55
    TBOOL      m_bStopRenderingScene;  // 0x56
    TBOOL      m_bGameSystemCreated;   // 0x57
    TBOOL      m_bStartedGame;
};
