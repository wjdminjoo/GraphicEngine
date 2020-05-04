#pragma once
#include "DescriptorAllocation.h"
#include <d3d12.h>
#include <cstdint> // 정수 고정 폭
#include <mutex> // mutex를 이용 여러 스레드에서 안전하게 할당 수행
#include <memory> // 포인터 클래스
#include <set> // 컨테이너 클래스. 사용 가능한 페이지에 대한 정렬 된 인덱스 목록 저장
#include <vector> // 컨테이너 클래스

class DescriptorAllocatorPage;


class DescriptorAllocator
{

	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256);
	virtual ~DescriptorAllocator();

	// CPU 표시 디스크립터 힙에서 여러개의 연속 디스크립터를 할당
	DescriptorAllocation Allocate(uint32_t numDescriptor = 1);

	// 재사용 가능한 설명자 목록으로 리턴 될 수 있도록하고 오래된 설명자는 해제.
	void ReleaseStaleDescriptors(uint64_t frameNumber);

private:
	using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

	std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();

	D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
	uint32_t m_NumDescriptorsPerHeap;

	DescriptorHeapPool m_HeapPool;
	std::set<size_t> m_AvailableHeaps;

	std::mutex m_AllocationMutex;
};

