#pragma once

inline XMIVECTOR XM_CALLCONV operator<< (XMIVECTOR V, int S)
{
	return _mm_slli_epi32(V, S);
};

inline XMIVECTOR XM_CALLCONV operator>> (XMIVECTOR V, int S)
{
	return _mm_srli_epi32(V, S);
};

inline XMIVECTOR XM_CALLCONV operator& (XMIVECTOR V, XMIVECTOR S)
{
	return _mm_and_si128(V, S);
};

inline XMIVECTOR XM_CALLCONV operator| (XMIVECTOR V, XMIVECTOR S)
{
	return _mm_or_si128(V, S);
};

inline XMIVECTOR XM_CALLCONV operator^ (XMIVECTOR V, XMIVECTOR S)
{
	return _mm_xor_si128(V, S);
};

inline bool		 XM_CALLCONV	operator== (XMIVECTOR V1, XMIVECTOR V2)
{
	return XMVector3Equal(V1, V2);
};

inline bool		 XM_CALLCONV	operator!= (XMIVECTOR V1, XMIVECTOR V2)
{
	return !XMVector3Equal(V1, V2);
};

inline XMIVECTOR&   XM_CALLCONV     operator+= (XMIVECTOR& V1, XMIVECTOR V2)
{
	return V1 = _mm_add_epi32(V1, V2);
};

inline XMIVECTOR&   XM_CALLCONV     operator-= (XMIVECTOR& V1, XMIVECTOR V2)
{
	return V1 = _mm_sub_epi32(V1, V2);
};

inline XMIVECTOR&   XM_CALLCONV     operator*= (XMIVECTOR& V1, XMIVECTOR V2)
{
	return V1 = _mm_mul_epi32(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV     operator+ (XMIVECTOR V1, XMIVECTOR V2)
{
	return _mm_add_epi32(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV     operator- (XMIVECTOR V1, XMIVECTOR V2)
{
	return _mm_sub_epi32(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV     operator* (XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_mullo_epi32(V1, V2);
};

inline int			XM_CALLCONV		XMVectorGetX(XMIVECTOR V1)
{
	return V1.m128i_i32[0];
};

inline int			XM_CALLCONV		XMVectorGetY(XMIVECTOR V1)
{
	return V1.m128i_i32[1];
};

inline int			XM_CALLCONV		XMVectorGetZ(XMIVECTOR V1)
{
	return V1.m128i_i32[2];
};

inline XMIVECTOR	 XM_CALLCONV	XMVectorLoadSInt3(const XMINT3* V1)
{
	__declspec(align(16)) int32_t i[4];
	memcpy(i, V1, sizeof(XMINT3));
	i[3] = 0;
	return _mm_load_si128((__m128i*)i);
};

inline XMIVECTOR	 XM_CALLCONV	XMVectorReplicateSInt(int i)
{
	return _mm_set1_epi32(i);
};

inline XMIVECTOR	 XM_CALLCONV	XMVectorSetSInt(int i0, int i1, int i2, int i3)
{
	return _mm_setr_epi32(i0, i1, i2, i3);
};

inline void XM_CALLCONV	XMVectorStoreInt3(XMINT3* V1, XMIVECTOR V2)
{
	__declspec(align(16)) int32_t i[4];
	_mm_storeu_si128((__m128i*)i, V2);
	memcpy(V1, i, sizeof(XMINT3));
};

inline void XM_CALLCONV	XMVectorStoreInt4(int32_t* V1, XMIVECTOR V2)
{
	_mm_storeu_si128((__m128i*)V1, V2);
};


//inline XMIVECTOR	 XM_CALLCONV	XMVectorConvert(XMVECTOR V1)
//{
//	return _mm_cvtps_epi32(XMVectorFloor(V1));
//};
//
//inline XMVECTOR	 XM_CALLCONV	XMVectorConvert(XMIVECTOR V1)
//{
//	return _mm_cvtepi32_ps(V1);
//};

inline XMIVECTOR	 XM_CALLCONV	XMVectorAdd(XMIVECTOR V1, XMIVECTOR V2)
{
	return _mm_add_epi32(V1, V2);
};

inline XMIVECTOR	 XM_CALLCONV	XMVectorSubtract(XMIVECTOR V1, XMIVECTOR V2)
{
	return _mm_sub_epi32(V1, V2);
};

inline XMIVECTOR	 XM_CALLCONV	XMVectorMultiply(XMIVECTOR V1, XMIVECTOR V2)
{
	return _mm_mullo_epi32(V1, V2);
};

inline int    XM_CALLCONV	XMVectorHorizontalAdd(XMIVECTOR V1)
{
	XMIVECTOR vResult = _mm_hadd_epi32(V1, V1);
	vResult = _mm_hadd_epi32(vResult, vResult);
	return vResult.m128i_i32[0];
};

inline int    XM_CALLCONV	XMVectorDot(XMIVECTOR V1, XMIVECTOR V2)
{
	XMIVECTOR vResult = _mm_sub_epi32(V1, V2);
	vResult = _mm_mullo_epi32(vResult, vResult);
	vResult = _mm_hadd_epi32(vResult, vResult);
	vResult = _mm_hadd_epi32(vResult, vResult);
	return vResult.m128i_i32[0];
};

inline int    XM_CALLCONV	XMVectorDot(XMIVECTOR V1)
{
	XMIVECTOR vResult = _mm_mullo_epi32(V1, V1);
	vResult = _mm_hadd_epi32(vResult, vResult);
	vResult = _mm_hadd_epi32(vResult, vResult);
	return vResult.m128i_i32[0];
};

inline XMIVECTOR    XM_CALLCONV	XMVectorLess(XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_cmplt_epi32(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorGreater(XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_cmpgt_epi32(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorEqual(XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_cmpeq_epi32(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorAnd(XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_and_si128(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorNand(XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_andnot_si128(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorOr(XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_or_si128(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorXor(XMIVECTOR V1, XMIVECTOR V2)
{
	return  _mm_xor_si128(V1, V2);
};


inline bool		 XM_CALLCONV	XMVector3Less(XMIVECTOR V1, XMIVECTOR V2)
{
	XMIVECTOR vTemp = _mm_cmplt_epi32(V1, V2);
	return (((_mm_movemask_epi8(vTemp) & 0xFFF) == 0xFFF) != 0);
};

inline bool		 XM_CALLCONV	XMVector3Greater(XMIVECTOR V1, XMIVECTOR V2)
{
	XMIVECTOR vTemp = _mm_cmpgt_epi32(V1, V2);
	return (((_mm_movemask_epi8(vTemp) & 0xFFF) == 0xFFF) != 0);
};

inline bool		 XM_CALLCONV	XMVector3Equal(XMIVECTOR V1, XMIVECTOR V2)
{
	XMIVECTOR vTemp = _mm_cmpeq_epi32(V1, V2);
	return (((_mm_movemask_epi8(vTemp) & 0xFFF) == 0xFFF) != 0);
};

inline bool		 XM_CALLCONV	XMVector3NotEqual(XMIVECTOR V1, XMIVECTOR V2)
{
	XMIVECTOR vTemp = _mm_cmpeq_epi32(V1, V2);
	return (((_mm_movemask_epi8(vTemp) & 0xFFF) == 0xFFF) == 0);
};

inline bool		 XM_CALLCONV	XMVector3AllTrue(XMIVECTOR V1)
{
	return (((_mm_movemask_epi8(V1) & 0xFFF) == 0xFFF) != 0);
};

inline bool		 XM_CALLCONV	XMVector3AnyTrue(XMIVECTOR V1)
{
	return (_mm_movemask_epi8(V1) != 0);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorMin(XMIVECTOR V1, XMIVECTOR V2)
{
	return _mm_min_epi32(V1, V2);
};

inline XMIVECTOR    XM_CALLCONV	XMVectorMax(XMIVECTOR V1, XMIVECTOR V2)
{
	return _mm_max_epi32(V1, V2);
};


inline XMIVECTOR	 XM_CALLCONV	XMVectorSelect(XMIVECTOR V1, XMIVECTOR V2, XMIVECTOR Control)
{
	XMIVECTOR vTemp1 = _mm_andnot_si128(Control, V1);
	XMIVECTOR vTemp2 = _mm_and_si128(V2, Control);
	return _mm_or_si128(vTemp1, vTemp2);
};
