#pragma once

#include <DirectXMath.h>

#include <d3d12.h>

struct VertexPosition
{
	VertexPosition() = default;
	
	explicit VertexPosition(const DirectX::XMFLOAT3& position)
		: Position(position)
	{}

	explicit VertexPosition(DirectX::FXMVECTOR position)
	{
		DirectX::XMStoreFloat3(&(this->Position), position);
	}

	DirectX::XMFLOAT3 Position;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementCount = 1;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct VertexPositionNormalTangentBitangentTexture
{
	VertexPositionNormalTangentBitangentTexture() = default;

	explicit VertexPositionNormalTangentBitangentTexture(const DirectX::XMFLOAT3& position,
		const DirectX::XMFLOAT3& normal,
		const DirectX::XMFLOAT3& texCoord)
		//const DirectX::XMFLOAT3& tangent = {0, 0, 0},
		//const DirectX::XMFLOAT3& bitangent = {0, 0, 0})
		: Position(position)
		, Normal(normal)
		//, Tangent(tangent)
		//, Bitangent(bitangent)
		, TexCoord(texCoord)
	{}

	explicit VertexPositionNormalTangentBitangentTexture(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal,
		DirectX::FXMVECTOR texCoord, DirectX::GXMVECTOR tangent = { 0,0,0,0 }, DirectX::HXMVECTOR bitangent = { 0, 0, 0, 0 })
	{
		DirectX::XMStoreFloat3(&(this->Position), position);
		DirectX::XMStoreFloat3(&(this->Normal), normal);
		//DirectX::XMStoreFloat3(&(this->Tangent), tangent);
		//DirectX::XMStoreFloat3(&(this->Bitangent), bitangent);
		DirectX::XMStoreFloat3(&(this->TexCoord), texCoord);
	}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	//DirectX::XMFLOAT3 Tangent;
	//DirectX::XMFLOAT3 Bitangent;
	DirectX::XMFLOAT3 TexCoord;

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementCount = 3;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};