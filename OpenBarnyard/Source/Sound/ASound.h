#pragma once

using ASoundWaveId = TINT32;

class AWaveBank;

// Note: seems to be deprecated since never actually created or played
class TDEPRECATED ASound
{
public:
	friend class ASoundManager;

public:
private:
	TINT         m_iId;
	TINT         m_iFlags;
	AWaveBank*   m_pWaveBank;
	ASoundWaveId m_iWaveId;
};
