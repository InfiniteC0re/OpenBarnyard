#pragma once

#include "fmod.hpp"
//#include "Platform/Windows/TSound_Win.h"

namespace Toshi
{
	class TSound
	{
	public:
		TSound() {}
		bool Create(void* poolmem, int poollen, int maxchannels, int filebuffersize, int unk);
	protected:
		
		bool ErrorCheck(FMOD_RESULT error);
		bool InitMem(void* poolmem, int poollen);
		inline FMOD::System* GetSystem() { TASSERT(TNULL != m_pSystem, ""); return m_pSystem; }

		FMOD::System* m_pSystem = NULL;
		
		void* m_poolmem;
		int m_poollen;
		int m_maxchannels;
		int m_fileBufferSize;
		int m_unk;
	};
}

