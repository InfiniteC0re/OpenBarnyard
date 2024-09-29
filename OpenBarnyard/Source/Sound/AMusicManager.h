#pragma once
#include "ASound.h"

#include <Toshi/TTask.h>

class AMusicManager : public Toshi::TTask
    , public Toshi::TSingleton<AMusicManager>
{
public:
	TDECLARE_CLASS( AMusicManager, Toshi::TTask );

public:
	AMusicManager();
	~AMusicManager() = default;

	void PlayBackgroundMusic( ASoundId a_eSoundId );
	void StopBackgroundMusic();

private:
	// ...
	ASoundId m_eBackgroundMusicCue;
	// ...
};
