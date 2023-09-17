#pragma once
#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "VertexManager.h"
#include "Texture.h"

struct Mesh
{
	VertexManager vm;
	std::vector<unsigned int> textureids;
};

class Model
{
public:
	Model(const std::string& FilePath, bool fliptextureaxis = false);
	Model(Model&& model) noexcept;
	~Model();
	void Draw(Shader& shd);
	/*Requires a 3° layout in the vertex shader to gain access to the vec3 offset variable
	EXAMPLE OF USAGE:
	layout(location = 0) in vec3 pos;
	(other code)
	layout(location = 3) in vec3 offsets;

	int main()
	{
		(other code)
		gl_Position = projection * view * ((model * vec4(pos, 1.0f)) + vec4(offsets,1.0f));
		(other code)
	}

	*/
	void DrawInstancedPositions(Shader& shd, u32 num_instances, glm::vec3* positions);

	void Rotate(f32 fRadians, const glm::vec3& dir);
	void Translate(const glm::vec3& dir);
	void Scale(const glm::vec3& dir);
	void Scale(f32 fScaleFactor);

	void LoadExternalTexture(const std::string& texturepath, const std::string& uniform);

	const std::vector<Mesh>& GetMeshesInVM() const { return m_Meshes; }

	//TODO: to be deleted by the user
	f32* GetRawBuffer() const;
	f32* GetRawAttribute(u32 begin, u32 end) const;
	u32 GetValuesCount() const;

	//The variable to_jump is an optimization which states how many vertices have to be discarded between two evaluations,
	//very useful for models with a large amount of vertices
	bool IsIntersectedBy(const glm::vec3& pos, const glm::vec3& dir, f32 fRadius, f32 ratio_vertex_center, int to_jump);
private:
	//Utility function for instanced rendering
	void LoadModelFromFile(const std::string& FilePath);
	void LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string& name, Mesh& m);
	void ProcessNode(const aiScene* scene, aiNode* node);
	void SetupMesh(const aiScene* scene, aiMesh* mesh);
private:
	bool m_FlipTextureAxis;
	std::vector<Mesh> m_Meshes;
	std::vector<Texture> m_Textures;
	std::vector<std::pair<Texture, std::string>> m_ExternalTextures;
	std::vector<std::pair<std::string, u32>> m_Cache;
	glm::mat4 m_ModelMatrix;
	std::string m_Directory;
	glm::vec3 m_Position;

	//For ray collision operations
	glm::vec3* m_Vertices;
};
