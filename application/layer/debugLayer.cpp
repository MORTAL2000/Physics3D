#include "core.h"

#include "debugLayer.h"

#include "../graphics/mesh/arrayMesh.h"
#include "../graphics/mesh/pointMesh.h"
#include "../graphics/mesh/vectorMesh.h"
#include "../graphics/mesh/indexedMesh.h"

#include "../physics/datastructures/boundsTree.h"
#include "../physics/math/mathUtil.h"
#include "../physics/math/bounds.h"
#include "../physics/physical.h"

#include "view/screen.h"
#include "ecs/material.h"
#include "../graphics/renderUtils.h"
#include "shader/shaders.h"

#include "../graphics/debug/profilerUI.h"
#include "../graphics/debug/visualDebug.h"

#include "../graphics/gui/gui.h"

#include "worlds.h"
#include "../graphics/meshLibrary.h"

namespace Application {

Vec4f colors[] {
	COLOR::BLUE,
	COLOR::GREEN,
	COLOR::YELLOW,
	COLOR::ORANGE,
	COLOR::RED,
	COLOR::PURPLE
};

void renderSphere(double radius, const Position& position, const Color& color) {
	ApplicationShaders::basicShader.updateMaterial(Material(color));
	ApplicationShaders::basicShader.updateModel(Mat4f(Mat3f::IDENTITY() * float(radius), Vec3f(position - Position(0,0,0)), Vec3f(0.0f,0.0f,0.0f),1.0f));

	Library::sphere->render();
}

void renderBox(const GlobalCFrame& cframe, double width, double height, double depth, const Color& color) {
	ApplicationShaders::basicShader.updateMaterial(Material(color));
	ApplicationShaders::basicShader.updateModel(Mat4f(cframe.getRotation().asRotationMatrix() * DiagonalMat3 { width, height, depth }, Vec3f(cframe.getPosition() - Position(0,0,0)), Vec3f(0.0f,0.0f,0.0f), 1.0f));

	Library::cube->render();
}

void renderBounds(const Bounds& bounds, const Color& color) {
	Vec3Fix diagonal = bounds.getDiagonal();
	Position position = bounds.getCenter();
	renderBox(GlobalCFrame(position), diagonal.x, diagonal.y, diagonal.z, color);
}

void recursiveRenderColTree(const TreeNode& node, int depth) {
	if (node.isLeafNode()) {
		//renderBounds(node.bounds, GUI::COLOR::AQUA);
	} else {
		for (const TreeNode& node : node) {
			recursiveRenderColTree(node, depth + 1);
		}
	}

	Vec4f color = colors[depth % 6];
	color.w = 0.3f;

	renderBounds(node.bounds.expanded((5 - depth) * 0.002), color);
}

bool recursiveColTreeForOneObject(const TreeNode& node, const Part* part, const Bounds& bounds) {
	if (node.isLeafNode()) {
		if (node.object == part)
			return true;
		/*for (const BoundedPhysical& p : *node.physicals) {
			if (p.object == physical) {
				return true;
			}
		}*/
	} else {
		//if (!intersects(node.bounds, bounds)) return false;
		for (const TreeNode& subNode : node) {
			if (recursiveColTreeForOneObject(subNode, part, bounds)) {
				Color green = COLOR::GREEN;
				green.w = 0.3f;

				renderBounds(node.bounds, green);
				return true;
			}
		}
	}
	return false;
}

void DebugLayer::onInit() {

	// Origin init
	originMesh = new ArrayMesh(nullptr, 1, 3, Renderer::POINT);

	// Vector init
	vectorMesh = new VectorMesh(nullptr, 0);

	// Point init
	pointMesh = new PointMesh(nullptr, 0);

}

void DebugLayer::onUpdate() {

}

void DebugLayer::onEvent(::Event& event) {

}

void DebugLayer::onRender() {
	Screen* screen = static_cast<Screen*>(this->ptr);

	graphicsMeasure.mark(GraphicsProcess::VECTORS);

	using namespace Debug;
	using namespace AppDebug;

	// Initialize vector log buffer
	AddableBuffer<ColoredVector>& vecLog = getVectorBuffer();
	AddableBuffer<ColoredPoint>& pointLog = getPointBuffer();

	for (const MotorizedPhysical* physical : screen->world->iterPhysicals()) {
		Position com = physical->getCenterOfMass();
		pointLog.add(ColoredPoint(com, CENTER_OF_MASS));
	}

	screen->world->syncReadOnlyOperation([this, &vecLog]() {
		Screen* screen = static_cast<Screen*>(this->ptr);
		for(const ConstraintGroup& constraintGroup : screen->world->constraints) {
			for(const BallConstraint& ballConstraint : constraintGroup.ballConstraints) {
				vecLog.add(ColoredVector(ballConstraint.a->getCFrame().getPosition(), ballConstraint.a->getCFrame().localToRelative(ballConstraint.attachA), INFO_VEC));
				vecLog.add(ColoredVector(ballConstraint.b->getCFrame().getPosition(), ballConstraint.b->getCFrame().localToRelative(ballConstraint.attachB), INFO_VEC));
			}
		}

		if (screen->selectedPart != nullptr) {
			const GlobalCFrame& selectedCFrame = screen->selectedPart->getCFrame();
			Motion partMotion = screen->selectedPart->getMotion();
			for (const Vec3f& corner : screen->selectedPart->hitbox.asPolyhedron().iterVertices()) {
				vecLog.add(ColoredVector(selectedCFrame.localToGlobal(corner), partMotion.getVelocityOfPoint(selectedCFrame.localToRelative(corner)), VELOCITY));
			}

			if (colissionSpheresMode == SphereColissionRenderMode::SELECTED) {
				Physical& selectedPhys = *screen->selectedPart->parent;

				for(Part& part : selectedPhys.rigidBody) {
					Color yellow = COLOR::YELLOW;
					yellow.w = 0.5;
					BoundingBox localBounds = screen->selectedPart->getLocalBounds();
					renderBox(screen->selectedPart->getCFrame().localToGlobal(CFrame(localBounds.getCenter())), localBounds.getWidth(), localBounds.getHeight(), localBounds.getDepth(), yellow);

					Color green = COLOR::GREEN;
					green.w = 0.5;
					renderSphere(part.maxRadius * 2, part.getPosition(), green);
				}
			}
		}

		if(colissionSpheresMode == SphereColissionRenderMode::ALL) {
			for(MotorizedPhysical* phys : screen->world->iterPhysicals()) {
				for(Part& part : phys->rigidBody) {
					Color yellow = COLOR::YELLOW;
					yellow.w = 0.5;
					BoundingBox localBounds = part.getLocalBounds();
					renderBox(part.getCFrame().localToGlobal(CFrame(localBounds.getCenter())), localBounds.getWidth(), localBounds.getHeight(), localBounds.getDepth(), yellow);

					Color green = COLOR::GREEN;
					green.w = 0.5;
					renderSphere(part.maxRadius * 2, part.getPosition(), green);
				}
			}
		}

		switch (colTreeRenderMode) {
			case ColTreeRenderMode::FREE:
				recursiveRenderColTree(screen->world->objectTree.rootNode, 0);
				break;
			case ColTreeRenderMode::TERRAIN:
				recursiveRenderColTree(screen->world->terrainTree.rootNode, 0);
				break;
			case ColTreeRenderMode::ALL:
				recursiveRenderColTree(screen->world->objectTree.rootNode, 0);
				recursiveRenderColTree(screen->world->terrainTree.rootNode, 0);
				break;
			case ColTreeRenderMode::SELECTED:
				if (screen->selectedPart != nullptr)
					recursiveColTreeForOneObject(screen->world->objectTree.rootNode, screen->selectedPart, screen->selectedPart->getStrictBounds());
				break;
		}
		});

	Renderer::disableDepthTest();

	// Update debug meshes
	graphicsMeasure.mark(GraphicsProcess::VECTORS);
	updateVectorMesh(vectorMesh, vecLog.data, vecLog.size);
	updatePointMesh(pointMesh, pointLog.data, pointLog.size);


	// Render vector mesh
	graphicsMeasure.mark(GraphicsProcess::VECTORS);
	ApplicationShaders::vectorShader.updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);
	vectorMesh->render();


	// Render point mesh
	graphicsMeasure.mark(GraphicsProcess::VECTORS);
	ApplicationShaders::pointShader.updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);
	pointMesh->render();

	Renderer::enableDepthTest();

	// Render origin mesh
	graphicsMeasure.mark(GraphicsProcess::ORIGIN);
	ApplicationShaders::originShader.updateProjection(screen->camera.viewMatrix, screen->camera.getViewRotation(), screen->camera.projectionMatrix, screen->camera.orthoMatrix, screen->camera.cframe.position);
	originMesh->render();
}

void DebugLayer::onClose() {

}

};