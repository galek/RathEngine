#pragma once
namespace Rath
{
	class Scene
	{
	public:
		enum RenderPass
		{
			BackgroundPass,
			DepthOnlyPass,
			ShadowPass,
			ShadowPass_Cascade1,
			ShadowPass_Cascade2,
			ShadowPass_Cascade3,
			ShadowPass_Cascade4,
			ScenePass,
			UIPass,
		};

	public:
		Scene();
		~Scene();

		void virtual Render(ID3D11DeviceContext* context, const XMMATRIX& mViewProj, RenderPass pass);
	};
}
