#include "framebuffer.h"
#include "application/application.h"

#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>

#include "glad/glad.h"
#include "../core/math.h"
#include <filesystem>

#include "program.h"
#include "mesh.h"

bool bIsInitialized = false;
GLuint ActiveFBO = 0;

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
}

void GLFramebuffers::Shutdown()
{
	BindDefault();
	for (RenderTarget target : RenderTargets)
	{
		DestroyRenderTarget(target);
	}
	RenderTargets.clear();
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

void GLFramebuffers::DrawOnQuad(class GLQuad& QuadMesh, GLuint FBO, float Opacity, glm::vec2 ScreenPos, glm::vec2 ScreenSize)
{
	auto& shader = App::shaders.screenspaceQuadShader;
	shader.Use();
	shader.SetUniformFloat("uOpacity", Opacity);
	shader.SetUniformVec2("uPos", ScreenPos*2.0f);
	shader.SetUniformVec2("uSize", ScreenSize);
	shader.SetUniformInt("uFlipY", 0);

	if (RenderTarget* Target = FindRenderTarget(FBO))
	{
		glBindTexture(GL_TEXTURE_2D, Target->texture);
	}
	QuadMesh.Draw();
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

void GLFramebuffers::SaveScreenshot(GLuint FBO)
{
	GLuint PreviousFBO = ActiveFBO;
	bool bSwitchFBO = ActiveFBO != FBO;
	if (bSwitchFBO)
	{
		Bind(FBO);
	}

	int width = App::settings.windowWidth;
	int height = App::settings.windowHeight;

	if (FBO != 0)
	{
		int miplevel = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &height);
	}

	int channelCount = 4;
	std::vector<GLubyte> data(channelCount * width * height);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

	GLTexture::FlipVertically(data, width, height, channelCount);
	GLTexture::SaveAsPNG(data, width, height, "screenshot.png", true);

	if (bSwitchFBO)
	{
		Bind(PreviousFBO);
	}
}

bool GLFramebuffers::IsValid(GLuint FBO)
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
