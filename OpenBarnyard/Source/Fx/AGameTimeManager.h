#pragma once

class AGameTimeManager :
    public Toshi::TSingleton<AGameTimeManager>
{
public:
	enum DAYPHASE
	{
		DAYPHASE_DAWN,
		DAYPHASE_MORNING,
		DAYPHASE_AFTERNOON,
		DAYPHASE_DUSK,
		DAYPHASE_NIGHT,
		DAYPHASE_NUMOF
	};

public:
	AGameTimeManager();
	~AGameTimeManager();

	DAYPHASE GetCurrentDayPhase() const;
	TBOOL    IsItNight() const;
};
