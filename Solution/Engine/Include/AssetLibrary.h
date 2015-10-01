#pragma once
#include "Model.h"
#include "Texture.h"
#include "Technique.h"

namespace Rath
{
	class AssetLibrary
	{
		friend class DeviceManager;
	protected:
		static AssetLibrary* s_library;

#if defined(MULTITHREADED_LOADING)
		concurrent_unordered_map<string, Loadable>				mLoadables;
#else
		std::map<const std::string, std::pair<Loadable*, const void*>> mLoadables;
#endif
		//std::vector<const TechniqueSetting>		mShaderSettings;
		ID3D11Device*				mpd3dDevice;

		Loadable*					GetLoadable(LPCSTR name);
		void						InsertLoadable(Loadable* pElem, const void* pData, LPCSTR name);
		void						CreateDevice(_In_ ID3D11Device* device);
	public:
		AssetLibrary();
		~AssetLibrary();

		static Texture*			GetTexture(_In_ LPCSTR name);
		static Model*			GetModel(_In_ LPCSTR name);
		static AnimatedModel*	GetAnimatedModel(_In_ LPCSTR name);
		static Technique*		GetTechnique(_In_ LPCSTR name, _In_opt_ const TechniqueSetting * pTechnique = nullptr);
	};
}