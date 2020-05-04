#pragma once

#include <Defines.h>

#include <wrl.h>
#include <d3d12.h>

#include <memory>
#include <deque>

class UploadBuffer
{

public:
	struct Allocation
	{
		void* CPU;
		D3D12_GPU_VIRTUAL_ADDRESS GPU;
	};

	explicit UploadBuffer(size_t pageSize = _2MB); // 기본 크기가 2MB
	virtual ~UploadBuffer() {}
	size_t GetPageSize() const { return m_PageSize; }
	Allocation Allocate(size_t sizeInBytes, size_t alignment);
	void Reset();
private:
	struct Page
	{
		Page(size_t sizeInBytes);
		~Page();
		bool HasSpace(size_t sizeInBytes, size_t alignment) const;
		Allocation Allocate(size_t sizeInBytes, size_t alignment);

		void Reset();
	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;

		// Base pointer.
		void* m_CPUPtr;
		D3D12_GPU_VIRTUAL_ADDRESS m_GPUPtr;

		// Allocated page size.
		size_t m_PageSize;
		// Current allocation offset in bytes.
		size_t m_Offset;
	};

	using PagePool = std::deque<std::shared_ptr<Page>>;
	std::shared_ptr<Page> RequestPage();
	PagePool m_PagePool;
	PagePool m_AvailablePages;

	std::shared_ptr<Page> m_CurrentPage;
	size_t m_PageSize;
};

