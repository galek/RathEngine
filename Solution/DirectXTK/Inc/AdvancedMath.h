#pragma once

#include <functional>
#include <memory.h>

#include <DirectXMath.h>

namespace DirectX
{
	typedef __m128i XMIVECTOR;

	XMIVECTOR    XM_CALLCONV     operator<< (XMIVECTOR V, int S);
	XMIVECTOR    XM_CALLCONV     operator>> (XMIVECTOR V, int S);

	XMIVECTOR    XM_CALLCONV     operator& (XMIVECTOR V, XMIVECTOR S);
	XMIVECTOR    XM_CALLCONV     operator| (XMIVECTOR V, XMIVECTOR S);
	XMIVECTOR    XM_CALLCONV     operator^ (XMIVECTOR V, XMIVECTOR S);

	bool		 XM_CALLCONV	operator== (XMIVECTOR V, XMIVECTOR S);
	bool		 XM_CALLCONV	operator!= (XMIVECTOR V, XMIVECTOR S);

	XMIVECTOR&   XM_CALLCONV     operator+= (XMIVECTOR& V1, XMIVECTOR V2);
	XMIVECTOR&   XM_CALLCONV     operator-= (XMIVECTOR& V1, XMIVECTOR V2);
	XMIVECTOR&   XM_CALLCONV     operator*= (XMIVECTOR& V1, XMIVECTOR V2);

	XMIVECTOR    XM_CALLCONV     operator+ (XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV     operator- (XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV     operator* (XMIVECTOR V1, XMIVECTOR V2);

	int			 XM_CALLCONV	XMVectorGetX(XMIVECTOR V1);
	int			 XM_CALLCONV	XMVectorGetY(XMIVECTOR V1);
	int			 XM_CALLCONV	XMVectorGetZ(XMIVECTOR V1);

	XMIVECTOR	 XM_CALLCONV	XMVectorLoadSInt3(const XMINT3* V1);
	XMIVECTOR	 XM_CALLCONV	XMVectorReplicateSInt(int i);
	XMIVECTOR	 XM_CALLCONV	XMVectorSetSInt(int i0, int i1, int i2, int i3);
	void		 XM_CALLCONV	XMVectorStoreInt3(XMINT3* V1, XMIVECTOR V2);
	void		 XM_CALLCONV	XMVectorStoreInt4(int32_t* V1, XMIVECTOR V2);

	XMIVECTOR	 XM_CALLCONV	XMVectorAdd(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR	 XM_CALLCONV	XMVectorSubtract(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR	 XM_CALLCONV	XMVectorMultiply(XMIVECTOR V1, XMIVECTOR V2);

	int			 XM_CALLCONV	XMVectorHorizontalAdd(XMIVECTOR V1);
	int			 XM_CALLCONV	XMVectorDot(XMIVECTOR V1, XMIVECTOR V2);
	int			 XM_CALLCONV	XMVectorDot(XMIVECTOR V1);

	XMIVECTOR    XM_CALLCONV	XMVectorLess(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV	XMVectorGreater(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV	XMVectorEqual(XMIVECTOR V1, XMIVECTOR V2);

	XMIVECTOR    XM_CALLCONV	XMVectorAnd(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV	XMVectorNand(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV	XMVectorOr(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV	XMVectorXor(XMIVECTOR V1, XMIVECTOR V2);

	XMIVECTOR    XM_CALLCONV	XMVectorMin(XMIVECTOR V1, XMIVECTOR V2);
	XMIVECTOR    XM_CALLCONV	XMVectorMax(XMIVECTOR V1, XMIVECTOR V2);

	bool		 XM_CALLCONV	XMVector3Less(XMIVECTOR V1, XMIVECTOR V2);
	bool		 XM_CALLCONV	XMVector3Greater(XMIVECTOR V1, XMIVECTOR V2);
	bool		 XM_CALLCONV	XMVector3Equal(XMIVECTOR V1, XMIVECTOR V2);
	bool		 XM_CALLCONV	XMVector3NotEqual(XMIVECTOR V1, XMIVECTOR V2);

	bool		 XM_CALLCONV	XMVector3AllTrue(XMIVECTOR V1);
	bool		 XM_CALLCONV	XMVectorAnyTrue(XMIVECTOR V1);

	XMIVECTOR	 XM_CALLCONV	XMVectorSelect(XMIVECTOR V1, XMIVECTOR V2, XMIVECTOR Control);

	struct VectorInt3 : public XMINT3
	{
		VectorInt3() : XMINT3(0, 0, 0) {};
		explicit VectorInt3(int x) : XMINT3(x, x, x) {};
		VectorInt3(int _x, int _y, int _z) : XMINT3(_x, _y, _z) {};
		explicit VectorInt3(_In_reads_(3) const int *pArray) : XMINT3(pArray) {};
		VectorInt3(XMIVECTOR V) { XMVectorStoreInt3(this, V); };
		VectorInt3(XMVECTOR V) { XMVectorStoreInt3(this, _mm_cvtps_epi32(XMVectorFloor(V))); };

		operator XMIVECTOR() const { return XMVectorLoadSInt3(this); };
		XMVECTOR Float() const { return _mm_cvtepi32_ps(XMVectorLoadSInt3(this)); };
	};

#include "AdvancedMath.inl"
};

namespace std
{
	template<> struct less<DirectX::XMIVECTOR>
	{
		bool operator()(const DirectX::XMIVECTOR& V1, const DirectX::XMIVECTOR& V2) const
		{
			DirectX::XMIVECTOR vTemp = _mm_cmplt_epi32(V1, V2);
			int flagsL = _mm_movemask_epi8(vTemp);
			vTemp = _mm_cmpeq_epi32(V1, V2);
			int flagsE = _mm_movemask_epi8(vTemp);
			flagsE = (flagsE << 8) | (flagsE << 4) | 0x00F;
			return (flagsE & flagsL & 0xFFF) != 0x000;
		}
	};
};

namespace std
{
	template<>
	class hash<DirectX::XMINT3> {
	public:
		uint64_t operator()(const DirectX::XMINT3 &k) const
		{
			size_t h1 = size_t(k.x) * 73856093; // Magic
			size_t h2 = size_t(k.y) * 19349663; // Prime
			size_t h3 = size_t(k.z) * 83492791; // Numbers
			return h1 ^ h2 ^ h3;
		}
	};

	template <typename T, size_t Alignment>
	class aligned_allocator
	{
	public:

		// The following will be the same for virtually all allocators.
		typedef T * pointer;
		typedef const T * const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		T * address(T& r) const
		{
			return &r;
		};

		const T * address(const T& s) const
		{
			return &s;
		};

		size_t max_size() const
		{
			// The following has been carefully written to be independent of
			// the definition of size_t and to avoid signed/unsigned warnings.
			return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
		};


		// The following must be the same for all allocators.
		template <typename U>
		struct rebind
		{
			typedef aligned_allocator<U, Alignment> other;
		};

		bool operator!=(const aligned_allocator& other) const
		{
			return !(*this == other);
		};

		void construct(T * const p, const T& t) const
		{
			void * const pv = static_cast<void *>(p);

			new (pv)T(t);
		};

		void destroy(T * const p) const
		{
			p->~T();
		};

		// Returns true if and only if storage allocated from *this
		// can be deallocated from other, and vice versa.
		// Always returns true for stateless allocators.
		bool operator==(const aligned_allocator& other) const
		{
			return true;
		};


		// Default constructor, copy constructor, rebinding constructor, and destructor.
		// Empty for stateless allocators.
		aligned_allocator() { };

		aligned_allocator(const aligned_allocator&) { };

		template <typename U> aligned_allocator(const aligned_allocator<U, Alignment>&) { };

		~aligned_allocator() { };


		// The following will be different for each allocator.
		T * allocate(const size_t n) const
		{
			// The return value of allocate(0) is unspecified.
			// Mallocator returns NULL in order to avoid depending
			// on malloc(0)'s implementation-defined behavior
			// (the implementation can define malloc(0) to return NULL,
			// in which case the bad_alloc check below would fire).
			// All allocators can return NULL in this case.
			if (n == 0) {
				return nullptr;
			}

			// All allocators should contain an integer overflow check.
			// The Standardization Committee recommends that std::length_error
			// be thrown in the case of integer overflow.
			if (n > max_size())
			{
				throw std::length_error("aligned_allocator<T>::allocate() - Integer overflow.");
			}

			// Mallocator wraps malloc().
			void * const pv = _mm_malloc(n * sizeof(T), Alignment);

			// Allocators should throw std::bad_alloc in the case of memory allocation failure.
			if (pv == nullptr)
			{
				throw std::bad_alloc();
			}

			return static_cast<T *>(pv);
		};

		void deallocate(T * const p, const size_t n) const
		{
			_mm_free(p);
		};


		// The following will be the same for all allocators that ignore hints.
		template <typename U>
		T * allocate(const size_t n, const U * /* const hint */) const
		{
			return allocate(n);
		};


		// Allocators are not required to be assignable, so
		// all allocators should have a private unimplemented
		// assignment operator. Note that this will trigger the
		// off-by-default (enabled under /Wall) warning C4626
		// "assignment operator could not be generated because a
		// base class assignment operator is inaccessible" within
		// the STL headers, but that warning is useless.
	private:
		aligned_allocator& operator=(const aligned_allocator&);
	};
};

namespace DirectX
{
	typedef std::vector<XMMATRIX, std::aligned_allocator<XMMATRIX, 16>> XMMATRIXLIST;
	typedef std::vector<XMVECTOR, std::aligned_allocator<XMVECTOR, 16>> XMVECTORLIST;

	__declspec(align(16)) struct XMBOUNDINGBOX
	{
		XMVECTOR Center;            // Center of the box.
		XMVECTOR Extents;           // Distance from the center to each side.

		// Creators
		XMBOUNDINGBOX() : Center(g_XMZero), Extents(g_XMOne) {}
		XMBOUNDINGBOX(_In_ const XMVECTOR& center, _In_ const XMVECTOR& extents) : Center(center), Extents(extents) {}
		XMBOUNDINGBOX(_In_ const XMBOUNDINGBOX& box) : Center(box.Center), Extents(box.Extents) {}

		// Methods
		XMBOUNDINGBOX& operator=(_In_ const XMBOUNDINGBOX& box) { Center = box.Center; Extents = box.Extents; return *this; }

		void    XM_CALLCONV     Transform(_Out_ XMBOUNDINGBOX& Out, _In_ DirectX::FXMMATRIX M) const;

		void GetCorners(_Out_writes_(8) XMVECTOR* Corners) const;

		bool Intersects(_In_ const XMBOUNDINGBOX& box) const;
		bool Intersects(_In_ const BoundingBox& box) const;
		ContainmentType Contains(_In_ const XMBOUNDINGBOX& box) const;

		XMVECTOR XM_CALLCONV GetMinCorner();
		XMVECTOR XM_CALLCONV GetMaxCorner();
		// Static methods
		static void CreateMerged(_Out_ XMBOUNDINGBOX& Out, _In_ const XMBOUNDINGBOX& b1, _In_ const XMBOUNDINGBOX& b2);
	};

	struct XMSPHERE
	{
		XMVECTOR Center;
		XMVECTOR RadiusSq;

		XMSPHERE(_In_ const XMVECTOR& center, _In_ FLOAT radius) : Center(center), RadiusSq(XMVectorReplicate(radius * radius)) {}

		bool Intersects(_In_ const XMBOUNDINGBOX& box) const;
		ContainmentType Contains(_In_ const XMBOUNDINGBOX& box) const;
	};

	struct XMFRUSTUM
	{
		XMVECTOR mPlanes[6];

		XMFRUSTUM(const XMMATRIX& matWorldViewProj);
		XMFRUSTUM();

		bool Intersects(const BoundingBox& box) const;
		bool Intersects(const XMBOUNDINGBOX& box) const;
	};
};