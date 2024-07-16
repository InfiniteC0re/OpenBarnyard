#pragma once

namespace Toshi {

	// Using this instead of LARGE_INTEGER because MSVC fucks up structure size after building
	typedef union _TLargeInteger {
		struct {
			TUINT32 LowPart;
			TINT32 HighPart;
		} u;

		LARGE_INTEGER& LargeInteger() { return *(LARGE_INTEGER*)this; }
		LONGLONG& QuadPart() { return *(LONGLONG*)this; }
	} TLargeInteger;

	class THPTimer
	{
	public:
		THPTimer()
		{
			Reset();
		}

		TUINT32 GetRaw32()
		{
			LARGE_INTEGER raw32;
			QueryPerformanceCounter( &raw32 );
			return static_cast<TUINT32>( raw32.QuadPart );
		}

		int64_t GetRaw64()
		{
			LARGE_INTEGER raw64;
			QueryPerformanceCounter( &raw64 );
			return raw64.QuadPart;
		}

		void Reset()
		{
			QueryPerformanceCounter( &m_iCurrentTime.LargeInteger() );
			QueryPerformanceFrequency( &m_iFrequency.LargeInteger() );
			m_iOldTime = m_iCurrentTime;
			m_fDelta = 0;
		}

		void Update()
		{
			m_iOldTime = m_iCurrentTime;
			QueryPerformanceCounter( &m_iCurrentTime.LargeInteger() );

			TFLOAT ratio = 1.0f / m_iFrequency.QuadPart();
			m_fDelta = ( m_iCurrentTime.QuadPart() - m_iOldTime.QuadPart() ) * ratio;
			m_fCurrentSeconds = m_iCurrentTime.QuadPart() * ratio;
		}

	public:
		TFLOAT GetDelta() const { return m_fDelta; }
		TFLOAT GetCurrentSeconds() const { return m_fCurrentSeconds; }

	private:
		TLargeInteger m_iFrequency;        // 0x00
		TFLOAT m_fCurrentSeconds;          // 0x08
		TFLOAT m_Unused;                   // 0x0C
		TLargeInteger m_iOldTime;          // 0x10
		TLargeInteger m_iCurrentTime;      // 0x18
		TFLOAT m_fDelta;                   // 0x20
	};

}

