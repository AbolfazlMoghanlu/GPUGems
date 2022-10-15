#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ---------- Window properties ----------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------
const LPCWSTR WindowClassName = L"MainWindowClass";
const LPCWSTR WindowName = L"MainWindow";
const long int MainWindowStyle = WS_CAPTION | WS_MINIMIZE | WS_SYSMENU;
const int WindowWidth = 1080;
const int WindowHeight = 720;
HWND MainWindow;
bool bOpenWindow = true;
float TimeMiliseconds = 0;

float MouseX = 0.0f;
float MouseY = 0.0f;
float MouseDeltaX = 0.0f;
float MouseDeltaY = 0.0f;

// ---------- Window functions -----------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

LRESULT CALLBACK MainWindowProc(HWND Handle, UINT Msg, WPARAM WParam, LPARAM LParam)
{
	if (ImGui_ImplWin32_WndProcHandler(Handle, Msg, WParam, LParam))
		return true;

	switch (Msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (WParam == 'Q')
		{
			bOpenWindow = false;
		}
		break;

	default:
		break;
	}

	return DefWindowProc(Handle, Msg, WParam, LParam);
}

bool IsRightClickDown()
{
	return GetKeyState(VK_RBUTTON) & 128;
}

float GetRightValue()
{
	float RightValue = 0.0f;
	RightValue += GetKeyState('D') & 128 ? 1.0f : 0.0f;
	RightValue += GetKeyState('A') & 128 ? -1.0f : 0.0f;

	return RightValue;
}

float GetUpValue()
{
	float UpValue = 0.0f;
	UpValue += GetKeyState('W') & 128 ? 1.0f : 0.0f;
	UpValue += GetKeyState('S') & 128 ? -1.0f : 0.0f;

	return UpValue;
}

void InitMainWindow(HINSTANCE hInstance)
{
	WNDCLASSEXW MainWindowClass = {};
	MainWindowClass.cbSize = sizeof(MainWindowClass);
	MainWindowClass.lpfnWndProc = MainWindowProc;
	MainWindowClass.hInstance = hInstance;
	MainWindowClass.lpszClassName = WindowClassName;
	MainWindowClass.style = CS_OWNDC;

	RegisterClassExW(&MainWindowClass);

	RECT rect = {0, 0, WindowWidth, WindowHeight};
	AdjustWindowRect(&rect, MainWindowStyle, false);

	int AdjustedXSize = rect.right - rect.left;
	int AdjustedYSize = rect.bottom - rect.top;

	MainWindow = CreateWindowExW(0, WindowClassName, WindowName, MainWindowStyle, 0, 0, AdjustedXSize,
		AdjustedYSize, NULL, NULL, hInstance, NULL);
	ShowWindow(MainWindow, SW_SHOW);
}

// ---------------------------------------------------------------------------------------------------
// ---------- App properties -------------------------------------------------------------------------

const int BufferNum = 2;
int FrameIndex = 0;
ComPtr<ID3D12Device> D_Device;
ComPtr<IDXGISwapChain3> D_SwapChain;
ComPtr<ID3D12CommandQueue> D_CommandQue;
ComPtr<ID3D12CommandAllocator> D_CommandAllocator;
ComPtr<ID3D12RootSignature> D_RootSignature;
ComPtr<ID3D12PipelineState> D_PipelineState;
ComPtr<ID3D12GraphicsCommandList> D_CommandList;
ComPtr<ID3D12DescriptorHeap> D_RVTDescriptorHeap;
ComPtr<ID3D12DescriptorHeap> D_DSTDescriptorHeap;
ComPtr<ID3D12Resource> D_ChainTargets[BufferNum];
ComPtr<ID3D12Resource> D_ChainDSTargets[BufferNum];
ComPtr<ID3D12DescriptorHeap> D_CBVDescriptorHeap;
ComPtr<ID3D12DescriptorHeap> D_ImguiDescriptorHeap;
ComPtr<ID3D12Resource> D_VertexBuffer;
ComPtr<ID3D12Resource> D_MatricesBuffer;
ComPtr<ID3D12Resource> D_PSConstantBuffer;
ComPtr<ID3D12Resource> D_UploadTexture;
ComPtr<ID3D12Resource> D_UVCheckerTexture;


D3D12_VERTEX_BUFFER_VIEW D_VertexBufferView;

ComPtr<ID3D12Fence> D_Fence;
unsigned long long int D_FenceValue = 0;
HANDLE D_FenceEvent;

unsigned int RVTHandleSize = 0;
unsigned int DSVHandleSize = 0;
unsigned int CBVHandleSize = 0;

UINT8* D_MatricesBeginP = nullptr;
UINT8* D_PSConstantBeginP = nullptr;

float CameraMoveSpeed = 0.1f;
float CameraRotationSpeed = 50.0f;

Vector3f CameraPosition(0.0f, 10.0f, 0.0f);
Rotatorf CameraRotation(0.0f);

Vector3f PlaneLocation = Vector3f(0.0f, 0.0f, 2.0f);
Rotatorf PlaneRotation(360.0f, 0.0f, 0.0f);
Vector3f PlaneScale(100.0f);

struct UV { float U; float V; };

struct Vertex
{
	Vector3f position;
	Vector3f color;
	UV UVs;
	Vector3f Pad;
};

std::vector<Vertex> PlaneVertecies;

Vertex triangleVertices[] =
{
	{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, {0, 0}, {0, 0, 0} },
	{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, {0, 1},  {0, 0, 0} },
	{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, {1, 1}, {0, 0, 0} },
	{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, {0, 0}, {0, 0, 0}},
	{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, {1, 1}, {0, 0, 0}},
	{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, {1, 0},{0, 0, 0}}
};

struct Matrices
{
	Matrix<float> TransformMatrix;
	Matrix<float> ViewMatrix;
	Matrix<float> ProjectionMatrix;
	Vector3f CameraPosition;
	float Pad[13];
} Mats;

struct PSConstantBufferLayout
{
	Vector3f ColorOverlay = Vector3f(1.0f);
	float TesselationAmount = 64;
	float Height = 2.0f;
	float TesselationOffset = 5.0f;
	float TesselationLength = 15.0f;
	float Pad1[57];
} PSConstantBuffer;

// ---------------------------------------------------------------------------------------------------
// ---------- App functions --------------------------------------------------------------------------

void WaitForPreviousFrame()
{
	const UINT64 fence = D_FenceValue;
	D_CommandQue->Signal(D_Fence.Get(), fence);
	D_FenceValue++;

	if (D_Fence->GetCompletedValue() < fence)
	{
		D_Fence->SetEventOnCompletion(fence, D_FenceEvent);
		WaitForSingleObject(D_FenceEvent, INFINITE);
	}

	FrameIndex = D_SwapChain->GetCurrentBackBufferIndex();
}

void SetViewportSize(int X, int Y)
{
	D3D12_VIEWPORT Viewport;
	Viewport.Width = float(X);
	Viewport.Height = float(Y);
	Viewport.MinDepth = 0;
	Viewport.MaxDepth = 1;
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;

	D3D12_RECT ScissorRect;
	ScissorRect.left = 0;
	ScissorRect.right = X;
	ScissorRect.top = 0;
	ScissorRect.bottom = Y;

	D_CommandList->RSSetViewports(1, &Viewport);
	D_CommandList->RSSetScissorRects(1, &ScissorRect);
}

void AppInit()
{
	// CREATE DEBUG LAYER
	ComPtr<ID3D12Debug> pdx12Debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
		pdx12Debug->EnableDebugLayer();
	// --------------------------------------------------------------------------------------
	// CREATE DEVICE
	ComPtr<IDXGIFactory> factory;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

	ComPtr<IDXGIAdapter> Adapter;

	SIZE_T MaxDedicatedVideoMemory = 0;
	UINT Index = 0;
	for (UINT i = 0; factory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC AdaptorDesc;
		Adapter->GetDesc(&AdaptorDesc);

		if (AdaptorDesc.DedicatedVideoMemory > MaxDedicatedVideoMemory)
		{
			MaxDedicatedVideoMemory = AdaptorDesc.DedicatedVideoMemory;
			Index = i;
		}
	}

	factory->EnumAdapters(Index, &Adapter);
	D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&D_Device));

	RVTHandleSize = D_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CBVHandleSize = D_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DSVHandleSize = D_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	// --------------------------------------------------------------------------------------
	// CREATE COMMAND QUEUE
	D3D12_COMMAND_QUEUE_DESC QueDesc = {};
	QueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	D_Device->CreateCommandQueue(&QueDesc, IID_PPV_ARGS(&D_CommandQue));
	// --------------------------------------------------------------------------------------
	// CREATE SWAP CHAIN
	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	SwapChainDesc.BufferCount = BufferNum;
	SwapChainDesc.BufferDesc.Width = WindowWidth;
	SwapChainDesc.BufferDesc.Height = WindowHeight;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.OutputWindow = MainWindow;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.Windowed = true;

	ComPtr<IDXGISwapChain> SwapChain;
	HRESULT a = factory->CreateSwapChain(
		D_CommandQue.Get(),
		&SwapChainDesc,
		&SwapChain
	);

	SwapChain.As(&D_SwapChain);
	FrameIndex = D_SwapChain->GetCurrentBackBufferIndex();
	// --------------------------------------------------------------------------------------
	// CREATING RVT HEAP
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = BufferNum;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	D_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&D_RVTDescriptorHeap));
	// --------------------------------------------------------------------------------------
	// CREATING DST HEAP
	D3D12_DESCRIPTOR_HEAP_DESC dtvHeapDesc = {};
	dtvHeapDesc.NumDescriptors = BufferNum;
	dtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	D_Device->CreateDescriptorHeap(&dtvHeapDesc, IID_PPV_ARGS(&D_DSTDescriptorHeap));
	// --------------------------------------------------------------------------------------
	// CREATE RTV
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D_RVTDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < BufferNum; n++)
	{
		D_SwapChain->GetBuffer(n, IID_PPV_ARGS(&D_ChainTargets[n]));
		D_Device->CreateRenderTargetView(D_ChainTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, RVTHandleSize);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle1(D_DSTDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < BufferNum; n++)
	{
		D3D12_RESOURCE_DESC dsResDesc;
		dsResDesc.Alignment = 0;
		dsResDesc.Width = WindowWidth;
		dsResDesc.Height = WindowHeight;
		dsResDesc.DepthOrArraySize = 1;
		dsResDesc.MipLevels = 1;
		dsResDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsResDesc.SampleDesc.Count = 1;
		dsResDesc.SampleDesc.Quality = 0;
		dsResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		dsResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		
		D3D12_CLEAR_VALUE clearValueDs;
		clearValueDs.Format = DXGI_FORMAT_D32_FLOAT;
		clearValueDs.DepthStencil.Depth = 1.0f;
		clearValueDs.DepthStencil.Stencil = 0;
		
		D_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&dsResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValueDs,
			IID_PPV_ARGS(&D_ChainDSTargets[n]));
		
		D3D12_DEPTH_STENCIL_VIEW_DESC dsViewDesk;
		dsViewDesk.Format = DXGI_FORMAT_D32_FLOAT;
		dsViewDesk.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsViewDesk.Flags = D3D12_DSV_FLAG_NONE;
		dsViewDesk.Texture2D.MipSlice = 0;

		D_Device->CreateDepthStencilView(D_ChainDSTargets[n].Get(), &dsViewDesk, rtvHandle1);
		rtvHandle1.Offset(1, DSVHandleSize);
	}

	// --------------------------------------------------------------------------------------
	// CREATE CBV HEAP
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 10;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	D_Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&D_CBVDescriptorHeap));
	// --------------------------------------------------------------------------------------
	// CREATE COMMAND ALLOCATOR
	D_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D_CommandAllocator));
	// --------------------------------------------------------------------------------------
	// CREATE ROOT SIGNITURE
	D3D12_DESCRIPTOR_RANGE ranges[3];
	D3D12_ROOT_PARAMETER rootParameters[1];

	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[0].NumDescriptors = 1;
	ranges[0].BaseShaderRegister = 0;
	ranges[0].OffsetInDescriptorsFromTableStart = 0;
	ranges[0].RegisterSpace = 0;

	ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[1].NumDescriptors = 1;
	ranges[1].BaseShaderRegister = 1;
	ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	ranges[1].RegisterSpace = 0;
	
	ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	ranges[2].NumDescriptors = 1;
	ranges[2].BaseShaderRegister = 0;
	ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	ranges[2].RegisterSpace = 0;

	D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable;
	DescriptorTable.NumDescriptorRanges = _countof(ranges);
	DescriptorTable.pDescriptorRanges = &ranges[0];

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].DescriptorTable = DescriptorTable;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC LinearSamplerDesc = {};
	LinearSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	LinearSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	LinearSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	LinearSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	LinearSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	LinearSamplerDesc.MaxAnisotropy = 16;
	LinearSamplerDesc.MaxLOD = 1;
	LinearSamplerDesc.MinLOD = 0;
	LinearSamplerDesc.MipLODBias = 0;
	LinearSamplerDesc.RegisterSpace = 0;
	LinearSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	LinearSamplerDesc.ShaderRegister = 0;
	LinearSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC StaticSamplers[] = { LinearSamplerDesc };

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, _countof(StaticSamplers), StaticSamplers
		, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	//std::cout << reinterpret_cast<const char*>(error->GetBufferPointer()) << std::flush;
	
	D_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&D_RootSignature));
	// --------------------------------------------------------------------------------------
	// SHADER COMPLATION
	ComPtr<ID3DBlob> TestVertexShader;
	ComPtr<ID3DBlob> TestPixelShader;
	ComPtr<ID3DBlob> HullShader;
	ComPtr<ID3DBlob> DomainShader;

	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

	D3DCompileFromFile(L"Source/Shader/TestVertexShader_vs.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &TestVertexShader, nullptr);
	D3DCompileFromFile(L"Source/Shader/TestPixelShader_ps.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &TestPixelShader, nullptr);
	D3DCompileFromFile(L"Source/Shader/HullShader_hl.hlsl", nullptr, nullptr, "main", "hs_5_0", compileFlags, 0, &HullShader, nullptr);
	D3DCompileFromFile(L"Source/Shader/DomainShader_dm.hlsl", nullptr, nullptr, "main", "ds_5_0", compileFlags, 0, &DomainShader, nullptr);
	// --------------------------------------------------------------------------------------
	// DECLARE INPUT LAYOUT
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Pad", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	// --------------------------------------------------------------------------------------
	// CREATE PSO
	D3D12_RENDER_TARGET_BLEND_DESC TargetBlendDesc;
	TargetBlendDesc.BlendEnable = false;
	TargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	TargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	TargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	TargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	TargetBlendDesc.DestBlendAlpha = D3D12_BLEND_DEST_ALPHA;
	TargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	TargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	TargetBlendDesc.LogicOpEnable = FALSE;

	D3D12_BLEND_DESC BlendDesc;
	BlendDesc.AlphaToCoverageEnable = false;
	BlendDesc.IndependentBlendEnable = false;
	BlendDesc.RenderTarget[0] = TargetBlendDesc;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = D_RootSignature.Get();
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.VS = { reinterpret_cast<UINT8*>(TestVertexShader->GetBufferPointer()), TestVertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(TestPixelShader->GetBufferPointer()), TestPixelShader->GetBufferSize() };
	psoDesc.HS = { reinterpret_cast<UINT8*>(HullShader->GetBufferPointer()), HullShader->GetBufferSize() };
	psoDesc.DS = { reinterpret_cast<UINT8*>(DomainShader->GetBufferPointer()), DomainShader->GetBufferSize() };
	D3D12_RASTERIZER_DESC RasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	RasterDesc.MultisampleEnable = FALSE;
	psoDesc.RasterizerState = RasterDesc;
	psoDesc.BlendState = BlendDesc;
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	D_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&D_PipelineState));
	// --------------------------------------------------------------------------------------
	// CREATE COMMAND LIST
	D_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D_CommandAllocator.Get(),
		D_PipelineState.Get(), IID_PPV_ARGS(&D_CommandList));
	// --------------------------------------------------------------------------------------
	// UPLOAD VERTEX DATA
	std::string PlaneFilePath = "../Content/B1C1/Plane.obj";

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(PlaneFilePath)) 
	{
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();

	size_t index_offset = 0;
	for (size_t f = 0; f < shapes[0].mesh.num_face_vertices.size(); f++) 
	{
		size_t fv = size_t(shapes[0].mesh.num_face_vertices[f]);

		for (size_t v = 0; v < fv; v++) 
		{
			tinyobj::index_t idx = shapes[0].mesh.indices[index_offset + v];
			tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
			tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
			tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
		
			if (idx.texcoord_index >= 0) {
				tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

				Vertex V;
				V.position = Vector3f(vx, vy, vz);
				V.color = Vector3f(1);
				V.UVs = {tx, ty};
				V.Pad = Vector3f(0);

				PlaneVertecies.push_back(V);
			}
		}

		index_offset += fv;
	}



	const unsigned int vertexBufferSize = sizeof(Vertex) * PlaneVertecies.size();
	//const unsigned int vertexBufferSize = sizeof(triangleVertices);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	D_Device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&D_VertexBuffer));

	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	D_VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, &PlaneVertecies[0], vertexBufferSize);
	D_VertexBuffer->Unmap(0, nullptr);

	D_VertexBufferView.BufferLocation = D_VertexBuffer->GetGPUVirtualAddress();
	D_VertexBufferView.StrideInBytes = sizeof(Vertex);
	D_VertexBufferView.SizeInBytes = vertexBufferSize;
	// --------------------------------------------------------------------------------------
	// CREATE MATRICES VIEW
	const unsigned int MatricesSize = sizeof(Matrices);
	D_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(MatricesSize), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&D_MatricesBuffer));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = D_MatricesBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = MatricesSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CBVHandle(D_CBVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	D_Device->CreateConstantBufferView(&cbvDesc, CBVHandle);

	D_MatricesBuffer->Map(0, &readRange, reinterpret_cast<void**>(&D_MatricesBeginP));
	memcpy(D_MatricesBeginP, &Mats, sizeof(Matrices));
	// --------------------------------------------------------------------------------------
	// CREATE PS CONSTANT BUFFER VIEW
	const unsigned int PSConstantsSize = sizeof(PSConstantBufferLayout);
	D_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(PSConstantsSize), D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&D_PSConstantBuffer));

	cbvDesc.BufferLocation = D_PSConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = PSConstantsSize;
	CBVHandle.Offset(1, CBVHandleSize);

	D_Device->CreateConstantBufferView(&cbvDesc, CBVHandle);

	D_PSConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&D_PSConstantBeginP));
	memcpy(D_PSConstantBeginP, &PSConstantBuffer, sizeof(PSConstantBufferLayout));

	// --------------------------------------------------------------------------------------
	// LOAD UV CHECKER TEXTURE
	unsigned char* UVCheckerBlob;
	int UVCheckerWidth, UVCheckerHeight, UVCheckerNumberOfChannels;

	UVCheckerBlob = stbi_load("../Content/Common/T_UVChecker.png",
		&UVCheckerWidth, &UVCheckerHeight, &UVCheckerNumberOfChannels, 0);

	D3D12_RESOURCE_DESC UVCheckerTextureDesc = {};
	UVCheckerTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	UVCheckerTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UVCheckerTextureDesc.Width = UVCheckerWidth;
	UVCheckerTextureDesc.Height = UVCheckerHeight;
	UVCheckerTextureDesc.DepthOrArraySize = 1;
	UVCheckerTextureDesc.MipLevels = 1;
	UVCheckerTextureDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	UVCheckerTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	UVCheckerTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	UVCheckerTextureDesc.SampleDesc.Count = 1;

	CD3DX12_HEAP_PROPERTIES UVCheckerTextureHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D_Device->CreateCommittedResource(&UVCheckerTextureHeapProperties, D3D12_HEAP_FLAG_NONE, &UVCheckerTextureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&D_UVCheckerTexture));

	UINT64 uploadBufferSize;
	auto Desc = D_UVCheckerTexture->GetDesc();
	D_Device->GetCopyableFootprints(&Desc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

	D_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&D_UploadTexture));

	D3D12_SUBRESOURCE_DATA UVCheckerData = {};
	UVCheckerData.pData = UVCheckerBlob;
	UVCheckerData.RowPitch = UVCheckerWidth * sizeof(unsigned char) * 3;
	UVCheckerData.SlicePitch = UVCheckerData.RowPitch * UVCheckerHeight;
	
	UpdateSubresources(D_CommandList.Get(), D_UVCheckerTexture.Get(), D_UploadTexture.Get(), 0, 0, 1, &UVCheckerData);
	D_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D_UVCheckerTexture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = UVCheckerTextureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CBVHandle.Offset(1, CBVHandleSize);
	D_Device->CreateShaderResourceView(D_UVCheckerTexture.Get(), &srvDesc, CBVHandle);

	// --------------------------------------------------------------------------------------
	// CREATE FENCE
	D_CommandList->Close();

	ID3D12CommandList* ppCommandLists[] = { D_CommandList.Get() };
	D_CommandQue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	
	D_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&D_Fence));
	D_FenceValue = 1;

	D_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (D_FenceEvent == nullptr)
	{
		HRESULT_FROM_WIN32(GetLastError());
	}

	WaitForPreviousFrame();
	// --------------------------------------------------------------------------------------
	// INIT IMGUI
	D3D12_DESCRIPTOR_HEAP_DESC ImGuiHeapDesc = {};
	ImGuiHeapDesc.NumDescriptors = 1;
	ImGuiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ImGuiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	D_Device->CreateDescriptorHeap(&ImGuiHeapDesc, IID_PPV_ARGS(&D_ImguiDescriptorHeap));
	D_ImguiDescriptorHeap->SetName(L"GuiHeap");


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(MainWindow);
	ImGui_ImplDX12_Init(D_Device.Get(), BufferNum, DXGI_FORMAT_R8G8B8A8_UNORM, D_ImguiDescriptorHeap.Get(),
		D_ImguiDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		D_ImguiDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

void AppTick(float DeltaTime)
{
	if (IsRightClickDown())
	{
		float CameraPitchOffset = CameraRotationSpeed * -MouseDeltaY;
		float CameraYawOffset = CameraRotationSpeed * MouseDeltaX;

		Rotatorf DeltaRotation = Rotatorf(CameraPitchOffset, CameraYawOffset, 0.0f);
		CameraRotation = Rotatorf::CombineRotators(CameraRotation, DeltaRotation);

		float CameraPitchClamped = CameraRotation.Pitch < 180.0f ?
			Math::Clamp<float>(CameraRotation.Pitch, 1.0f, 89.0f) : Math::Clamp<float>(CameraRotation.Pitch, 271.0f, 359.0f);

		CameraRotation = Rotatorf(CameraPitchClamped, CameraRotation.Yaw, CameraRotation.Roll);


		Vector3f CameraForwardVector = CameraRotation.Vector();
		Vector3f CameraRightVector = Vector3f::CrossProduct(Vector3f::UpVector, CameraForwardVector);

		CameraPosition += (CameraForwardVector * GetUpValue() + CameraRightVector * GetRightValue()) * CameraMoveSpeed;
	}

	// --------------------------------------------------------------------------------------
	Vector3f CameraForwardVector = CameraRotation.Vector();

	Mats.CameraPosition = CameraPosition;

	Matrix<float> TransformMatrix = ScaleRotationTranslationMatrix<float>
		(PlaneScale, PlaneRotation, PlaneLocation);
	Mats.TransformMatrix = TransformMatrix;

	Matrix<float> ViewMatrix = Math::LookAt(CameraPosition, CameraForwardVector, Vector3f::UpVector);
	Mats.ViewMatrix = ViewMatrix;

	Matrix<float> ProjectionMatrix = PerspectiveMatrix<float>(90.0f,
		(float)WindowWidth / (float)WindowHeight, 0.1f, 1000.0f);
	Mats.ProjectionMatrix = ProjectionMatrix;

	memcpy(D_MatricesBeginP, &Mats, sizeof(Matrices));
	// --------------------------------------------------------------------------------------

	memcpy(D_PSConstantBeginP, &PSConstantBuffer, sizeof(PSConstantBufferLayout));

	// --------------------------------------------------------------------------------------
	D_CommandAllocator->Reset();
	D_CommandList->Reset(D_CommandAllocator.Get(), D_PipelineState.Get());

	D_CommandList->SetGraphicsRootSignature(D_RootSignature.Get());
	SetViewportSize(WindowWidth, WindowHeight);

	ID3D12DescriptorHeap* ppHeaps[] = { D_CBVDescriptorHeap.Get() };
	D_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	D_CommandList->SetGraphicsRootDescriptorTable(0, D_CBVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(D_ChainTargets[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	D_CommandList->ResourceBarrier(1, &barrier);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(D_ChainDSTargets[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	D_CommandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D_RVTDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex, RVTHandleSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle1(D_DSTDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex, DSVHandleSize);
	D_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &rtvHandle1);

	const float clearColor[] = { 0.47f, 0.78f, 0.89f, 1.0f };
	D_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	D_CommandList->ClearDepthStencilView(rtvHandle1, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);
	//D_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	D_CommandList->IASetVertexBuffers(0, 1, &D_VertexBufferView);
	D_CommandList->DrawInstanced(PlaneVertecies.size(), 1, 0, 0);

	
	// --------------------------------------------------------------

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	bool show_another_window = true;
	
	ImGui::Begin("Setting", &show_another_window, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
	ImGui::ColorEdit3("Color Overlay", &PSConstantBuffer.ColorOverlay.X);
	ImGui::SliderFloat("TesselationAmount", &PSConstantBuffer.TesselationAmount, 1.0f, 64.0f);
	ImGui::SliderFloat("Height", &PSConstantBuffer.Height, 0.0f, 10.0f);
	ImGui::End();

	ImGui::EndFrame();

	ppHeaps[0] = D_ImguiDescriptorHeap.Get();
	D_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), D_CommandList.Get());
	// --------------------------------------------------------------

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(D_ChainTargets[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	D_CommandList->ResourceBarrier(1, &barrier);
	
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(D_ChainDSTargets[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PRESENT);
	D_CommandList->ResourceBarrier(1, &barrier);

	D_CommandList->Close();

	// --------------------------------------------------------------
	ID3D12CommandList* ppCommandLists[] = { D_CommandList.Get() };
	D_CommandQue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	D_SwapChain->Present(1, 0);

	WaitForPreviousFrame();

}

void AppShutdown()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// ---------------------------------------------------------------------------------------------------
// ---------- Entry point ----------------------------------------------------------------------------

int WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
	_In_ int nShowCmd )
{
	InitMainWindow(hInstance);

	AllocConsole();
	static std::ofstream conout("CONOUT$", std::ios::out);
	std::cout.rdbuf(conout.rdbuf());

	auto Now = std::chrono::high_resolution_clock().now();
	TimeMiliseconds = (float)std::chrono::duration_cast<std::chrono::milliseconds>
		(Now.time_since_epoch()).count();

	AppInit();

	while (bOpenWindow)
	{
		{
			MSG Msg;
			bool bResult = PeekMessageW(&Msg, MainWindow, 0, 0, PM_REMOVE) > 0;

			if (bResult)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}

		Now = std::chrono::high_resolution_clock().now();
		float TickTime = (float)std::chrono::duration_cast<std::chrono::milliseconds>
			(Now.time_since_epoch()).count();
		float DeltaTime = TickTime - TimeMiliseconds;
		TimeMiliseconds = TickTime;


		POINT P = POINT();
		GetCursorPos(&P);

		ScreenToClient(MainWindow, &P);

		float NewX = Math::Clamp((float)P.x / WindowWidth, 0.0f, 1.0f);
		float NewY = Math::Clamp((float)P.y / WindowHeight, 0.0f, 1.0f);

		MouseDeltaX = NewX - MouseX;
		MouseDeltaY = NewY - MouseY;
		MouseX = NewX;
		MouseY = NewY;


		AppTick(DeltaTime);
	}

	AppShutdown();

	UnregisterClassW(WindowClassName, hInstance);

	return 0;
}
