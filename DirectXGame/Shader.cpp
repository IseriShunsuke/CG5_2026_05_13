#include "Shader.h"
#include <d3dcompiler.h>
#include <cassert>

void Shader::Load(const std::wstring& filePath, const std::string& shaderModel) 
{
	ID3DBlob* shaderblob = nullptr;
	ID3DBlob* errorblob = nullptr;

	HRESULT hr =
	    D3DCompileFromFile(filePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", shaderModel.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shaderblob, &errorblob);
	if (FAILED(hr)) {
		if (errorblob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorblob->GetBufferPointer()));
			errorblob->Release();
		}
		assert(false);
	}
	blob_ = shaderblob;
}

ID3DBlob* Shader::GetBlob() { return blob_; }

Shader::Shader() {}

Shader::~Shader() 
{ 
	if (blob_ != nullptr)
	{
		blob_->Release();
		blob_ = nullptr;
	}
}
