#pragma once

#include "../util/resource/resource.h"

#include "../graphics/mesh/indexedMesh.h"
#include "../graphics/visualShape.h"

class MeshResource;

class MeshAllocator : public ResourceAllocator<MeshResource> {
public:
	virtual MeshResource* load(const std::string& name, const std::string& path) override;
};

class MeshResource : public Resource {
private:
	IndexedMesh* mesh;
	VisualShape shape;
public:
	DEFINE_RESOURCE(Mesh, "../res/fonts/default/default.ttf");

	MeshResource(const std::string& path, IndexedMesh* mesh, VisualShape shape) : Resource(path, path), mesh(mesh), shape(shape) {

	}

	MeshResource(const std::string& name, const std::string& path, IndexedMesh* mesh, VisualShape shape) : Resource(name, path), mesh(mesh), shape(shape) {

	}

	IndexedMesh* getMesh() {
		return mesh;
	};

	VisualShape getShape() {
		return shape;
	}

	virtual void close() override {
		mesh->close();
	}

	static MeshAllocator getAllocator() {
		return MeshAllocator();
	}
};

