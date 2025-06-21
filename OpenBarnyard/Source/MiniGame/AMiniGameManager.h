#pragma once
#include "AMiniGame.h"

#include <Toshi/TTask.h>
#include <Toshi/TPString8.h>
#include <ToshiTools/T2DynamicVector.h>

class AMiniGameManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AMiniGameManager>
{
public:
	TDECLARE_CLASS( AMiniGameManager, Toshi::TTask );

public:
	AMiniGameManager();
	~AMiniGameManager();

	AMiniGame& RegisterMiniGame(
	    const TCHAR*            a_szMiniGameName,
	    const TCHAR*            a_szClassStateName,
	    const TCHAR*            a_szIconName,
	    TINT                    a_eLocaleTitle,
	    TINT                    a_eLocaleDescription,
	    TINT                    a_iUnk1,
	    const Toshi::TPString8& a_strLoadingScreenName,
	    TBOOL                   a_bUnk2,
	    TINT                    a_iUnk3,
	    TINT                    a_iUnk4,
	    TINT                    a_iUnk5,
	    TINT                    a_iUnk6,
	    TINT                    a_iUnk7
	);

	AMiniGame& RegisterMiniGame(
	    const TCHAR*            a_szMiniGameName,
	    const TCHAR*            a_szClassStateName,
	    const TCHAR*            a_szIconName,
	    TINT                    a_eLocaleTitle,
	    TINT                    a_eLocaleDescription,
	    TINT                    a_iUnk1,
	    const Toshi::TPString8& a_strLoadingScreenName,
	    TINT                    a_iUnk2,
	    TINT                    a_iUnk3
	);

private:
	Toshi::T2DynamicVector<AMiniGame> m_vecMiniGames;
	Toshi::TPString8                  m_strNextMiniGameName;
	// ...
	TINT m_iNextMiniGameVariant;
	// ...
	TINT        field611_0x27c;
	Toshi::TTRB field612_0x280;
};
