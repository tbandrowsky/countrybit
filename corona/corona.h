#pragma once

#define WINDESKTOP_GUI 1

#include "windows.h"
#include "windowsx.h"
#include "process.h"
#include "combaseapi.h"
#include "commctrl.h"
#include "commdlg.h"
#include "Shlwapi.h"
#include "Shlobj.h"
#include "Winhttp.h"
#include "shellapi.h"
#include <io.h>
#include <fcntl.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
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
#include "sys/stat.h"

#include <cstddef>  // For std::ptrdiff_t
#include <vector>
#include <functional>
#include <cassert>
#include <coroutine>
#include <string>
#include <string_view>
#include <map>
#include <charconv>
#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <bit>
#include <cctype>
#include <format>
#include <cstdint>
#include <cmath>
#include <numbers>
#include <vector>
#include <list>
#include <algorithm>
#include <thread>
#include <atomic>
#include <filesystem>
#include <stdexcept>
#include <format>
#include <ctime>
#include <stack>

#include "assert_if.h"
#include "constants.h"
#include "store_box.h"
#include "filterable_iterator.h"
#include "string_box.h"
#include "array_box.h"
#include "table.h"
#include "list_box.h"
#include "sorted_index.h"
#include "grouping.h"

#include "read_all_string.h"
#include "messages.h"
#include "queue.h"
#include "function.h"
#include "file.h"
#include "application.h"
#include "directory.h"

#include "remote_box.h"
#include "jfield.h"
#include "int_box.h"
#include "float_box.h"
#include "point_box.h"
#include "time_box.h"
#include "rectangle_box.h"
#include "layout_rect_box.h"
#include "collection_id_box.h"
#include "object_id_box.h"
#include "image_box.h"
#include "wave_box.h"
#include "midi_box.h"
#include "color_box.h"
#include "sql_remote_box.h"
#include "http_remote_box.h"
#include "file_remote_box.h"
#include "sorted_index.h"
#include "query_box.h"
#include "visual.h"
#include "extractor.h"
#include "jvalue.h"
#include "jobject.h"
#include "jquery.h"
#include "analytics.h"
#include "presentation.h"
#include "jdatabase.h"
#include "jdatabase_client.h"
#include "jdatabase_server.h"
#include "loader.h"

#ifdef WINDESKTOP_GUI

#include "utility.h"
#include "datatransfer.h"
#include "controllerhost.h"
#include "controller.h"
#include "directapplication.h"

const int WM_SWITCH_CONTROLLER = WM_APP + 101;
const int WM_PUSH_CONTROLLER = WM_APP + 102;
const int WM_POP_CONTROLLER = WM_APP + 103;

#endif
