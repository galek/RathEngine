#pragma once

#include "InterfacePointers.h"

namespace Rath
{
	class Referencable : public IUnknown
	{
	private:
		mutable ULONG 	m_cRef;
	protected:
		virtual ~Referencable(){} // Destructor is not public.  Must release instead of delete.
	public:
		Referencable() : m_cRef(0) {};

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR * ppvObj) override
		{
			// Always set out parameter to NULL, validating it first.
			if (!ppvObj)
				return E_INVALIDARG;
			*ppvObj = NULL;
			//if (riid == IID_IUnknown || riid == IID_IMAPIProp ||
			//	riid == IID_IMAPIStatus)
			{
				// Increment the reference count and return the pointer.
				*ppvObj = (_COM_Outptr_ void __RPC_FAR *__RPC_FAR *)this;
				AddRef();
				return NOERROR;
			}
			return E_NOINTERFACE;
		}

		ULONG STDMETHODCALLTYPE AddRef() override
		{
			InterlockedIncrement(&m_cRef);
			return m_cRef;
		};

		ULONG STDMETHODCALLTYPE GetRefCount() const
		{ 
			return m_cRef; 
		};

		ULONG STDMETHODCALLTYPE Release() override
		{
			ULONG ulRefCount = InterlockedDecrement(&m_cRef);
			if (0 == m_cRef)
			{
				delete this;
			}
			return ulRefCount;
		};
	};

	class Loadable : public Referencable
	{
		friend class AssetLibrary;
	protected:
		virtual HRESULT Create(ID3D11Device* pd3dDevice, LPCSTR filename, const void* data) = 0;
	public:
#if defined(DEBUG) | defined(_DEBUG)
		//static void* operator new (size_t size)
		//{
		//	return malloc(size);
		//};

		//static void	 operator delete (void *p)
		//{
		//	free(p);
		//};
#endif
	};
}

