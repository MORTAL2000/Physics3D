#include "import.h"

#include <vector>
#include <fstream>
#include <map>

#include "../view/texture.h"
#include "../util/stringUtil.h"
#include "../util/log.h"

#include "../view/material.h"

#include "../engine/physical.h"

#include "../partFactory.h"

#include "../visualShape.h"

/*
	Import
*/

int Import::parseInt(std::string num) {
	return std::stoi(num);
}

double Import::parseDouble(std::string num) {
	return std::stod(num);
}

float Import::parseFloat(std::string num) {
	return std::stof(num);
}

Vec3 Import::parseVec3(std::string vec) {
	std::vector<std::string> tokens = split(vec, ' ');
	Vec3 vector = Vec3();
	for (int i = 0; i < 3; i++) {
		vector.v[i] = Import::parseDouble(tokens[i]);
	}
	return vector;
}

Vec4 Import::parseVec4(std::string vec) {
	std::vector<std::string> tokens = split(vec, ' ');
	Vec4 vector = Vec4();
	for (int i = 0; i < 4; i++) {
		vector.v[i] = Import::parseDouble(tokens[i]);
	}
	return vector;
}

Vec4f Import::parseVec4f(std::string vec) {
	std::vector<std::string> tokens = split(vec, ' ');
	Vec4f vector = Vec4f();
	for (int i = 0; i < 4; i++) {
		vector.v[i] = Import::parseFloat(tokens[i]);
	}
	return vector;
}

Vec3f Import::parseVec3f(std::string vec) {
	std::vector<std::string> tokens = split(vec, ' ');
	Vec3f vector = Vec3f();
	for (int i = 0; i < 3; i++) {
		vector.v[i] = Import::parseFloat(tokens[i]);
	}
	return vector;
}

DiagonalMat3 Import::parseDiagonalMat3(std::string mat) {
	std::vector<std::string> tokens = split(mat, ' ');
	DiagonalMat3 matrix = DiagonalMat3();
	for (int i = 0; i < 3; i++) {
		matrix.m[i] = Import::parseDouble(tokens[i]);
	}
	return matrix;
}

Mat3 Import::parseMat3(std::string mat) {
	std::vector<std::string> tokens = split(mat, ' ');
	Mat3 matrix = Mat3();
	for (int i = 0; i < 9; i++) {
		matrix.m[i] = Import::parseDouble(tokens[i]);
	}
	return matrix;
}

template<typename T>
T Import::read(std::istream& input) {
	char * buffer = (char*) alloca(sizeof(T));
	input.read(buffer, sizeof(T));
	return *reinterpret_cast<T*>(buffer);
}

/*
	End of Import
*/



/*
	OBJImport
*/

struct Group {
	unsigned int position;
	unsigned int normal;
	unsigned int uv;

	Group() {
		position = -1;
		normal = -1;
		uv = -1;
	}
};

struct Flags {
	bool normals;
	bool uvs;
};

struct Face {
	Group groups[3];

	Face(std::string v1, std::string v2, std::string v3) {
		groups[0] = parseLine(v1);
		groups[1] = parseLine(v2);
		groups[2] = parseLine(v3);
	}

	Group parseLine(std::string line) {
		Group group = Group();

		std::vector<std::string> tokens = split(line, '/');
		size_t length = tokens.size();

		// Positions
		group.position = std::stoi(tokens[0]) - 1;

		// Uvs
		if (length > 1) {
			std::string uv = tokens[1];
			group.uv = uv.size() > 0 ? std::stoi(uv) - 1 : -1;
		}

		// Normals
		if (length > 2) {
			group.normal = std::stoi(tokens[2]) - 1;
		}

		return group;
	}
};

VisualShape reorder(const std::vector<Vec3f>& positions, const std::vector<Vec3f>& normals, const std::vector<Vec2f>& uvs, const std::vector<Face>& faces, Flags flags) {
	
	// Positions
	Vec3f* positionArray = new Vec3f[positions.size()];
	for (int i = 0; i < positions.size(); i++) {
		positionArray[i] = positions[i];
	}

	Triangle* triangleArray = new Triangle[faces.size()];

	Vec3f* normalArray = nullptr;
	Vec2f* uvArray = nullptr;
	if (flags.normals) normalArray = new Vec3f[positions.size()];
	if (flags.uvs) uvArray = new Vec2f[positions.size()];

	for (int i = 0; i < faces.size(); i++) {
		const Group* indices = faces[i].groups;

		triangleArray[i] = { indices[0].position, indices[1].position, indices[2].position };

		for (int i = 0; i < 3; i++) {
			const Group& index = indices[i];

			if (flags.normals && indices->normal != -1) {
				normalArray[index.position] = normals[index.normal];
			}

			if (flags.uvs && indices->uv != -1) {
				uvArray[index.position] = Vec2(uvs[index.uv].x, 1.0 - uvs[index.uv].y);
			}
		}
	}

	return VisualShape(positionArray, SharedArrayPtr<const Vec3f>(normalArray), SharedArrayPtr<const Vec2f>(uvArray), SharedArrayPtr<const Triangle>(triangleArray), (int)positions.size(), (int)faces.size());
}

VisualShape loadBinaryObj(std::istream& input) {
	char flag = Import::read<char>(input);
	int vertexCount = Import::read<int>(input);
	int triangleCount = Import::read<int>(input);

	char V = 0;
	char VN = 1;
	char VT = 2;
	char VNT = 3;

	Vec3f* vertices = new Vec3f[vertexCount];
	for (int i = 0; i < vertexCount; i++) {
		Vec3 t = Import::read<Vec3f>(input);

		vertices[i] = t;
	}

	Vec3f* normals = nullptr;
	if (flag == VN || flag == VNT) {
		normals = new Vec3f[vertexCount];
		for (int i = 0; i < vertexCount; i++) {
			normals[i] = Import::read<Vec3f>(input);
		}
	}

	Vec2f* uvs = nullptr;
	if (flag == VT || flag == VNT) {
		uvs = new Vec2f[vertexCount];
		for (int i = 0; i < vertexCount; i++) {
			uvs[i] = Import::read<Vec2f>(input);
		}
	}

	Triangle* triangles = new Triangle[triangleCount];
	for (int i = 0; i < triangleCount; i++) {
		triangles[i] = Import::read<Triangle>(input);
	}

	return VisualShape(vertices, SharedArrayPtr<const Vec3f>(normals), SharedArrayPtr<const Vec2f>(uvs), SharedArrayPtr<const Triangle>(triangles), vertexCount, triangleCount);
}

VisualShape loadNonBinaryObj(std::istream& input) {
	std::vector<Vec3f> vertices;
	std::vector<Vec3f> normals;
	std::vector<Vec2f> uvs;
	std::vector<Face> faces;
	Flags flags = { false, false };

	std::string line;
	while (getline(input, line)) {
		std::vector<std::string> tokens = split(line, ' ');

		if (tokens.size() == 0)
			continue;

		if (tokens[0] == "v") {
			Vec3f vertex = Vec3f(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]));
			vertices.push_back(vertex);
		} else if (tokens[0] == "f") {
			Face face = Face(tokens[1], tokens[2], tokens[3]);
			faces.push_back(face);

			if (tokens.size() > 4) {
				Face face = Face(tokens[1], tokens[3], tokens[4]);
				faces.push_back(face);
			}
		} else if (tokens[0] == "vt") {
			flags.uvs = true;
			Vec2f uv = Vec2f(stof(tokens[1]), stof(tokens[2]));
			uvs.push_back(uv);
		} else if (tokens[0] == "vn") {
			flags.normals = true;
			Vec3f normal = Vec3f(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]));
			normals.push_back(normal);
		}
	}

	return reorder(vertices, normals, uvs, faces, flags);
}

VisualShape OBJImport::load(std::istream& file, bool binary) {
	if (binary)
		return loadBinaryObj(file);
	else
		return loadNonBinaryObj(file);
}

VisualShape OBJImport::load(std::string file, bool binary) {
	struct stat buffer;

	if (stat(file.c_str(), &buffer) == -1) {
		Log::setSubject(file.c_str());
		Log::error("File not found: %s", file.c_str());
		Log::resetSubject();
		return VisualShape();
	}

	std::ifstream input;

	if (binary)
		input.open(file, std::ios::binary);
	else
		input.open(file);

	VisualShape shape = load(input, binary);

	input.close();

	return shape;
}

/*
	End of OBJImport
*/



/*
	WorldImport
*/

enum class Subject {
	PART = 2,
	CAMERA = 1,
	GLOBAL = 0,
	NONE = -1
};

void parseSubject(Subject subject, std::string path, std::map<std::string, std::string>& fields, std::map<std::string, PartFactory>& factories , World<ExtendedPart>& world, Screen& screen) {
	if (subject == Subject::PART) {
		Material material = Material();
		material.ambient = Import::parseVec4(fields.at("ambient"));
		material.diffuse = Import::parseVec3(fields.at("diffuse"));
		material.specular = Import::parseVec3(fields.at("specular"));
		material.reflectance = Import::parseFloat(fields.at("reflectance"));

		if (!fields.at("texture").empty())
			material.texture = load(fields.at("texture"));

		if (!fields.at("normals").empty())
			material.normal = load(fields.at("normals"));

		double density = Import::parseDouble(fields.at("density"));
		double friction = Import::parseDouble(fields.at("friction"));
		double mass = Import::parseDouble(fields.at("mass"));

		Vec3 pos = Import::parseVec3(fields.at("position"));
		Mat3 rot = Import::parseMat3(fields.at("rotation"));

		std::string name = fields.at("name");
		int mode = Import::parseInt(fields.at("mode"));

		Vec3 velocity = Import::parseVec3(fields.at("velocity"));
		Vec3 angularvelocity = Import::parseVec3(fields.at("angularvelocity"));
		DiagonalMat3 inertia = Import::parseDiagonalMat3(fields.at("inertia"));

		bool anchored = Import::parseInt(fields.at("anchored"));

		std::string shapeReference = fields.at("shape");

		CFrame cframe = CFrame(pos, rot);

		if (!factories.count(shapeReference)) {
			Log::info("Loading part %s in factory", (path + shapeReference).c_str());
			VisualShape shape = OBJImport::load(path + shapeReference);
			factories[shapeReference] = PartFactory(shape, screen, name);
		}

		ExtendedPart* part = factories.at(shapeReference).produce(cframe, density, friction, name);

		Physical physical = Physical(part, mass, inertia);
		physical.angularVelocity = angularvelocity;
		physical.velocity = velocity;

		part->parent = &physical;
		part->material = material;
		part->renderMode = mode;

		world.addPart(part, anchored);
	} else if (subject == Subject::CAMERA) {
		Vec3 pos = Import::parseVec3(fields.at("position"));
		Mat3 rot = Import::parseMat3(fields.at("rotation"));
		double speed = Import::parseDouble(fields.at("speed"));
		double rspeed = Import::parseDouble(fields.at("rspeed"));
		bool fly = Import::parseInt(fields.at("flying"));

		screen.camera.cframe = CFrame(pos, rot);
		screen.camera.speed = speed;
		screen.camera.rspeed = rspeed;
		screen.camera.flying = fly;
	} else if (subject == Subject::GLOBAL) {

	}

	fields.clear();
}

void loadBinaryWorld(std::string name, World<ExtendedPart>& world, Screen& screen) {

}

void loadNonBinaryWorld(std::string name, World<ExtendedPart>& world, Screen& screen) {
	Log::setSubject(name);

	Subject subject = Subject::NONE;
	std::map<std::string, std::string> fields;
	std::map<std::string, PartFactory> factories;
	
	std::string path = std::string("./") + name + "/";
	std::string filename = path + "world.txt";
	std::ifstream input(filename.c_str());

	std::string line;
	while (getline(input, line)) {
		line = trim(line);
		if (line.empty() || line.at(0) == '#') {
			// next line
		} else {
			if (startWith(line, "part:")) {
				parseSubject(subject, path, fields, factories, world, screen);
				subject = Subject::PART;
				continue;
			}

			if (startWith(line, "camera:")) {
				parseSubject(subject, path, fields, factories, world, screen);
				subject = Subject::CAMERA;
				continue;
			}

			if (startWith(line, "global:")) {
				parseSubject(subject, path, fields, factories, world, screen);
				subject = Subject::GLOBAL;
				continue;
			}

			size_t pos = line.find("=");
			std::string property = rtrim(line.substr(0, pos));
			std::string value = ltrim(line.substr(pos + 1, line.length()));

			fields[property] = value;
		}
	}

	parseSubject(subject, path, fields, factories, world, screen);

	Log::resetSubject();
}

void WorldImport::load(std::string name, World<ExtendedPart>& world, Screen& screen, bool binary) {
	if (binary)
		loadBinaryWorld(name, world, screen);
	else
		loadNonBinaryWorld(name, world, screen);
}

/*
	End of WorldImport
*/