
#ifndef CORONA_WINDOWS_ALL
#define CORONA_WINDOWS_ALL

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include "winsock2.h"
#include "windows.h"
#include "windowsx.h"

#include "http.h"
#include "bcrypt.h"
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

#include "richedit.h"
#include "Lmcons.h"

#include "mfapi.h"
#include "mfidl.h"
#include "mfreadwrite.h"

#include "opencv2/opencv.hpp"
#include "ZXing/ReadBarcode.h"

#endif
