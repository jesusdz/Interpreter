/*
 * tools.h
 * Author: Jesus Diaz Garcia
 *
 * Graphics API abstraction made on top of Vulkan.
 *
 * Some concepts abstracted from the low-level API are:
 * - Device creation
 * - Bind groups
 * - Render passes
 * - Pipelines
 */

#ifndef TOOLS_GFX_H
#define TOOLS_GFX_H


////////////////////////////////////////////////////////////////////////
// Includes and definitions
////////////////////////////////////////////////////////////////////////

#include "tools.h"

#if USE_XCB
#	define VK_USE_PLATFORM_XCB_KHR 1
#elif USE_ANDROID
#	define VK_USE_PLATFORM_ANDROID_KHR 1
#elif USE_WINAPI
#	define VK_USE_PLATFORM_WIN32_KHR 1
#else
#	error "Configure the proper platform for Vulkan"
#endif

// This extension removes a VK_ERROR_INCOMPATIBLE_DRIVER when calling vkCreateInstance
// on some platforms implementing Vulkan on top of another API.
// An example of this could be Apple machines with MoltenVK on top of Metal.
#define USE_VK_EXT_PORTABILITY_ENUMERATION 0

#define VOLK_IMPLEMENTATION
#include "volk/volk.h"

#if USE_IMGUI

// Unity build Dear ImGui files
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"

#if PLATFORM_WINDOWS
#include "imgui/imgui_impl_win32.cpp"
#elif PLATFORM_LINUX
#include "imgui/imgui_impl_xcb.cpp"
#else
#error "Missing ImGui implementation file"

#endif // #if USE_IMGUI

// Undefining VK_NO_PROTOTYPES here to avoid ImGui to retrieve Vulkan functions again.
#undef VK_NO_PROTOTYPES
#include "imgui/imgui_impl_vulkan.cpp"
#endif

#define SPV_ASSERT ASSERT
#define SPV_PRINTF(...) LOG(Info, ##__VA_ARGS__)
#define SPV_IMPLEMENTATION
#define SPV_PRINT_FUNCTIONS
#include "tools_spirv.h"

#define MAX_DESCRIPTOR_SETS 4
#define MAX_SHADER_BINDINGS 16
#if PLATFORM_ANDROID
#define MAX_SWAPCHAIN_IMAGE_COUNT 5
#else
#define MAX_SWAPCHAIN_IMAGE_COUNT 3
#endif
#define MAX_FRAMES_IN_FLIGHT 2

#define VULKAN_ALLOCATORS NULL

#define VK_CALL( call ) CheckVulkanResult( call, #call )


////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////

enum HeapType
{
	HeapType_General,
	HeapType_RTs,
	HeapType_Staging,
	HeapType_Dynamic,
	HeapType_Readback,
	HeapType_COUNT,
};

enum BorderColor
{
	BorderColorBlackInt,
	BorderColorWhiteInt,
	BorderColorBlackFloat,
	BorderColorWhiteFloat,
	BorderColorCount,
};

enum AddressMode
{
	AddressModeRepeat,
	AddressModeClampToBorder,
	AddressModeCount,
};

enum CompareOp
{
	CompareOpNone,
	CompareOpLess,
	CompareOpGreater,
	CompareOpCount,
};

struct Heap
{
	HeapType type;
	u32 size;
	u32 memoryTypeIndex;
	VkDeviceMemory memory;
	u8* data;
	u32 used;
};

struct Alloc
{
	HeapType heap;
	u64 offset;
	u64 size;
};

struct BufferBinding
{
	VkBuffer handle;
	u32 offset;
	u32 range;
};

struct BufferViewBinding
{
	VkBufferView handle;
};

struct TextureBinding
{
	VkImageView handle;
};

struct SamplerBinding
{
	VkSampler handle;
};

union ResourceBinding
{
	BufferBinding buffer;
	BufferViewBinding bufferView;
	TextureBinding texture;
	SamplerBinding sampler;
};

struct ShaderBinding
{
	u8 set;
	u8 binding;
	SpvType type;
	SpvStageFlags stageFlags;
	const char *name;
};

struct ShaderBindings
{
	ShaderBinding bindings[MAX_SHADER_BINDINGS];
	u8 bindingCount;
};

struct BindGroupAllocatorCounts
{
	u32 uniformBufferCount;
	u32 storageBufferCount;
	u32 storageTexelBufferCount;
	u32 textureCount;
	u32 samplerCount;
	u32 combinedImageSamplerCount;
	u32 groupCount;
	bool allowIndividualFrees;
};

struct BindGroupAllocator
{
	BindGroupAllocatorCounts maxCounts;
	BindGroupAllocatorCounts usedCounts;
	VkDescriptorPool handle;
};

struct BindGroupLayout
{
	VkDescriptorSetLayout handle;
	const ShaderBinding *bindings;
	u8 bindingCount;
};

struct BindGroupDesc
{
	BindGroupLayout layout;
	ResourceBinding bindings[MAX_SHADER_BINDINGS];
};

struct BindGroup
{
	VkDescriptorSet handle;
};

struct PipelineLayout
{
	VkPipelineLayout handle;
	BindGroupLayout bindGroupLayouts[MAX_DESCRIPTOR_SETS];
	ShaderBindings shaderBindings;
};

struct Pipeline
{
	const char *name;
	VkPipeline handle;
	PipelineLayout layout;
	VkPipelineBindPoint bindPoint;
};

typedef u32 PipelineH;

struct Buffer
{
	VkBuffer handle;
	Alloc alloc;
	u32 size;
};

typedef u32 BufferH;

struct BufferArena
{
	BufferH buffer;
	u32 used;
	u32 size;
};

struct BufferChunk
{
	BufferH buffer;
	u32 offset;
	u32 size;
};

struct BufferView
{
	VkBufferView handle;
};

typedef u32 BufferViewH;

struct Image
{
	VkImage image;
	VkFormat format;
	Alloc alloc;
};

struct Texture
{
	const char *name;
	Image image;
	u32 mipLevels;
	VkImageView imageView;
};

typedef u32 TextureH;

struct ShaderSource
{
	u8 *data;
	u64 dataSize;
};

struct ShaderModule
{
	VkShaderModule handle;
};

struct RenderPass
{
	const char *name;
	VkRenderPass handle;
};

typedef u32 RenderPassH;

struct Framebuffer
{
	VkFramebuffer handle;
	VkRenderPass renderPassHandle;
	VkExtent2D extent;
	bool isDisplay : 1;
	bool isShadowmap : 1;
};

struct SamplerDesc
{
	AddressMode addressMode;
	BorderColor borderColor;
	CompareOp compareOp;
};

struct Sampler
{
	VkSampler handle;
};

typedef u32 SamplerH;

struct SwapchainInfo
{
	VkFormat format;
	VkColorSpaceKHR colorSpace;
	VkPresentModeKHR presentMode;
};

struct Swapchain
{
	VkSwapchainKHR handle;
	VkExtent2D extent;
	u32 imageCount;
	VkImage images[MAX_SWAPCHAIN_IMAGE_COUNT];
	VkImageView imageViews[MAX_SWAPCHAIN_IMAGE_COUNT];
	float preRotationDegrees;
	bool outdated;
	u32 currentImageIndex;
};

struct Alignment
{
	u32 uniformBufferOffset;
	u32 optimalBufferCopyOffset;
	u32 optimalBufferCopyRowPitch;
	// u32 nonCoherentAtomSize;
	// VkExtent3D minImageTransferGranularity;
};

struct CommandList
{
	VkCommandBuffer handle;

	VkDescriptorSet descriptorSetHandles[4];
	u8 descriptorSetDirtyMask;

	const Pipeline *pipeline;

	// State
	union
	{
		struct // For gfx pipelines
		{
			VkBuffer vertexBufferHandle;
			VkBuffer indexBufferHandle;
		};
	};
};

struct SubmitResult
{
	VkSemaphore signalSemaphore;
};

struct GraphicsDevice
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice handle;

	Alignment alignment;

	u32 graphicsQueueFamilyIndex;
	u32 presentQueueFamilyIndex;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkCommandPool commandPools[MAX_FRAMES_IN_FLIGHT];
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
	VkCommandPool transientCommandPool;

	VkSurfaceKHR surface;
	SwapchainInfo swapchainInfo;
	Swapchain swapchain;

	VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
	VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

	VkPipelineCache pipelineCache;

	Heap heaps[HeapType_COUNT];

	u32 currentFrame;
};


////////////////////////////////////////////////////////////////////////
// Helper functions to map Vulkan types
////////////////////////////////////////////////////////////////////////

static const char *VkPhysicalDeviceTypeToString( VkPhysicalDeviceType type )
{
	static const char *toString[] = {
		"VK_PHYSICAL_DEVICE_TYPE_OTHER", // 0
		"VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU", // 1
		"VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU", // 2
		"VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU", // 3
		"VK_PHYSICAL_DEVICE_TYPE_CPU", // 4
	};
	ASSERT( type < ARRAY_COUNT(toString) );
	return toString[type];
}

#define CONCAT_FLAG(flag) if ( flags & flag ) StrCat(outString, #flag "|" );

static const char *VkMemoryPropertyFlagsToString( VkMemoryPropertyFlags flags, char *outString )
{
	*outString = 0;
	CONCAT_FLAG(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CONCAT_FLAG(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	CONCAT_FLAG(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	CONCAT_FLAG(VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
	CONCAT_FLAG(VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
	CONCAT_FLAG(VK_MEMORY_PROPERTY_PROTECTED_BIT);
	CONCAT_FLAG(VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
	CONCAT_FLAG(VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD);
	CONCAT_FLAG(VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV);
	return outString;
}

static const char *VkMemoryHeapFlagsToString( VkMemoryHeapFlags flags, char *outString )
{
	*outString = 0;
	CONCAT_FLAG(VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
	CONCAT_FLAG(VK_MEMORY_HEAP_MULTI_INSTANCE_BIT);
	return outString;
}

#undef CONCAT_FLAG

static const char *HeapTypeToString(HeapType heapType)
{
	static const char *toString[] = {
		"HeapType_General",
		"HeapType_RTs",
		"HeapType_Staging",
		"HeapType_Dynamic",
		"HeapType_Readback",
	};
	CT_ASSERT( ARRAY_COUNT(toString) == HeapType_COUNT );
	ASSERT( heapType < ARRAY_COUNT(toString) );
	return toString[heapType];
}

static VkMemoryPropertyFlags HeapTypeToVkMemoryPropertyFlags( HeapType heapType )
{
	static const VkMemoryPropertyFlags flags[] = {
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // HeapType_General,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // HeapType_RTs,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // HeapType_Staging,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // HeapType_Dynamic,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, // HeapType_Readback,
	};
	CT_ASSERT( ARRAY_COUNT(flags) == HeapType_COUNT );
	return flags[heapType];
};

static VkDescriptorType SpvDescriptorTypeToVulkan(SpvType type)
{
	static const VkDescriptorType vkDescriptorTypes[] = {
		VK_DESCRIPTOR_TYPE_MAX_ENUM,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // Should be DYNAMIC if we use dynamic offsets
		VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	};
	CT_ASSERT(ARRAY_COUNT(vkDescriptorTypes) == SpvTypeCount);
	ASSERT(type != SpvTypeNone);
	ASSERT(type < SpvTypeCount);
	const VkDescriptorType vkDescriptorType = vkDescriptorTypes[type];
	return vkDescriptorType;
}

static VkShaderStageFlags SpvStageFlagsToVulkan(u8 stageFlags)
{
	VkShaderStageFlags vkStageFlags = 0;
	vkStageFlags |= ( stageFlags & SpvStageFlagsVertexBit ) ? VK_SHADER_STAGE_VERTEX_BIT : 0;
	vkStageFlags |= ( stageFlags & SpvStageFlagsFragmentBit ) ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
	vkStageFlags |= ( stageFlags & SpvStageFlagsComputeBit ) ? VK_SHADER_STAGE_COMPUTE_BIT : 0;
	return vkStageFlags;
}

static VkFormat FormatToVulkan(Format format)
{
	ASSERT(format < FormatCount);
	static VkFormat vkFormats[] = {
		VK_FORMAT_R32G32_SFLOAT,
		VK_FORMAT_R32G32B32_SFLOAT,
	};
	CT_ASSERT(ARRAY_COUNT(vkFormats) == FormatCount);
	const VkFormat vkFormat = vkFormats[format];
	return vkFormat;
}

static VkBorderColor BorderColorToVulkan(BorderColor color)
{
	static const VkBorderColor vkBorderColors[] = {
		VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		VK_BORDER_COLOR_INT_OPAQUE_WHITE,
		VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	};
	CT_ASSERT(ARRAY_COUNT(vkBorderColors) == BorderColorCount);
	ASSERT(color < BorderColorCount);
	const VkBorderColor vkBorderColor = vkBorderColors[color];
	return vkBorderColor;
};

static VkSamplerAddressMode AddressModeToVulkan(AddressMode mode)
{
	static const VkSamplerAddressMode vkAddressModes[] = {
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	};
	CT_ASSERT(ARRAY_COUNT(vkAddressModes) == AddressModeCount);
	ASSERT(mode < AddressModeCount);
	const VkSamplerAddressMode vkAddressMode = vkAddressModes[mode];
	return vkAddressMode;
}

static VkCompareOp CompareOpToVulkan(CompareOp compareOp)
{
	static const VkCompareOp vkCompareOps[] = {
		VK_COMPARE_OP_NEVER,
		VK_COMPARE_OP_LESS,
		VK_COMPARE_OP_GREATER,
	};
	CT_ASSERT(ARRAY_COUNT(vkCompareOps) == CompareOpCount);
	ASSERT(compareOp < CompareOpCount);
	const VkCompareOp vkCompareOp = vkCompareOps[compareOp];
	return vkCompareOp;
}

static bool IsDepthFormat(VkFormat format)
{
	const bool isDepthFormat =
		format == VK_FORMAT_D24_UNORM_S8_UINT ||
		format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		format == VK_FORMAT_D32_SFLOAT ||
		format == VK_FORMAT_D16_UNORM_S8_UINT ||
		format == VK_FORMAT_D16_UNORM;
	return isDepthFormat;
}

static bool HasStencilComponent(VkFormat format)
{
	const bool hasStencil =
		format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		format == VK_FORMAT_D24_UNORM_S8_UINT ||
		format == VK_FORMAT_D16_UNORM_S8_UINT;
	return hasStencil;
}

static VkAttachmentLoadOp LoadOpToVulkan( LoadOp loadOp )
{
	ASSERT(loadOp <= LoadOpDontCare);
	static VkAttachmentLoadOp vkLoadOps[] = {
		VK_ATTACHMENT_LOAD_OP_LOAD,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	};
	const VkAttachmentLoadOp vkLoadOp = vkLoadOps[loadOp];
	return vkLoadOp;
}

static VkAttachmentStoreOp StoreOpToVulkan( StoreOp storeOp )
{
	ASSERT(storeOp <= StoreOpDontCare);
	static VkAttachmentStoreOp vkStoreOps[] = {
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
	};
	const VkAttachmentStoreOp vkStoreOp = vkStoreOps[storeOp];
	return vkStoreOp;
}



////////////////////////////////////////////////////////////////////////
// Internal functions
////////////////////////////////////////////////////////////////////////

static const char *VkResultToString(VkResult result)
{
	switch (result)
	{
		case VK_SUCCESS: return "VK_SUCCESS";
		case VK_NOT_READY: return "VK_NOT_READY";
		case VK_TIMEOUT: return "VK_TIMEOUT";
		case VK_EVENT_SET: return "VK_EVENT_SET";
		case VK_EVENT_RESET: return "VK_EVENT_RESET";
		case VK_INCOMPLETE: return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
		default: break;
	}
	return "VK_ERROR_UNHANDLED";
}

static void CheckVulkanResult(VkResult result, const char *callString)
{
	if (result == VK_SUCCESS)
		return;
	LOG(Error, "[vulkan] VkResult error:\n");
	LOG(Error, "[vulkan] - errorCode: %d\n", result);
	LOG(Error, "[vulkan] - errorString: %s\n", VkResultToString(result));
	LOG(Error, "[vulkan] - callString: %s\n", callString);
	if (result < VK_SUCCESS)
		QUIT_ABNORMALLY();
}

#if USE_IMGUI
static void CheckVulkanResultImGui(VkResult result)
{
	CheckVulkanResult(result, "ImGui");
}
#endif

static Heap CreateHeap(const GraphicsDevice &device, HeapType heapType, u32 size, bool mapMemory)
{
	u32 memoryTypeIndex = -1;

	VkMemoryPropertyFlags requiredFlags = HeapTypeToVkMemoryPropertyFlags( heapType );

	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &memoryProperties);

	for ( u32 i = 0; i < memoryProperties.memoryTypeCount; ++i )
	{
		const VkMemoryType &memoryType = memoryProperties.memoryTypes[i];

		if ( ( memoryType.propertyFlags & requiredFlags ) == requiredFlags )
		{
			memoryTypeIndex = i;
			break;
		}
	}

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory memory;
	VK_CALL( vkAllocateMemory(device.handle, &memoryAllocateInfo, VULKAN_ALLOCATORS, &memory) );

	u8 *data = 0;
	if ( mapMemory )
	{
		VK_CALL( vkMapMemory(device.handle, memory, 0, size, 0, (void**)&data) );
	}

	Heap heap = {};
	heap.type = heapType;
	heap.size = size;
	heap.memoryTypeIndex = memoryTypeIndex;
	heap.memory = memory;
	heap.data = data;
	return heap;
}

static ShaderModule CreateShaderModule(const GraphicsDevice &device, const ShaderSource &source)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = source.dataSize;
	createInfo.pCode = reinterpret_cast<const u32*>(source.data);

	VkShaderModule shaderModuleHandle;
	if ( vkCreateShaderModule(device.handle, &createInfo, VULKAN_ALLOCATORS, &shaderModuleHandle) != VK_SUCCESS )
	{
		LOG(Error, "Error in CreateShaderModule.\n");
		shaderModuleHandle = VK_NULL_HANDLE;
	}

	ShaderModule shaderModule = {};
	shaderModule.handle = shaderModuleHandle;
	return shaderModule;
}

static void DestroyShaderModule(const GraphicsDevice &device, const ShaderModule &shaderModule)
{
	vkDestroyShaderModule(device.handle, shaderModule.handle, VULKAN_ALLOCATORS);
}

static void UpdateDescriptorSets(const GraphicsDevice &device, VkWriteDescriptorSet *descriptorWrites, u32 descriptorWriteCount)
{
	if ( descriptorWriteCount > 0 )
	{
		vkUpdateDescriptorSets(device.handle, descriptorWriteCount, descriptorWrites, 0, NULL);
	}
}

static const ShaderBinding *GetBindGroupBindingPointer(const ShaderBindings &shaderBindings, u8 bindGroupIndex)
{
	for (u32 i = 0; i < shaderBindings.bindingCount; ++i) {
		if ( shaderBindings.bindings[i].set == bindGroupIndex ) {
			return &shaderBindings.bindings[i];
		}
	}
	return NULL;
}

union VkDescriptorGenericInfo
{
	VkDescriptorImageInfo imageInfo;
	VkDescriptorBufferInfo bufferInfo;
	VkBufferView bufferView;
};

static bool AddDescriptorWrite(const ResourceBinding *bindingTable, const ShaderBinding &binding, VkDescriptorSet descriptorSet, VkDescriptorGenericInfo *descriptorInfos, VkWriteDescriptorSet *descriptorWrites, u32 &descriptorWriteCount)
{
	const ResourceBinding &resourceBinding = bindingTable[binding.binding];

	VkDescriptorImageInfo *imageInfo = 0;
	VkDescriptorBufferInfo *bufferInfo = 0;
	VkBufferView *bufferView = 0;

	if ( binding.type == SpvTypeSampler )
	{
		imageInfo = &descriptorInfos[descriptorWriteCount].imageInfo;
		imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo->imageView = VK_NULL_HANDLE;
		imageInfo->sampler = resourceBinding.sampler.handle;
	}
	else if ( binding.type == SpvTypeImage )
	{
		imageInfo = &descriptorInfos[descriptorWriteCount].imageInfo;
		imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo->imageView = resourceBinding.texture.handle;
		imageInfo->sampler = VK_NULL_HANDLE;
	}
	else if ( binding.type == SpvTypeUniformBuffer || binding.type == SpvTypeStorageBuffer )
	{
		bufferInfo = &descriptorInfos[descriptorWriteCount].bufferInfo;
		bufferInfo->buffer = resourceBinding.buffer.handle;
		bufferInfo->offset = resourceBinding.buffer.offset;
		bufferInfo->range = resourceBinding.buffer.range;
	}
	else if ( binding.type == SpvTypeStorageTexelBuffer )
	{
		bufferView = &descriptorInfos[descriptorWriteCount].bufferView;
		*bufferView = resourceBinding.bufferView.handle;
	}
	else
	{
		LOG(Warning, "Unhandled descriptor type (%u) for binding %s.\n", binding.type, binding.name);
		return false;
	}

	VkWriteDescriptorSet *descriptorWrite = &descriptorWrites[descriptorWriteCount++];
	descriptorWrite->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite->dstSet = descriptorSet;
	descriptorWrite->dstBinding = binding.binding;
	descriptorWrite->dstArrayElement = 0;
	descriptorWrite->descriptorType = SpvDescriptorTypeToVulkan( binding.type );
	descriptorWrite->descriptorCount = 1;
	descriptorWrite->pBufferInfo = bufferInfo;
	descriptorWrite->pImageInfo = imageInfo;
	descriptorWrite->pTexelBufferView = bufferView;
	return true;
}

static ShaderSource GetShaderSource(Arena &arena, const char *filename)
{
	FilePath shaderPath = MakePath(filename);
	DataChunk *chunk = PushFile( arena, shaderPath.str );
	if ( !chunk ) {
		LOG( Error, "Could not open shader file %s.\n", shaderPath.str );
		QUIT_ABNORMALLY();
	}
	ShaderSource shaderSource = { chunk->bytes, chunk->size };
	return shaderSource;
}

static VkFormat FindSupportedFormat(const GraphicsDevice &device, const VkFormat candidates[], u32 candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (u32 i = 0; i < candidateCount; ++i)
	{
		VkFormat format = candidates[i];
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(device.physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (features & properties.linearTilingFeatures) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (features & properties.optimalTilingFeatures) == features)
		{
			return format;
		}
	}

	INVALID_CODE_PATH();
	return VK_FORMAT_MAX_ENUM;
}

static VkFormat FindDepthFormat(const GraphicsDevice &device)
{
	const VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
	return FindSupportedFormat(device, candidates, ARRAY_COUNT(candidates),
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

static ShaderBindings ReflectShaderBindings( Arena scratch, byte* microcodeData[], const u64 microcodeSize[], u32 microcodeCount)
{
	void *tempMem = scratch.base + scratch.used;
	const u32 tempMemSize = scratch.size - scratch.used;

	SpvDescriptorSetList spvDescriptorList = {};
	for (u32 i = 0; i < microcodeCount; ++i)
	{
		SpvParser parser = SpvParserInit( microcodeData[i], microcodeSize[i] );
		SpvParseDescriptors( &parser, &spvDescriptorList, tempMem, tempMemSize );
	}

	ShaderBindings shaderBindings = {};

	for (u32 setIndex = 0; setIndex < SPV_MAX_DESCRIPTOR_SETS; ++setIndex)
	{
		for (u32 bindingIndex = 0; bindingIndex < SPV_MAX_DESCRIPTORS_PER_SET; ++bindingIndex)
		{
			SpvDescriptor &descriptor = spvDescriptorList.sets[setIndex].bindings[bindingIndex];

			if ( descriptor.type != SpvTypeNone )
			{
				ASSERT( shaderBindings.bindingCount < ARRAY_COUNT(shaderBindings.bindings) );
				ShaderBinding &shaderBinding = shaderBindings.bindings[shaderBindings.bindingCount++];
				shaderBinding.binding = descriptor.binding;
				shaderBinding.set = setIndex;
				shaderBinding.type = (SpvType)descriptor.type;
				shaderBinding.stageFlags = descriptor.stageFlags;
				shaderBinding.name = InternString( descriptor.name );
				//LOG(Info, "Descriptor name: %s\n", descriptor.name);
			}
		}
	}

	return shaderBindings;
}

static ShaderBindings ReflectShaderBindings( Arena scratch, const ShaderSource &source )
{
	byte* microcodeData[] = { source.data };
	const u64 microcodeSize[] = { source.dataSize };
	const ShaderBindings shaderBindings = ReflectShaderBindings(scratch, microcodeData, microcodeSize, ARRAY_COUNT(microcodeData));
	return shaderBindings;
}

static ShaderBindings ReflectShaderBindings( Arena scratch, const ShaderSource &vertexSource, const ShaderSource &fragmentSource )
{
	byte* microcodeData[] = { vertexSource.data, fragmentSource.data };
	const u64 microcodeSize[] = { vertexSource.dataSize, fragmentSource.dataSize };
	ShaderBindings shaderBindings = ReflectShaderBindings(scratch, microcodeData, microcodeSize, ARRAY_COUNT(microcodeData));
	return shaderBindings;
}

static void BindDescriptorSets(CommandList &commandList)
{
	if ( commandList.descriptorSetDirtyMask )
	{
		const u32 descriptorSetFirst = CTZ(commandList.descriptorSetDirtyMask);

		u32 descriptorSetCount = 0;
		VkDescriptorSet descriptorSets[4] = {};
		for (u32 i = descriptorSetFirst; i < 4; ++i )
		{
			if ( commandList.descriptorSetDirtyMask & (1 << i) )
			{
				descriptorSets[descriptorSetCount++] = commandList.descriptorSetHandles[i];
			}
			else
			{
				break;
			}
		}

		VkPipelineBindPoint bindPoint = commandList.pipeline->bindPoint;
		VkPipelineLayout pipelineLayout = commandList.pipeline->layout.handle;
		vkCmdBindDescriptorSets(commandList.handle, bindPoint, pipelineLayout, descriptorSetFirst, descriptorSetCount, descriptorSets, 0, NULL);
		commandList.descriptorSetDirtyMask = 0;
	}
}







////////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////////

//////////////////////////////
// BindGroupAllocator
//////////////////////////////

BindGroupAllocator CreateBindGroupAllocator(const GraphicsDevice &device, const BindGroupAllocatorCounts &counts)
{
	u32 poolSizeCount = 0;
	VkDescriptorPoolSize poolSizes[8] = {};

	if (counts.uniformBufferCount > 0) {
		poolSizes[poolSizeCount++] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, counts.uniformBufferCount };
	}
	if (counts.storageBufferCount > 0) {
		poolSizes[poolSizeCount++] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, counts.storageBufferCount };
	}
	if (counts.storageTexelBufferCount > 0) {
		poolSizes[poolSizeCount++] = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, counts.storageTexelBufferCount };
	}
	if (counts.textureCount > 0) {
		poolSizes[poolSizeCount++] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, counts.textureCount };
	}
	if (counts.samplerCount > 0) {
		poolSizes[poolSizeCount++] = { VK_DESCRIPTOR_TYPE_SAMPLER, counts.samplerCount };
	}
	if (counts.combinedImageSamplerCount > 0) {
		poolSizes[poolSizeCount++] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, counts.combinedImageSamplerCount };
	}

	ASSERT(poolSizeCount <= ARRAY_COUNT(poolSizes));

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.flags = counts.allowIndividualFrees ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
	createInfo.poolSizeCount = poolSizeCount;
	createInfo.pPoolSizes = poolSizes;
	createInfo.maxSets = counts.groupCount;

	VkDescriptorPool descriptorPool;
	VK_CALL( vkCreateDescriptorPool( device.handle, &createInfo, VULKAN_ALLOCATORS, &descriptorPool) );

	BindGroupAllocator allocator = {
		.maxCounts = counts,
		.handle = descriptorPool,
	};
	return allocator;
}

void DestroyBindGroupAllocator( const GraphicsDevice &device, const BindGroupAllocator &bindGroupAllocator )
{
	vkDestroyDescriptorPool( device.handle, bindGroupAllocator.handle, VULKAN_ALLOCATORS );
}

void ResetBindGroupAllocator(const GraphicsDevice &device, BindGroupAllocator &bindGroupAllocator)
{
	const VkDescriptorPool descriptorPool = bindGroupAllocator.handle;
	VK_CALL( vkResetDescriptorPool(device.handle, descriptorPool, 0) );

	bindGroupAllocator.usedCounts = {};
}


//////////////////////////////
// BindGroup
//////////////////////////////

BindGroupLayout CreateBindGroupLayout(const GraphicsDevice &device, const ShaderBindings &shaderBindings, u32 bindGroupIndex)
{
	BindGroupLayout layout = {};

	VkDescriptorSetLayoutBinding vkBindings[SPV_MAX_DESCRIPTORS_PER_SET] = {};
	u32 bindingCount = 0;

	for (u32 bindingIndex = 0; bindingIndex < shaderBindings.bindingCount; ++bindingIndex)
	{
		const ShaderBinding &shaderBinding = shaderBindings.bindings[bindingIndex];

		if ( shaderBinding.set == bindGroupIndex )
		{
			VkDescriptorSetLayoutBinding &binding = vkBindings[bindingCount++];
			binding.binding = shaderBinding.binding;
			binding.descriptorType = SpvDescriptorTypeToVulkan((SpvType)shaderBinding.type);
			binding.descriptorCount = 1;
			binding.stageFlags = SpvStageFlagsToVulkan(shaderBinding.stageFlags);
			binding.pImmutableSamplers = NULL;
		}
	}

	if (bindingCount > 0)
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
		descriptorSetLayoutCreateInfo.pBindings = vkBindings;

		VkDescriptorSetLayout layoutHandle;
		VK_CALL( vkCreateDescriptorSetLayout(device.handle, &descriptorSetLayoutCreateInfo, VULKAN_ALLOCATORS, &layoutHandle) );

		layout.handle = layoutHandle;
		layout.bindingCount = bindingCount;
		layout.bindings = GetBindGroupBindingPointer(shaderBindings, bindGroupIndex);
	}

	return layout;
}

BindGroup CreateBindGroup(const GraphicsDevice &device, const BindGroupLayout &layout, BindGroupAllocator &allocator)
{
	BindGroup bindGroup = {};

	if (layout.handle)
	{
		VkDescriptorSetAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = allocator.handle;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &layout.handle;
		VK_CALL( vkAllocateDescriptorSets(device.handle, &allocateInfo, &bindGroup.handle) );

		// Update the used descriptor counters in the allocator
		for (u32 i = 0; i < layout.bindingCount; ++i)
		{
			const ShaderBinding &binding = layout.bindings[i];
			switch (binding.type) {
				case SpvTypeImage: allocator.usedCounts.textureCount++; break;
				case SpvTypeSampler: allocator.usedCounts.samplerCount++; break;
				case SpvTypeSampledImage: allocator.usedCounts.combinedImageSamplerCount++; break;
				case SpvTypeUniformBuffer: allocator.usedCounts.uniformBufferCount++; break;
				case SpvTypeStorageBuffer: allocator.usedCounts.storageBufferCount++; break;
				case SpvTypeStorageTexelBuffer: allocator.usedCounts.storageTexelBufferCount++; break;
				default: INVALID_CODE_PATH();
			}
		}
	}

	return bindGroup;
}

BindGroup CreateBindGroup(const GraphicsDevice &device, const BindGroupDesc &desc, BindGroupAllocator &allocator)
{
	BindGroup bindGroup = CreateBindGroup(device, desc.layout, allocator);

	const BindGroupLayout &layout = desc.layout;

	VkDescriptorGenericInfo descriptorInfos[MAX_SHADER_BINDINGS] = {};
	VkWriteDescriptorSet descriptorWrites[MAX_SHADER_BINDINGS] = {};
	u32 descriptorWriteCount = 0;

	for (u32 i = 0; i < layout.bindingCount; ++i)
	{
		const ShaderBinding &binding = layout.bindings[i];
		if ( !AddDescriptorWrite(desc.bindings, binding, bindGroup.handle, descriptorInfos, descriptorWrites, descriptorWriteCount) )
		{
			LOG(Warning, "Could not add descriptor write for binding %s of pipeline %s.\n", binding.name, "<pipeline>");
		}
	}

	UpdateDescriptorSets(device, descriptorWrites, descriptorWriteCount);

	return bindGroup;
}


//////////////////////////////
// Pipelines
//////////////////////////////

// TODO: Avoid having to pass renderPass as a parameter instead of within the PipelineDesc
Pipeline CreateGraphicsPipeline(const GraphicsDevice &device, Arena &arena, const PipelineDesc &desc, const RenderPass &renderPass)
{
	Arena scratch = MakeSubArena(arena);

	// Create pipeline
	const ShaderSource vertexShaderSource = GetShaderSource(scratch, desc.vsFilename);
	const ShaderSource fragmentShaderSource = GetShaderSource(scratch, desc.fsFilename);
	const ShaderModule vertexShaderModule = CreateShaderModule(device, vertexShaderSource);
	const ShaderModule fragmentShaderModule = CreateShaderModule(device, fragmentShaderSource);
	const ShaderBindings shaderBindings = ReflectShaderBindings(scratch, vertexShaderSource, fragmentShaderSource);

	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageCreateInfo.module = vertexShaderModule.handle;
	vertexShaderStageCreateInfo.pName = desc.vsFunction ? desc.vsFunction : "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageCreateInfo.module = fragmentShaderModule.handle;
	fragmentShaderStageCreateInfo.pName = desc.fsFunction ? desc.fsFunction : "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

	VkVertexInputBindingDescription bindingDescriptions[1] = {};
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attributeDescriptions[4] = {};
	for (u32 i = 0; i < desc.vertexAttributeCount; ++i)
	{
		attributeDescriptions[i].binding = desc.vertexAttributes[i].bufferIndex;
		attributeDescriptions[i].location = desc.vertexAttributes[i].location;
		attributeDescriptions[i].format = FormatToVulkan(desc.vertexAttributes[i].format);
		attributeDescriptions[i].offset = desc.vertexAttributes[i].offset;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions; // Optional
	vertexInputCreateInfo.vertexAttributeDescriptionCount = desc.vertexAttributeCount;
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = ARRAY_COUNT(dynamicStates);
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.scissorCount = 1;
	// NOTE: We don't set values for the viewport and scissor
	// rect because they will be set dynamically using commands

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.lineWidth = 1.0f;
	//rasterizerCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
	rasterizerCreateInfo.depthBiasClamp = 0.0f; // Optional
	rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingCreateInfo.minSampleShading = 1.0f; // Optional
	multisamplingCreateInfo.pSampleMask = nullptr; // Optional
	multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	multisamplingCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = USE_REVERSE_Z ? VK_COMPARE_OP_GREATER : VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilCreateInfo.front = {}; // Optional
	depthStencilCreateInfo.back = {}; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	// Color replace
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	// Alpha blending
	//colorBlendAttachmentState.blendEnable = VK_TRUE;
	//colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
	colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlendingCreateInfo.attachmentCount = 1;
	colorBlendingCreateInfo.pAttachments = &colorBlendAttachmentState;
	colorBlendingCreateInfo.blendConstants[0] = 0.0f; // Optional
	colorBlendingCreateInfo.blendConstants[1] = 0.0f; // Optional
	colorBlendingCreateInfo.blendConstants[2] = 0.0f; // Optional
	colorBlendingCreateInfo.blendConstants[3] = 0.0f; // Optional

	// DescriptorSet/BindGroup layouts
	VkDescriptorSetLayout descriptorSetLayouts[SPV_MAX_DESCRIPTOR_SETS] = {};
	u32 descriptorSetLayoutCount = 0;

	BindGroupLayout bindGroupLayouts[MAX_DESCRIPTOR_SETS] = {};
	for (u32 bindGroup = 0; bindGroup < ARRAY_COUNT(bindGroupLayouts); ++bindGroup)
	{
		bindGroupLayouts[bindGroup] = CreateBindGroupLayout(device, shaderBindings, bindGroup);

		if (bindGroupLayouts[bindGroup].handle) {
			descriptorSetLayouts[descriptorSetLayoutCount++] = bindGroupLayouts[bindGroup].handle;
		}
	}

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutCount;
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;

	VkPipelineLayout pipelineLayout;
	VK_CALL( vkCreatePipelineLayout(device.handle, &pipelineLayoutCreateInfo, VULKAN_ALLOCATORS, &pipelineLayout) );

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = ARRAY_COUNT(shaderStages);
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass.handle;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

	VkPipeline vkPipelineHandle;
	VK_CALL( vkCreateGraphicsPipelines( device.handle, device.pipelineCache, 1, &graphicsPipelineCreateInfo, VULKAN_ALLOCATORS, &vkPipelineHandle ) );

	DestroyShaderModule(device, vertexShaderModule);
	DestroyShaderModule(device, fragmentShaderModule);

	Pipeline pipeline = {
		.name = desc.name,
		.handle = vkPipelineHandle,
		.layout = {
			.handle = pipelineLayout,
			.shaderBindings = shaderBindings,
		},
		.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
	};

	for (u32 bindGroup = 0; bindGroup < MAX_DESCRIPTOR_SETS; ++bindGroup) {
		pipeline.layout.bindGroupLayouts[bindGroup] = bindGroupLayouts[bindGroup];
		pipeline.layout.bindGroupLayouts[bindGroup].bindings = GetBindGroupBindingPointer(pipeline.layout.shaderBindings, bindGroup);
	}

	return pipeline;
}

Pipeline CreateComputePipeline(const GraphicsDevice &device, Arena &arena, const ComputeDesc &desc)
{
	Arena scratch = arena;

	const ShaderSource shaderSource = GetShaderSource(scratch, desc.filename);
	const ShaderModule shaderModule = CreateShaderModule(device, shaderSource);
	const ShaderBindings shaderBindings = ReflectShaderBindings(scratch, shaderSource);

	VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = shaderModule.handle;
	computeShaderStageInfo.pName = desc.function ? desc.function : "main";

	// DescriptorSet/BindGroup layouts
	VkDescriptorSetLayout descriptorSetLayouts[SPV_MAX_DESCRIPTOR_SETS] = {};
	u32 descriptorSetLayoutCount = 0;

	BindGroupLayout bindGroupLayouts[MAX_DESCRIPTOR_SETS] = {};
	for (u32 bindGroup = 0; bindGroup < ARRAY_COUNT(bindGroupLayouts); ++bindGroup)
	{
		bindGroupLayouts[bindGroup] = CreateBindGroupLayout(device, shaderBindings, bindGroup);

		if (bindGroupLayouts[bindGroup].handle) {
			descriptorSetLayouts[descriptorSetLayoutCount++] = bindGroupLayouts[bindGroup].handle;
		}
	}

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutCount;
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;

	VkPipelineLayout pipelineLayout;
	VK_CALL( vkCreatePipelineLayout(device.handle, &pipelineLayoutCreateInfo, VULKAN_ALLOCATORS, &pipelineLayout) );

	VkComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.stage = computeShaderStageInfo;

	VkPipeline computePipeline;
	VK_CALL( vkCreateComputePipelines(device.handle, device.pipelineCache, 1, &pipelineInfo, VULKAN_ALLOCATORS, &computePipeline) );

	DestroyShaderModule(device, shaderModule);

	Pipeline pipeline = {
		.name = desc.name,
		.handle = computePipeline,
		.layout = {
			.handle = pipelineLayout,
			.shaderBindings = shaderBindings,
		},
		.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE,
	};

	for (u32 bindGroup = 0; bindGroup < MAX_DESCRIPTOR_SETS; ++bindGroup) {
		pipeline.layout.bindGroupLayouts[bindGroup] = bindGroupLayouts[bindGroup];
		pipeline.layout.bindGroupLayouts[bindGroup].bindings = GetBindGroupBindingPointer(pipeline.layout.shaderBindings, bindGroup);
	}

	return pipeline;
}

void DestroyPipeline(const GraphicsDevice &device, const Pipeline &pipeline)
{
	vkDestroyPipeline( device.handle, pipeline.handle, VULKAN_ALLOCATORS );
	vkDestroyPipelineLayout( device.handle, pipeline.layout.handle, VULKAN_ALLOCATORS );
	for (u32 j = 0; j < ARRAY_COUNT(pipeline.layout.bindGroupLayouts); ++j) {
		vkDestroyDescriptorSetLayout( device.handle, pipeline.layout.bindGroupLayouts[j].handle, VULKAN_ALLOCATORS );
	}
}


//////////////////////////////
// CommandList
//////////////////////////////

CommandList BeginCommandList(const GraphicsDevice &device)
{
	VkCommandBuffer commandBuffer = device.commandBuffers[device.currentFrame];

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0; // Optional
	commandBufferBeginInfo.pInheritanceInfo = NULL; // Optional
	VK_CALL( vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) );

	CommandList commandList = {
		.handle = commandBuffer,
	};
	return commandList;
}

void EndCommandList(const CommandList &commandList)
{
	VK_CALL( vkEndCommandBuffer( commandList.handle ) );
}


//////////////////////////////
// Commands
//////////////////////////////

void BeginRenderPass(const CommandList &commandList, const Framebuffer &framebuffer)
{
	VkClearValue clearValues[2] = {};
	u32 clearValueCount = 0;
	const float depthClearValue = USE_REVERSE_Z ? 0.0f : 1.0f;
	if (framebuffer.isDisplay)
	{
		clearValueCount = 2;
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clearValues[1].depthStencil = {depthClearValue, 0};
	}
	if (framebuffer.isShadowmap)
	{
		clearValueCount = 1;
		clearValues[0].depthStencil = {depthClearValue, 0};
	}

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = framebuffer.renderPassHandle;
	renderPassBeginInfo.framebuffer = framebuffer.handle;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = framebuffer.extent;
	renderPassBeginInfo.clearValueCount = clearValueCount;
	renderPassBeginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass( commandList.handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void SetViewportAndScissor(const CommandList &commandList, uint2 size)
{
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = static_cast<float>(size.y);
	viewport.width = static_cast<float>(size.x);
	viewport.height = -static_cast<float>(size.y);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandList.handle, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = {size.x, size.y};
	vkCmdSetScissor(commandList.handle, 0, 1, &scissor);
}

void SetPipeline(CommandList &commandList, const Pipeline &pipeline)
{
	if ( commandList.pipeline != &pipeline )
	{
		commandList.pipeline = &pipeline;
		vkCmdBindPipeline( commandList.handle, pipeline.bindPoint, pipeline.handle );
	}
}

void SetBindGroup(CommandList &commandList, u32 bindGroupIndex, const BindGroup &bindGroup)
{
	if ( commandList.descriptorSetHandles[bindGroupIndex] != bindGroup.handle )
	{
		commandList.descriptorSetDirtyMask |= 1 << bindGroupIndex;
		commandList.descriptorSetHandles[bindGroupIndex] = bindGroup.handle;
	}
}

void SetVertexBuffer(CommandList &commandList, const Buffer &buffer)
{
	if ( buffer.handle && buffer.handle != commandList.vertexBufferHandle )
	{
		commandList.vertexBufferHandle = buffer.handle;
		VkBuffer vertexBuffers[] = { buffer.handle };
		VkDeviceSize vertexBufferOffsets[] = { 0 };
		vkCmdBindVertexBuffers(commandList.handle, 0, ARRAY_COUNT(vertexBuffers), vertexBuffers, vertexBufferOffsets);
	}
}

void SetIndexBuffer(CommandList &commandList, const Buffer &buffer)
{
	if ( buffer.handle && buffer.handle != commandList.indexBufferHandle )
	{
		commandList.indexBufferHandle = buffer.handle;
		VkDeviceSize indexBufferOffset = 0;
		vkCmdBindIndexBuffer(commandList.handle, buffer.handle, indexBufferOffset, VK_INDEX_TYPE_UINT16);
	}
}

void DrawIndexed(CommandList &commandList, u32 indexCount, u32 firstIndex, u32 firstVertex, u32 instanceIndex)
{
	BindDescriptorSets(commandList);

	vkCmdDrawIndexed(commandList.handle, indexCount, 1, firstIndex, firstVertex, instanceIndex);
}

void Dispatch(CommandList &commandList, u32 x, u32 y, u32 z)
{
	BindDescriptorSets(commandList);

	vkCmdDispatch(commandList.handle, x, y, z);
}

void EndRenderPass(const CommandList &commandList)
{
	vkCmdEndRenderPass(commandList.handle);
}


//////////////////////////////
// Submission and Presentation
//////////////////////////////

SubmitResult Submit(const GraphicsDevice &device, const CommandList &commandList)
{
	const u32 frameIndex = device.currentFrame;

	VkSemaphore waitSemaphores[] = { device.imageAvailableSemaphores[frameIndex] };
	VkSemaphore signalSemaphores[] = { device.renderFinishedSemaphores[frameIndex] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = ARRAY_COUNT(waitSemaphores);
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = ARRAY_COUNT(signalSemaphores);
	submitInfo.pSignalSemaphores = signalSemaphores;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandList.handle;

	VK_CALL( vkQueueSubmit( device.graphicsQueue, 1, &submitInfo, device.inFlightFences[frameIndex] ) );

	SubmitResult res = {
		.signalSemaphore = signalSemaphores[0],
	};
	return res;
}

bool Present(GraphicsDevice &device, SubmitResult submitResult)
{
	const VkSemaphore signalSemaphores[] = { submitResult.signalSemaphore };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = ARRAY_COUNT(signalSemaphores);
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &device.swapchain.handle;
	presentInfo.pImageIndices = &device.swapchain.currentImageIndex;
	presentInfo.pResults = NULL; // Optional

	VkResult presentResult = vkQueuePresentKHR( device.presentQueue, &presentInfo );

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
	{
		LOG(Warning, "vkQueuePresentKHR - result: VK_ERROR_OUT_OF_DATE_KHR || VK_SUBOPTIMAL_KHR\n");
		device.swapchain.outdated = true;
	}
	else if (presentResult != VK_SUCCESS)
	{
		LOG(Error, "vkQueuePresentKHR failed.\n");
		return false;
	}

	return true;
}



//////////////////////////////
// Frame
//////////////////////////////

bool BeginFrame(GraphicsDevice &device)
{
	const u32 frameIndex = device.currentFrame;

	// Swapchain sync
	VK_CALL( vkWaitForFences( device.handle, 1, &device.inFlightFences[frameIndex], VK_TRUE, UINT64_MAX ) );

	u32 imageIndex;
	VkResult acquireResult = vkAcquireNextImageKHR( device.handle, device.swapchain.handle, UINT64_MAX, device.imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex );

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		LOG(Warning, "vkAcquireNextImageKHR - result: VK_ERROR_OUT_OF_DATE_KHR\n");
		device.swapchain.outdated = true;
		return false;
	}
	else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
	{
		LOG(Error, "vkAcquireNextImageKHR failed.\n");
		return false;
	}

	VK_CALL( vkResetFences( device.handle, 1, &device.inFlightFences[frameIndex] ) );
	device.swapchain.currentImageIndex = imageIndex;

	// Reset commands for this frame
	VkCommandPool commandPool = device.commandPools[frameIndex];
	VK_CALL( vkResetCommandPool(device.handle, commandPool, 0) );

	return true;
}

void EndFrame(GraphicsDevice &device)
{
	device.currentFrame = ( device.currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;
}



#endif // #ifndef TOOLS_GFX_H

