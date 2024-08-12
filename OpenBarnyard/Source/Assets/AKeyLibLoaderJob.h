#pragma once
#include "AKeyframeLibStreamJob.h"

class AKeyLibLoaderJob : public AMainThreadJob
{
public:
    virtual void  BeginJob() override;
    virtual TBOOL RunJob() override;

    void InitJob( const TCHAR* a_szFileName );

    AKeyframeLibStreamJob& GetStream() { return m_oStreamJob; }

protected:
    AKeyframeLibStreamJob     m_oStreamJob;
    Toshi::TKeyframeLibrary** m_ppOutKeyframeLib;
};
