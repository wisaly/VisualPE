// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>
#include <OleCtl.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: 在此处引用程序需要的其他头文件
#include "..\..\duilib\include\UIlib.h"

using namespace DuiLib;

#ifdef _DEBUG
# ifdef _UNICODE
# pragma comment(lib, "..\\..\\duilib\\lib\\Duilib_ud.lib")
# else # pragma comment(lib, "..\\..\\duilib\\lib\\Duilib_d.lib")
# endif
#else
# ifdef _UNICODE
# pragma comment(lib, "..\\..\\duilib\\lib\\Duilib_u.lib")
# else
# pragma comment(lib, "..\\..\\duilib\\lib\\Duilib.lib")
# endif
#endif

namespace std
{
#ifdef _UNICODE
#define tstring string
#else
#define tstring wstring
#endif
}



#include <vector>
#include <string>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/weak_ptr.hpp>
using namespace boost;