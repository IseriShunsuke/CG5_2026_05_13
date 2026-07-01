#include "IndexBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "kamataEngine.h"
#include <Windows.h>
#include <cassert>

using namespace KamataEngine;

ID3DBlob* CompileShader(const std::wstring& filePath, const std::string& shaderModel);

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

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

	/*vb.Get()->Unmap(0, nullptr);*/

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		dxcommon->PreDraw();

		commondList->SetGraphicsRootSignature(rs.Get());
		commondList->SetPipelineState(pipelinaState.Get());
		commondList->IASetVertexBuffers(0, 1, vb.GetView());
		commondList->IASetIndexBuffer(ib.GetView());

		commondList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		/*commondList->DrawInstanced(3, 1, 0, 0);*/
		commondList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		dxcommon->PostDraw();
	}

	KamataEngine::Finalize();

	return 0;
}