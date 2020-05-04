#include "DescriptorAllocator.h"
#include <DX12LibPCH.h>

#include <DescriptorAllocator.h>
#include <DescriptorAllocatorPage.h>

DescriptorAllocator::~DescriptorAllocator()
{
}

DescriptorAllocation DescriptorAllocator::Allocate(uint32_t numDescriptor)
{
	return DescriptorAllocation();
}

void DescriptorAllocator::ReleaseStaleDescriptors(uint64_t frameNumber)
{
}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
{
	return std::shared_ptr<DescriptorAllocatorPage>();
}
