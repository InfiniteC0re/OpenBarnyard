#pragma once

namespace Toshi
{
	class TError : public TSingleton<TError>
	{
	public:
		TError(uint32_t size1, uint32_t size2);
		~TError();
		
		void AddError(const char* error, ...);
		const char* GetError(uint32_t index) { return index > -1 && index < m_currentSize2 ? &m_Buffer1[m_Buffer2[index]] : "invalid error id"; }

		char* m_Buffer1;            // 0x0
		uint32_t m_size1;           // 0x4
		int* m_Buffer2;             // 0x8
		uint32_t m_size2;           // 0xC
		uint32_t m_currentSize;     // 0x10
		uint32_t m_currentSize2;    // 0x14
	};
}
