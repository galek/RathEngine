#pragma once
class Chunk;
class ChunkMesh
{
protected:
	static UINT							uiStride;
	UINT								uiOffset;
	UINT								uiSize;
	UINT								uiRealSize;
	UINT								uiShadowSize;
	ID3D11Buffer*						pVertexBuffer;
	ID3D11Texture3D*					pLightTexture;
	ID3D11ShaderResourceView*			pLightTextureSRV;

	physx::PxRigidStatic*				pCollisionShape;

	virtual void GenerateMesh(std::vector<BLOCK_MESH>& Mesh, 
							  std::vector<uint32>& Light) = 0;
	virtual void GenerateShape() = 0;
public:
	ChunkMesh();
	~ChunkMesh();

	void Create(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext);
	void Render(ID3D11DeviceContext* pImmediateContext);
	void RenderDepthOnly(ID3D11DeviceContext* pImmediateContext, uint32 flags = 0);
	void BindLightTexture(ID3D11DeviceContext* pImmediateContext);
};

