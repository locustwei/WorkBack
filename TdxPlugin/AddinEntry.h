#pragma once

#include "stdafx.h"

BOOL InstallHooks(DWORD tid);
void UnInstallHooks();
int InitOnMainThread(LPARAM param);