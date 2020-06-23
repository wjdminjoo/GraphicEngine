#ifndef __AKILIB_D3D12HELPER__
#define __AKILIB_D3D12HELPER__

#include <tchar.h>
#include <Windows.h>

#include <memory>

#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <wrl.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3DCompiler.lib")

bool showErrorMessage(HRESULT hr, const LPTSTR text)
{
	if (FAILED(hr))
	{
		MessageBox(nullptr, text, TEXT("Error"), MB_OK);
		return true;
	}

	return false;
}

template<typename T>
void safeRelease(T*& object)
{
	if (object)
	{
		object->Release();
		object = nullptr;
	}
}

bool compileShaderFlomFile(LPCWSTR pFileName, LPCSTR pEntrypoint, LPCSTR pTarget, ID3DBlob** ppblob)
{
	HRESULT hr;
	LPD3DBLOB pError = nullptr;
	hr = D3DCompileFromFile(
		pFileName,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		pEntrypoint,
		pTarget,
		0,
		0,
		ppblob,
		&pError);

	if (FAILED(hr))
	{
		if (pError && pError->GetBufferPointer())
		{
#ifdef _UNICODE
			int bufferSize = static_cast<int>(pError->GetBufferSize());
			int strLen = MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<CHAR*>(pError->GetBufferPointer()), bufferSize, nullptr, 0);

			std::shared_ptr<wchar_t> errorStr(new wchar_t[strLen]);
			MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<char*>(pError->GetBufferPointer()), bufferSize, errorStr.get(), strLen);

			OutputDebugString(errorStr.get());
#else
			OutputDebugString(reinterpret_cast<LPSTR>(pError->GetBufferPointer()));
#endif
		}

		safeRelease(pError);

		return false;
	}

	return true;
}

void setResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER descBarrier = {};
	ZeroMemory(&descBarrier, sizeof(descBarrier));

	descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descBarrier.Transition.pResource = resource;
	descBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	descBarrier.Transition.StateBefore = StateBefore;
	descBarrier.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &descBarrier);
}

#endif	// __AKILIB_D3D12HELPER__