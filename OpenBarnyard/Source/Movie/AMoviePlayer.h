#pragma once
#include <Toshi/TTask.h>

class AMoviePlayer : public Toshi::TTask, public Toshi::TSingleton< AMoviePlayer >
{
public:
    TDECLARE_CLASS( AMoviePlayer, Toshi::TTask );

public:
    AMoviePlayer();

    virtual TBOOL OnCreate() override;
    virtual void  OnDestroy() override;

    virtual TBOOL PlayMovie( const TCHAR* a_szFileName, TUINT32 a_Unk ) = 0;
    virtual void  StopMovie()                                           = 0;
    virtual void  PauseMovie( TBOOL a_bPause )                          = 0;
    virtual void  Mute( TBOOL a_bMute = TTRUE ) { m_bIsMuted = a_bMute; }
    virtual TBOOL IsMoviePlaying() = 0;
    virtual TBOOL IsMoviePaused()  = 0;
    virtual TBOOL IsFullScreen() { return m_bIsFullscreen; }
    virtual void  SetHideMovie( TBOOL a_bHide = TTRUE ) { m_bIsHidden = a_bHide; }
    virtual void  SetFullScreen( TBOOL a_bFullscreen = TTRUE ) { m_bIsFullscreen = a_bFullscreen; }
    virtual void  Render( TFLOAT a_fDeltaTime ) = 0;
    virtual void  Unk1( void* a_Unk ) {}
    virtual void  Unk2( void* a_Unk ) {}

private:
    Toshi::TEmitter< AMoviePlayer, TBOOL > m_SomeEmitter;
    TBOOL                                  m_bCreated;
    TBOOL                                  m_bIsFullscreen;
    TBOOL                                  m_bIsMuted;
    TBOOL                                  m_bIsHidden;
};
