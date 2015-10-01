// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <process.h>

// MSVC COM Support
#include <comip.h>
#include <comdef.h>

#include <wrl/client.h>

// GDI+
#include <gdiplus.h>

// DirectX Includes
#ifdef _DEBUG
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#include <dxgi.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <d3d9.h>

// STL includes
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>
#include <wincodec.h>
#pragma warning(pop)

// C++ Standard Library Header Files
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <complex>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include <array>

#include <thread>
#include <atomic>
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>

#include "SimpleMath.h"
#include "AdvancedMath.h"
#include "Typedefs.h"

#include "PhysicsManager.h"

#define MODELFOLDER "../../../Models/"
#define TEXTUREFOLDER "../../../Models/"
#define SHADERFOLDER "Shader/"

using namespace DirectX;
using namespace SimpleMath;

template <typename T>
inline const T* select(const char* narrow, const wchar_t* wide);

template <>
inline const char* select<char>(const char* narrow, const wchar_t* /*wide*/)
{
	return narrow;
};

template <>
inline const wchar_t* select<wchar_t>(const char* /*narrow*/, const wchar_t* wide)
{
	return wide;
};

#define _L(str) select<charT>(str, L ## str)

#define CHUNK_WIDTH				32
#define CHUNK_HEIGHT			256
#define CHUNK_SQRWIDTH			(CHUNK_WIDTH * CHUNK_WIDTH)
#define CHUNK_WIDTH_INDEX		(CHUNK_WIDTH - 1)
#define CHUNK_HEIGHT_INDEX		(CHUNK_HEIGHT - 1)
#define CHUNK_WIDTH_MASK		CHUNK_WIDTH_INDEX
#define CHUNK_HEIGHT_MASK		CHUNK_HEIGHT_INDEX
#define CHUNK_WIDTH_INVMASK		(~CHUNK_WIDTH_MASK)
#define CHUNK_HEIGHT_INVMASK	(~CHUNK_HEIGHT_MASK)
#define REGION_WIDTH			16
#define REGION_REALWIDTH		(REGION_WIDTH * CHUNK_WIDTH)
#define REGION_SQRWIDTH			(REGION_WIDTH * REGION_WIDTH)
#define REGION_WIDTH_MASK		((REGION_REALWIDTH - 1) ^ CHUNK_WIDTH_MASK)
#define REGION_WIDTH_INVMASK	(~(REGION_REALWIDTH - 1))

#define HASH(p) \
int32 x = p.x & REGION_WIDTH_INVMASK; \
int32 z = p.z & REGION_WIDTH_INVMASK; \
int64 hash = int64(x) + (int64(z) << 32);

inline float clamp(float val, float minval, float maxval)
{
	// Branchless SSE clamp.
	// return minss( maxss(val,minval), maxval );
	_mm_store_ss(&val, _mm_min_ss(_mm_max_ss(_mm_set_ss(val), _mm_set_ss(minval)), _mm_set_ss(maxval)));
	return val;
}

inline double clamp(double val, double minval, double maxval)
{
	// Branchless SSE2 clamp.
	// return minsd( maxsd(val,minval), maxval );
	_mm_store_sd(&val, _mm_min_sd(_mm_max_sd(_mm_set_sd(val), _mm_set_sd(minval)), _mm_set_sd(maxval)));
	return val;
}

struct BLOCK_MESH
{
	Vector3 Position;
	Vector3 Tangent;
	Vector3 Binormal;
	Vector3 Color;
	Vector3 Texture;
	Vector2 TDimension;

	void XM_CALLCONV Transform(_In_ XMMATRIX world);
};

enum CollisionType : uint32
{
	eEntityItem = (1 << 0),
	ePlayer		= (1 << 1),
	eEnemy		= (1 << 2),
	eHasard		= (1 << 3),
};


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