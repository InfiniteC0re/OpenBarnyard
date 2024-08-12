#pragma once
#include "ASound.h"
#include "Memory/AMemory.h"

#include <Toshi/TBitArray.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

class AWaveBank;

class ASoundAdvanced
{
public:
    friend class ASoundManager;

    struct Wave
    {
        AWaveBank* m_pWaveBank            = TNULL;
        TINT       m_iId                  = -1;
        TFLOAT     m_fVolume              = 1.0f;
        TFLOAT     m_fMinVolumeMultiplier = 0.0f;
        TFLOAT     m_fMaxVolumeMultiplier = 0.0f;
        TFLOAT     m_fPitch               = 1.0f;
        TFLOAT     m_fMinPitch            = 0.0f;
        TFLOAT     m_fMaxPitch            = 0.0f;
        TINT       m_iFrequency           = 44100;
        TFLOAT     m_fStart               = 0.0f;
        TFLOAT     m_fVarDelay            = 0.0f;
        TINT       m_iWeight              = 50;
    };

public:
    ASoundAdvanced() :
        m_iFlags( 0 ),
        m_iId( 0 ),
        m_uiCategoryIndex( 0 ),
        m_ui8Priority( 255 ),
        m_fMinDist( 1.0f ),
        m_vecWaves( AMemory::GetAllocator( AMemory::POOL_Sound ), 1, 1 ),
        m_vecTracks( AMemory::GetAllocator( AMemory::POOL_Sound ), 1, 1 )
    {
    }


private:
    TINT                            m_iFlags;
    TINT                            m_iId;
    TINT                            m_uiCategoryIndex;
    TUINT8                          m_ui8Priority;
    TFLOAT                          m_fMinDist;
    Toshi::T2DynamicVector< Wave >  m_vecWaves;
    Toshi::T2DynamicVector< TUINT > m_vecTracks;
    Toshi::TBitArray                m_TrackLoop;
    Toshi::T2Vector< TINT, 15 >     m_vecLoopStarts;
};
