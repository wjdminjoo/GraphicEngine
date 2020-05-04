#pragma once

#include <Game.h>
#include <Window.h>

#include <DirectXMath.h>

class Tutorial : public Game
{
public:
	using super = Game;

	Tutorial(const std::wstring& name, int width, int height, bool vSync = false);
	virtual bool LoadContent() override;
	virtual void UnloadContent() override;

protected:
	virtual void OnUpdate(UpdateEventArgs& e) override;
	virtual void OnRender(RenderEventArgs& e) override;
	virtual void OnKeyPressed(KeyEventArgs& e) override;
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
	virtual void OnResize(ResizeEventArgs& e) override;

private:
	// 리소스 교체
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	// 렌더 타겟 뷰 초기화
	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

	// Depth-stencil 뷰 초기화
	void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

	// GPU buffer 초기화
	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		size_t numElements, size_t elementSize, const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// depth buffer 사이즈 클라이언트에 맞게 설정
	void ResizeDepthBuffer(int width, int height);


	uint64_t m_FenceValues[Window::BufferCount] = {};

	// Vertex Buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	// Index Buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	// Depth Buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;

	// Depth buffer Descriptor Heap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	// Root signature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	// pipline state object
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

	//initialize rasterizer statge of the rendering pipline
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	float m_FoV = 0;

	DirectX::XMMATRIX m_ModelMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_ViewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_ProjectionMatrix = DirectX::XMMatrixIdentity();

	bool m_ContentLoaded = 0;
};