//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0600
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <process.h>

// MSVC COM Support
#include <comip.h>
#include <comdef.h>

#include <gdiplus.h>


#include <wrl/client.h>

#include <dxgi.h>
#include <d3d9.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <D3Dcompiler.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>
#include <wincodec.h>
#pragma warning(pop)

// C++ Standard Library Header Files
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <vector>
#include <cmath>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <complex>
#include <cstdio>
#include <cstdarg>
#include <exception>
#include <iostream>

#include "SimpleMath.h"
#include "AdvancedMath.h"

#include "Typedefs.h"

#include "Debug/DXErr.h"
#include "Debug/PAssert.h"

using namespace DirectX;

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }
}

#define MODELFOLDER "../../../Models/"
#define TEXTUREFOLDER "../../../Models/"
#define SHADERFOLDER "../Shader/"
#define FONTFOLDER "../../../Models/"

#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           { hr = (x); if( FAILED(hr) ) { DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif
#else
#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = nullptr; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p) = nullptr; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = nullptr; } }
#endif