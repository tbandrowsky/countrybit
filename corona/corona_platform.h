#pragma once

#define _WIN32_WINNT _WIN32_WINNT_WIN10 
#define NTDDI_VERSION NTDDI_WIN10_19H1 

#include "windows.h"
#include "windowsx.h"
#include "combaseapi.h"
#include "Winhttp.h"
#include "shellapi.h"
#include "oleauto.h"
#include "dwmapi.h"
#include "commctrl.h"
#include "commdlg.h"
#include "Shlwapi.h"
#include "Shlobj.h"

#include <tchar.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <clocale>
#include <locale>
