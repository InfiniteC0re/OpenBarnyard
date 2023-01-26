#include "pch.h"
#include "ADX11MoviePlayer.h"
#include <ogg/ogg.h>

void ADX11MoviePlayer::PlayMovie(const char* fileName, void* unused, uint8_t flags)
{
    if (IsMoviePlaying())
    {
        StopMovieImpl();
        Toshi::TUtil::MemClear(m_CurrentFileName, MAX_FILE_NAME);
    }

    if (fileName == TNULL)
    {
        return;
    }

    char path[256];
    Toshi::T2String8::CopySafe(m_CurrentFileName, fileName, MAX_FILE_NAME);
    Toshi::T2String8::Format(path, "Data\\Movies\\%s.bik.ogv", fileName);
    m_pFile = fopen(path, "rb");

    if (m_pFile == TNULL)
    {
        m_pFile = fopen(m_CurrentFileName, "rb");
    }

    if (m_pFile != TNULL)
    {
        th_info_init
        //th_decode_alloc()
    }
}

void ADX11MoviePlayer::StopMovie()
{

}

void ADX11MoviePlayer::PauseMovie()
{

}

bool ADX11MoviePlayer::IsMoviePlaying()
{
    return false;
}

bool ADX11MoviePlayer::IsMoviePaused()
{
    return false;
}

void ADX11MoviePlayer::OnRender(float deltaTime)
{

}
