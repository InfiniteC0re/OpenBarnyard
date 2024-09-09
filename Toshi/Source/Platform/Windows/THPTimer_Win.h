#pragma once

TOSHI_NAMESPACE_START

// Using this instead of LARGE_INTEGER because MSVC fucks up structure size after building
typedef union _TLargeInteger
{
	struct
	{
		TUINT32 LowPart;
		TINT32  HighPart;
	} u;

	LARGE_INTEGER& LargeInteger() { return *(LARGE_INTEGER*)this; }
	LONGLONG&      QuadPart() { return *(LONGLONG*)this; }
} TLargeInteger;

class TOSHI_API THPTimer
{
public:
	THPTimer();
	~THPTimer();

	TUINT32 GetRaw32();
	int64_t GetRaw64();

	void Update();
	void Reset();

public:
	TFLOAT GetDelta() const { return m_fDelta; }
	TFLOAT GetCurrentSeconds() const { return m_fCurrentSeconds; }

private:
	TLargeInteger m_iFrequency;      // 0x00
	TFLOAT        m_fCurrentSeconds; // 0x08
	TFLOAT        m_Unused;          // 0x0C
	TLargeInteger m_iOldTime;        // 0x10
	TLargeInteger m_iCurrentTime;    // 0x18
	TFLOAT        m_fDelta;          // 0x20
};

TOSHI_NAMESPACE_END
