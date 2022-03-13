#pragma once

#include "Device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace hyd {

class DescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(Device &device) : m_device{device} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<DescriptorSetLayout> build() const;

   private:
    Device &m_device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  DescriptorSetLayout(
      Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~DescriptorSetLayout();
  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
  Device &m_device;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class DescriptorWriter;
};

class DescriptorPool {
 public:
  class Builder {
   public:
    Builder(Device &device) : m_device{device} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<DescriptorPool> build() const;

   private:
    Device& m_device;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  DescriptorPool(
      Device &m_device,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~DescriptorPool();
  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool &operator=(const DescriptorPool &) = delete;

  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  Device &m_device;
  VkDescriptorPool descriptorPool;

  friend class DescriptorWriter;
};

class DescriptorWriter {
 public:
  DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

  DescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  DescriptorSetLayout &setLayout;
  DescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

/*New more centralized allocator*/

class DescriptorAllocator
{
public:

    struct PoolSizes {
        std::vector<std::pair<VkDescriptorType,float>> sizes =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
        };
    };

    DescriptorAllocator(Device& device);
    ~DescriptorAllocator();

    DescriptorAllocator(const DescriptorAllocator&) = delete;
    DescriptorAllocator operator=(const DescriptorAllocator&) = delete;

    bool allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);
    void reset_pools();

private:
    VkDescriptorPool grab_pool();

    /* data */
    Device& m_device;

    VkDescriptorPool currentPool{VK_NULL_HANDLE};
    PoolSizes descriptorSizes;

    std::vector<VkDescriptorPool> m_freePools;
    std::vector<VkDescriptorPool> m_usedPools;

friend class DescriptorBuilder;

};


class DescriptorLayoutCache {
public:
    DescriptorLayoutCache(Device& device);
    ~DescriptorLayoutCache();

    VkDescriptorSetLayout create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info);

    struct DescriptorLayoutInfo {
        bool operator==(const DescriptorLayoutInfo& other) const;
        size_t hash() const;

        //good idea to turn this into a inlined array
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

private:
    struct DescriptorLayoutHash
    {
        std::size_t operator()(const DescriptorLayoutInfo& k) const
        {
            return k.hash();
        }
    };

    std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_layoutCache;
    Device& m_device;
};


class DescriptorBuilder {
public:

    DescriptorBuilder(DescriptorLayoutCache& layoutCache, DescriptorAllocator& allocator );
    DescriptorBuilder& bind_buffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
    DescriptorBuilder& bind_image(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

    bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
    bool build(VkDescriptorSet& set);
private:
    
    std::vector<VkWriteDescriptorSet> m_writes;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;

    DescriptorLayoutCache& m_cache;
    DescriptorAllocator& m_alloc;

};

}  // namespace hyd