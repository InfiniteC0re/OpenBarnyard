#pragma once
#include "AMainThreadJob.h"

#include <Render/TAnimation.h>
#include <File/TFileStream.h>

class AKeyframeLibStreamJob : public Toshi::TFileStreamJob
{
public:
	AKeyframeLibStreamJob();
	// $Barnyard: FUNCTION 006065b0
	virtual ~AKeyframeLibStreamJob() = default;

	virtual void Process() override;

	void Init( const TCHAR* a_szFileName );

	Toshi::TKeyframeLibrary* GetLibrary() const { return m_pLibrary; }
	const TCHAR*             GetName() const { return m_szName; }

private:
	Toshi::TKeyframeLibrary* m_pLibrary;
	TCHAR                    m_szName[ 64 ];
};
