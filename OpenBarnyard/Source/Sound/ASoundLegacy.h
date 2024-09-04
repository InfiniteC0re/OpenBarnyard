#pragma once

#define ASOUNDID_INVALID -1

using ASoundId = TINT32;

class AWaveBank;

// Note: seems to be deprecated since never actually created or played
class TDEPRECATED ASoundLegacy
{
public:
	friend class ASoundManager;

public:
private:
	TINT       m_iId;
	TINT       m_iFlags;
	AWaveBank* m_pWaveBank;
	ASoundId   m_iWaveId;
};
