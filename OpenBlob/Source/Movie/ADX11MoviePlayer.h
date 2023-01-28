#pragma once
#include "AMoviePlayer.h"
#include <Platform/Windows/DX11/Includes.h>
#include <theoraplay/theoraplay.h>
#include <fmod/fmod.hpp>

class ADX11MoviePlayer : public AMoviePlayer
{
public:
	static constexpr size_t MAX_FILE_NAME = 256;

public:
	ADX11MoviePlayer();

	virtual void PlayMovie(const char* fileName, void* unused, uint8_t flags) override;
	virtual void StopMovie() override;
	virtual void PauseMovie() override;
	virtual bool IsMoviePlaying() override;
	virtual bool IsMoviePaused() override;
	virtual void OnRender(float deltaTime) override;
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnCreate() override;

	static ADX11MoviePlayer* Create()
	{
		return new ADX11MoviePlayer();
	}
	void ReadBuffer(void* data, uint32_t datalen);

private:
	void StopMovieImpl();
	
	void CreateTextures(UINT width, UINT height);
	void ReleaseTextures();
	void CompileShader();

	bool IsMovieLooping() const
	{
		return m_bIsMovieLooping;
	}

private:
	ID3D11ShaderResourceView* m_Textures[6];    // 0x018 
	UINT m_iTexIndex;                           // 0x01C
	ID3D11PixelShader* m_pPixelShader;          // 0x034
	ID3D11VertexShader* m_pVertexShader;        // 0x038
	ID3D11Buffer* m_pBuffer;                    // 0x03C
	ID3D11InputLayout* m_pInputLayout;          // 0x040
	bool m_bIsPlaying;                          // 0x044
	bool m_bIsPaused;                           // 0x045
	bool m_bIsMovieLooping;                     // 0x046
	char m_CurrentFileName[MAX_FILE_NAME + 1];  // 0x047
	UINT m_FrameMS;                             // 0x158
	UINT m_TexturesWidth;                       // 0x15C
	UINT m_TexturesHeight;                      // 0x160
	FILE* m_pFile;                              // 0x16C
	THEORAPLAY_Decoder* m_TheoraDecoder;        // 0x170
	CONST THEORAPLAY_VideoFrame* m_TheoraVideo; // 0x174
	bool m_bHasAudioStream;                     // 0x178
	FMOD::Channel* m_pChannel;                  // 0x17C
	THEORAPLAY_AudioPacket* m_TheoraAudio;      // 0x180
	int m_Unk;                                  // 0x184
	double m_Position;                          // 0x188
};