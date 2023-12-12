#pragma once
#include "AMainThreadJob.h"

#include <Toshi/Render/TAnimation.h>
#include <Toshi/File/TFileStream.h>

class AKeyframeLibStreamJob : public Toshi::TFileStreamJob
{
public:
	AKeyframeLibStreamJob();
	virtual void Process() override;

	void Init(const char* a_szFileName);

	Toshi::TKeyframeLibrary* GetLibrary() const { return m_pLibrary; }
	const char* GetName() const { return m_szName; }

private:
	Toshi::TKeyframeLibrary* m_pLibrary;
	char m_szName[64];
};
