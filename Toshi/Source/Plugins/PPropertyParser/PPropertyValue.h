#pragma once
#include "Toshi/TClass.h"
#include "PProperties.h"

class PPropertyValue
{
public:
	inline static const Toshi::TClass* TYPE_INT       = &TGetClass( PProperties::INT );
	inline static const Toshi::TClass* TYPE_UINT32    = &TGetClass( PProperties::UINT32 );
	inline static const Toshi::TClass* TYPE_FLOAT     = &TGetClass( PProperties::FLOAT );
	inline static const Toshi::TClass* TYPE_BOOL      = &TGetClass( PProperties::BOOL );
	inline static const Toshi::TClass* TYPE_TPSTRING8 = &TGetClass( PProperties::TPString8 );
	inline static const Toshi::TClass* TYPE_STRING16  = &TGetClass( PProperties::String16 );
	inline static const Toshi::TClass* TYPE_STRING16A = &TGetClass( PProperties::String16A );
	inline static const Toshi::TClass* TYPE_LOCALEID  = &TGetClass( PProperties::LocaleID );
	inline static const Toshi::TClass* TYPE_PROPS     = &TGetClass( PProperties );
	inline static const Toshi::TClass* TYPE_PROPNAME  = &TGetClass( PProperties::PPropertyName );
	inline static const Toshi::TClass* TYPE_ARRAY     = &TGetClass( PProperties::PPropertyValueArray );
	inline static const Toshi::TClass* TYPE_UNDEF     = TNULL;
};
