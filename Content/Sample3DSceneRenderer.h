#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

#define ALIGN_256(x) (x + 255) & ~255;

namespace SpinningCube
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Sample3DSceneRenderer();
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void Update(DX::StepTimer const& timer);
		bool Render();
		void OnKeyUp(uint32_t keyCode);

	private:
		void Rotate(float radians);

		enum VertexProcessingMode
		{
			InputAssembler,
			MeshShader
		};
		void SetVertexProcessingMode(VertexProcessingMode m);

	private:
		// Constant buffers must be 256-byte aligned.
		static const UINT c_alignedConstantBufferSize = ALIGN_256(sizeof(ModelViewProjectionConstantBuffer))
		static const UINT c_alignedConstantBufferSize2 = ALIGN_256(8 * 4 * sizeof(float))

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6>	m_commandList;

		// MS path involves a root argument for transmitting primitive data. 
		// IA path doesn't, since that information is transmitted in a more fixed-function way there; the root argument is just ignored for that path.
		Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_rootSignature;

		Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_inputAssemblerPipelineState;
        Microsoft::WRL::ComPtr<ID3D12PipelineState>         m_meshShaderPipelineState;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_cbvHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer2;
		ModelViewProjectionConstantBuffer					m_constantBufferData;
		UINT8*												m_mappedConstantBuffer;
		UINT												m_cbvDescriptorSize;
		D3D12_RECT											m_scissorRect;
		std::vector<byte>									m_vertexShader;
		std::vector<byte>									m_pixelShader;
		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

		VertexProcessingMode							    m_vertexProcessingMode;
		bool												m_supportsMeshShaders;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_radiansPerSecond;
		float	m_angle;
		bool	m_tracking;
	};
}

