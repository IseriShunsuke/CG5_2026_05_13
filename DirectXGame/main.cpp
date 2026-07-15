#include "IndexBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "kamataEngine.h"
#include "worldTransformEx.h"
#include <Windows.h>
#include <cassert>

using namespace KamataEngine;

ID3DBlob* CompileShader(const std::wstring& filePath, const std::string& shaderModel);

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const FLOAT* clearColor);

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[2] = {};
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDesc[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDesc[1].SemanticName = "TEXCOORD";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = _countof(inputElementDesc);

	D3D12_BLEND_DESC blendDesc{};

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipeLineStateDesc{};
	graphicsPipeLineStateDesc.pRootSignature = rs.Get();
	graphicsPipeLineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipeLineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()};
	graphicsPipeLineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()};
	graphicsPipeLineStateDesc.BlendState = blendDesc;
	graphicsPipeLineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPipeLineStateDesc.NumRenderTargets = 1;
	graphicsPipeLineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPipeLineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipeLineStateDesc.SampleDesc.Count = 1;
	graphicsPipeLineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	pipelineState.Create(graphicsPipeLineStateDesc);
}

ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT clearFormat, const FLOAT* clearColor)
{ 
	D3D12_RESOURCE_DESC resourceDesc{};

	resourceDesc.Width = UINT(width);
	resourceDesc.Height = UINT(height);
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = clearFormat;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height)
{ 
	D3D12_RESOURCE_DESC resourceDesc{};

	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"LE3D_03_イセリ_シュンスケ");

	DirectXCommon* dxcommon = DirectXCommon::GetInstance();

	int w = dxcommon->GetBackBufferWidth();
	int h = dxcommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("with : {},height : {}\n", w, h).c_str());

	ID3D12GraphicsCommandList* commondList = dxcommon->GetCommandList();

	RootSignature rs;
	rs.Create();

	Shader vs;
	Shader ps;

	PipelineState pipelinaState;
	SetupPipelineState(pipelinaState, rs, vs, ps);

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	VertexData vertices[] = {
	    {{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, 
		{{1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}},
	};

	VertexBuffer vb;
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));
	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	uint16_t indices[] = {0, 1, 2, 0, 2, 3};

	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));
	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	
	ID3D12Device* device = dxcommon->GetDevice();
	HRESULT hr;

	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	
	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);

	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	device->CreateRenderTargetView(renderTextureResource, nullptr, rtvHandleCPU);


	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);


	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc{};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescriptorHeapDesc.NumDescriptors = 1;
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(renderTextureResource, &srvDesc, srvHandleCPU);


	Model* model = Model::CreateFromOBJ("terrain");

	worldTransformEx worldTransform;
	worldTransform.Initialize();
	worldTransform.scale_ = Vector3(1.0f, 1.0f, 1.0f);

	Camera camera;
	camera.Initialize();
	camera.translation_ = Vector3(0.0f, 1.0f, 0.0f);



	/*vb.Get()->Unmap(0, nullptr);*/

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}
		worldTransform.rotation_.y += 0.005f;
		worldTransform.UpdateMatrix();

		camera.UpdateMatrix();

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commondList->ResourceBarrier(1, &barrier);

		commondList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::kWindowWidth;
		viewport.Height = WinApp::kWindowHeight;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		commondList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.right = WinApp::kWindowWidth;
		scissorRect.top = 0;
		scissorRect.bottom = WinApp::kWindowHeight;

		commondList->RSSetScissorRects(1, &scissorRect);

		commondList->ClearRenderTargetView(rtvHandleCPU, kRenderTargetClearColor, 0, nullptr);

		commondList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		Model::PreDraw();
		model->Draw(worldTransform, camera);
		Model::PostDraw();

		dxcommon->PreDraw();

		commondList->SetGraphicsRootSignature(rs.Get());
		commondList->SetPipelineState(pipelinaState.Get());
		commondList->IASetVertexBuffers(0, 1, vb.GetView());
		commondList->IASetIndexBuffer(ib.GetView());

		commondList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		commondList->SetDescriptorHeaps(srvDescriptorHeap->GetDesc().NumDescriptors, &srvDescriptorHeap);

		commondList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);


		/*commondList->DrawInstanced(3, 1, 0, 0);*/
		commondList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		


		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commondList->ResourceBarrier(1, &barrier);


		dxcommon->PostDraw();
	}

	delete model;

	renderTextureResource->Release();
	srvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();

	depthStencilResource->Release();
	dsvDescriptorHeap->Release();

	KamataEngine::Finalize();

	return 0;
}