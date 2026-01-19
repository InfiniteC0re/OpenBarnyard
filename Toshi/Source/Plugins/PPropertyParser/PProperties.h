#pragma once
#include <Toshi/TObject.h>

class PProperties
    : public Toshi::TObject
{
public:
	struct UINT32
	{
		TDECLARE_CLASS_NO_PARENT( UINT32 );
	};

	struct INT
	{
		TDECLARE_CLASS_NO_PARENT( INT );
	};

	struct FLOAT
	{
		TDECLARE_CLASS_NO_PARENT( FLOAT );
	};

	struct BOOL
	{
		TDECLARE_CLASS_NO_PARENT( BOOL );
	};

	struct TPString8
	{
		TDECLARE_CLASS_NO_PARENT( TPString8 );
	};

	struct String16
	{
		TDECLARE_CLASS_NO_PARENT( String16 );
	};

	struct String16A
	{
		TDECLARE_CLASS_NO_PARENT( String16 );
	};

	struct LocaleID
	{
		TDECLARE_CLASS_NO_PARENT( LocaleID );
	};

	struct PPropertyName
	{
		TDECLARE_CLASS_NO_PARENT( PPropertyName );
	};

	struct PPropertyValueArray
	{
		TDECLARE_CLASS_NO_PARENT( PPropertyValueArray );
	};

public:
	PProperties();
	~PProperties();
};
