#pragma once

class APlayerProgress
	: public Toshi::TSingleton<APlayerProgress>
{
public:
	APlayerProgress();
	~APlayerProgress();

	TBOOL IsMiniGameAvailable( TINT a_iIndex ) const { return ( m_eMiniGameFlags & ( 1 << a_iIndex ) ) != 0; }

private:
	// ...
	TUINT m_eMiniGameFlags; // 0x28
};

TSINGLETON_DECLARE_ALIAS( APlayerProgress, PlayerProgress );
