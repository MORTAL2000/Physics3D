#pragma once

#include "abstractMesh.h"

#include "../buffers/bufferLayout.h"

class IndexBuffer;
class VertexBuffer;
struct VisualShape;

class IndexedMesh : public AbstractMesh {
public:
	BufferLayout vertexBufferLayout;
	BufferLayout normalBufferLayout;
	BufferLayout uvBufferLayout;

	IndexBuffer* indexBuffer = nullptr;
	VertexBuffer* vertexBuffer = nullptr;
	VertexBuffer* normalBuffer = nullptr;
	VertexBuffer* uvBuffer = nullptr;

	const int vertexCount;
	const int triangleCount;

	IndexedMesh(const VisualShape& shape);
	IndexedMesh(const float* vertices, const float* normals, const float* uvs, const unsigned int* indices, const int vertexCount, const int triangleCount);

	void render(unsigned int mode);

	void render() override;
	void close() override;
};
