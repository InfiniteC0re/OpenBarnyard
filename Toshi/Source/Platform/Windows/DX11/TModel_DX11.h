#pragma once

#include "Toshi2/T2ResourceManager.h"

namespace Toshi
{

	struct TTMDWin
	{
		struct TTRBWinHeader
		{
			char* m_txsModelName;
			uint32_t m_version;
			uint32_t m_remainingBytes;
		};
	};

	class TModel
	{
		void CreateResource(const char* a_resourceName);
		//bool Create(const char* name, bool a_bLoadImmediately);
	};

	class TModelHAL
	{
		TTMDWin::TTRBWinHeader* m_tmdHeader; // 0xBC
		char* m_txsModelName; // 0xDC

	public:
		void Create(TTMDWin::TTRBWinHeader* a_tmdHeader);
	};
}
