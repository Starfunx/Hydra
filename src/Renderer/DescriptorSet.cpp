#include "DescriptorSet.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace hyd {

// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
  return std::make_unique<DescriptorSetLayout>(m_device, bindings);
}

// *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(
    Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : m_device{device}, bindings{bindings} {
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(
          m_device.device(),
          &descriptorSetLayoutInfo,
          nullptr,
          &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

DescriptorSetLayout::~DescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(m_device.device(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
  return std::make_unique<DescriptorPool>(m_device, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(
    Device &device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : m_device{device} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(m_device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

DescriptorPool::~DescriptorPool() {
  vkDestroyDescriptorPool(m_device.device(), descriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(m_device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(
      m_device.device(),
      descriptorPool,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
}

void DescriptorPool::resetPool() {
  vkResetDescriptorPool(m_device.device(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool)
    : setLayout{setLayout}, pool{pool} {}

DescriptorWriter &DescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  writes.push_back(write);
  return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo *imageInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;

  writes.push_back(write);
  return *this;
}

bool DescriptorWriter::build(VkDescriptorSet &set) {
  bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool.m_device.device(), writes.size(), writes.data(), 0, nullptr);
}

/** New more centralized allocator **/
/*********** Descriptor Allocator ********/

VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
{
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(poolSizes.sizes.size());
    for (auto sz : poolSizes.sizes) {
        sizes.push_back({ sz.first, uint32_t(sz.second * count) });
    }
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = flags;
    pool_info.maxSets = count;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

    return descriptorPool;
}


DescriptorAllocator::DescriptorAllocator(Device& device):
    m_device{device}
{
}

DescriptorAllocator::~DescriptorAllocator()
{
    //delete every pool held
    for (auto p : m_freePools)
    {
        vkDestroyDescriptorPool(m_device.device(), p, nullptr);
    }
    for (auto p : m_usedPools)
    {
        vkDestroyDescriptorPool(m_device.device(), p, nullptr);
    }
}


void DescriptorAllocator::reset_pools()
{
    for (auto p : m_usedPools)
    {
        vkResetDescriptorPool(m_device.device(), p, 0);
    }

    m_freePools = m_usedPools;
    m_usedPools.clear();
    currentPool = VK_NULL_HANDLE;
}


bool DescriptorAllocator::allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
{
    if (currentPool == VK_NULL_HANDLE)
    {
        currentPool = grab_pool();
        m_usedPools.push_back(currentPool);
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;

    allocInfo.pSetLayouts = &layout;
    allocInfo.descriptorPool = currentPool;
    allocInfo.descriptorSetCount = 1;		
    

    VkResult allocResult = vkAllocateDescriptorSets(m_device.device(), &allocInfo, set);
    bool needReallocate = false;

    switch (allocResult) {
    case VK_SUCCESS:
        //all good, return
        return true;

        break;
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        //reallocate pool
        needReallocate = true;
        break;
    default:
        //unrecoverable error
        return false;
    }
    
    if (needReallocate)
    {
        //allocate a new pool and retry
        currentPool = grab_pool();
        m_usedPools.push_back(currentPool);

        allocResult = vkAllocateDescriptorSets(m_device.device(), &allocInfo, set);

        //if it still fails then we have big issues
        if (allocResult == VK_SUCCESS)
        {
            return true;
        }
    }

    return false;
}


VkDescriptorPool DescriptorAllocator::grab_pool()
{
    if (m_freePools.size() > 0)
    {
        VkDescriptorPool pool = m_freePools.back();
        m_freePools.pop_back();
        return pool;
    }
    else {
        return createPool(m_device.device(), descriptorSizes, 1000, 0);
    }
}

/*********** Descriptor Latyout Cache ********/
DescriptorLayoutCache::DescriptorLayoutCache(Device& device)
: m_device{device}
{
}

DescriptorLayoutCache::~DescriptorLayoutCache(){
    //delete every descriptor layout held
    for (auto pair : m_layoutCache)
    {
        vkDestroyDescriptorSetLayout(m_device.device(), pair.second, nullptr);
    }
}


VkDescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info)
{
    DescriptorLayoutInfo layoutinfo;
    layoutinfo.bindings.reserve(info->bindingCount);
    bool isSorted = true;
    int32_t lastBinding = -1;
    for (uint32_t i = 0; i < info->bindingCount; i++) {
        layoutinfo.bindings.push_back(info->pBindings[i]);

        //check that the bindings are in strict increasing order
        if (static_cast<int32_t>(info->pBindings[i].binding) > lastBinding)
        {
            lastBinding = info->pBindings[i].binding;
        }
        else{
            isSorted = false;
        }
    }
    if (!isSorted)
    {
        std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b ) {
            return a.binding < b.binding;
        });
    }
    
    auto it = m_layoutCache.find(layoutinfo);
    if (it != m_layoutCache.end())
    {
        return (*it).second;
    }
    else {
        VkDescriptorSetLayout layout;
        vkCreateDescriptorSetLayout(m_device.device(), info, nullptr, &layout);

        //layoutCache.emplace()
        //add to cache
        m_layoutCache[layoutinfo] = layout;
        return layout;
    }
}


bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
{
    if (other.bindings.size() != bindings.size())
    {
        return false;
    }
    else {
        //compare each of the bindings is the same. Bindings are sorted so they will match
        for (int i = 0; i < bindings.size(); i++) {
            if (other.bindings[i].binding != bindings[i].binding)
            {
                return false;
            }
            if (other.bindings[i].descriptorType != bindings[i].descriptorType)
            {
                return false;
            }
            if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
            {
                return false;
            }
            if (other.bindings[i].stageFlags != bindings[i].stageFlags)
            {
                return false;
            }
        }
        return true;
    }
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
{
    using std::size_t;
    using std::hash;

    size_t result = hash<size_t>()(bindings.size());

    for (const VkDescriptorSetLayoutBinding& b : bindings)
    {
        //pack the binding data into a single int64. Not fully correct but its ok
        size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

        //shuffle the packed binding data and xor it with the main hash
        result ^= hash<size_t>()(binding_hash);
    }

    return result;
}

/*********** Descriptor Builder ********/

   
DescriptorBuilder::DescriptorBuilder(DescriptorLayoutCache& layoutCache, DescriptorAllocator& allocator)
: m_cache{layoutCache}, m_alloc{allocator}
{}


DescriptorBuilder& DescriptorBuilder::bind_buffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
    VkDescriptorSetLayoutBinding newBinding{};

    newBinding.descriptorCount = 1;
    newBinding.descriptorType = type;
    newBinding.pImmutableSamplers = nullptr;
    newBinding.stageFlags = stageFlags;
    newBinding.binding = binding;

    m_bindings.push_back(newBinding);

    VkWriteDescriptorSet newWrite{};
    newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    newWrite.pNext = nullptr;

    newWrite.descriptorCount = 1;
    newWrite.descriptorType = type;
    newWrite.pBufferInfo = bufferInfo;
    newWrite.dstBinding = binding;

    m_writes.push_back(newWrite);
    return *this;
}


DescriptorBuilder& DescriptorBuilder::bind_image(uint32_t binding,  VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
    VkDescriptorSetLayoutBinding newBinding{};

    newBinding.descriptorCount = 1;
    newBinding.descriptorType = type;
    newBinding.pImmutableSamplers = nullptr;
    newBinding.stageFlags = stageFlags;
    newBinding.binding = binding;

    m_bindings.push_back(newBinding);

    VkWriteDescriptorSet newWrite{};
    newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    newWrite.pNext = nullptr;

    newWrite.descriptorCount = 1;
    newWrite.descriptorType = type;
    newWrite.pImageInfo = imageInfo;
    newWrite.dstBinding = binding;

    m_writes.push_back(newWrite);
    return *this;
}

bool DescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
{
    //build layout first
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;

    layoutInfo.pBindings = m_bindings.data();
    layoutInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());

    layout = m_cache.create_descriptor_layout(&layoutInfo);


    //allocate descriptor
    bool success = m_alloc.allocate(&set, layout);
    if (!success) { return false; };

    //write descriptor

    for (VkWriteDescriptorSet& w : m_writes) {
        w.dstSet = set;
    }

    vkUpdateDescriptorSets(m_alloc.m_device.device(), static_cast<uint32_t>(m_writes.size()), m_writes.data(), 0, nullptr);

    return true;
}


bool DescriptorBuilder::build(VkDescriptorSet& set)
{
    VkDescriptorSetLayout layout;
    return build(set, layout);
}


}  // namespace hyd