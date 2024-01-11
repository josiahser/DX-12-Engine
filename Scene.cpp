#include "framework.h"

#include "Scene.h"
#include "CommandList.h"
#include "Device.h"
#include "Material.h"
#include "Mesh.h"
#include "SceneNode.h"
#include "Texture.h"
#include "VertexTypes.h"

//Helper function to create a DirectX::BoundingBoc from an aiAABB
inline DirectX::BoundingBox CreateBoundingBox(const aiAABB& aabb)
{
	XMVECTOR min = XMVectorSet(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z, 1.0f);
	XMVECTOR max = XMVectorSet(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z, 1.0f);

	DirectX::BoundingBox bb;
	BoundingBox::CreateFromPoints(bb, min, max);

	return bb;
}
//TODO: Add Assimp

//bool Scene::LoadSceneFromFile(CommandList& commandList, const std::wstring& fileName, const std::function<bool(float)>& loadingProgress)
//{
//	fs::path filePath = fileName;
//	fs::path exportPath = fs::path(filePath).replace_extension("assbin");
//
//	fs::path parentPath;
//	if (filePath.has_parent_path())
//	{
//		parentPath = filePath.parent_path();
//	}
//	else
//	{
//		parentPath = fs::current_path();
//	}
//}

//bool Scene::LoadSceneFromString(CommandList& commandList, const std::string& sceneStr, const std::string& format)
//{
//
//}

void Scene::ImportScene(CommandList& commandList, const aiScene& scene, std::filesystem::path parentPath)
{
	if (m_RootNode)
	{
		m_RootNode.reset();
	}

	m_MaterialMap.clear();
	m_Materials.clear();
	m_Meshes.clear();

	//Import scene materials
	for (unsigned int i = 0; i < scene.mNumMaterials; ++i)
	{
		ImportMaterial(commandList, *(scene.mMaterials[i]), parentPath);
	}
}