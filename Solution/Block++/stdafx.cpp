// stdafx.cpp : source file that includes just the standard includes
// Block++.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


void XM_CALLCONV BLOCK_MESH::Transform(_In_ XMMATRIX world)
{
	Position = XMVector3TransformCoord(Position, world);
	Tangent = XMVector3TransformNormal(Tangent, world);
	Binormal = XMVector3TransformNormal(Binormal, world);
}