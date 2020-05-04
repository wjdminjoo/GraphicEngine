#pragma once
#include "DescriptorAllocation.h"
#include <d3d12.h>
#include <cstdint> // ���� ���� ��
#include <mutex> // mutex�� �̿� ���� �����忡�� �����ϰ� �Ҵ� ����
#include <memory> // ������ Ŭ����
#include <set> // �����̳� Ŭ����. ��� ������ �������� ���� ���� �� �ε��� ��� ����
#include <vector> // �����̳� Ŭ����

class DescriptorAllocatorPage;


class DescriptorAllocator
{

	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256);
	virtual ~DescriptorAllocator();

	// CPU ǥ�� ��ũ���� ������ �������� ���� ��ũ���͸� �Ҵ�
	DescriptorAllocation Allocate(uint32_t numDescriptor = 1);

	// ���� ������ ������ ������� ���� �� �� �ֵ����ϰ� ������ �����ڴ� ����.
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

