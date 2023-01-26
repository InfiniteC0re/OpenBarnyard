#pragma once
#include "AMoviePlayer.h"

#include <theora/theoradec.h>

class ADX11MoviePlayer : public AMoviePlayer
{
public:
	static constexpr size_t MAX_FILE_NAME = 256;

public:
	virtual void PlayMovie(const char* fileName, void* unused, uint8_t flags) override;
	virtual void StopMovie() override;
	virtual void PauseMovie() override;
	virtual bool IsMoviePlaying() override;
	virtual bool IsMoviePaused() override;
	virtual void OnRender(float deltaTime) override;

private:
	void StopMovieImpl()
	{
		TIMPLEMENT();
	}

private:
	char m_CurrentFileName[MAX_FILE_NAME + 1]; // 0x047
	//th_info m_TheoraInfo;                      // 0x164
	FILE* m_pFile;                             // 0x16C
};