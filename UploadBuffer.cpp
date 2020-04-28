#include <DX12LibPCH.h>

#include <UploadBuffer.h>

#include <Application.h>
#include <Helpers.h>
#include <cmath>

#include <new> // for std::bad_alloc
// Math::AlignUp

UploadBuffer::UploadBuffer(size_t pageSize) : m_PageSize(pageSize){
	// Lazy Initialization의 전형적인 예
}

UploadBuffer::Allocation UploadBuffer::Allocate(size_t sizeInBytes, size_t alignment) {
	if (sizeInBytes > m_PageSize) {
		// 할당 크기가 메모리 페이지 크기를 초과하면 예외 발생
		throw std::bad_alloc();
	}

	if (!m_CurrentPage || !m_CurrentPage->HasSpace(sizeInBytes, alignment)) {
		m_CurrentPage = RequestPage();
	}

	return m_CurrentPage->Allocate(sizeInBytes, alignment);
}

std::shared_ptr<UploadBuffer::Page> UploadBuffer::RequestPage() {
	std::shared_ptr<Page> page;

	if (!m_AvailablePages.empty()) {
		page = m_AvailablePages.front();
		m_AvailablePages.pop_front();
	}
	else {
		page = std::make_shared<Page>(m_PageSize);
		m_PagePool.push_back(page);
	}
	return page;
}

void UploadBuffer::Reset()
{
	m_CurrentPage = nullptr;
	// 모든 메모리 할당을 재설정
	m_AvailablePages = m_PagePool;

	for (auto page : m_AvailablePages)
	{
		page->Reset();
	}
}

UploadBuffer::Page::Page(size_t sizeInBytes)
	: m_PageSize(sizeInBytes)
	, m_Offset(0)
	, m_CPUPtr(nullptr)
	, m_GPUPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
	auto device = Application::Get().GetDevice();

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_PageSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_d3d12Resource)
	));

	m_GPUPtr = m_d3d12Resource->GetGPUVirtualAddress();
	m_d3d12Resource->Map(0, nullptr, &m_CPUPtr);
}

UploadBuffer::Page::~Page()
{
	m_d3d12Resource->Unmap(0, nullptr);
	m_CPUPtr = nullptr;
	m_GPUPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool UploadBuffer::Page::HasSpace(size_t sizeInBytes, size_t alignment) const
{
	size_t alignedSize = Math::AlignUp(sizeInBytes, alignment);
	size_t alignedOffset = Math::AlignUp(m_Offset, alignment);

	return alignedOffset + alignedSize <= m_PageSize;
}

UploadBuffer::Allocation UploadBuffer::Page::Allocate(size_t sizeInBytes, size_t alignment)
{
	if (!HasSpace(sizeInBytes, alignment))
	{
		// Can't allocate space from page.
		throw std::bad_alloc();
	}

	size_t alignedSize = Math::AlignUp(sizeInBytes, alignment);
	m_Offset = Math::AlignUp(m_Offset, alignment);

	Allocation allocation;
	allocation.CPU = static_cast<uint8_t*>(m_CPUPtr) + m_Offset;
	allocation.GPU = m_GPUPtr + m_Offset;

	m_Offset += alignedSize;

	return allocation;
}

void UploadBuffer::Page::Reset()
{
	m_Offset = 0;
}