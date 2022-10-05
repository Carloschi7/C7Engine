#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "GL/glew.h"
#include "Shader.h"
#include "VertexManager.h"

class Entity // The actual vertices match the drawing of a perfect centered octagon between a -1.0f to 1.0f index range, if it's needed an orthographical representation of the obj, please scale it
{
public:
	Entity();
	Entity(const VertexManager& vm);
	Entity(VertexManager&& vm);
	Entity(const Entity&) = delete;
	Entity(Entity&& e) noexcept;
	~Entity();

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
	void DrawInstancedPositions(Shader& shd, uint32_t num_instances, glm::vec3* positions);
	
	void SetVertexManager(const VertexManager& vm);
	void Rotate(float fRadians, const glm::vec3& dir);
	void Translate(const glm::vec3& dir);
	void Scale(const glm::vec3& dir);
	void Scale(float fScaleFactor);
	void ResetPosition();

	bool IsIntersectedBy(const glm::vec3& pos, const glm::vec3& dir, float fRadius, float ratio_vertex_center);

	const glm::mat4& ModelMatrix() const;
	const glm::vec3& GetPosition() const { return m_Position; }
	const VertexManager* GetVertexManager() const { return m_VertexManager; }

private:
	VertexManager* m_VertexManager;
	glm::mat4 m_ModelMatrix;
	glm::vec3 m_Position;

	//For ray collision operations
	glm::vec3* m_Vertices;
};