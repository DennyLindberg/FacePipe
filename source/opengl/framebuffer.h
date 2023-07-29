#pragma once

#include <stack>
#include "glad/glad.h"
#include "core/math.h"

class GLFramebuffers;

struct GLFramebufferScopedBind
{
	friend class GLFramebuffers;

// Only allow GLFramebuffer to create this struct by factory
private:
	GLFramebufferScopedBind() = delete;
	explicit GLFramebufferScopedBind(GLuint FBO);
	GLFramebufferScopedBind(const GLFramebufferScopedBind&) {};
	GLFramebufferScopedBind& operator=(const GLFramebufferScopedBind&) {};

public:
	~GLFramebufferScopedBind();

	// enable scoped bind to work like if (GLFramebufferScopedBind F = FBO.BindScoped())
	explicit operator bool() const
	{
		return true;
	}
};

class GLFramebuffers
{
public:
	GLFramebuffers() = delete;
	~GLFramebuffers() = delete;

	static void Initialize(GLuint Width, GLuint Height, glm::vec4 DefaultClearColor);
	static void Shutdown();

	static GLuint Create(GLuint Width, GLuint Height, glm::vec4 ClearColor);
	static void Destroy(GLuint& FBO);

	static GLFramebufferScopedBind BindScoped(GLuint FBO);
	static void Bind(GLuint FBO);
	static void BindDefault();

	static void ClearActive();
	static void ClearActiveDepth();
	static void DrawOnQuad(class GLScreenSpaceQuad& QuadMesh, GLuint FBO, float Opacity = 1.0f, glm::vec2 ScreenPos = { 0.5f, 0.5f }, glm::vec2 ScreenSize = { 1.0f, 1.0f });
	static bool GetTexture(GLuint FBO, GLuint& Texture, GLuint& Width, GLuint& Height);

	static bool IsValid(GLuint FBO);
};
