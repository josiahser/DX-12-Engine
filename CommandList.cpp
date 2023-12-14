#include "framework.h"

#include "CommandList.h"

#include "Application.h"
#include "ByteAddressBuffer.h"
#include "ConstantBuffer.h"
#include "CommandQueue.h"
#include "DynamicDescriptorHeap.h"
#include "GenerateMipsPSO.h"
#include "IndexBuffer.h"
#include "PanoToCubemapPSO.h"
#include "RenderTarget.h"
#include "Resource.h"
#include "ResourceStateTracker.h"
#include "RootSignature.h"
#include "StructuredBuffer.h"
#include "Texture.h"
#include "UploadBuffer.h"
#include "VertexBuffer.h"

//Static Global member variables
std::map< std::wstring, ID3D12Resource* > CommandList::ms_TextureCache;
std::mutex CommandList::ms_TextureCacheMutex;

CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type)
	:m_d3d12CommandListType(type)
{
	auto device = Application::Get().GetDevice();

	ThrowIfFailed(device->CreateCommandAllocator(m_d3d12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator)));

	ThrowIfFailed(device->CreateCommandList(0, m_d3d12CommandListType, m_d3d12CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_d3d12CommandList)));

	m_UploadBuffer = std::make_unique<UploadBuffer>();
	m_ResourceStateTracker = std::make_unique<ResourceStateTracker>();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_DynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
		m_DescriptorHeaps[i] = nullptr;
	}
}

CommandList::~CommandList()
{}

void CommandList::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
{
	auto d3d12Resource = resource.GetD3D12Resource();
	if (d3d12Resource)
	{
		//The before state isn't important, itll be solved by the resource state tracker
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource);
		m_ResourceStateTracker->ResourceBarrier(barrier);
	}
	if (flushBarriers)
	{
		FlushResourceBarriers();
	}
}

void CommandList::UAVBarrier(const Resource& resource, bool flushBarriers)
{
	auto d3d12Resource = resource.GetD3D12Resource();
	auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(d3d12Resource.Get());

	m_ResourceStateTracker->ResourceBarrier(barrier);

	if (flushBarriers)
	{
		FlushResourceBarriers();
	}
}

void CommandList::AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers)
{
	auto d3d12BeforeResource = beforeResource.GetD3D12Resource();
	auto d3d12AfterResource = afterResource.GetD3D12Resource();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(d3d12BeforeResource.Get(), d3d12AfterResource.Get());

	m_ResourceStateTracker->ResourceBarrier(barrier);

	if (flushBarriers)
	{
		FlushResourceBarriers();
	}
}

void CommandList::FlushResourceBarriers()
{
	m_ResourceStateTracker->FlushResourceBarriers(*this);
}

void CommandList::CopyResource(Resource& dstRes, const Resource& srcRes)
{
	TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

	FlushResourceBarriers();

	m_d3d12CommandList->CopyResource(dstRes.GetD3D12Resource().Get(), srcRes.GetD3D12Resource().Get());

	TrackResource(dstRes);
	TrackResource(srcRes);
}

void CommandList::ResolveSubResource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubresource, uint32_t srcSubresource)
{
	TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubresource);
	TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubresource);

	FlushResourceBarriers();

	m_d3d12CommandList->ResolveSubresource(dstRes.GetD3D12Resource().Get(), dstSubresource, srcRes.GetD3D12Resource().Get(), srcSubresource, dstRes.GetD3D12ResourceDesc().Format);

	TrackResource(srcRes);
	TrackResource(dstRes);
}

void CommandList::CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
	auto device = Application::Get().GetDevice();

	size_t bufferSize = numElements * elementSize;

	ComPtr<ID3D12Resource> d3d12Resource;
	if (bufferSize == 0)
	{
		//This will result in a NULL resource (which may be desired to define a default null resource)
	}
	else
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&d3d12Resource)));

		//Add resource to the global resource state tracker
		ResourceStateTracker::AddGlobalResourceState(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

		if (bufferData != nullptr)
		{
			//Create an upload resource to use as an intermediate buffer to copy the buffer resource
			ComPtr<ID3D12Resource> uploadResource;
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&uploadResource)));

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = bufferData;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			m_ResourceStateTracker->TransitionResource(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
			FlushResourceBarriers();

			UpdateSubresources(m_d3d12CommandList.Get(), d3d12Resource.Get(), uploadResource.Get(), 0, 0, 1, &subresourceData);

			//Add references to resource so they stay in scope until the command list is reset
			TrackObject(uploadResource);
		}
		TrackObject(d3d12Resource);
	}
	buffer.SetD3D12Resource(d3d12Resource);
	buffer.CreateViews(numElements, elementSize);
}

void CommandList::CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData)
{
	CopyBuffer(vertexBuffer, numVertices, vertexStride, vertexBufferData);
}

void CommandList::CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData)
{
	size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
	CopyBuffer(indexBuffer, numIndices, indexSizeInBytes, indexBufferData);
}

void CommandList::CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* bufferData)
{
	CopyBuffer(byteAddressBuffer, 1, bufferSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void CommandList::CopyStructuredBuffer(StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* bufferData)
{
	CopyBuffer(structuredBuffer, numElements, elementSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	m_d3d12CommandList->IASetPrimitiveTopology(primitiveTopology);
}

void CommandList::LoadTextureFromFile(Texture& texture, const std::wstring& fileName, TextureUsage textureUsage)
{
	fs::path filePath(fileName);
	if (!fs::exists(filePath))
	{
		throw std::exception("File not found");
	}

	std::lock_guard<std::mutex> lock(ms_TextureCacheMutex);
	auto iter = ms_TextureCache.find(fileName);
	if (iter != ms_TextureCache.end())
	{
		texture.SetTextureUsage(textureUsage);
		texture.SetD3D12Resource(iter->second);
		texture.CreateViews();
		texture.SetName(fileName);
	}
	else
	{
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage scratchImage;
		
		if (filePath.extension() == ".dds")
		{
			ThrowIfFailed(DirectX::LoadFromDDSFile(fileName.c_str(), DirectX::DDS_FLAGS_FORCE_RGB, &metadata, scratchImage));
		}
		else if (filePath.extension() == ".hdr")
		{
			ThrowIfFailed(DirectX::LoadFromHDRFile(fileName.c_str(), &metadata, scratchImage));
		}
		else if (filePath.extension() == ".tga")
		{
			ThrowIfFailed(DirectX::LoadFromTGAFile(fileName.c_str(), &metadata, scratchImage));
		}
		else
		{
			ThrowIfFailed(DirectX::LoadFromWICFile(fileName.c_str(), DirectX::WIC_FLAGS_FORCE_RGB, &metadata, scratchImage));
		}

		if (textureUsage == TextureUsage::Albedo)
		{
			metadata.format = DirectX::MakeSRGB(metadata.format);
		}

		D3D12_RESOURCE_DESC textureDesc = {};
		switch (metadata.dimension)
		{
		case DirectX::TEX_DIMENSION_TEXTURE1D:
			textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT16>(metadata.arraySize));
			break;
		case DirectX::TEX_DIMENSION_TEXTURE2D:
			textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.arraySize));
			break;
		case DirectX::TEX_DIMENSION_TEXTURE3D:
			textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.depth));
			break;
		default:
			throw std::exception("Invalid texture dimension");
			break;
		}

		auto device = Application::Get().GetDevice();
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&textureResource)));

		texture.SetTextureUsage(textureUsage);
		texture.SetD3D12Resource(textureResource);
		texture.CreateViews();
		texture.SetName(fileName);

		//Update the global state tracker
		ResourceStateTracker::AddGlobalResourceState(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);

		std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
		const DirectX::Image* pImages = scratchImage.GetImages();
		for (int i = 0; i < scratchImage.GetImageCount(); ++i)
		{
			auto& subresource = subresources[i];
			subresource.RowPitch = pImages[i].rowPitch;
			subresource.SlicePitch = pImages[i].slicePitch;
			subresource.pData = pImages[i].pixels;
		}

		CopyTextureSubresource(texture, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

		if (subresources.size() < textureResource->GetDesc().MipLevels)
		{
			GenerateMips(texture);
		}

		ms_TextureCache[fileName] = textureResource.Get();
	}
}

void CommandList::GenerateMips(Texture& texture)
{
	if (m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
	{
		if (!m_ComputeCommandList)
		{
			m_ComputeCommandList = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->GetCommandList();
		}
		m_ComputeCommandList->GenerateMips(texture);
		return;
	}

	auto d3d12Resource = texture.GetD3D12Resource();

	//If the texture doesn't have a valid resource, do nothing
	if (!d3d12Resource) return;
	auto d3d12ResourceDesc = d3d12Resource->GetDesc();

	//If the texture only has a single mip level (level 0), do nothing
	if (d3d12ResourceDesc.MipLevels == 1) return;
	//Currently, only 2D textures are supported
	if (d3d12ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || d3d12ResourceDesc.DepthOrArraySize != 1)
	{
		throw std::exception("Generate Mips only supports 2D textures");
	}

	if (Texture::IsUAVCompatibleFormat(d3d12ResourceDesc.Format))
	{
		GenerateMips_UAV(texture);
	}
	else if (Texture::IsBGRFormat(d3d12ResourceDesc.Format))
	{
		GenerateMips_BGR(texture);
	}
	else if (Texture::ISSRGBFormat(d3d12ResourceDesc.Format))
	{
		GenerateMips_sRGB(texture);
	}
	else
	{
		throw std::exception("Unsupported texture format for mipmap generation");
	}
}

void CommandList::GenerateMips_UAV(Texture& texture)
{
	if (!m_GenerateMipsPSO)
	{
		m_GenerateMipsPSO = std::make_unique<GenerateMipsPSO>();
	}

	auto device = Application::Get().GetDevice();
	
	auto resource = texture.GetD3D12Resource();
	auto resourceDesc = resource->GetDesc();

	auto stagingResource = resource;
	Texture stagingTexture(stagingResource);
	//If the passed in resource doesn't allow for UAV access then create a staging resource that is used to generate the mipmap chain
	if ((resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
	{
		auto stagingDesc = resourceDesc;
		stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&stagingDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&stagingResource)));

		ResourceStateTracker::AddGlobalResourceState(stagingResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

		stagingTexture.SetD3D12Resource(stagingResource);
		stagingTexture.CreateViews();
		stagingTexture.SetName(L"Generate Mips UAV Staging Texture");

		CopyResource(stagingTexture, texture);
	}

	m_d3d12CommandList->SetPipelineState(m_GenerateMipsPSO->GetPipelineState().Get());
	SetComputeRootSignature(m_GenerateMipsPSO->GetRootSignature());

	GenerateMipsCB generateMipsCB;

	for (uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1u;)
	{
		uint64_t srcWidth = resourceDesc.Width >> srcMip;
		uint32_t srcHeight = resourceDesc.Height >> srcMip;
		uint32_t dstWidth = static_cast<uint32_t>(srcWidth >> 1);
		uint32_t dstHeight = srcHeight >> 1;

		//Determine the compute shader to use based on the dimension of the source texture
		//0b00(0): Both width and height are even
		//0b01(1): Width is odd, height is even
		//0b10(2): Width is even, height is odd
		//0b11(3): both width and height are odd
		generateMipsCB.SrcDimension = (srcHeight & 1) << 1 | (srcWidth & 1);

		//How many mipmap levels to compute this pass (max 4 per pass)
		DWORD mipCount;

		//Number of times we can half the size of the texture and get exactly a 50% reduction in size
		//a 1 bit in the width or height indicates an odd dimension
		//Case where either height or width is exactly 1 is handled as a special case
		_BitScanForward(&mipCount, (dstWidth == 1 ? dstHeight : dstWidth) | (dstHeight == 1 ? dstWidth : dstHeight));

		//Max number of mips to generate is 4
		mipCount = std::min<DWORD>(4, mipCount + 1);
		//Clamp to total number of mips left over
		mipCount = (srcMip + mipCount) > resourceDesc.MipLevels ? resourceDesc.MipLevels - srcMip : mipCount;

		//Dimensions should not reduce to 0, can happen if the width and height aren't the same
		dstWidth = std::max<DWORD>(1, dstWidth);
		dstHeight = std::max<DWORD>(1, dstHeight);

		generateMipsCB.SrcMipLevel = srcMip;
		generateMipsCB.NumMipLevels = mipCount;
		generateMipsCB.TexelSize.x = 1.0f / (float)dstWidth;
		generateMipsCB.TexelSize.y = 1.0f / (float)dstHeight;

		SetCompute32BitConstants(GenerateMips::GenerateMipsCB, generateMipsCB);

		SetShaderResourceView(GenerateMips::SrcMip, 0, stagingTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip, 1);
		for (uint32_t mip = 0; mip < mipCount; ++mip)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = resourceDesc.Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

			SetUnorderedAccessView(GenerateMips::OutMip, mip, stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip + mip + 1, 1, &uavDesc);
		}

		//Pad any unused mip levels with a default UAV, keeps DX12 runtime happy
		if (mipCount < 4)
		{
			m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(GenerateMips::OutMip, mipCount, 4 - mipCount, m_GenerateMipsPSO->GetDefaultUAV());
		}

		Dispatch(Math::DivideByMultiple(dstWidth, 8), Math::DivideByMultiple(dstHeight, 8));

		UAVBarrier(stagingTexture);

		srcMip += mipCount;
	}

	//Copy back to the original texture
	if (stagingResource != resource)
	{
		CopyResource(texture, stagingTexture);
	}
}

void CommandList::GenerateMips_BGR(Texture& texture)
{
	auto device = Application::Get().GetDevice();

	auto resource = texture.GetD3D12Resource();
	auto resourceDesc = resource->GetDesc();

	//Create a new resource with a UAV compatible format and UAV flags
	auto copyDesc = resourceDesc;
	copyDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	copyDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	//Create a heap to alias the resource. Used to copy the resource without failing GPU validation
	auto allocationInfo = device->GetResourceAllocationInfo(0, 1, &resourceDesc);
	auto bufferSize = GetRequiredIntermediateSize(resource.Get(), 0, resourceDesc.MipLevels);

	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
	heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ComPtr<ID3D12Heap> heap;
	ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	ComPtr<ID3D12Resource> resourceCopy;
	ThrowIfFailed(device->CreatePlacedResource(heap.Get(), 0, &copyDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resourceCopy)));

	ResourceStateTracker::AddGlobalResourceState(resourceCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	Texture copyTexture(resourceCopy);

	//Create an alias for which to perform the copy operation
	auto aliasDesc = resourceDesc;
	aliasDesc.Format = (resourceDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM || resourceDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB) ? DXGI_FORMAT_B8G8R8X8_UNORM : DXGI_FORMAT_B8G8R8A8_UNORM;

	ComPtr<ID3D12Resource> aliasCopy;
	ThrowIfFailed(device->CreatePlacedResource(heap.Get(), 0, &aliasDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&aliasCopy)));

	ResourceStateTracker::AddGlobalResourceState(aliasCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	//Copy the original texture to the aliased texture
	Texture aliasTexture(aliasCopy);
	AliasingBarrier(Texture(), aliasTexture); //There's no "before" texture. Default construct texture is a null texture
	
	CopyResource(aliasTexture, texture);

	//Alias the UAV compatible texture back
	AliasingBarrier(aliasTexture, copyTexture);
	//Now use the resource copy to generate the mips
	GenerateMips_UAV(copyTexture);

	//Copyback to the original(via the alias to ensure GPU validation)
	AliasingBarrier(copyTexture, aliasTexture);
	CopyResource(texture, aliasTexture);

	//Track resource to ensure the lifetime
	TrackObject(heap);
	TrackResource(copyTexture);
	TrackResource(aliasTexture);
	TrackResource(texture);
}

void CommandList::GenerateMips_sRGB(Texture& texture)
{
	auto device = Application::Get().GetDevice();

	//Create a UAV compatible texture
	auto resource = texture.GetD3D12Resource();
	auto resourceDesc = resource->GetDesc();

	//Create a copy of the original texture with UAV compatible format and UAV flags
	auto copyDesc = resourceDesc;
	copyDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	copyDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	//Create the resource as a placed resource in a heap to perform an aliased copy
	//Createa a heap to alias the resource. Used to copy the resource without failing GPU validation
	auto allocationInfo = device->GetResourceAllocationInfo(0, 1, &resourceDesc);
	auto bufferSize = GetRequiredIntermediateSize(resource.Get(), 0, resourceDesc.MipLevels);

	D3D12_HEAP_DESC heapDesc = {};
	heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
	heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ComPtr<ID3D12Heap> heap;
	ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	ComPtr<ID3D12Resource> resourceCopy;
	ThrowIfFailed(device->CreatePlacedResource(heap.Get(), 0, &copyDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resourceCopy)));

	ResourceStateTracker::AddGlobalResourceState(resourceCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	Texture copyTexture(resourceCopy);

	//Create an alias for which to perform the copy operation
	auto aliasDesc = resourceDesc;

	ComPtr<ID3D12Resource> aliasCopy;
	ThrowIfFailed(device->CreatePlacedResource(heap.Get(), 0, &aliasDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&aliasCopy)));

	ResourceStateTracker::AddGlobalResourceState(aliasCopy.Get(), D3D12_RESOURCE_STATE_COMMON);

	//Copy original texture to the aliased texture
	Texture aliasTexture(aliasCopy);
	AliasingBarrier(Texture(), aliasTexture); //No before texture. Default constructed texture is null

	CopyResource(aliasTexture, texture);

	//Alias the UAV compatible texture back
	AliasingBarrier(aliasTexture, copyTexture);
	//Now use the resource copy to generate the mips
	GenerateMips_UAV(copyTexture);

	//Copy back to the original via the alias to ensure GPU validation
	AliasingBarrier(copyTexture, aliasTexture);
	CopyResource(texture, aliasTexture);

	//Track resource to ensure the lifetime
	TrackObject(heap);
	TrackResource(copyTexture);
	TrackResource(aliasTexture);
	TrackResource(texture);

}

void CommandList::PanoToCubemap(Texture& cubemapTexture, const Texture& panoTexture)
{
	if (m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
	{
		if (!m_ComputeCommandList)
		{
			m_ComputeCommandList = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->GetCommandList();
		}
		m_ComputeCommandList->PanoToCubemap(cubemapTexture, panoTexture);
		return;
	}

	if (!m_PanoToCubemapPSO)
	{
		m_PanoToCubemapPSO = std::make_unique<PanoToCubemapPSO>();
	}

	auto device = Application::Get().GetDevice();

	auto cubemapResource = cubemapTexture.GetD3D12Resource();
	if (!cubemapResource) return;

	CD3DX12_RESOURCE_DESC cubemapDesc(cubemapResource->GetDesc());

	auto stagingResource = cubemapResource;
	Texture stagingTexture(stagingResource);
	//If the passed in resource doesn't allow for UAV access, then create a staging resource used to generate the cubemap
	if ((cubemapDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
	{
		auto stagingDesc = cubemapDesc;
		stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&stagingResource)));

		ResourceStateTracker::AddGlobalResourceState(stagingResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

		stagingTexture.SetD3D12Resource(stagingResource);
		stagingTexture.CreateViews();
		stagingTexture.SetName(L"Pano to Cubemap Staging Texture");

		CopyResource(stagingTexture, cubemapTexture);
	}

	TransitionBarrier(stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	m_d3d12CommandList->SetPipelineState(m_PanoToCubemapPSO->GetPipelineState().Get());
	SetComputeRootSignature(m_PanoToCubemapPSO->GetRootSignature());

	PanoToCubemapCB panoToCubemapCB;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = cubemapDesc.Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	uavDesc.Texture2DArray.ArraySize = 6;

	for (uint32_t mipSlice = 0; mipSlice < cubemapDesc.MipLevels;)
	{
		//Max number of mips to generate per pass is 5
		uint32_t numMips = std::min<uint32_t>(5, cubemapDesc.MipLevels - mipSlice);

		panoToCubemapCB.FirstMip = mipSlice;
		panoToCubemapCB.CubemapSize = std::max<uint32_t>(static_cast<uint32_t>(cubemapDesc.Width), cubemapDesc.Height) >> mipSlice;
		panoToCubemapCB.NumMips = numMips;

		SetCompute32BitConstants(PanoToCubemapRS::PanoToCubemapCB, panoToCubemapCB);

		SetShaderResourceView(PanoToCubemapRS::SrcTexture, 0, panoTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		for (uint32_t mip = 0; mip < numMips; ++mip)
		{
			uavDesc.Texture2DArray.MipSlice = mipSlice + mip;
			SetUnorderedAccessView(PanoToCubemapRS::DstMips, mip, stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, 0, &uavDesc);
		}

		if (numMips < 5)
		{
			//Pad unused mips to keep DX12 runtime happy
			m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(PanoToCubemapRS::DstMips, panoToCubemapCB.NumMips, 5 - numMips, m_PanoToCubemapPSO->GetDefaultUAV());
		}

		Dispatch(Math::DivideByMultiple(panoToCubemapCB.CubemapSize, 16), Math::DivideByMultiple(panoToCubemapCB.CubemapSize, 16), 6);

		mipSlice += numMips;
	}

	if (stagingResource != cubemapResource)
	{
		CopyResource(cubemapTexture, stagingTexture);
	}
}

void CommandList::ClearTexture(const Texture& texture, const float clearColor[4])
{
	TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_d3d12CommandList->ClearRenderTargetView(texture.GetRenderTargetView(), clearColor, 0, nullptr);

	TrackResource(texture);
}

void CommandList::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth, uint8_t stencil)
{
	TransitionBarrier(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_d3d12CommandList->ClearDepthStencilView(texture.GetDepthStencilView(), clearFlags, depth, stencil, 0, nullptr);

	TrackResource(texture);
}

void CommandList::CopyTextureSubresource(Texture& texture, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData)
{
	auto device = Application::Get().GetDevice();
	auto destinationResource = texture.GetD3D12Resource();
	
	if (destinationResource)
	{
		//Resource must be in a copy-destination state
		TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
		FlushResourceBarriers();

		UINT64 requiredSize = GetRequiredIntermediateSize(destinationResource.Get(), firstSubresource, numSubresources);

		//Create a temporary (intermediate) resource for uploading the subresources
		ComPtr<ID3D12Resource> intermediateResource;
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&intermediateResource)));

		UpdateSubresources(m_d3d12CommandList.Get(), destinationResource.Get(), intermediateResource.Get(), 0, firstSubresource, numSubresources, subresourceData);

		TrackObject(intermediateResource);
		TrackObject(destinationResource);
	}
}

void CommandList::SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData)
{
	//Constant buffers must be 256-byte aligned
	auto heapAllocation = m_UploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(heapAllocation.CPU, bufferData, sizeInBytes);

	m_d3d12CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllocation.GPU);
}

void CommandList::SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
{
	m_d3d12CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
}

void CommandList::SetShaderResourceView(uint32_t rootParameterIndex,
	uint32_t descriptorOffset,
	const Resource& resource,
	D3D12_RESOURCE_STATES stateAfter,
	UINT firstSubresource,
	UINT numSubresources,
	const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
{
	if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		for (uint32_t i = 0; i < numSubresources; ++i)
		{
			TransitionBarrier(resource, stateAfter, firstSubresource + i);
		}
	}
	else
	{
		TransitionBarrier(resource, stateAfter);
	}

	m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetShaderResourceView(srv));

	TrackResource(resource);
}

void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
{
	FlushResourceBarriers();

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
	}

	m_d3d12CommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
}