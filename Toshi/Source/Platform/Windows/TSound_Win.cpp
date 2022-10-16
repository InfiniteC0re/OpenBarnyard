#include "ToshiPCH.h"
#include "TSound_Win.h"

bool Toshi::TSound_Win::Initialise(int maxchannels, int unk)
{
    TASSERT(!m_bInitialised, "FMOD system has already been initialised. Only one FMOD system can be initialised at one time.");

    if (unk == 0 || unk == 1)
    {
        // setspeakmode stuff??
    }
    else if (unk == 2)
    {
        //m_pSystem->getDriverInfo(0, NULL, 0, NULL, NULL, NULL, NULL);

        int numOutputDrivers;

        FMOD_RESULT result = m_pSystem->getNumDrivers(&numOutputDrivers);

        TASSERT(!ErrorCheck(result), "");

        TOSHI_ERROR("No soundcard, initializing to FMOD_OUTPUTTYPE_NOSOUND ");
        m_pSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
    }
    
    if (maxchannels < 0) maxchannels = 0x100;

    FMOD_SYSTEM_CALLBACK callback;

    m_pSystem->setCallback(callback);

    
    FMOD_RESULT result = m_pSystem->init(maxchannels, FMOD_INIT_MIX_FROM_UPDATE, NULL);

    if (!ErrorCheck(result))
    {
        TOSHI_ERROR("TSound::Initialise() - Failed to initialise FMOD System with error code %d", result);
    }

    m_bInitialised = true;

    return false;
}
