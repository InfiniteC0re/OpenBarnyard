#pragma once

TOSHI_NAMESPACE_START

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
	TUINT64 m_iFrequency;      // 0x00
	TFLOAT  m_fCurrentSeconds; // 0x08
	TFLOAT  m_Unused;          // 0x0C
	TUINT64 m_iOldTime;        // 0x10
	TUINT64 m_iCurrentTime;    // 0x18
	TFLOAT  m_fDelta;          // 0x20
};

TOSHI_NAMESPACE_END
