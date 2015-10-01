#include "pch.h"
#include "AssetLibrary.h"

namespace Rath
{
	AssetLibrary* AssetLibrary::s_library = nullptr;
	AssetLibrary::AssetLibrary() :
		mpd3dDevice(nullptr)
	{
		if (s_library)
		{
			throw std::exception("Assetlibrary is a singleton");
		}

		s_library = this;
	}

	AssetLibrary::~AssetLibrary()
	{
		for (auto it : mLoadables)
		{
			SAFE_RELEASE(it.second.first);
			SAFE_DELETE(it.second.second);
		}

		mLoadables.clear();

		s_library = nullptr;
	}

	_Use_decl_annotations_
		void AssetLibrary::CreateDevice(ID3D11Device* device)
	{
		if (mpd3dDevice == nullptr)
		{
			mpd3dDevice = device;

			if (mLoadables.size())
			{
#if defined(MULTITHREADED_LOADING)
				for_each(begin(mLoadables), end(mLoadables), [&](pair<const string, Loadable>& it)
				{
					if (!it.second.mLoadable->isLoaded())
						it.second.mLoadable->Create(instanz.mpd3dDevice, it.first.c_str(), it.second.second);
				});
#else
				for (auto it : mLoadables)
					it.second.first->Create(mpd3dDevice, it.first.c_str(), it.second.second);
#endif
			}
		}
	}

	Loadable* AssetLibrary::GetLoadable(LPCSTR name)
	{
		Loadable* pElem = nullptr;
		auto it = mLoadables.find(name);
		if (it != mLoadables.end())
		{
			pElem = it->second.first;
			pElem->AddRef();
		}
		return pElem;
	}

	void AssetLibrary::InsertLoadable(Loadable* pElem, const void* pData, LPCSTR name)
	{
		auto ref = mLoadables.insert(std::pair<const std::string, std::pair<Loadable*, const void*>>(name, { pElem, pData }));
		if (!ref.second)
		{
			pElem->Release();
			pElem = ref.first->second.first;
		}
		pElem->AddRef();
	}

	_Use_decl_annotations_
		Texture* AssetLibrary::GetTexture(LPCSTR name)
	{
		Loadable* pElem = s_library->GetLoadable(name);
		if (pElem == nullptr) {
			pElem = new Texture();
			if (s_library->mpd3dDevice)
				DX::ThrowIfFailed(pElem->Create(s_library->mpd3dDevice, name, nullptr));
			s_library->InsertLoadable(pElem, nullptr, name);
		}
		return (Texture*)pElem;
	}

	_Use_decl_annotations_
		Model* AssetLibrary::GetModel(LPCSTR name)
	{
		Loadable* pElem = s_library->GetLoadable(name);
		if (pElem == nullptr) {
			pElem = new Model();
			if (s_library->mpd3dDevice)
				DX::ThrowIfFailed(pElem->Create(s_library->mpd3dDevice, name, nullptr));
			s_library->InsertLoadable(pElem, nullptr, name);
		}
		return (Model*)pElem;
	}

	_Use_decl_annotations_
		AnimatedModel* AssetLibrary::GetAnimatedModel(LPCSTR name)
	{
		Loadable* pElem = s_library->GetLoadable(name);
		if (pElem == nullptr) {
			pElem = new AnimatedModel();
			if (s_library->mpd3dDevice)
				DX::ThrowIfFailed(pElem->Create(s_library->mpd3dDevice, name, nullptr));
			s_library->InsertLoadable(pElem, nullptr, name);
		}
		return (AnimatedModel*)pElem;
	}

	//_Use_decl_annotations_
	//	void AssetLibrary::AddTechniqueSetting(const TechniqueSetting& setting)
	//{
	//	auto it = std::find(Technique::g_SavedSettings.begin(), Technique::g_SavedSettings.end(), setting);
	//	if (it == Technique::g_SavedSettings.end())
	//	{
	//		Technique::g_SavedSettings.push_back(setting);
	//	}
	//}

	_Use_decl_annotations_
		Technique* AssetLibrary::GetTechnique(LPCSTR name, const TechniqueSetting * pTechnique)
	{
		if (pTechnique != NULL)
		{
			Technique::AddTechnique(*pTechnique);
		}

		Loadable* pElem = s_library->GetLoadable(name);
		if (pElem == nullptr) 
		{
			const void* data = nullptr;
			auto it = std::find(Technique::g_SavedSettings.begin(), Technique::g_SavedSettings.end(), name);
			if (it != Technique::g_SavedSettings.end())
			{
				TechniqueSetting* setting = new TechniqueSetting(*it);
				pElem = new Technique(*setting);
				data = setting;
			}
			else
			{
				return nullptr;
			}
			if (s_library->mpd3dDevice)
				pElem->Create(s_library->mpd3dDevice, name, data);
			s_library->InsertLoadable(pElem, data, name);
		}
		return (Technique*)pElem;
	}
}