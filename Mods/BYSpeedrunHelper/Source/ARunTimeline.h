#pragma once
#include <ToshiTools/T2DynamicVector.h>

class ARunTimeline
{
public:
	enum class EventType : TUINT64
	{
		Stop,
		LoadStart,
		LoadEnd,
		StoryProgress,
		LevelTransition,
		PlayerPosition,
		Split
	};

	struct Event
	{
		TUINT64   uiTime : 55;
		TUINT64   bData : 1;
		EventType eType : 8;
		TUINT16   uiDataSize;
	};

public:
	ARunTimeline();
	~ARunTimeline();

	void Reset();
	void Start();
	void AddEvent( EventType a_eEventType, void* a_pData = TNULL, TUINT16 a_uiDataSize = 0 );
	void Stop();

	void SaveToFile( const TCHAR* a_pchBaseName );
	void GetDuration( TDOUBLE& a_rDuration, TDOUBLE& a_rDurationNoLoads, TBOOL a_bForceLiveTime = TFALSE );

private:
	Toshi::T2DynamicVector<Event> m_vecEvents;
	Toshi::T2DynamicVector<TBYTE> m_vecEventsData;

	TUINT64 m_uiStartTime;
	TUINT64 m_uiLastEventTime;
	TUINT64 m_uiDuration;
	TUINT64 m_uiDurationNoLoads;

	TBOOL m_bIsLive;
};
