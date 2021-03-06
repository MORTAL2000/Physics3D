#include "core.h"

#include "meshResource.h"
#include "../io/import.h"
#include "../graphics/visualShape.h"

MeshResource* MeshAllocator::load(const std::string& name, const std::string& path) {
	VisualShape shape = OBJImport::load(path);
	IndexedMesh* mesh = new IndexedMesh(shape);
	return new MeshResource(name, path, mesh, shape);
}