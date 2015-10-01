#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "FileIO.h"
#include "AnimationController.h"

#define MAX_BONE_MATRICES 256

struct aiScene;
namespace Rath
{
	class Model : public Loadable
	{
	protected:
		std::vector<Mesh*>		mppMeshList;
		std::vector<Texture*>	mppMaterialList;
		DirectX::BoundingBox	mBoundingBox;

		virtual HRESULT	Create(ID3D11Device* pd3dDevice, LPCSTR pFile, const void* data = nullptr);
		virtual void	ExtractMeshs(ID3D11Device* pd3dDevice, const aiScene* pScene);
		virtual void	ExtractMeshs(ID3D11Device* pd3dDevice, std::istream& pFile);
	public:
		~Model();
		void			Render(ID3D11DeviceContext* pd3dImmediateContext);

		const DirectX::BoundingBox& GetBoundingBox() { return mBoundingBox; };
	};

	class AnimatedModel : public Model, public AnimationController
	{
	protected:
		virtual void	ExtractMeshs(ID3D11Device* pd3dDevice, const aiScene* pScene);
		virtual void	ExtractMeshs(ID3D11Device* pd3dDevice, std::istream&pFile);
	public:
		void			LoadAnimation(LPCSTR pFile);
	};
}