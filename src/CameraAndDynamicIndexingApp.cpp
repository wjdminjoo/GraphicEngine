#include <D3D12Helper.h>
#include <Dxgi1_4.h>
#include <D3d12SDKLayers.h>
#include <DirectXMath.h>
#include <iostream>
#include "FBXLoader.h"
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#define RESOURCE_SETUP	

const D3D12_INPUT_ELEMENT_DESC INPUT_LAYOUT[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

struct cbMatrix
{
	DirectX::XMMATRIX	_WVP;
};

static const int SCREEN_WIDTH = 1280;
static const int SCREEN_HEIGHT = 800;
static const LPTSTR	CLASS_NAME = TEXT("Engine");
static const UINT BACKBUFFER_COUNT = 2;
static const UINT CONSTANT_BUFFER_COUNT = 2;

// ディスクリプ?ヒ?プ?イプ
enum DESCRIPTOR_HEAP_TYPE
{
	DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	DESCRIPTOR_HEAP_TYPE_SAMPLER,
	DESCRIPTOR_HEAP_TYPE_RTV,
	DESCRIPTOR_HEAP_TYPE_DSV,
	DESCRIPTOR_HEAP_TYPE_MAX,
	DESCRIPTOR_HEAP_TYPE_SET = DESCRIPTOR_HEAP_TYPE_SAMPLER + 1,
};

#if _DEBUG
ID3D12Debug* g_pDebug;
#endif

ID3D12Device* g_pDevice;
ID3D12CommandAllocator* g_pCommandAllocator;
ID3D12CommandQueue* g_pCommandQueue;
IDXGIDevice2* g_pGIDevice;
IDXGIAdapter* g_pGIAdapter;
IDXGIFactory2* g_pGIFactory;
IDXGISwapChain3* g_pGISwapChain;
ID3D12PipelineState* g_pPipelineState;
ID3D12DescriptorHeap* g_pDescripterHeapArray[DESCRIPTOR_HEAP_TYPE_MAX];
ID3D12GraphicsCommandList* g_pGraphicsCommandList;
ID3D12RootSignature* g_pRootSignature;
D3D12_VIEWPORT				g_viewPort;
ID3D12Fence* g_pFence;
HANDLE						g_hFenceEvent;
UINT64						g_CurrentFenceIndex = 0;

UINT						g_CurrentBuckBufferIndex = 0;
ID3D12Resource* g_pBackBufferResource[BACKBUFFER_COUNT];
D3D12_CPU_DESCRIPTOR_HANDLE	g_hBackBuffer[BACKBUFFER_COUNT];

ID3D12Resource* g_pDepthStencilResource;
D3D12_CPU_DESCRIPTOR_HANDLE	g_hDepthStencil;

LPD3DBLOB					g_pVSBlob;
LPD3DBLOB					g_pPSBlob;

ID3D12Resource* g_pVertexBufferResource;
D3D12_VERTEX_BUFFER_VIEW	g_VertexBufferView;

ID3D12Resource* g_pConstantBufferResource;
cbMatrix					g_ConstantBufferData;
D3D12_CPU_DESCRIPTOR_HANDLE	g_hConstantBuffer[CONSTANT_BUFFER_COUNT];

VertexDataArray				g_vertexDataArray;

bool initDirectX(HWND hWnd)
{
	HRESULT hr;
	UINT GIFlag = 0;


#if _DEBUG
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&g_pDebug));
	if (showErrorMessage(hr, TEXT("Not GetDebugInterface")))
	{
		return false;
	}
	if (g_pDebug)
	{
		g_pDebug->EnableDebugLayer();
	}
	GIFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif

	hr = CreateDXGIFactory2(GIFlag, IID_PPV_ARGS(&g_pGIFactory));
	if (showErrorMessage(hr, TEXT("Not CreateFactory")))
	{
		return false;
	}

	IDXGIAdapter* pGIAdapter = nullptr;
	hr = g_pGIFactory->EnumAdapters(0, &pGIAdapter);
	if (showErrorMessage(hr, TEXT("Not GIAdapter")))
	{
		return false;
	}

	hr = D3D12CreateDevice(
		pGIAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&g_pDevice));

	safeRelease(pGIAdapter);

	if (showErrorMessage(hr, TEXT("Not CreateDevice")))
	{
		return false;
	}

	hr = g_pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&g_pCommandAllocator));

	if (showErrorMessage(hr, TEXT("Not CommandAllocator")))
	{
		hr = g_pDevice->GetDeviceRemovedReason();
		return false;
	}


	D3D12_COMMAND_QUEUE_DESC commandQueueDesk;
	commandQueueDesk.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesk.Priority = 0;
	commandQueueDesk.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesk.NodeMask = 0x00000000;

	hr = g_pDevice->CreateCommandQueue(&commandQueueDesk, IID_PPV_ARGS(&g_pCommandQueue));



	if (showErrorMessage(hr, TEXT("Not Create Queue")))
	{
		return false;
	}


	DXGI_SWAP_CHAIN_DESC descSwapChain;
	ZeroMemory(&descSwapChain, sizeof(descSwapChain));
	descSwapChain.BufferCount = BACKBUFFER_COUNT;
	descSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	descSwapChain.OutputWindow = hWnd;
	descSwapChain.SampleDesc.Count = 1;
	descSwapChain.Windowed = TRUE;

	hr = g_pGIFactory->CreateSwapChain(g_pCommandQueue, &descSwapChain, reinterpret_cast<IDXGISwapChain**>(&g_pGISwapChain));
	if (showErrorMessage(hr, TEXT("Now SwapChain")))
	{
		return false;
	}

	hr = g_pDevice->CreateCommandList(
		0x00000000,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		g_pCommandAllocator,
		g_pPipelineState,
		IID_PPV_ARGS(&g_pGraphicsCommandList));

	if (showErrorMessage(hr, TEXT("Not CommandList")))
	{
		return false;
	}

	if (compileShaderFlomFile(L"shaders/VertexShader.hlsl", "main", "vs_5_1", &g_pVSBlob) == false)
	{
		showErrorMessage(E_FAIL, TEXT("Not VertexShader"));
	}

	if (compileShaderFlomFile(L"shaders/PixelShader.hlsl", "main", "ps_5_1", &g_pPSBlob) == false)
	{
		showErrorMessage(E_FAIL, TEXT("Not PixelShader"));
	}

	D3D12_DESCRIPTOR_RANGE range;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range.NumDescriptors = 1;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_DESCRIPTOR_TABLE rootDescriptorTable;
	rootDescriptorTable.NumDescriptorRanges = 1;
	rootDescriptorTable.pDescriptorRanges = &range;

	D3D12_ROOT_PARAMETER rootParameter;
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter.DescriptorTable = rootDescriptorTable;

	LPD3DBLOB pOutBlob = nullptr;
	D3D12_ROOT_SIGNATURE_DESC descRootSignature = D3D12_ROOT_SIGNATURE_DESC();
	descRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	descRootSignature.NumParameters = 1;
	descRootSignature.pParameters = &rootParameter;

	D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &pOutBlob, nullptr);
	hr = g_pDevice->CreateRootSignature(
		0x00000001,
		pOutBlob->GetBufferPointer(),
		pOutBlob->GetBufferSize(),
		IID_PPV_ARGS(&g_pRootSignature));

	safeRelease(pOutBlob);
	if (showErrorMessage(hr, TEXT("")))
	{
		return false;
	}

	D3D12_RASTERIZER_DESC descRasterizer;
	descRasterizer.FillMode = D3D12_FILL_MODE_SOLID;
	descRasterizer.CullMode = D3D12_CULL_MODE_NONE;
	descRasterizer.FrontCounterClockwise = FALSE;
	descRasterizer.DepthBias = 0;
	descRasterizer.SlopeScaledDepthBias = 0.0f;
	descRasterizer.DepthBiasClamp = 0.0f;
	descRasterizer.DepthClipEnable = TRUE;
	descRasterizer.MultisampleEnable = FALSE;
	descRasterizer.AntialiasedLineEnable = FALSE;
	descRasterizer.ForcedSampleCount = 0;
	descRasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_BLEND_DESC descBlend;
	descBlend.AlphaToCoverageEnable = FALSE;
	descBlend.IndependentBlendEnable = FALSE;
	descBlend.RenderTarget[0].BlendEnable = FALSE;
	descBlend.RenderTarget[0].LogicOpEnable = FALSE;
	descBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	descBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	descBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	descBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	descBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	descBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	descBlend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	descBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPSO;
	ZeroMemory(&descPSO, sizeof(descPSO));
	descPSO.InputLayout = { INPUT_LAYOUT, ARRAYSIZE(INPUT_LAYOUT) };
	descPSO.pRootSignature = g_pRootSignature;
	descPSO.VS = { reinterpret_cast<BYTE*>(g_pVSBlob->GetBufferPointer()), g_pVSBlob->GetBufferSize() };
	descPSO.PS = { reinterpret_cast<BYTE*>(g_pPSBlob->GetBufferPointer()), g_pPSBlob->GetBufferSize() };
	descPSO.RasterizerState = descRasterizer;
	descPSO.BlendState = descBlend;
	descPSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPSO.SampleMask = UINT_MAX;
	descPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPSO.NumRenderTargets = 1;
	descPSO.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	descPSO.SampleDesc.Count = 1;

	hr = g_pDevice->CreateGraphicsPipelineState(&descPSO, IID_PPV_ARGS(&g_pPipelineState));

	if (showErrorMessage(hr, TEXT("")))
	{
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	ZeroMemory(&heapDesc, sizeof(heapDesc));

	heapDesc.NumDescriptors = BACKBUFFER_COUNT;

	for (int i = 0; i < DESCRIPTOR_HEAP_TYPE_MAX; ++i)
	{
		heapDesc.Flags = (i == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || i == D3D12_DESCRIPTOR_HEAP_TYPE_DSV) ? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i);

		hr = g_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&g_pDescripterHeapArray[i]));
		if (showErrorMessage(hr, TEXT("Not DescriptorHeap")))
		{
			return false;
		}
	}

	g_pGraphicsCommandList->SetDescriptorHeaps(DESCRIPTOR_HEAP_TYPE_SET, g_pDescripterHeapArray);

	UINT strideHandleBytes = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i = 0; i < BACKBUFFER_COUNT; ++i)
	{
		hr = g_pGISwapChain->GetBuffer(i, IID_PPV_ARGS(&g_pBackBufferResource[i]));
		if (showErrorMessage(hr, TEXT("Not GetBuffer")))
		{
			return false;
		}
		g_hBackBuffer[i] = g_pDescripterHeapArray[DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart();
		g_hBackBuffer[i].ptr += i * strideHandleBytes;
		g_pDevice->CreateRenderTargetView(g_pBackBufferResource[i], nullptr, g_hBackBuffer[i]);
	}

	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		hr = g_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, SCREEN_WIDTH, SCREEN_HEIGHT, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&g_pDepthStencilResource));

		if (showErrorMessage(hr, TEXT("Not CommittedResource")))
		{
			return false;
		}

		g_hDepthStencil = g_pDescripterHeapArray[DESCRIPTOR_HEAP_TYPE_DSV]->GetCPUDescriptorHandleForHeapStart();
		g_pDevice->CreateDepthStencilView(
			g_pDepthStencilResource,
			&depthStencilDesc,
			g_hDepthStencil);
	}

	g_viewPort.TopLeftX = 0;
	g_viewPort.TopLeftY = 0;
	g_viewPort.Width = SCREEN_WIDTH;
	g_viewPort.Height = SCREEN_HEIGHT;
	g_viewPort.MinDepth = 0.0f;
	g_viewPort.MaxDepth = 1.0f;

	g_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_pFence));

	if (showErrorMessage(hr, TEXT("Not CreateFence")))
	{
		return false;
	}

	g_hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX12_Init(g_pDevice, 3, DXGI_FORMAT_R8G8B8A8_UNORM,
			g_pDescripterHeapArray[0],
			g_pDescripterHeapArray[0]->GetCPUDescriptorHandleForHeapStart(),
			g_pDescripterHeapArray[0]->GetGPUDescriptorHandleForHeapStart());
	}

	return true;
}

void cleanupDirectX()
{
	CloseHandle(g_hFenceEvent);
	safeRelease(g_pFence);
	safeRelease(g_pDepthStencilResource);
	for (UINT i = 0; i < BACKBUFFER_COUNT; ++i)
	{
		safeRelease(g_pBackBufferResource[i]);
	}
	for (int i = 0; i < DESCRIPTOR_HEAP_TYPE_MAX; ++i)
	{
		safeRelease(g_pDescripterHeapArray[i]);
	}
	safeRelease(g_pPipelineState);
	safeRelease(g_pRootSignature);
	safeRelease(g_pPSBlob);
	safeRelease(g_pVSBlob);

	safeRelease(g_pGISwapChain);
	safeRelease(g_pGIFactory);
	safeRelease(g_pGIAdapter);
	safeRelease(g_pGIDevice);
	safeRelease(g_pCommandQueue);
	safeRelease(g_pGraphicsCommandList);
	safeRelease(g_pCommandAllocator);
	safeRelease(g_pDevice);
#if _DEBUG
	safeRelease(g_pDebug);
#endif
}

#if defined(RESOURCE_SETUP)
bool setupResource()
{
	HRESULT hr;

	{
		if (LoadFBXConvertToVertexData("output.fbx", g_vertexDataArray) == false)
		{
			if (showErrorMessage(E_FAIL, TEXT("Not Found FBX")))
			{
				return false;
			}
		}


		D3D12_HEAP_PROPERTIES heapPropaty;
		heapPropaty.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapPropaty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropaty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropaty.CreationNodeMask = 1;
		heapPropaty.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC descResource;
		descResource.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		descResource.Alignment = 0;
		descResource.Width = g_vertexDataArray.size() * sizeof(VertexData);
		descResource.Height = 1;
		descResource.DepthOrArraySize = 1;
		descResource.MipLevels = 1;
		descResource.Format = DXGI_FORMAT_UNKNOWN;
		descResource.SampleDesc.Count = 1;
		descResource.SampleDesc.Quality = 0;
		descResource.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		descResource.Flags = D3D12_RESOURCE_FLAG_NONE;

		hr = g_pDevice->CreateCommittedResource(
			&heapPropaty,
			D3D12_HEAP_FLAG_NONE,
			&descResource,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&g_pVertexBufferResource));

		if (showErrorMessage(hr, TEXT("Not CreateCommittedResource")))
		{
			return false;
		}

		UINT8* dataBegin;
		if (SUCCEEDED(g_pVertexBufferResource->Map(0, nullptr, reinterpret_cast<void**>(&dataBegin))))
		{
			VertexData* temp = reinterpret_cast<VertexData*>(dataBegin);
			for (size_t i = 0; i < g_vertexDataArray.size(); ++i)
			{
				temp[i] = g_vertexDataArray[i];
			}
			g_pVertexBufferResource->Unmap(0, nullptr);
		}
		else
		{
			showErrorMessage(E_FAIL, TEXT("Not dataBegin"));
			return false;
		}

		g_VertexBufferView.BufferLocation = g_pVertexBufferResource->GetGPUVirtualAddress();
		g_VertexBufferView.StrideInBytes = sizeof(VertexData);
		g_VertexBufferView.SizeInBytes = g_vertexDataArray.size() * sizeof(VertexData);
	}

	{
		D3D12_HEAP_PROPERTIES heapPropaty;
		heapPropaty.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapPropaty.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropaty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropaty.CreationNodeMask = 1;
		heapPropaty.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC descResource;
		descResource.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		descResource.Alignment = 0;
		descResource.Width = sizeof(cbMatrix);
		descResource.Height = 1;
		descResource.DepthOrArraySize = 1;
		descResource.MipLevels = 1;
		descResource.Format = DXGI_FORMAT_UNKNOWN;
		descResource.SampleDesc.Count = 1;
		descResource.SampleDesc.Quality = 0;
		descResource.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		descResource.Flags = D3D12_RESOURCE_FLAG_NONE;

		hr = g_pDevice->CreateCommittedResource(
			&heapPropaty,
			D3D12_HEAP_FLAG_NONE,
			&descResource,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&g_pConstantBufferResource));

		if (showErrorMessage(hr, TEXT("CreateCommittedResource")))
		{
			return false;
		}

		D3D12_CONSTANT_BUFFER_VIEW_DESC descConstantBufferView = {};

		descConstantBufferView.BufferLocation = g_pConstantBufferResource->GetGPUVirtualAddress();
		descConstantBufferView.SizeInBytes = (sizeof(cbMatrix) + 255) & ~255;

		g_pDevice->CreateConstantBufferView(
			&descConstantBufferView,
			g_pDescripterHeapArray[DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart());

		UINT strideHandleBytes = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		for (int i = 0; i < CONSTANT_BUFFER_COUNT; ++i)
		{
			g_hConstantBuffer[i] = g_pDescripterHeapArray[DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart();
			g_hConstantBuffer[i].ptr += i * strideHandleBytes;

			g_pDevice->CreateConstantBufferView(
				&descConstantBufferView,
				g_hConstantBuffer[i]);
		}
	}


	return true;
}
#endif

#if defined(RESOURCE_SETUP)
void cleanupResource()
{
	safeRelease(g_pConstantBufferResource);
	safeRelease(g_pVertexBufferResource);
}
#endif

void Render()
{
	ID3D12GraphicsCommandList* pCommand = g_pGraphicsCommandList;

	pCommand->SetPipelineState(g_pPipelineState);
	pCommand->SetGraphicsRootSignature(g_pRootSignature);

	D3D12_RECT clearRect;
	clearRect.left = 0;
	clearRect.top = 0;
	clearRect.right = SCREEN_WIDTH;
	clearRect.bottom = SCREEN_HEIGHT;

	g_CurrentBuckBufferIndex = g_pGISwapChain->GetCurrentBackBufferIndex();

	pCommand->OMSetRenderTargets(
		1,
		&g_hBackBuffer[g_CurrentBuckBufferIndex],
		FALSE,
		&g_hDepthStencil);

	setResourceBarrier(
		pCommand,
		g_pBackBufferResource[g_CurrentBuckBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	static float count = 0;
	count = fmod(count + 0.01f, 1.0f);
	float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	pCommand->RSSetViewports(1, &g_viewPort);
	pCommand->RSSetScissorRects(1, &clearRect);
	pCommand->ClearRenderTargetView(g_hBackBuffer[g_CurrentBuckBufferIndex], clearColor, 1, &clearRect);
	pCommand->ClearDepthStencilView(g_hDepthStencil, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

#if defined(RESOURCE_SETUP)
	using namespace DirectX;

	XMMATRIX view = XMMatrixLookAtLH(
		XMLoadFloat3(&XMFLOAT3(0, 0, -50)),
		XMLoadFloat3(&XMFLOAT3(0, 0, 0)),
		XMLoadFloat3(&XMFLOAT3(0, 1, 0)));

	XMMATRIX proj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60),
		static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT),
		1,
		1000);

	static XMFLOAT3 rotation(45, 45, 45);
	rotation.x = fmod(rotation.x + 1.f, 360.f);
	rotation.y = fmod(rotation.y + 2.f, 360.f);
	rotation.z = fmod(rotation.z + 3.f, 360.f);

	XMMATRIX world;

	{
		world = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(rotation.x),
			XMConvertToRadians(rotation.x),
			XMConvertToRadians(rotation.z));

		g_ConstantBufferData._WVP = XMMatrixTranspose(world * view * proj);

		UINT8* dataBegin;
		if (SUCCEEDED(g_pConstantBufferResource->Map(0, nullptr, reinterpret_cast<void**>(&dataBegin))))
		{
			memcpy(dataBegin, &g_ConstantBufferData, sizeof(g_ConstantBufferData));
			g_pConstantBufferResource->Unmap(0, nullptr);
		}
		else
		{
			showErrorMessage(S_FALSE, TEXT("Not ConstantBuffer"));
		}

		ID3D12DescriptorHeap* pHeaps[] = { g_pDescripterHeapArray[DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] };
		pCommand->SetDescriptorHeaps(ARRAYSIZE(pHeaps), pHeaps);
		pCommand->SetGraphicsRootDescriptorTable(0, g_pDescripterHeapArray[DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetGPUDescriptorHandleForHeapStart());

		pCommand->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommand->IASetVertexBuffers(0, 1, &g_VertexBufferView);
		//pCommand->DrawIndexedInstanced(36, 1, 0, 0, 0);
		pCommand->DrawInstanced(g_vertexDataArray.size(), g_vertexDataArray.size() / 3, 0, 0);
	}

#endif

	//static std::string path = "res/";
	//static std::string name = "";
	////TEXT//
	//static bool show_demo_window = true;
	//static bool show_another_window = true;

	////Draw Gui
	//ImGui_ImplDX12_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	//ImGui::NewFrame();

	//ImGui::SetNextWindowSize(ImVec2(400, 200));
	//ImGui::SetNextWindowCollapsed(false);
	//ImGui::SetNextWindowPos(ImVec2(0, (400)));
	//ImGui::Begin("Hello, world!");

	//ImGui::Text("This is some useful text.");
	//ImGui::Checkbox("Demo Window", &show_demo_window);
	//ImGui::Checkbox("Another Window", &show_another_window);
	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::End();

	//if (show_another_window)
	//{
	//	ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH / 2, 200));
	//	ImGui::SetNextWindowCollapsed(false);
	//	ImGui::SetNextWindowPos(ImVec2(0, (SCREEN_HEIGHT - 250)));
	//	ImGui::Begin("Another Window", &show_another_window);
	//	ImGui::Text("Hello from another window!");
	//	if (ImGui::Button("Close Me"))
	//		show_another_window = false;
	//	ImGui::End();
	//}

	//ImGui::Render();
	//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommand);

	setResourceBarrier(
		pCommand,
		g_pBackBufferResource[g_CurrentBuckBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	pCommand->Close();
	ID3D12CommandList* pTemp = pCommand;
	g_pCommandQueue->ExecuteCommandLists(1, &pTemp);
	g_pGISwapChain->Present(1, 0);

	const UINT64 FENCE_INDEX = g_CurrentFenceIndex;
	g_pCommandQueue->Signal(g_pFence, FENCE_INDEX);
	g_CurrentFenceIndex++;

	if (g_pFence->GetCompletedValue() < FENCE_INDEX)
	{
		g_pFence->SetEventOnCompletion(FENCE_INDEX, g_hFenceEvent);
		WaitForSingleObject(g_hFenceEvent, INFINITE);
	}


	g_pCommandAllocator->Reset();
	pCommand->Reset(g_pCommandAllocator, g_pPipelineState);

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winc.lpfnWndProc = WndProc;
	winc.cbClsExtra = 0;
	winc.cbWndExtra = 0;
	winc.hInstance = hInstance;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = CLASS_NAME;

	if (RegisterClass(&winc) == false)
	{
		return 1;
	}

	RECT windowRect = { 0, 0, static_cast<LONG>(SCREEN_WIDTH), static_cast<LONG>(SCREEN_HEIGHT) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindow(
		CLASS_NAME,
		CLASS_NAME,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (hwnd == NULL)
	{
		return 1;
	}

	if (initDirectX(hwnd) == false)
	{
		return 1;
	}

#if defined(RESOURCE_SETUP)
	if (setupResource() == false)
	{
		return 1;
	}
#endif

	ShowWindow(hwnd, nCmdShow);

	do {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	} while (msg.message != WM_QUIT);

#if defined(RESOURCE_SETUP)
	cleanupResource();
#endif
	cleanupDirectX();

	UnregisterClass(CLASS_NAME, hInstance);

	return 0;
}