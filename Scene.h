#pragma once

#include <DirectXCollision.h>

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>

class aiMaterial;
class aiMesh;
class aiNode;
class aiScene;

class CommandList;
class Device;
class SceneNode;
class Mesh;
class Material;
class Visitor;

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	void SetRootNode(std::shared_ptr<SceneNode> node)
	{
		m_RootNode = node;
	}

	std::shared_ptr<SceneNode> GetRootNode() const
	{
		return m_RootNode;
	}

	//Get the AABB of the scene. This returns the AABB of the root node of the scene
	DirectX::BoundingBox GetAABB() const;

	virtual void Accept(Visitor& visitor);

protected:
	friend class CommandList;

	//Load a scene from a file
	bool LoadSceneFromFile(CommandList& commandList, const std::wstring& fileName, const std::function<bool(float)>& loadingProgress);

	//Load a scene from a string
	//Scene can be preloaded into a byte array and then scene can be loaded from the loaded byte array
	//@param scene is the byte encoded scene file
	//@param format is the format of the scene file
	bool LoadSceneFromString(CommandList& commandList, const std::string& sceneStr, const std::string& format);

private:
	void ImportScene(CommandList& commandList, const aiScene& scene, std::filesystem::path parentPath);
	void ImportMaterial(CommandList& commandList, const aiMaterial& material, std::filesystem::path parentPath);
	void ImportMesh(CommandList& commandList, const aiMesh& mesh);
	std::shared_ptr<SceneNode> ImportSceneNode(CommandList& commandList, std::shared_ptr<SceneNode> parent, const aiNode* aiNode);
	

	using MaterialMap = std::map<std::string, std::shared_ptr<Material>>;
	using MaterialList = std::vector<std::shared_ptr<Material>>;
	using MeshList = std::vector<std::shared_ptr<Mesh>>;

	MaterialMap m_MaterialMap;
	MaterialList m_Materials;
	MeshList m_Meshes;

	std::shared_ptr<SceneNode> m_RootNode;
	std::wstring m_SceneFile;
};