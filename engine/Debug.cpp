#include "MainIncl.h"

void engine_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch(severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		log_message_debug("[OpenGL HIGH_SEVERITY error]: {}\n", message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		log_message_debug("[OpenGL MEDIUM_SEVERITY error]: {}\n", message);
		break;
	case GL_DEBUG_SEVERITY_LOW:
		log_message_debug("[OpenGL LOW_SEVERITY error]: {}\n", message);
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		log_message_debug("[OpenGL NOTIFICATION]: {}\n", message);
		break;
	}
}