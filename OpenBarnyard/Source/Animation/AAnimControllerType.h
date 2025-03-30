#pragma once
#include <Toshi/T2DList.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class AAnimControllerType
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimControllerType>::Node
{
public:
	TDECLARE_CLASS( AAnimControllerType, Toshi::TObject );

public:
	AAnimControllerType();
	virtual ~AAnimControllerType();

	virtual TBOOL          Create( const PBProperties* a_pProperties );
	virtual Toshi::TClass* GetControllerClass() = 0;
};
