#include "Model.h"

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
