#pragma once
#include "AFade.h"

#include <Toshi/TTask.h>

class AFadeManager : public Toshi::TTask, public Toshi::TSingleton< AFadeManager >
{
public:
    TDECLARE_CLASS( AFadeManager, Toshi::TTask );

public:
    AFadeManager() = default;

    virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

    virtual void StopAllFades();

    AFade* StartFade( const AFade::Color& a_rFadeFrom, const AFade::Color& a_rFadeTo, TFLOAT a_fFadeTime );
    TBOOL  HasAnyFadesInProgress( TBOOL a_bThrowFadeOver );

private:
    Toshi::T2ObjectPool< AFade, 8 > m_Fades;
    Toshi::TQList< AFade >          m_ActiveFades;
};
