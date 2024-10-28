#pragma once
#include <iostream>
#include <vector>

#define ASSIMP_STATIC_LIB

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "VertexManager.h"
#include "Texture.h"

namespace gfx
{
    static constexpr u32 max_bone_movement_per_vertex = 4;

    struct ModelData
    {
        VertexMesh mesh_data;
        u32 mesh_count;
        u32 vertex_weight_buffer;
        //INFO: we try to store all vertex/index data in a single vertex/index buffer,
        //then we render all the meshes in separate drawcalls depeding on their offsets
        //in the global buffer
        u32* vertex_divisors;
        u32* index_divisors;

        //TODO(C7) probably also rewriting texture is better
        Texture* textures;

        bool initialized;
    };

    struct VertexWeight
    {
        u32 vertex_id;
        u32 bone_count;
        u32 bone_id[max_bone_movement_per_vertex];
        f32 bone_weight[max_bone_movement_per_vertex];
    };

	ModelData model_create(const char* filepath, bool load_textures);
	void model_load_textures(ModelData& data, std::string* texture_locations, u32 locations_count);
	void model_render(const ModelData& model, Shader& shader, const char* diffuse_uniform);
	void model_get_count_of_vertices_and_indices(const aiScene* scene, u32* num_vertices, u32* num_indices);
	bool model_mesh_has_weights(const aiMesh* mesh);
	void model_print_tree_transformations(const aiNode* node, const std::string& tabspace = "");

	void model_parse_bones(const aiMesh* mesh, VertexWeight* weight_data, u32 weight_count);

	void model_cleanup(ModelData* mesh);
}


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
