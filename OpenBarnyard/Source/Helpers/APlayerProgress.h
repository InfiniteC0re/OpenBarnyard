#pragma once

class APlayerProgress
	: public Toshi::TSingleton<APlayerProgress>
{
public:
	APlayerProgress();
	~APlayerProgress();

private:
	// ...
	TUINT m_eMiniGameFlags; // 0x28
};

TSINGLETON_DECLARE_ALIAS( APlayerProgress, PlayerProgress );
