#include "MainIncl.h"
#include "Model.h"
#include "math_basics.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace gfx
{
	ModelData model_create(const std::string& filepath, bool load_textures)
	{
		ModelData model_data = {};

		Assimp::Importer importer;
		auto& scene = model_data.scene;
		u32 assimp_flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals;
		//INFO: this will call external allocations
		const aiScene* _scene = importer.ReadFile(filepath.c_str(), assimp_flags);
		scene = importer.GetOrphanedScene();

		if (!scene) {
			log_message("the given model was not found by the loader\n");
			model_data.initialized = false;
			return model_data;
		}

		model_data.mesh_count = scene->mNumMeshes;
		u32 vertices_count = 0;
		u32 indices_count = 0;
		u32 bones_count = 0;
		const u32 vertex_stride = 8;

		model_get_vertices_indices_bones_count(scene, &vertices_count, &indices_count, &bones_count);
		model_data.bone_count = bones_count;

		f32* vertices = new f32[vertices_count * vertex_stride];
		VertexWeight* vertices_weight = new VertexWeight[vertices_count];
		std::memset(vertices_weight, 0, vertices_count * sizeof(VertexWeight));
		u32* indices = new u32[indices_count];

		model_data.vertex_divisors = new u32[scene->mNumMeshes];
		model_data.index_divisors = new u32[scene->mNumMeshes];

		defer {
				delete[] vertices;
				delete[] indices;
				delete[] vertices_weight;
		};

		u32 vertices_parsed_so_far = 0;
		u32 indices_parsed_so_far  = 0;
		u32 bones_incremental_idx  = 0;

		auto& bone_transformations = model_data.bone_transformations;
		bone_transformations = new BoneInfo[bones_count];
		model_map_bone_names_to_id(scene, bone_transformations, bones_count);

		for (u32 i = 0; i < model_data.mesh_count; i++) {
			const aiMesh* mesh = scene->mMeshes[i];

			bool has_normals = mesh->mNormals;
			bool has_tex_coords = mesh->mTextureCoords[0];
			aiVector3D zero_vector = { 0.0f, 0.0f, 0.0f };

			for (u32 j = 0; j < mesh->mNumVertices; j++) {
				const u32 base_index = (vertices_parsed_so_far + j) * vertex_stride;

				auto vec = mesh->mVertices[j];
				vertices[base_index + 0] = vec.x;
				vertices[base_index + 1] = vec.y;
				vertices[base_index + 2] = vec.z;

				vec = has_normals ? mesh->mNormals[j] : zero_vector;
				vertices[base_index + 3] = vec.x;
				vertices[base_index + 4] = vec.y;
				vertices[base_index + 5] = vec.z;

				vec = has_tex_coords ? mesh->mTextureCoords[0][j] : zero_vector;
				vertices[base_index + 6] = vec.x;
				vertices[base_index + 7] = vec.y;
			}

	        u32 current_mesh_indices = 0;
			for (u32 j = 0; j < mesh->mNumFaces; j++) {
				const aiFace& face = mesh->mFaces[j];
				for (u32 k = 0; k < face.mNumIndices; k++) {
				    const u32 relative_index = (indices_parsed_so_far + j * 3) + k;
					indices[relative_index] = face.mIndices[k];
				}
				current_mesh_indices += face.mNumIndices;
			}

			if(model_mesh_has_weights(mesh)) {
				VertexWeight* vertices_weight_current = vertices_weight + vertices_parsed_so_far;
				model_parse_weights(mesh, vertices_weight_current, mesh->mNumVertices, bone_transformations, bones_count);
				std::sort(vertices_weight_current, vertices_weight_current + mesh->mNumVertices,
					[](const VertexWeight& first, const VertexWeight& second) {return first.vertex_id < second.vertex_id;});
			}

			model_data.vertex_divisors[i] = vertices_parsed_so_far;
			model_data.index_divisors[i]  = current_mesh_indices;

			vertices_parsed_so_far += mesh->mNumVertices;
			indices_parsed_so_far  += current_mesh_indices;
		}
		model_data.mesh_data.indices_count = indices_count;

		//Parsing bone matrices
		model_parse_bone_transformations(scene, scene->mRootNode, 135.0f, bone_transformations, bones_count, model_data.world_transformation);

		//Position, normals, texcoords
		LayoutElement attributes[3] = {
			{ 3, GL_FLOAT, GL_FALSE, vertex_stride * sizeof(f32), 0 },
			{ 3, GL_FLOAT, GL_FALSE, vertex_stride * sizeof(f32), 3 * sizeof(f32) },
			{ 2, GL_FLOAT, GL_FALSE, vertex_stride * sizeof(f32), 6 * sizeof(f32) },
		};

		LayoutElement weight_attributes[3] = {
			{ 1, GL_UNSIGNED_INT, 0, sizeof(VertexWeight), offsetof(VertexWeight, bone_count) },
			{ 4, GL_UNSIGNED_INT, 0, sizeof(VertexWeight), offsetof(VertexWeight, bone_id) },
			{ 4, GL_FLOAT, GL_FALSE, sizeof(VertexWeight), offsetof(VertexWeight, bone_weight) },
		};

		model_data.mesh_data = create_mesh_with_indices_and_push_attributes(vertices, vertices_count * vertex_stride * sizeof(f32), indices, indices_count * sizeof(u32), attributes, sizeof(attributes));

		glGenBuffers(1, &model_data.vertex_weight_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, model_data.vertex_weight_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices_count * sizeof(VertexWeight), vertices_weight, GL_STATIC_DRAW);

		push_mesh_attributes(&model_data.mesh_data, weight_attributes, sizeof(weight_attributes), 3);
		//Default texture loading might not work depending on where the textures are stored
		if(load_textures) {
			model_data.textures = new TextureData[scene->mNumMeshes];
			model_data.texture_info = new ModelTextureInfo[scene->mNumMeshes];
			model_data.texture_count = scene->mNumMeshes;

			std::memset(model_data.textures, 0, sizeof(TextureData) * model_data.texture_count);

			auto& texture_info = model_data.texture_info;
			std::string current_working_dir;

			{
				bool backslash_defined = filepath.find_last_of('\\');
				bool forwardslash_defined = filepath.find_last_of('/');

				assert(backslash_defined || forwardslash_defined, "invalid path syntax");

				if (backslash_defined) {
				    current_working_dir = filepath.substr(0, filepath.find_last_of('\\'));
				}

				if (forwardslash_defined) {
				    current_working_dir = filepath.substr(0, filepath.find_last_of('/'));
				}
			}

			current_working_dir += "/";

			for(u32 i = 0; i < scene->mNumMeshes; i++) {
				const aiMesh* mesh = scene->mMeshes[i];
			    const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			    if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			        aiString path;

					if(material->GetTexture(aiTextureType_DIFFUSE, 0, &path, 0, 0, 0, 0, 0) == AI_SUCCESS) {
						texture_info[i].name = path.data;
						bool texture_already_loaded = false;

						for(u32 j = 0; j < i; j++) {
							if(texture_info[j].name == texture_info[i].name) {
								texture_info[i].index = j;
								texture_already_loaded = true;
								break;
							}
						}

			        	if(texture_already_loaded) continue;

			            std::string loading_path = current_working_dir + path.C_Str();
						texture_info[i].index = i;
			            model_data.textures[i] = texture_create(loading_path.c_str());
			        }
			    }
			}
		}

		//Assuming the first channel has as many position/rotation frames as the other ones
		if(scene->mNumAnimations > 0) {
			model_data.keyframes_last_timestamp = scene->mAnimations[0]->mDuration;
		}

	    model_data.initialized = true;
	    return model_data;
	}

	//Load textures from a custom position, requires locations to have full path
	//binds the n texture to the n mesh
	//it is recommended to try and load the texture automatically with model_create
	void model_load_textures(ModelData& model_data, std::string* texture_paths, u32 texture_count)
	{
		assert(texture_paths, "the variable needs to be defined in this scope\n");
		auto& texture_info = model_data.texture_info;
		const auto& scene  = model_data.scene;

		if(model_data.textures || model_data.texture_info) {
			for(u32 i = 0; i < model_data.texture_count; i++)
				texture_cleanup(&model_data.textures[i]);

			delete[] model_data.textures;
			delete[] model_data.texture_info;
		}

		model_data.textures = new TextureData[texture_count];
		model_data.texture_info = new ModelTextureInfo[texture_count];

		for(u32 i = 0; i < texture_count; i++) {
			const aiMesh* mesh = scene->mMeshes[i];
			const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		    texture_info[i].name = texture_paths[i];
		    bool texture_already_loaded = false;

			for(u32 j = 0; j < i; j++) {
				if(texture_info[j].name == texture_info[i].name) {
					texture_info[i].index = j;
					texture_already_loaded = true;
					break;
				}
			}

		    if(texture_already_loaded) continue;

			texture_info[i].index = i;
		    model_data.textures[i] = texture_create(texture_paths[i].c_str());
		}
	}

	void model_render(const ModelData& model, Shader& shader, const char* diffuse_uniform)
	{
		assert(model.initialized, "the model needs to be initialized\n");
		bind_vertex_array(model.mesh_data);
		bind_index_buffer(model.mesh_data);

		u32 indices_drawn = 0;
		for(u32 i = 0; i < model.mesh_count; i++) {
			if(model.textures) {
				auto& diffuse_texture = model.textures[model.texture_info[i].index];
				texture_bind(diffuse_texture, 0);
			}

			shader.Uniform1i(0, std::string(diffuse_uniform));
			glDrawElementsBaseVertex(GL_TRIANGLES, model.index_divisors[i], GL_UNSIGNED_INT,
			    (void*)(sizeof(u32) * indices_drawn), model.vertex_divisors[i]);

		    indices_drawn += model.index_divisors[i];
		}
	}

	//INFO(C7): the num_bones count can contain some repeated bones because they are part
	//of multiple vertices
	void model_get_vertices_indices_bones_count(const aiScene* scene, u32* num_vertices, u32* num_indices,
		u32* num_bones)
	{
		assert(num_vertices && num_indices, "these pointers need to be defined in this scope");

		*num_vertices = 0;
		*num_indices = 0;
		*num_bones = 0;

		for(u32 i = 0; i < scene->mNumMeshes; i++) {
			const aiMesh* mesh = scene->mMeshes[i];
			(*num_vertices) += mesh->mNumVertices;
			for(u32 j = 0; j < mesh->mNumFaces; j++) {
				(*num_indices) += mesh->mFaces[j].mNumIndices;
			}

			(*num_bones) += mesh->mNumBones;
		}
	}

	bool model_mesh_has_weights(const aiMesh* mesh)
	{
		for(u32 i = 0; i < mesh->mNumBones; i++) {
			if(mesh->mBones[i]->mNumWeights > 0) {
				return true;
			}
		}

		return false;
	}

	void model_map_bone_names_to_id(const aiScene* scene, BoneInfo* bone_info, u32 bones_count)
	{
		u32 unique_bone_index = 0;
		for(u32 i = 0; i < scene->mNumMeshes; i++) {
			for(u32 j = 0; j < scene->mMeshes[i]->mNumBones; j++) {
				auto& bone = scene->mMeshes[i]->mBones[j];
				std::string bone_name(bone->mName.C_Str());

				s32 bone_index = model_find_bone_info(bone_info, bones_count, bone_name);
				if(bone_index == -1) {
					auto& transformation = bone_info[unique_bone_index];
					transformation.name = bone_name;
					transformation.id = unique_bone_index;
					transformation.local_transformation = glm_mat_cast(bone->mOffsetMatrix);

					unique_bone_index++;
				}
			}
		}
	}

	s32 model_find_bone_info(const BoneInfo* data, u32 size, std::string& name)
	{
		for(u32 i = 0; i < size; i++) {
			if(data[i].name == name)
				return i;
		}

		return -1;
	}

	aiNodeAnim* model_find_animation_channel(const aiAnimation* anim, const std::string& name)
	{
		for(u32 i = 0; i < anim->mNumChannels; i++) {
			aiNodeAnim* ptr = anim->mChannels[i];
			if(std::strcmp(ptr->mNodeName.C_Str(), name.c_str()) == 0) {
				return ptr;
			}
		}

		return nullptr;
	}

	glm::vec3 model_lerp_keyframes_positions(const aiNodeAnim* node_anim, f32 ticks)
	{
		if(node_anim->mNumPositionKeys == 1)
			return glm::make_vec3(&node_anim->mPositionKeys[0].mValue[0]);

		u32 first_index = 0;
		f32 first_time = 0.0f;
		f32 second_time = 0.0f;
		for(u32 i = 1; node_anim->mNumPositionKeys; i++) {
			auto& key = node_anim->mPositionKeys[i];
			auto& prev_key = node_anim->mPositionKeys[i - 1];
			if(key.mTime >= ticks) {
				first_time = prev_key.mTime;
				second_time = key.mTime;
				first_index = i - 1;
				break;
			}
		}

		auto& first_key  = node_anim->mPositionKeys[first_index];
		auto& second_key = node_anim->mPositionKeys[first_index + 1];
		glm::vec3 first_vector  = glm::make_vec3(&first_key.mValue[0]);
		glm::vec3 second_vector = glm::make_vec3(&second_key.mValue[0]);

		f32 delta = (ticks - first_time) / (second_time - first_time);
		glm::vec3 result = (1.0f - delta) * first_vector + delta * second_vector;
		return result;
	}

	glm::quat model_lerp_keyframes_rotations(const aiNodeAnim* node_anim, f32 ticks)
	{
		if(node_anim->mNumRotationKeys == 1)
			return glm_quat_cast(node_anim->mRotationKeys[0].mValue);

		u32 first_index = 0;
		f32 first_time = 0.0f;
		f32 second_time = 0.0f;
		for(u32 i = 1; node_anim->mNumPositionKeys; i++) {
			auto& key = node_anim->mPositionKeys[i];
			auto& prev_key = node_anim->mPositionKeys[i - 1];
			if(key.mTime >= ticks) {
				first_time = prev_key.mTime;
				second_time = key.mTime;
				first_index = i - 1;
				break;
			}
		}

		auto& first_key  = node_anim->mRotationKeys[first_index];
		auto& second_key = node_anim->mRotationKeys[first_index + 1];
		f32 delta = (ticks - first_time) / (second_time - first_time);

		aiQuaternion result;
		aiQuaternion::Interpolate(result, first_key.mValue, second_key.mValue, delta);
		return glm::normalize(glm_quat_cast(result));
	}
	glm::vec3 model_lerp_keyframes_scales(const aiNodeAnim* node_anim, f32 ticks)
	{
		if(node_anim->mNumScalingKeys == 1)
			return glm::make_vec3(&node_anim->mScalingKeys[0].mValue[0]);

		u32 first_index = 0;
		f32 first_time = 0.0f;
		f32 second_time = 0.0f;
		for(u32 i = 1; node_anim->mNumPositionKeys; i++) {
			auto& key = node_anim->mPositionKeys[i];
			auto& prev_key = node_anim->mPositionKeys[i - 1];
			if(key.mTime >= ticks) {
				first_time = prev_key.mTime;
				second_time = key.mTime;
				first_index = i - 1;
				break;
			}
		}

		auto& first_key  = node_anim->mScalingKeys[first_index];
		auto& second_key = node_anim->mScalingKeys[first_index + 1];
		glm::vec3 first_vector  = glm::make_vec3(&first_key.mValue[0]);
		glm::vec3 second_vector = glm::make_vec3(&second_key.mValue[0]);

		f32 delta = (ticks - first_time) / (second_time - first_time);
		glm::vec3 result = (1.0f - delta) * first_vector + delta * second_vector;
		return result;
	}

	void model_parse_bone_transformations(ModelData& model_data, f32 ticks)
	{
		model_parse_bone_transformations(model_data.scene, model_data.scene->mRootNode, ticks, model_data.bone_transformations, model_data.bone_count, model_data.world_transformation);
	}

	void model_parse_bone_transformations(const aiScene* scene, const aiNode* node, f32 ticks, BoneInfo* bone_info, u32 bone_info_count, glm::mat4& world_transformation, const glm::mat4& parent_transform)
	{
		if(!node) return;
		glm::mat4 current_transformation;

		bool animation_file_loaded = (scene->mNumAnimations > 0);
		std::string bone_name(node->mName.C_Str());

		//INFO(C7) apparently the mTransform of the root node stores information about the
		//physical rototranslation of the model in the environment, so the matrix is not used
		//for in-model coordinate system shifting
		if(node != scene->mRootNode) {
			glm::mat4 node_transform = glm_mat_cast(node->mTransformation);

			if(animation_file_loaded && ticks != 0.0f) {
				//Default animation atm, should take this in as a parameter
				aiAnimation* animation = scene->mAnimations[0];
				aiNodeAnim* current_channel = model_find_animation_channel(animation, bone_name);

				if(current_channel) {
					glm::vec3 position = model_lerp_keyframes_positions(current_channel, ticks);
					glm::quat rotation = model_lerp_keyframes_rotations(current_channel, ticks);
					glm::vec3 scale    = model_lerp_keyframes_scales(current_channel, ticks);

					glm::mat4 position_mat = glm::translate(glm::mat4(1.0f), position);
					glm::mat4 rotation_mat = glm::toMat4(rotation);
					glm::mat4 scaling_mat  = glm::scale(glm::mat4(1.0f), scale);

					node_transform = position_mat * rotation_mat * scaling_mat;
				}
			}

			current_transformation = parent_transform * node_transform;

		} else {
			current_transformation = parent_transform;
			world_transformation = glm_mat_cast(node->mTransformation);
		}

		s32 bone_index = model_find_bone_info(bone_info, bone_info_count, bone_name);
		if(bone_index != -1) {
			auto& current_bone_info = bone_info[bone_index];
			current_bone_info.name = bone_name;
			current_bone_info.final_transformation = current_transformation * current_bone_info.local_transformation;
			current_bone_info.initialized = true;
		}

		for(u32 i = 0; i < node->mNumChildren; i++) {
			model_parse_bone_transformations(scene, node->mChildren[i], ticks, bone_info, bone_info_count, world_transformation, current_transformation);
		}
	}

	void model_parse_weights(const aiMesh* mesh, VertexWeight* weight_data, u32 weight_count,
		const BoneInfo* bone_info, u32 bone_info_count)
	{
	    assert(mesh, "this variable needs to be defined in this scope");

		auto find_element = [](VertexWeight* data, u32 count, u32 vertex_id) -> s32 {
			for(s32 i = 0; i < count; i++) {
				if(data[i].vertex_id == vertex_id)
					return i;
			}

			return -1;
		};

		u32 count = 0;
		for(u32 i = 0; i < mesh->mNumBones; i++) {
			s32 bone_index = -1;

	    	{
				std::string bone_name(mesh->mBones[i]->mName.C_Str());
				bone_index = model_find_bone_info(bone_info, bone_info_count, bone_name);
				assert(bone_index != -1, "the bone name should be loaded by this point");
			}

	        for(u32 j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
				auto weight = mesh->mBones[i]->mWeights[j];
				s32 element_index = find_element(weight_data, weight_count, weight.mVertexId);
				auto& vertex_weight = (element_index != -1) ? weight_data[element_index] : weight_data[count++];

				assert(count <= weight_count, "range specified too small");

				if (vertex_weight.bone_count == max_bone_movement_per_vertex)
					continue;


				vertex_weight.vertex_id = weight.mVertexId;
				u32 idx = vertex_weight.bone_count++;
				vertex_weight.bone_id[idx] = bone_index;
				vertex_weight.bone_weight[idx] = weight.mWeight;
	        }
	    }
	}

	void model_cleanup(ModelData* model)
	{
	    assert(model, "model needs to be defined in this scope");
	    cleanup_mesh(&model->mesh_data);
	    glDeleteBuffers(1, &model->vertex_weight_buffer);
	    delete[] model->vertex_divisors;
	    delete[] model->index_divisors;
	    delete[] model->bone_transformations;
	    delete[] model->texture_info;
	    delete model->scene;

	    if(model->textures) {
			for(u32 i = 0; i < model->texture_count; i++)
				texture_cleanup(&model->textures[i]);

	        delete[] model->textures;
	    }
	}
}

Model::Model(const std::string& FilePath, bool fliptextureaxis)
	:m_ModelMatrix(1.0f), m_Vertices(nullptr), m_Position(0.0f)
{
	m_FlipTextureAxis = fliptextureaxis;
	LoadModelFromFile(FilePath);
}

Model::Model(Model&& model) noexcept
{
	m_Meshes = std::move(model.m_Meshes);
	m_Textures = std::move(model.m_Textures);
	m_ExternalTextures = std::move(model.m_ExternalTextures);
	m_FlipTextureAxis = model.m_FlipTextureAxis;

	m_Cache = model.m_Cache;
	m_ModelMatrix = model.m_ModelMatrix;
	m_Directory = model.m_Directory;
	m_Position = model.m_Position;

	m_Vertices = model.m_Vertices;
	model.m_Vertices = nullptr;
}

Model::~Model()
{
	//Deleting vertices
	if (m_Vertices)
	{
		::operator delete(m_Vertices);
	}

	m_Cache.clear();
	m_Meshes.clear();
	m_Textures.clear();
	m_ExternalTextures.clear();
}

void Model::Draw(Shader& shd)
{
	for (int i = 0; i < m_Meshes.size(); i++)
	{
		for (int j = 0; j < m_Meshes[i].textureids.size(); j++)
		{
			glActiveTexture(GL_TEXTURE0 + j);
			glBindTexture(GL_TEXTURE_2D, m_Meshes[i].textureids[j]);
			std::string uniformname = "texture" + std::to_string(j + 1);

			if (shd.IsUniformDefined(uniformname))
				shd.Uniform1i(j, uniformname);
		}

		//Manually textures loaded externally have priority over the ones defined in the meshes
		for (int i = 0; i < m_ExternalTextures.size(); i++)
		{
			m_ExternalTextures[i].first.Bind(i);
			std::string uniformname = m_ExternalTextures[i].second;

			if (shd.IsUniformDefined(uniformname))
				shd.Uniform1i(i, uniformname);
		}

		shd.UniformMat4f(m_ModelMatrix, "model");
		m_Meshes[i].vm.BindVertexArray();
		glDrawElements(GL_TRIANGLES, m_Meshes[i].vm.GetIndicesCount(), GL_UNSIGNED_INT, nullptr);
	}
}

void Model::DrawInstancedPositions(Shader& shd, u32 num_instances, glm::vec3* positions)
{
	u32* instancebuffers = new u32[m_Meshes.size()];

	for (int i = 0; i < m_Meshes.size(); i++)
	{
		//The following operation require the corrispondent VAO to be bound
		m_Meshes[i].vm.BindVertexArray();


		//Generating a buffer which handles offsets for each mesh of the model
		//The rest of the code is almost identical to the function Draw's code
		glGenBuffers(1, &instancebuffers[i]);
		glBindBuffer(GL_ARRAY_BUFFER, instancebuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, num_instances * sizeof(glm::vec3), positions, GL_STATIC_DRAW);

		//We enable the 3rd index because we have already used the 0,1,2 for respectively positions, normals
		//and texture coordinates
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0);

		//Telling opengl that we want to load only one attribute on 3rd location (the offset) per instance
		glVertexAttribDivisor(3, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		for (int j = 0; j < m_Meshes[i].textureids.size(); j++)
		{
			glActiveTexture(GL_TEXTURE0 + j);
			glBindTexture(GL_TEXTURE_2D, m_Meshes[i].textureids[j]);
			std::string uniformname = "texture" + std::to_string(j + 1);

			if (shd.IsUniformDefined(uniformname))
				shd.Uniform1i(j, uniformname);
		}

		//Manually textures loaded externally have priority over the ones defined in the meshes
		for (int i = 0; i < m_ExternalTextures.size(); i++)
		{
			m_ExternalTextures[i].first.Bind(i);
			std::string uniformname = m_ExternalTextures[i].second;

			if (shd.IsUniformDefined(uniformname))
				shd.Uniform1i(i, uniformname);
		}

		shd.UniformMat4f(m_ModelMatrix, "model");
		//And drawing the number of instances using the instanced version of the drawcall
		glDrawElementsInstanced(GL_TRIANGLES, m_Meshes[i].vm.GetIndicesCount(), GL_UNSIGNED_INT, nullptr, num_instances);
	}

	//Then cleaning up the buffers(this process can be optimized, made just to make the code more straight-forward)
	for (int i = 0; i < m_Meshes.size(); i++)
		glDeleteBuffers(1, &instancebuffers[i]);

	delete[] instancebuffers;
}


void Model::Rotate(f32 fRadians, const glm::vec3& dir)
{
	m_ModelMatrix = glm::rotate(m_ModelMatrix, fRadians, dir);
}

void Model::Translate(const glm::vec3& dir)
{
	m_ModelMatrix = glm::translate(m_ModelMatrix, dir);
	m_Position += dir;
}

void Model::Scale(const glm::vec3& dir)
{
	m_ModelMatrix = glm::scale(m_ModelMatrix, dir);
}

void Model::Scale(f32 fScaleFactor)
{
	Scale(glm::vec3(fScaleFactor));
}

void Model::LoadExternalTexture(const std::string& texturepath, const std::string& uniform)
{
	Texture tx(texturepath.c_str(), m_FlipTextureAxis);
	m_ExternalTextures.push_back(std::make_pair(std::move(tx), uniform));
}

f32* Model::GetRawBuffer() const
{
	f32* res, *cpy;
	long int ptr_dim = 0;
	long int offset = 0;

	for (const auto& m : m_Meshes)
	{
		ptr_dim += m.vm.GetValuesCount();
	}

	res = (f32*)::operator new(ptr_dim * sizeof(f32));
	if (!res)
	{
		std::cout << "Not enough memory for model ptr allocation\n";
		return nullptr;
	}

	for (int i = 0; i < m_Meshes.size(); i++)
	{
		offset = 0;
		for (int k = 0; k < i; k++)
		{
			offset += m_Meshes[k].vm.GetValuesCount();
		}

		cpy = m_Meshes[i].vm.GetRawBuffer();
		memcpy(res + offset, cpy, m_Meshes[i].vm.GetValuesCount() * sizeof(f32));
		::operator delete(cpy);
	}

	return res;
}

f32* Model::GetRawAttribute(u32 begin, u32 end) const
{
	f32* res, * cpy;
	long int ptr_dim = 0;
	long int offset = 0;
	u32 values_per_attrib;

	//8 is the stride lenght for every model
	if (begin >= 8 || end >= 8 || end <= begin) return nullptr;

	values_per_attrib = end - begin;
	for (const auto& m : m_Meshes)
	{
		ptr_dim += m.vm.GetValuesCount() / m.vm.GetStrideLenght() * values_per_attrib;
	}

	res = (f32*)::operator new(ptr_dim * sizeof(f32));

	if (!res)
	{
		std::cout << "Not enough memory for model ptr allocation\n";
		return nullptr;
	}

	for (int i = 0; i < m_Meshes.size(); i++)
	{
		offset = 0;
		for (int k = 0; k < i; k++)
		{
			offset += m_Meshes[k].vm.GetValuesCount() / m_Meshes[k].vm.GetStrideLenght() * values_per_attrib;
		}

		cpy = m_Meshes[i].vm.GetRawAttribute(begin, end);
		memcpy(res + offset, cpy, m_Meshes[i].vm.GetValuesCount() / m_Meshes[i].vm.GetStrideLenght() * values_per_attrib * sizeof(f32));
		::operator delete(cpy);
	}

	return res;
}

u32 Model::GetValuesCount() const
{
	u32 count = 0;

	for (const auto& m : m_Meshes)
	{
		count += m.vm.GetValuesCount();
	}

	return count;
}

bool Model::IsIntersectedBy(const glm::vec3& pos, const glm::vec3& dir, f32 fRadius, f32 ratio_vertex_center, int to_jump)
{
	if (!m_Vertices)
	{
		m_Vertices = (glm::vec3*)GetRawAttribute(0, 3);
	}

	if (to_jump + 1 < 1) return false;

	for (int j = 0; j < GetValuesCount() / 8; j++)
	{
		//Getting the world space coordinates of m_Vertices[j]
		glm::vec3 vertexpos = glm::vec3(m_ModelMatrix * glm::vec4(m_Vertices[j], 1.0f));
		//Calculating the ray which goes between the vertex and the center of the shape
		glm::vec3 midray = (vertexpos - m_Position) * ratio_vertex_center;
		//Converting midray into a world space point
		glm::vec3 translated = vertexpos - midray;

		f32 dist = glm::length(pos - translated);
		glm::vec3 res = pos + dir * dist;

		if (res.x >= translated.x - fRadius && res.y >= translated.y - fRadius && res.z >= translated.z - fRadius &&
			res.x <= translated.x + fRadius && res.y <= translated.y + fRadius && res.z <= translated.z + fRadius)
		{
			return true;
		}

	}

	return false;
}

void Model::LoadModelFromFile(const std::string& FilePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		FilePath,
		aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		std::cout << "Model could not be found" << std::endl;
		return;
	}

	m_Directory = FilePath.substr(0, FilePath.find_last_of('\\'));

	ProcessNode(scene, scene->mRootNode);
	importer.FreeScene();
}

void Model::LoadMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string& name, Mesh& m)
{
	u32 count = mat->GetTextureCount(type);

	for (int i = 0; i < count; i++)
	{
		aiString texture_folder_name;
		mat->GetTexture(type, i, &texture_folder_name);

		std::string texpath = m_Directory + '\\' + std::string(texture_folder_name.C_Str());

		bool bTextureLoaded = false;

		for (auto& str : m_Cache)
		{
			if (strcmp(texture_folder_name.C_Str(), str.first.c_str()) == 0)
			{
				//If the texture has already been loaded, simply push back into the textureids vector
				//of the mesh the texture id
				m.textureids.push_back(str.second);
				bTextureLoaded = true;
			}
		}

		if (!bTextureLoaded)
		{
			//We load the texture and then we push into the m_Cache vector a copy of the texture
			//filename and the opengl texture id, so when we found another mesh which uses one of there
			//texture, we know which id to assign
			m_Textures.push_back(Texture(texpath.c_str(), m_FlipTextureAxis));
			m_Cache.push_back(
				std::make_pair(std::string(texture_folder_name.C_Str()), m_Textures[m_Textures.size() - 1].ID())
			);

			m.textureids.push_back(m_Textures[m_Textures.size() - 1].ID());
		}

	}
}

void Model::ProcessNode(const aiScene* scene, aiNode* node)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		SetupMesh(scene, mesh);
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(scene, node->mChildren[i]);
	}
}

void Model::SetupMesh(const aiScene* scene, aiMesh* mesh)
{
	std::vector<f32> vb;
	std::vector<u32> ib;

	//Loading the data into our vectors
	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		vb.push_back(mesh->mVertices[i].x);
		vb.push_back(mesh->mVertices[i].y);
		vb.push_back(mesh->mVertices[i].z);

		if (mesh->mNormals)
		{
			vb.push_back(mesh->mNormals[i].x);
			vb.push_back(mesh->mNormals[i].y);
			vb.push_back(mesh->mNormals[i].z);
		}
		else
		{
			vb.push_back(0.0f);
			vb.push_back(0.0f);
			vb.push_back(0.0f);
		}

		if (mesh->mTextureCoords[0])
		{
			vb.push_back(mesh->mTextureCoords[0][i].x);
			vb.push_back(mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vb.push_back(0.0f);
			vb.push_back(0.0f);
		}
	}

	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; j++)
		{
			ib.push_back(face.mIndices[j]);
		}
	}

	//Setting the layout and sending the data to OpenGL using the already defined VertexManager class
	Layout layout;
	layout.PushAttribute({ 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), 0 });
	layout.PushAttribute({ 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), 3 * sizeof(f32) });
	layout.PushAttribute({ 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), 6 * sizeof(f32) });

	VertexManager vm(&vb[0], vb.size() * sizeof(f32), &ib[0], ib.size() * sizeof(u32), layout);
	m_Meshes.push_back({ std::move(vm), {} });

	//Materials (only loading diffuse textures for now)
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	LoadMaterialTexture(mat, aiTextureType_DIFFUSE, "texture_diffuse", m_Meshes[m_Meshes.size() - 1]);
}
