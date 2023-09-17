#include "Entity.h"
#include <cstring>

Entity::Entity()
	: m_ModelMatrix(1.0f), m_Position(glm::vec3(0.0f)), m_VertexManager(nullptr), m_Vertices(nullptr)
{
}

Entity::Entity(const VertexManager& vm)
	:Entity()
{
	SetVertexManager(vm);
}

Entity::Entity(VertexManager&& vm)
	:Entity()
{
	SetVertexManager(vm);
	vm.ReleaseResources();
}

Entity::Entity(Entity&& e) noexcept
{
	m_ModelMatrix = e.m_ModelMatrix;
	m_Position = e.m_Position;
	m_VertexManager = std::move(e.m_VertexManager);
	m_Vertices = e.m_Vertices;
	e.m_VertexManager = nullptr;
	e.m_Vertices = nullptr;
}

Entity::~Entity()
{
	if (m_VertexManager) { ::operator delete(m_VertexManager); };
	if (m_Vertices) { ::operator delete[](m_Vertices); };
}

void Entity::SetVertexManager(const VertexManager& vm)
{
	m_VertexManager = (VertexManager*)::operator new(sizeof(VertexManager));
	memcpy(m_VertexManager, &vm, sizeof(VertexManager));
}

void Entity::Draw(Shader& shd)
{

	if (!m_VertexManager || !m_VertexManager->IsLoaded())
	{
		std::cout << GLError << "Invalid vertex manager bound! Please make sure to bind it correctly" << std::endl;
		throw std::runtime_error("Error in file Entity.cpp");
	}

	shd.Use();
	m_VertexManager->BindVertexArray();
	if (shd.IsUniformDefined("model")) 
		shd.UniformMat4f(m_ModelMatrix, "model");

	if (m_VertexManager->HasIndices())
		glDrawElements(GL_TRIANGLES, m_VertexManager->GetIndicesCount(), GL_UNSIGNED_INT, nullptr);
	else
		glDrawArrays(GL_TRIANGLES, 0, m_VertexManager->GetIndicesCount());
}

void Entity::DrawInstancedPositions(Shader& shd, u32 num_instances, glm::vec3* positions)
{
	if (!m_VertexManager || !m_VertexManager->IsLoaded())
	{
		std::cout << GLError << "Invalid vertex manager bound! Please make sure to bind it correctly" << std::endl;
		return;
	}

	m_VertexManager->BindVertexArray();

	u32 instancebuffer;
	glGenBuffers(1, &instancebuffer);

	glBindBuffer(GL_ARRAY_BUFFER, instancebuffer);
	glBufferData(GL_ARRAY_BUFFER, num_instances * sizeof(glm::vec3), positions, GL_STATIC_DRAW);

	//this GetAttribCount used in this way returns also the first attribute index unused
	//And we do not need to increment it because we destroy the buffer every time this function gets called
	glEnableVertexAttribArray(m_VertexManager->GetAttribCount());
	glVertexAttribPointer(m_VertexManager->GetAttribCount(), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glVertexAttribDivisor(m_VertexManager->GetAttribCount(), 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shd.Use();
	if (shd.IsUniformDefined("model"))
		shd.UniformMat4f(m_ModelMatrix, "model");

	if (m_VertexManager->HasIndices())
		glDrawElementsInstanced(GL_TRIANGLES, m_VertexManager->GetIndicesCount(), GL_UNSIGNED_INT, nullptr, num_instances);
	else
		glDrawArraysInstanced(GL_TRIANGLES, 0, m_VertexManager->GetIndicesCount(), num_instances);

	glDeleteBuffers(1, &instancebuffer);
}

void Entity::Rotate(float fRadians, const glm::vec3& dir)
{
	m_ModelMatrix = glm::rotate(m_ModelMatrix, fRadians, dir);
}

void Entity::Translate(const glm::vec3& dir)
{
	m_ModelMatrix = glm::translate(m_ModelMatrix, dir);
	m_Position += dir;
}

void Entity::Scale(const glm::vec3& dir)
{
	m_ModelMatrix = glm::scale(m_ModelMatrix, dir);
}

void Entity::Scale(float fScaleFactor)
{
	Scale(glm::vec3(fScaleFactor));
}

void Entity::ResetPosition()
{
	Translate(-m_Position);
	m_Position = glm::vec3(0.0f);
}

bool Entity::IsIntersectedBy(const glm::vec3& pos, const glm::vec3& dir, float fRadius, float ratio_vertex_center)
{
	if (!m_Vertices) m_Vertices = (glm::vec3*)m_VertexManager->GetRawAttribute(0, 3);
	std::vector<glm::vec3> vec3cache;

	for (int i = 0; i < m_VertexManager->GetIndicesCount(); i++)
	{
		//We translate the test ray collision points more to the center of the object, so that
		//we can then check the radial distance from the point, with a radius retrieved by the approximate
		//scale factor

		//Checking if the vertex has already been evaluated
		bool HasBeenEvaluated = false;
		for (auto v : vec3cache)
		{
			if (v == m_Vertices[i])
				HasBeenEvaluated = true;
		}

		if (HasBeenEvaluated) continue;

		vec3cache.push_back(m_Vertices[i]);
		//Getting the world space coordinates of m_Vertices[i]
		glm::vec3 vertexpos = glm::vec3(m_ModelMatrix * glm::vec4(m_Vertices[i], 1.0f));
		//Calculating the ray which goes between the vertex and the center of the shape
		glm::vec3 midray = (vertexpos - m_Position) * ratio_vertex_center;
		//Converting midray into a world space point
		glm::vec3 translated = vertexpos - midray;

		//Computing the view direction ray
		float dist = glm::length(pos - translated);
		glm::vec3 res = pos + dir * dist;

		if (res.x >= translated.x - fRadius && res.y >= translated.y - fRadius && res.z >= translated.z - fRadius &&
			res.x <= translated.x + fRadius && res.y <= translated.y + fRadius && res.z <= translated.z + fRadius)
		{
			return true;
		}
	}

	return false;
}

const glm::mat4& Entity::ModelMatrix() const
{
	return m_ModelMatrix;
}


