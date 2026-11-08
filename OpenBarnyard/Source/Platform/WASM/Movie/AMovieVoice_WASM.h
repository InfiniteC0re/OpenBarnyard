#pragma once

// Movie sound, the mixer in AFMOD_WASM.cpp plays it. Only the movie's thread writes, only the mixer reads. The position
// moves with the mixer even if there's nothing to play yet, which makes it the clock of the picture: late sound is
// dropped and never pushes the movie back.
TBOOL   AMovieVoice_IsMixerRunning();
void    AMovieVoice_Start( TINT a_iSampleRate ); // starts paused
void    AMovieVoice_Stop( TBOOL a_bPlayRemaining );
void    AMovieVoice_Write( const TFLOAT* a_pSamples, TINT a_iChannels, TINT a_iFrames );
void    AMovieVoice_SetPaused( TBOOL a_bPaused );
void    AMovieVoice_SetVolume( TINT a_iVolume ); // 32768 is the original level
void    AMovieVoice_SetSoundOn( TBOOL a_bOn );
TUINT32 AMovieVoice_GetPosition(); // frames at the movie's sample rate
