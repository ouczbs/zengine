#pragma once
#include "refl/pch.h"
#include <cstdint>
namespace api {
    using pmr::Name;
    enum class ShaderDescriptorType : uint8_t {
        UNIFORM_BUFFER,
        SAMPLER,
    };
    enum class ShaderStage : uint8_t {
        NONE = 0,
        VERTEX = 0x1,
        FRAGMENT = 0x2,
    };
	enum class GraphicsAPI : uint8_t
	{
		OpenGL,
		Vulkan,
		D3D12
	};
    struct MaterialResource {
        Name name;
        bool isDirty;
        bool isUnique;
        refl::Any obj;
        void* Ptr()const {
            return (void*)obj.ptr;
        }
        uint32_t Size()const {
            return obj.cls ? obj.cls->size : 0;
        }
    };
    struct MaterialResourceBlock {
        uint32_t count;
        uint32_t blockSize;
        MaterialResource* pResource;
        static MaterialResourceBlock FromFields(std::span<const refl::FieldPtr> fields);
        operator bool() {
            return pResource;
        }
        MaterialResource* FindMaterialResource(Name name) const {
            for (uint32_t i = 0; i < count; i++) {
                MaterialResource* res = (pResource + i);
                if (res->name == name) {
                    return res;
                }
            }
            return nullptr;
        }
        void Upload(void** ppMappingAddr) const {
            for (uint32_t i = 0; i < count; i++) {
                auto res = pResource[i];
                if (res.isDirty || res.isUnique) {
                    memcpy(*(ppMappingAddr + i), res.Ptr(), res.Size());
                }
            }
        }
    };
    struct MaterialGpuResourceBlock {
        uint16_t blockSize;
        uint16_t bufferOffset;
        bool  isInit;
        void** pMappingAddr;
        operator bool() {
            return isInit;
        }
        void* BufferPtr() {
            return (char*)pMappingAddr + bufferOffset;
        }
    };
    struct MaterialInfo {
        bool isInit = false;
        MaterialGpuResourceBlock gpuBlock;
        MaterialResourceBlock  classBlock;
        MaterialResourceBlock staticBlock;
    };
    struct ShaderDescriptorLayout {
        Name     name;
        uint32_t set : 8;
        uint32_t binding : 8;
        uint32_t size : 16;
        ShaderDescriptorType type;
        bool        isShared;
        ShaderStage stageFlags;
    };
    using ShaderDescriptorSet = pmr::vector<ShaderDescriptorLayout>;
}