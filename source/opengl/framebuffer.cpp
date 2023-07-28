#include "framebuffer.h"

#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>

#include "glad/glad.h"
#include "../core/math.h"
#include <filesystem>

#include "program.h"
#include "mesh.h"

class GLFramebufferQuad
{
protected:
	GLuint vao = 0;
	GLuint positionBuffer = 0;
	GLuint texCoordBuffer = 0;

public:
	GLFramebufferQuad()
	{	
		// See shader initialization in GLFramebuffer::Initialize
		const GLuint positionAttribId = 0;
		const GLuint texCoordAttribId = 1;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint valuesPerPosition, valuesPerCoord;
		std::vector<float> positions, tcoords;
		GLQuad::GenerateTriangles(positions, tcoords, valuesPerPosition, valuesPerCoord);

		// Generate buffers
		glGenBuffers(1, &positionBuffer);
		glGenBuffers(1, &texCoordBuffer);

		// Load positions
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glEnableVertexAttribArray(positionAttribId);
		glVertexAttribPointer(positionAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);
		GLMeshInterface::glBufferVector(GL_ARRAY_BUFFER, positions, GL_STATIC_DRAW);

		// Load UVs
		glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
		glEnableVertexAttribArray(texCoordAttribId);
		glVertexAttribPointer(texCoordAttribId, valuesPerCoord, GL_FLOAT, false, 0, 0);
		GLMeshInterface::glBufferVector(GL_ARRAY_BUFFER, tcoords, GL_STATIC_DRAW);
	}

	~GLFramebufferQuad()
	{
		glBindVertexArray(0);
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &texCoordBuffer);
		glDeleteVertexArrays(1, &vao);
	}

	void Draw()
	{
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};

bool bIsInitialized = false;
GLuint ActiveFBO = 0;
GLProgram* QuadShader = nullptr;
GLFramebufferQuad* QuadMesh = nullptr;

struct RenderTarget
{
	GLuint width = 0;
	GLuint height = 0;
	GLuint fbo = 0;
	GLuint rbo = 0;
	GLuint texture = 0;
	glm::vec4 clearColor = glm::vec4(0.0f);

	inline bool operator==(GLuint otherfbo) const { return fbo == otherfbo; }
};

std::stack<GLuint> ScopedBinds;
std::vector<RenderTarget> RenderTargets;

RenderTarget* FindRenderTarget(GLuint FBO)
{
	for (size_t i=0; i<RenderTargets.size(); i++)
	{
		if (RenderTargets[i] == FBO)
			return &RenderTargets[i];
	}

	return nullptr;
}

void DestroyRenderTarget(RenderTarget& Target)
{
	glDeleteTextures(1, &Target.texture);
	glDeleteRenderbuffers(1, &Target.rbo);
	glDeleteFramebuffers(1, &Target.fbo);
}

bool IsValidBuffer(GLuint FBO)
{
	return FindRenderTarget(FBO) != nullptr;
}

GLFramebufferScopedBind::GLFramebufferScopedBind(GLuint FBO)
{
	ScopedBinds.push(FBO);
	GLFramebuffers::Bind(FBO);
}

GLFramebufferScopedBind::~GLFramebufferScopedBind()
{
	ScopedBinds.pop();
	if (!ScopedBinds.empty() && IsValidBuffer(ScopedBinds.top()))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ScopedBinds.top());
		ActiveFBO = ScopedBinds.top();
	}
	else
		GLFramebuffers::BindDefault();
}

void GLFramebuffers::Initialize(GLuint Width, GLuint Height, glm::vec4 DefaultClearColor)
{
	RenderTarget DefaultRenderTarget;
	DefaultRenderTarget.width = Width;
	DefaultRenderTarget.height = Height;
	DefaultRenderTarget.clearColor = DefaultClearColor;
	RenderTargets.push_back(DefaultRenderTarget);

	if (!QuadShader)
	{
		QuadShader = new GLProgram();

		QuadShader->LoadVertexShader(R"(
			#version 330

			layout(location = 0) in vec3 aPos;
			layout(location = 1) in vec4 aTexCoords;

			uniform vec2 uPos;
			uniform vec2 uSize;

			out vec2 TexCoords;

			void main()
			{
				// Scale based on center
				gl_Position.x = aPos.x*uSize.x + uPos.x - 1.0f;
				gl_Position.y = aPos.y*uSize.y - uPos.y + 1.0f;
				gl_Position.z = aPos.z;
				gl_Position.w = 1.0f;

				TexCoords = vec2(aTexCoords.x, 1.0f-aTexCoords.y);
			}
		)");

		QuadShader->LoadFragmentShader(R"(
			#version 330

			in vec2 TexCoords;

			uniform sampler2D quadTexture;
			uniform float uOpacity;

			out vec4 FragColor; 

			void main()
			{
				FragColor = texture(quadTexture, TexCoords);
				FragColor.a *= uOpacity;
			}
		)");

		QuadShader->CompileAndLink();
	}

	if (!QuadMesh)
	{
		QuadMesh = new GLFramebufferQuad();
	}
}

void GLFramebuffers::Shutdown()
{
	BindDefault();
	for (RenderTarget target : RenderTargets)
	{
		DestroyRenderTarget(target);
	}
	RenderTargets.clear();

	if (QuadShader)
	{
		delete QuadShader;
		QuadShader = nullptr;
	}

	if (QuadMesh)
	{
		delete QuadMesh;
		QuadMesh = nullptr;
	}
}

GLuint GLFramebuffers::Create(GLuint Width, GLuint Height, glm::vec4 ClearColor)
{
	RenderTarget NewTarget;
	NewTarget.width = Width;
	NewTarget.height = Height;
	NewTarget.clearColor = ClearColor;
	
	// Bind new framebuffer
	glGenFramebuffers(1, &NewTarget.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, NewTarget.fbo);

	// Create new texture to associate with buffer
	glGenTextures(1, &NewTarget.texture);
	glBindTexture(GL_TEXTURE_2D, NewTarget.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Attach texture to new framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, NewTarget.texture, 0);

	// Enable depth and stencil for rendering
	glGenRenderbuffers(1, &NewTarget.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, NewTarget.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Attach depth and stencil to framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, NewTarget.rbo);

	if (!GLFramebuffers::IsValid(NewTarget.fbo))
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		BindDefault();
		return 0;
	}
	else
	{
		BindDefault();

		// Store framebuffer and texture in array for future reference
		RenderTargets.push_back(NewTarget);
		return NewTarget.fbo;
	}
}

void GLFramebuffers::Destroy(GLuint& FBO)
{
	auto it = std::find(RenderTargets.begin(), RenderTargets.end(), FBO);
	if (it != RenderTargets.end())
	{
		DestroyRenderTarget(*it);
		it = RenderTargets.erase(it);
	}

	FBO = 0;
}

GLFramebufferScopedBind GLFramebuffers::BindScoped(GLuint FBO)
{
	return GLFramebufferScopedBind(FBO);
}

void GLFramebuffers::Bind(GLuint FBO)
{
	if (RenderTarget* Target = FindRenderTarget(FBO))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // Same as GL_READ_FRAMEBUFFER + GL_DRAW_FRAMEBUFFER at the same time
		glViewport(0, 0, Target->width, Target->height);
		ActiveFBO = FBO;
	}
}

void GLFramebuffers::BindDefault()
{
	Bind(0);
}

void GLFramebuffers::ClearActive()
{
	if (RenderTarget* Target = FindRenderTarget(ActiveFBO))
	{
		const auto& c = Target->clearColor;
		glClearColor(c.r, c.g, c.b, c.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}

void GLFramebuffers::ClearActiveDepth()
{
	if (RenderTarget* Target = FindRenderTarget(ActiveFBO))
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}
}

void GLFramebuffers::DrawAsQuad(GLuint FBO, float Opacity, glm::vec2 ScreenPos, glm::vec2 ScreenSize)
{
	QuadShader->Use();
	QuadShader->SetUniformFloat("uOpacity", Opacity);
	QuadShader->SetUniformVec2("uPos", ScreenPos*2.0f);
	QuadShader->SetUniformVec2("uSize", ScreenSize);
	if (RenderTarget* Target = FindRenderTarget(FBO))
	{
		glBindTexture(GL_TEXTURE_2D, Target->texture);
	}
	QuadMesh->Draw();
}

bool GLFramebuffers::GetTexture(GLuint FBO, GLuint& Texture, GLuint& Width, GLuint& Height)
{
	if (RenderTarget* Target = FindRenderTarget(FBO))
	{
		Texture = Target->texture;
		Width = Target->width;
		Height = Target->height;
		return true;
	}

	return false;
}

bool GLFramebuffers::IsValid(GLuint FBO)
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
