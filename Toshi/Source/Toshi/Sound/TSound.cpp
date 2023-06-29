#include "ToshiPCH.h"
#include "TSound.h"
#include "Platform/Windows/TSound_Win.h"
#include <fmod/core/fmod_errors.h>

namespace Toshi
{
    TBOOL TSound::Create(void* poolmem, int poollen, int maxchannels, int filebuffersize, SpeakerType speakerType)
    {
        if (m_pSystem != NULL)
        {
            TOSHI_ERROR("TSound::Create() - FMOD system has already been created. Only one FMOD system can be created at one time.\n");
            return TFALSE;
        }
        m_SoundInitValues.m_pPoolmem = poolmem;
        m_SoundInitValues.m_iPoolSize = poollen;

        InitMem(poolmem, poollen);

        FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
        if (ErrorCheck(result))
        {
            TOSHI_ERROR("TSound::Create() - Failed to create FMOD System");
            return TFALSE;
        }

        m_SoundInitValues.m_iMaxChannels = maxchannels;
        m_SoundInitValues.m_eSpeakerType = speakerType;

        TBOOL bInitialiseResult = ((TSound_Win*)this)->Initialise(maxchannels, speakerType);

        TASSERT(TTRUE == bInitialiseResult);

        if (filebuffersize < 0) filebuffersize = 0x20000;

        m_SoundInitValues.m_ifileBufferSize = filebuffersize;

        m_pSystem->setStreamBufferSize(filebuffersize, FMOD_TIMEUNIT_RAWBYTES);

        return TTRUE;
    }

    TBOOL TSound::ErrorCheck(FMOD_RESULT error)
    {
        if (error != FMOD_OK)
        {
            const char* errorStr = FMOD_ErrorString(error);
            TOSHI_ERROR("TSound::ErrorCheck() - FMOD Error: %s", errorStr);
            return TTRUE;
        }
        return TFALSE;
    }

    TBOOL TSound::InitMem(void* poolmem, int poollen)
    {
        if (poolmem == NULL)
        {
            if (poollen < 0)
            {
                poollen = 0x400000;
            }
            poolmem = malloc(poollen);
        }

        FMOD_RESULT eResult = FMOD::Memory_Initialize(poolmem, poollen, NULL, NULL, NULL);

        if (eResult != FMOD_OK)
        {
            TOSHI_ERROR("FMOD: Failed to Initialise Memory");
            return TFALSE;
        }

        return TTRUE;
    }

}