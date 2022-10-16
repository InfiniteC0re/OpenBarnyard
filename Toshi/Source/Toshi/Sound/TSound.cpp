#include "ToshiPCH.h"
#include "TSound.h"
#include "fmod_errors.h"

bool Toshi::TSound::Create(void* poolmem, int poollen, int maxchannels, int filebuffersize, int unk)
{
    if (m_pSystem == NULL)
    {
        TOSHI_ERROR("TSound::Create() - FMOD system has already been created. Only one FMOD system can be created at one time.\n");
        return false;
    }

    m_poolmem = poolmem;
    m_poollen = poollen;

    InitMem(poolmem, poollen);

    FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
    if (!ErrorCheck(result))
    {
        TOSHI_ERROR("TSound::Create() - Failed to create FMOD System");
        return false;
    }

    m_maxchannels = maxchannels;
    m_unk = unk;

    //bool bInitialiseResult = ((TSound_Win*)this)->Initialise(maxchannels, unk);

    //TASSERT(TTRUE == bInitialiseResult, "");
    
    if (filebuffersize < 0) filebuffersize = 0x20000;

    m_fileBufferSize = filebuffersize;

    m_pSystem->setStreamBufferSize(filebuffersize, FMOD_TIMEUNIT_RAWBYTES);

    return true;
}

bool Toshi::TSound::ErrorCheck(FMOD_RESULT error)
{
    if (error != FMOD_OK)
    {
        const char* errorStr = FMOD_ErrorString(error);
        TOSHI_ERROR("TSound::ErrorCheck() - FMOD Error: %s", errorStr);
        return true;
    }
    return false;
}

bool Toshi::TSound::InitMem(void* poolmem, int poollen)
{
    if (poolmem == NULL)
    {
        if (poollen < 0)
        {
            poollen = 0x400000;
        }
        poolmem = malloc(poollen);
    }

    FMOD_RESULT result = FMOD::Memory_Initialize(poolmem, poollen, NULL, NULL, NULL);

    if (result != FMOD_OK)
    {
        TOSHI_ERROR("FMOD: Failed to Initialise Memory");
        return false;
    }

    return true;
}
