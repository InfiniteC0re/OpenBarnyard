#include "ToshiPCH.h"
#include "TSound_Win.h"

FMOD_SYSTEM_CALLBACK Toshi::TSound_Win::SoundCB = [](FMOD_SYSTEM* system, FMOD_SYSTEM_CALLBACK_TYPE type, void* commanddata1, void* commanddata2, void* userdata)
{
    switch (type)
    {
    case FMOD_SYSTEM_CALLBACK_DEVICELOST:
        TOSHI_ERROR("FMOD reported a device loss. Setting system as uninitialized");
        TSound_Win::GetSingletonWeak()->m_bInitialised = false;
        break;
    }
    return FMOD_OK;
};

bool Toshi::TSound_Win::Initialise(int maxchannels, int unk)
{
    TASSERT(!m_bInitialised, "FMOD system has already been initialised. Only one FMOD system can be initialised at one time.");

    if (unk == 0 || unk == 1)
    {
        m_pSystem->setSoftwareFormat(48000, FMOD_SPEAKERMODE_RAW, 1);
    }
    else if (unk == 2)
    {
        int numOutputDrivers;

        m_pSystem->setSoftwareFormat(48000, FMOD_SPEAKERMODE_7POINT1, 1);

        FMOD_RESULT eResult = m_pSystem->getNumDrivers(&numOutputDrivers);

        TASSERT(!ErrorCheck(eResult));

        if (numOutputDrivers == 0)
        {
            TOSHI_ERROR("No soundcard, initializing to FMOD_OUTPUTTYPE_NOSOUND ");
            m_pSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        }
    }
    
    if (maxchannels < 0) maxchannels = 0x100;

    m_pSystem->setCallback(SoundCB);

    FMOD_RESULT result = m_pSystem->init(maxchannels, FMOD_INIT_MIX_FROM_UPDATE, NULL);

    if (ErrorCheck(result))
    {
        TOSHI_ERROR("TSound::Initialise() - Failed to initialise FMOD System with error code %d", result);
        return false;
    }

    m_bInitialised = true;

    return true;
}