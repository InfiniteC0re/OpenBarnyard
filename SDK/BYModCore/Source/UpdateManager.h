#pragma once
#include <Toshi/TVersion.h>
#include <Toshi/TString8.h>
#include <Toshi/T2String.h>

namespace UpdateManager
{

struct VersionInfo
{
	Toshi::TVersion uiVersion;
	Toshi::TString8 strUpdateUrl;
};

void CleanUp();

void AskAutoUpdate();

TBOOL CheckVersion( Toshi::T2StringView strUpdateInfoUrl, Toshi::TVersion uiCurrentVersion, VersionInfo* pOutVersionInfo );

}