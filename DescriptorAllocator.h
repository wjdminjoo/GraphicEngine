#pragma once
#include "DesciptorAllocation.h"
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

};

