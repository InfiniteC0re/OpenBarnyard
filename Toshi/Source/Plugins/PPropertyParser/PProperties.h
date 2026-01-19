#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TFreeList.h>

class PProperties
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( PProperties, Toshi::TObject );
	TDECLARE_FREELIST_ALLOCATOR( PProperties );

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
		TDECLARE_CLASS_NO_PARENT( String16A );
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
