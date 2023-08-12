#include "mesh.h"
#include "application/application.h"

#pragma warning(push,0)
#include "tiny_obj_loader.h"
#pragma warning(pop)

#include <string>
#include <iostream>

void InitializeVAOAndBuffers(GLuint& vao, std::function<void()> f)
{
	if (vao != 0) 
		return;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	f();
}

void ShutdownVAOAndBuffers(GLuint& vao, std::function<void()> f)
{
	if (vao == 0) 
		return;

	f();

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
	vao = 0;
}

void GLGrid::Draw(GLQuad& mesh, const glm::mat4& mvp, const glm::fvec3& planeUp, const glm::fvec3& planeSide)
{
	glm::fvec3 forward = glm::cross(planeSide, planeUp);
	const glm::mat4 planeRotationMatrix (
		glm::fvec4(planeSide.x, planeSide.y, planeSide.z, 0.0f), 
		glm::fvec4(forward.x, forward.y, forward.z, 0.0f),
		glm::fvec4(planeUp.x, planeUp.y, planeUp.z, 0.0f),
		glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	glm::mat4 mvpOffset = mvp * planeRotationMatrix;

	auto& shader = App::shaders.gridShader;
	shader.Use();
	shader.SetUniformFloat("gridSpacing", gridSpacing);
	shader.SetUniformFloat("size", size);
	shader.SetUniformFloat("opacity", opacity);
	shader.SetUniformMat4("mvp", mvpOffset);
	mesh.Draw();
}

void GLTriangleMesh::Initialize()
{
	InitializeVAOAndBuffers(vao, [this]() {
		glGenBuffers(1, &positionBuffer);
		glGenBuffers(1, &normalBuffer);
		glGenBuffers(1, &colorBuffer);
		glGenBuffers(1, &texCoordBuffer);
		glGenBuffers(1, &indexBuffer);

		// Position buffer
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glEnableVertexAttribArray(ShaderManager::positionAttribId);
		glVertexAttribPointer(ShaderManager::positionAttribId, 3, GL_FLOAT, false, 0, 0);

		// Normal buffer
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glEnableVertexAttribArray(ShaderManager::normalAttribId);
		glVertexAttribPointer(ShaderManager::normalAttribId, 3, GL_FLOAT, false, 0, 0);

		// Color buffer
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glEnableVertexAttribArray(ShaderManager::colorAttribId);
		glVertexAttribPointer(ShaderManager::colorAttribId, 4, GL_FLOAT, false, 0, 0);

		// TexCoord buffer
		glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
		glEnableVertexAttribArray(ShaderManager::texCoordAttribId);
		glVertexAttribPointer(ShaderManager::texCoordAttribId, 4, GL_FLOAT, false, 0, 0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	});
}

void GLTriangleMesh::Destroy()
{
	ShutdownVAOAndBuffers(vao, [this](){
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &normalBuffer);
		glDeleteBuffers(1, &colorBuffer);
		glDeleteBuffers(1, &texCoordBuffer);
		glDeleteBuffers(1, &indexBuffer);
	});
}

void GLTriangleMesh::Clear()
{
	positions.clear();
	normals.clear();
	colors.clear();
	texCoords.clear();
	indices.clear();

	positions.shrink_to_fit();
	normals.shrink_to_fit();
	colors.shrink_to_fit();
	texCoords.shrink_to_fit();
	indices.shrink_to_fit();

	SendToGPU();
}

void GLTriangleMesh::SendToGPU()
{
	if (!vao) return;

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferVector(GL_ARRAY_BUFFER, positions, usage);

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferVector(GL_ARRAY_BUFFER, normals, usage);

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferVector(GL_ARRAY_BUFFER, colors, usage);

	glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
	glBufferVector(GL_ARRAY_BUFFER, texCoords, usage);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferVector(GL_ELEMENT_ARRAY_BUFFER, indices, usage);
}

void GLTriangleMesh::Draw(GLenum drawMode)
{
	if (vao && positions.size() > 0 && indices.size() > 0)
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(drawMode, GLsizei(indices.size()), GL_UNSIGNED_INT, (void*)0);
	}
}

void GLTriangleMesh::AddVertex(glm::fvec3 pos, glm::fvec4 color, glm::fvec4 texcoord)
{
	positions.push_back(std::move(pos));
	normals.push_back(glm::fvec3{ 0.0f });
	colors.push_back(std::move(color));
	texCoords.push_back(std::move(texcoord));
}

void GLTriangleMesh::AddVertex(glm::fvec3 pos, glm::fvec3 normal, glm::fvec4 color, glm::fvec4 texcoord)
{
	positions.push_back(std::move(pos));
	normals.push_back(std::move(normal));
	colors.push_back(std::move(color));
	texCoords.push_back(std::move(texcoord));
}

void GLTriangleMesh::DefineNewTriangle(unsigned int index1, unsigned int index2, unsigned int index3)
{
	indices.push_back(index1);
	indices.push_back(index2);
	indices.push_back(index3);
}

void GLTriangleMesh::AppendMesh(const GLTriangleMesh& other)
{
	// These two values are used to re-calculate the indices of the other mesh
	// after it has been appended.
	int newIndicesOffset = int(positions.size());
	int newIndicesStart = int(indices.size());

	positions.reserve(positions.size() + other.positions.size());
	normals.reserve(normals.size() + other.normals.size());
	colors.reserve(colors.size() + other.colors.size());
	texCoords.reserve(texCoords.size() + other.texCoords.size());
	indices.reserve(indices.size() + other.indices.size());

	positions.insert(positions.end(), other.positions.begin(), other.positions.end());
	normals.insert(normals.end(), other.normals.begin(), other.normals.end());
	colors.insert(colors.end(), other.colors.begin(), other.colors.end());
	texCoords.insert(texCoords.end(), other.texCoords.begin(), other.texCoords.end());
	indices.insert(indices.end(), other.indices.begin(), other.indices.end());

	for (int i = newIndicesStart; i < indices.size(); ++i)
	{
		indices[i] += newIndicesOffset;
	}
}

void GLTriangleMesh::AppendMeshTransformed(const GLTriangleMesh & other, glm::mat4 transform)
{
	int newPositionsStart = int(positions.size());
	AppendMesh(other);
	int newPositionsEnd = int(positions.size() - 1);
	ApplyMatrix(transform, newPositionsStart, newPositionsEnd);
}

void GLTriangleMesh::ApplyMatrix(glm::mat4 transform, int firstIndex, int lastIndex)
{
	firstIndex = (firstIndex < 0)? 0 : firstIndex;
	lastIndex = (lastIndex >= positions.size()) ? int(positions.size() - 1) : lastIndex;

	for (int i=firstIndex; i<=lastIndex; i++)
	{
		positions[i] = transform * glm::fvec4(positions[i], 1.0f);
		normals[i] = transform * glm::fvec4(normals[i], 0.0f);
	}
}

void GLTriangleMesh::ApplyMatrix(glm::mat4 transform)
{
	ApplyMatrix(transform, 0, int(positions.size() - 1));
}

void GLTriangleMesh::SetUsage(GLenum newUsage)
{
	if (usage != newUsage)
	{
		usage = newUsage;
		SendToGPU();
	}
}

void GLTriangleMesh::SetColors(const glm::fvec4& color)
{
	for (auto& c : colors)
	{
		c = color;
	}
}

void GLLine::Initialize()
{
	InitializeVAOAndBuffers(vao, [this]() {
		// Generate buffers
		glGenBuffers(1, &positionBuffer);
		glGenBuffers(1, &colorBuffer);

		// Load positions
		int valuesPerPosition = 3; // glm::fvec3 has 3 floats
		glEnableVertexAttribArray(ShaderManager::positionAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(ShaderManager::positionAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Load colors
		valuesPerPosition = 4; // glm::fvec4 has 4 floats
		glEnableVertexAttribArray(ShaderManager::colorAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glVertexAttribPointer(ShaderManager::colorAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);
	});
}

void GLLine::Destroy()
{
	ShutdownVAOAndBuffers(vao, [this](){
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &colorBuffer);
	});
}

void GLLine::AddLine(glm::fvec3 start, glm::fvec3 end, glm::fvec4 color)
{
	lineSegments.push_back(LineSegment{ std::move(start), std::move(end) });
	colors.push_back(color);
	colors.push_back(std::move(color));
}

void GLLine::Clear()
{
	lineSegments.clear();
	lineSegments.shrink_to_fit();
	colors.clear();
	colors.shrink_to_fit();
	SendToGPU();
}

void GLLine::SendToGPU()
{
	if (!vao) return;

	glBindVertexArray(vao);

	// Positions
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferVector(GL_ARRAY_BUFFER, lineSegments, usage);

	// Colors
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferVector(GL_ARRAY_BUFFER, colors, usage);
}

void GLLine::Draw()
{
	if (vao && lineSegments.size() > 0)
	{
		glBindVertexArray(vao);
		glDrawArrays(GL_LINES, 0, GLsizei(lineSegments.size()) * 2 * 3);
	}
}


void GLLineStrips::Initialize()
{
	InitializeVAOAndBuffers(vao, [this]() {
		// Generate buffers
		glGenBuffers(1, &positionBuffer);
		glGenBuffers(1, &indexBuffer);
	
		// Load positions
		int valuesPerPosition = 3; // glm::fvec3 has 3 floats
		glEnableVertexAttribArray(ShaderManager::positionAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(ShaderManager::positionAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		SendToGPU();
	});
}

void GLLineStrips::Destroy()
{
	ShutdownVAOAndBuffers(vao, [this](){
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &indexBuffer);
	});
}

void GLLineStrips::AddLineStrip(const std::vector<glm::fvec3>& points)
{
	if (points.size() == 0)
	{
		return; // because there is no data to add
	}

	size_t newLineStart = lineStrips.size();
	size_t newIndicesStart = indices.size();

	numStrips++;
	lineStrips.resize(lineStrips.size() + points.size());
	indices.resize(indices.size() + points.size() + 1); // include restart_index at the end
	for (size_t i = 0; i < points.size(); ++i)
	{
		lineStrips[newLineStart + i] = points[i];
		indices[newIndicesStart + i] = static_cast<unsigned int>(newLineStart + i);
	}

	indices[indices.size()-1] = GLMesh::RESTART_INDEX;
}

void GLLineStrips::Clear()
{
	lineStrips.clear();
	indices.clear();

	lineStrips.shrink_to_fit();
	indices.shrink_to_fit();

	SendToGPU();
}

void GLLineStrips::SendToGPU()
{
	if (!vao) return;

	glBindVertexArray(vao);

	// Positions
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferVector(GL_ARRAY_BUFFER, lineStrips, usage);

	// Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferVector(GL_ELEMENT_ARRAY_BUFFER, indices, usage);
}

void GLLineStrips::Draw()
{
	if (!vao || lineStrips.size() == 0 || indices.size() == 0)
	{
		return; // because there is no data to render
	}

	/*
		See these references for primitive restart
			https://www.khronos.org/opengl/wiki/Vertex_Rendering#Common
			https://gist.github.com/roxlu/51fc685b0303ee55c05b3ad96992f3ec
	*/
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(GLMesh::RESTART_INDEX);

	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(GL_LINE_STRIP, GLsizei(indices.size()), GL_UNSIGNED_INT, (GLvoid*)0);
	}

	glDisable(GL_PRIMITIVE_RESTART);
}

void GLBezierStrips::Initialize()
{
	InitializeVAOAndBuffers(vao, [this]() {
		const GLuint bezierPositionAttribId = 0;
		const GLuint bezierNormalAttribId = 1;
		const GLuint bezierTangentAttribId = 2;
		const GLuint bezierTexcoordAttribId = 3;
		const GLuint bezierWidthAttribId = 4;
		const GLuint bezierThicknessAttribId = 5;
		const GLuint bezierShapeAttribId = 6;
		const GLuint bezierSubdivAttribId = 7;

		glBindVertexArray(vao);

		// Generate buffers
		glGenBuffers(1, &positionBuffer);
		glGenBuffers(1, &normalBuffer);
		glGenBuffers(1, &tangentBuffer);
		glGenBuffers(1, &texcoordBuffer);
		glGenBuffers(1, &widthBuffer);
		glGenBuffers(1, &thicknessBuffer);
		glGenBuffers(1, &shapeBuffer);
		glGenBuffers(1, &subdivisionsBuffer);

		glGenBuffers(1, &indexBuffer);

		// Define positions
		int valuesPerPosition = 3; // glm::fvec3 has 3 floats
		glEnableVertexAttribArray(bezierPositionAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(bezierPositionAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Define normals
		valuesPerPosition = 3;
		glEnableVertexAttribArray(bezierNormalAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glVertexAttribPointer(bezierNormalAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Define tangents
		valuesPerPosition = 3;
		glEnableVertexAttribArray(bezierTangentAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
		glVertexAttribPointer(bezierTangentAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Define texcoords
		valuesPerPosition = 3; // glm::fvec3
		glEnableVertexAttribArray(bezierTexcoordAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
		glVertexAttribPointer(bezierTexcoordAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Define widths
		valuesPerPosition = 1; // float
		glEnableVertexAttribArray(bezierWidthAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, widthBuffer);
		glVertexAttribPointer(bezierWidthAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Define thickness
		valuesPerPosition = 1; // float
		glEnableVertexAttribArray(bezierThicknessAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, thicknessBuffer);
		glVertexAttribPointer(bezierThicknessAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);

		// Define shapes
		valuesPerPosition = 1; // int
		glEnableVertexAttribArray(bezierShapeAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, shapeBuffer);
		glVertexAttribIPointer(bezierShapeAttribId, valuesPerPosition, GL_INT, 0, 0);

		// Define segment subdivisions
		valuesPerPosition = 1; // int
		glEnableVertexAttribArray(bezierSubdivAttribId);
		glBindBuffer(GL_ARRAY_BUFFER, subdivisionsBuffer);
		glVertexAttribIPointer(bezierSubdivAttribId, valuesPerPosition, GL_INT, 0, 0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		SendToGPU();
	});
}

void GLBezierStrips::Destroy()
{
	ShutdownVAOAndBuffers(vao, [this](){
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &normalBuffer);
		glDeleteBuffers(1, &tangentBuffer);
		glDeleteBuffers(1, &texcoordBuffer);
		glDeleteBuffers(1, &widthBuffer);
		glDeleteBuffers(1, &thicknessBuffer);
		glDeleteBuffers(1, &shapeBuffer);
		glDeleteBuffers(1, &subdivisionsBuffer);

		glDeleteBuffers(1, &indexBuffer);
	});
}

bool GLBezierStrips::AddBezierStrip(
	const std::vector<glm::fvec3>& points,
	const std::vector<glm::fvec3>& normals,
	const std::vector<glm::fvec3>& tangents,
	const std::vector<glm::fvec3>& texcoords,
	const std::vector<float>& widths,
	const std::vector<float>& thickness,
	const std::vector<int>& shapes,
	const std::vector<int>& subdivisions
)
{
	if (points.size() == 0)
	{
		return false; // because there is no data to add
	}

	if (normals.size() != points.size() || 
		tangents.size() != points.size() || 
		texcoords.size() != points.size() || 
		widths.size() != points.size() ||
		thickness.size() != points.size() ||
		shapes.size() != points.size() ||
		subdivisions.size() != points.size())
	{
		return false; // because of size mismatch
	}

	size_t newLineStart = controlPoints.size();
	size_t newIndicesStart = indices.size();

	numStrips++;

	size_t newVectorSize = controlPoints.size() + points.size();
	controlPoints.resize(newVectorSize);
	controlNormals.resize(newVectorSize);
	controlTangents.resize(newVectorSize);
	controlTexcoords.resize(newVectorSize);
	controlWidths.resize(newVectorSize);
	controlThickness.resize(newVectorSize);
	controlShapes.resize(newVectorSize);
	controlSubdivisions.resize(newVectorSize);

	indices.resize(indices.size() + points.size() + 1); // include restart_index at the end
	for (size_t i = 0; i < points.size(); ++i)
	{
		controlPoints[newLineStart + i] = points[i];
		controlNormals[newLineStart + i] = normals[i];
		controlTangents[newLineStart + i] = tangents[i];
		controlTexcoords[newLineStart + i] = texcoords[i];
		controlWidths[newLineStart + i] = widths[i];
		controlThickness[newLineStart + i] = thickness[i];
		controlShapes[newLineStart + i] = shapes[i];
		controlSubdivisions[newLineStart + i] = subdivisions[i];

		indices[newIndicesStart + i] = static_cast<unsigned int>(newLineStart + i);
	}

	indices[indices.size() - 1] = GLMesh::RESTART_INDEX;
	return true;
}

void GLBezierStrips::Clear()
{
	controlPoints.clear();
	controlNormals.clear();
	controlTangents.clear();
	controlTexcoords.clear();
	controlWidths.clear();
	controlThickness.clear();
	controlShapes.clear();
	controlSubdivisions.clear();

	indices.clear();

	controlPoints.shrink_to_fit();
	controlNormals.shrink_to_fit();
	controlTangents.shrink_to_fit();
	controlTexcoords.shrink_to_fit();
	controlWidths.shrink_to_fit();
	controlThickness.shrink_to_fit();
	controlShapes.shrink_to_fit();
	controlSubdivisions.shrink_to_fit();

	indices.shrink_to_fit();

	SendToGPU();
}

void GLBezierStrips::SendToGPU()
{
	if (!vao) return;

	glBindVertexArray(vao);

	// Positions
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlPoints, usage);

	// Normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlNormals, usage);

	// Tangents
	glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlTangents, usage);

	// Texcoords
	glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlTexcoords, usage);

	// Widths
	glBindBuffer(GL_ARRAY_BUFFER, widthBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlWidths, usage);

	// Thickness
	glBindBuffer(GL_ARRAY_BUFFER, thicknessBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlThickness, usage);

	// Shapes
	glBindBuffer(GL_ARRAY_BUFFER, shapeBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlShapes, usage);

	// Subdivisions
	glBindBuffer(GL_ARRAY_BUFFER, subdivisionsBuffer);
	glBufferVector(GL_ARRAY_BUFFER, controlSubdivisions, usage);

	// Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferVector(GL_ELEMENT_ARRAY_BUFFER, indices, usage);
}

void GLBezierStrips::Draw()
{
	if (!vao || controlPoints.size() == 0 || indices.size() == 0)
	{
		return; // because there is no data to render
	}

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(GLMesh::RESTART_INDEX);

	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(GL_LINE_STRIP, GLsizei(indices.size()), GL_UNSIGNED_INT, (GLvoid*)0);
	}

	glDisable(GL_PRIMITIVE_RESTART);
}

void GLQuad::Initialize()
{
	InitializeVAOAndBuffers(vao, [this]() {
		glGenBuffers(1, &positionBuffer);
		glGenBuffers(1, &texCoordBuffer);

		float windowWidth = float(App::settings.windowWidth);
		float windowHeight = float(App::settings.windowHeight);

		// TODO: Repair this
	
		// GL coordinate system has the origin in the middle of the screen and
		// ranges between -1.0 to 1.0. UI coordinates must be remapped.
		//float relativeWidth = properties.width / windowWidth;
		//float relativeHeight = properties.height / windowHeight;
		//float relativeX = properties.positionX / windowWidth;
		//float relativeY = properties.positionY / windowHeight;

		float left = -1.0f; //-1.0f + 2.0f * relativeX;
		float right = 1.0f; //-1.0f + 2.0f * (relativeX + relativeWidth);
		float top = 1.0f; //1.0f - 2.0f * relativeY;
		float bottom = -1.0f; //1.0f - 2.0f * (relativeY + relativeHeight);

		GLuint valuesPerPosition = 3;
		std::vector<float> positions = {
			// Triangle 1
			left, top, 0.0f,
			left, bottom, 0.0f,
			right, bottom, 0.0f,

			// Triangle 2
			right, bottom, 0.0f,
			right, top, 0.0f,
			left, top, 0.0f,
		};

		// UVs work top to bottom, V is reversed to get image in correct orientation
		GLuint valuesPerCoord = 4;
		std::vector<float> tcoords = {
			0.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,

			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f
		};

		// Load positions
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glEnableVertexAttribArray(ShaderManager::positionAttribId);
		glVertexAttribPointer(ShaderManager::positionAttribId, valuesPerPosition, GL_FLOAT, false, 0, 0);
		glBufferVector(GL_ARRAY_BUFFER, positions, usage);

		// Load UVs
		glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
		glEnableVertexAttribArray(ShaderManager::texCoordAttribId);
		glVertexAttribPointer(ShaderManager::texCoordAttribId, valuesPerCoord, GL_FLOAT, false, 0, 0);
		glBufferVector(GL_ARRAY_BUFFER, tcoords, usage);
	});
}

void GLQuad::Destroy()
{
	ShutdownVAOAndBuffers(vao, [this](){
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &texCoordBuffer);
	});
}

void GLQuad::Draw()
{
	if (!vao) return;

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

namespace GLMesh
{
	bool LoadOBJ(std::filesystem::path FilePath, GLTriangleMesh& OutMesh)
	{
		OutMesh.Clear();

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;

		std::string inputfile{ FilePath.string() };
		bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str(), nullptr, true);

		if (!loaded || shapes.size() == 0)
		{
			if (!err.empty())
			{
				std::cerr << err << std::endl;
			}
			return false;
		}

		// Copy indices
		auto& MainMesh = shapes[0].mesh;
		OutMesh.indices.resize(MainMesh.indices.size());
		for (size_t i = 0; i < MainMesh.indices.size(); i++)
		{
			OutMesh.indices[i] = MainMesh.indices[i].vertex_index;
		}

		// Resize OutMesh data to match the obj contents
		size_t vertexcount = attrib.vertices.size() / 3;
		OutMesh.positions.resize(vertexcount);
		OutMesh.normals.resize(vertexcount);
		OutMesh.colors.resize(vertexcount);
		OutMesh.texCoords.resize(vertexcount);

		// Copy all data (could probably use memcpy, but meh)
		for (size_t i = 0; i < vertexcount; i++)
		{
			size_t last_vertex_index = (3 * i + 2);
			if (last_vertex_index < attrib.vertices.size())
			{
				OutMesh.positions[i] = glm::fvec3{ 
					attrib.vertices[3*i+0], 
					attrib.vertices[3*i+1], 
					attrib.vertices[3*i+2]
				};
			}

			size_t last_normal_index = (3 * i + 2);
			if (last_normal_index < attrib.normals.size())
			{
				OutMesh.normals[i] = glm::fvec3{
					attrib.normals[3 * i + 0],
					attrib.normals[3 * i + 1],
					attrib.normals[3 * i + 2]
				};
			}

			size_t last_texcoord_index = (2 * i + 2);
			if (last_texcoord_index < attrib.texcoords.size())
			{
				OutMesh.texCoords[i] = glm::fvec4{
					attrib.texcoords[2 * i + 0],
					attrib.texcoords[2 * i + 1],
					0.0f,
					0.0f
				};
			}
		}

		OutMesh.SendToGPU();

		return true;
	}

	bool LoadPLY(std::filesystem::path FilePath, GLTriangleMesh& OutMesh)
	{
		enum class ElementType { x, y, z, nx, ny, nz, s, t, unknown };
		auto ToType = [](const std::string& token) -> ElementType {
			if (token == "x") return ElementType::x;
			if (token == "y") return ElementType::y;
			if (token == "z") return ElementType::z;
			if (token == "nx") return ElementType::nx;
			if (token == "ny") return ElementType::ny;
			if (token == "nz") return ElementType::nz;
			if (token == "s") return ElementType::s;
			if (token == "t") return ElementType::t;

			return ElementType::unknown;
		};

		OutMesh.Clear();

		if (!std::filesystem::exists(FilePath)) 
			return false;

		std::ifstream fileStream(FilePath);
		if (!fileStream || !fileStream.is_open())
			return false;

		std::string line;
		std::getline(fileStream, line);
		if (line != "ply")
		{
			std::cout << "LoadPLY::Error: Not a valid ply ASCII file" << std::endl;
			return false;
		}

		std::vector<ElementType> ElementTypeOrder;
		bool bFoundEndOfHeader = false;
		int VertexCount = -1;
		int FaceCount = -1;
		while (std::getline(fileStream, line))
		{
			std::istringstream ss(line);
			std::string token;
			if (!std::getline(ss, token, ' '))
				continue;

			if (token == "element")
			{
				std::string type, value;
				if (std::getline(ss, type, ' ') && std::getline(ss, value))
				{
					try
					{
						if (type == "vertex")
							VertexCount = std::stoi(value);
						else if (type == "face")
							FaceCount = std::stoi(value);
					}
					catch (...) 
					{
						std::cout << "LoadPLY::Error: Non-integer value for vertex or face count" << std::endl;
						return false;
					}
				}
				else
				{
					std::cout << "LoadPLY::Error: Bad element definition" << std::endl;
					return false;
				}				
			}
			else if (token == "property")
			{
				std::istringstream ss(line);
				std::string token;
				while (std::getline(ss, token, ' '));
				ElementTypeOrder.push_back(ToType(token));
			}
			else if (token == "end_header")
			{
				bFoundEndOfHeader = true;
				break;
			}
		}

		if (VertexCount < 0 || FaceCount < 0 || !bFoundEndOfHeader)
		{
			std::cout << "Failed to parse header" << std::endl;
			return false;
		}

		//std::cout << "LoadPLY::VertexCount: " << VertexCount << std::endl;
		//std::cout << "LoadPLY::FaceCount: " << FaceCount << std::endl;
		//std::cout << "LoadPLY::Order: " << ElementTypeOrder.size() << std::endl;

		struct VertexEntry {
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			float nx = 0.0f;
			float ny = 0.0f;
			float nz = 0.0f;
			float s = 0.0f;
			float t = 0.0f;
		};

		OutMesh.positions.reserve(VertexCount);
		OutMesh.normals.reserve(VertexCount);
		OutMesh.colors.reserve(VertexCount);
		OutMesh.texCoords.reserve(VertexCount);
		while (VertexCount > 0 && std::getline(fileStream, line))
		{
			VertexCount--;
			
			VertexEntry Entry;

			std::istringstream ss(line);
			std::string token;
			for (size_t i=0; i<ElementTypeOrder.size(); i++)
			{
				if (!std::getline(ss, token, ' ') || ElementTypeOrder[i] == ElementType::unknown)
					break;
				
				float* Value = &Entry.x + ((int)ElementTypeOrder[i]);
				try { *Value = std::stof(token); } catch (...) {}
			}

			//std::cout << "LoadPLY::Pos: " << Entry.x << " " << Entry.y << " " << Entry.z << std::endl;
			//std::cout << "LoadPLY::Normal: " << Entry.nx << " " << Entry.ny << " " << Entry.nz << std::endl;
			//std::cout << "LoadPLY::UV: " << Entry.s << " " << Entry.t << std::endl;

			OutMesh.positions.push_back(glm::fvec3(Entry.x, Entry.y, Entry.z));
			OutMesh.normals.push_back(glm::fvec3(Entry.nx, Entry.ny, Entry.nz));
			OutMesh.colors.push_back(glm::fvec4(1.0f));
			OutMesh.texCoords.push_back(glm::fvec4(Entry.s, Entry.t, 1.0f, 1.0f));
		}

		OutMesh.indices.reserve(FaceCount*6); // we don't know if we get triangles or quads, reserve for triangulation of quads
		while (FaceCount > 0 && std::getline(fileStream, line))
		{
			FaceCount--;

			std::vector<int> FaceIndices;
			FaceIndices.reserve(6); // 5 or 6 entries every time

			std::istringstream ss(line);
			std::string token;
			while (std::getline(ss, token, ' '))
			{
				try { FaceIndices.push_back(std::stoi(token)); } catch (...) {}
			}

			//std::cout << "LoadPLY::Parsing face, got: " << FaceIndices.size() << std::endl;

			if (FaceIndices.size() >= 4 && FaceIndices[0] >= 3) // triangle
			{
				OutMesh.indices.push_back(FaceIndices[1]);
				OutMesh.indices.push_back(FaceIndices[2]);
				OutMesh.indices.push_back(FaceIndices[3]);

				//std::cout << "LoadPLY::Triangle1: " << FaceIndices[1] << " " << FaceIndices[2] << " " << FaceIndices[3] << std::endl;

				if (FaceIndices.size() >= 5 && FaceIndices[0] >= 4) // quad or more (we don't handle the remaining scenario)
				{
					OutMesh.indices.push_back(FaceIndices[3]);
					OutMesh.indices.push_back(FaceIndices[4]);
					OutMesh.indices.push_back(FaceIndices[1]);

					//std::cout << "LoadPLY::Triangle2: " << FaceIndices[3] << " " << FaceIndices[4] << " " << FaceIndices[1] << std::endl;
				}
			}
		}

		OutMesh.SendToGPU();

		return true;
	}

	bool LoadLinesFromMeshNormals(GLTriangleMesh& OutMesh, GLLine& OutLines, float lineLength)
	{
		OutLines.Clear();

		glm::vec4 lineColor(0.0f, 1.0f, 0.0f, 1.0f);
		for (size_t i=0; i<OutMesh.normals.size(); i++)
		{
			glm::fvec3& pos = OutMesh.positions[i];
			glm::fvec3& normal = OutMesh.normals[i];

			OutLines.AddLine(pos, pos + normal*lineLength, lineColor);
		}

		OutLines.SendToGPU();

		return true;
	}

	bool LoadCurves(std::filesystem::path FilePath, GLBezierStrips& OutStrips)
	{
		OutStrips.Clear();

		std::ifstream ifs(FilePath);
		if (!ifs) 
		{
			printf("\r\nCould not open %ws", FilePath.c_str());
			return false;
		}

		// Target vectors to fill up
		std::vector<glm::fvec3> points;
		std::vector<glm::fvec3> normals;
		std::vector<glm::fvec3> tangents;
		std::vector<glm::fvec3> texcoords;
		std::vector<float> widths;
		std::vector<float> thickness;
		std::vector<int> shapes;
		std::vector<int> subdivisions;

		// Temporaries during parsing
		int lineid = 0;
		int ivalue = 0;
		float fvalue = 0.0f;
		glm::fvec3 cachevec3{ 0.0f };
		std::vector<glm::fvec3>* targetvec3 = nullptr;
		std::vector<float>* targetfloat = nullptr;
		std::vector<int>* targetint = nullptr;

		// Parse loop
		int num_loaded_curves = 0;
		std::string line;
		while (std::getline(ifs, line))
		{
			std::istringstream string_of_values(line);
			
			// Determine vector to write to based on line id
			switch (lineid)
			{
			case 0: { targetvec3 = &points; break; }
			case 1: { targetvec3 = &normals; break; }
			case 2: { targetvec3 = &tangents; break; }
			case 3: { targetvec3 = &texcoords; break; }
			case 4: { targetfloat = &widths; break; }
			case 5: { targetfloat = &thickness; break; }
			case 6: { targetint = &shapes; break; }
			case 7: { targetint = &subdivisions; break; }
			default: {}
			}

			if (lineid >= 0 && lineid <= 3)
			{
				int i = 0;
				while (string_of_values >> fvalue)
				{
					cachevec3[i % 3] = fvalue;
					if (i % 3 == 2)
					{
						targetvec3->push_back(cachevec3);
					}
					i++;
				}
			}
			else if (lineid >= 4 && lineid <= 5)
			{
				while (string_of_values >> fvalue)
				{
					targetfloat->push_back(fvalue);
				}
			}
			else if (lineid >= 6 && lineid <= 7)
			{
				while (string_of_values >> ivalue)
				{
					targetint->push_back(ivalue);
				}
			}

			lineid = (++lineid) % 8;
			
			if (lineid == 0)
			{
				bool new_strip_success = OutStrips.AddBezierStrip(points, normals, tangents, texcoords, widths, thickness, shapes, subdivisions);
				points.clear();
				normals.clear();
				tangents.clear();
				texcoords.clear();
				widths.clear();
				thickness.clear();
				shapes.clear();
				subdivisions.clear();

				if (!new_strip_success)
				{
					printf("\r\nFailed to parse bezier strip");
				}
				else
				{
					num_loaded_curves++;
				}
			}
		}
		
		OutStrips.SendToGPU();
		printf("\r\nLoaded %d curves from %ws", num_loaded_curves, FilePath.c_str());

		return true;
	}

	void AppendCoordinateAxis(GLLine& OutLines, const glm::fvec3& origin, const glm::fvec3& x, const glm::fvec3& y, const glm::fvec3& z, float scale)
	{
		OutLines.AddLine(origin, origin + x*scale, glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f));
		OutLines.AddLine(origin, origin + y*scale, glm::fvec4(0.0f, 1.0f, 0.0f, 1.0f));
		OutLines.AddLine(origin, origin + z*scale, glm::fvec4(0.0f, 0.0f, 1.0f, 1.0f));
	}

	void AppendCoordinateAxis(GLLine& OutLines, const glm::mat4& Transform, float scale)
	{
		glm::fvec3 origin{ Transform[3][0], Transform[3][1], Transform[3][2] };
		glm::fvec3 x{Transform[0]};
		glm::fvec3 y{Transform[1]};
		glm::fvec3 z{Transform[2]};
		AppendCoordinateAxis(OutLines, origin, x*scale, y*scale, z*scale);
	}
}
