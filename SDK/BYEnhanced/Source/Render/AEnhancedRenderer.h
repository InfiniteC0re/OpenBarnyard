#pragma once
#include <Toshi/TSingleton.h>
#include <Platform/GL/T2Render_GL.h>

class AEnhancedRenderer
    : public Toshi::TSingleton<AEnhancedRenderer>
{
public:
	AEnhancedRenderer() = default;
	~AEnhancedRenderer() = default;

	TBOOL Create();
	TBOOL Update( TFLOAT a_fDeltaTime );

private:
	void InstallHooks();
};
