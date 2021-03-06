#pragma once

#include <string>
#include <sstream>

#include "../math/linalg/vec.h"
#include "../math/linalg/mat.h"
#include "../math/linalg/eigen.h"
#include "../math/linalg/largeMatrix.h"
#include "../math/cframe.h"
#include "../math/position.h"
#include "../math/globalCFrame.h"
#include "../motion.h"
#include "../relativeMotion.h"

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const LargeMatrix<T>& matrix) {
	for(int i = 0; i < matrix.height; i++) {
		for(int j = 0; j < matrix.width; j++) {
			os << matrix[i][j] << '\t';
		}
		os << '\n';
	}

	return os;
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const LargeVector<T>& vector) {
	for(int i = 0; i < vector.size; i++) {
		os << vector[i] << ',';
	}

	return os;
}

template<typename T, size_t Size>
inline std::ostream& operator<<(std::ostream& os, const Vector<T, Size>& vector) {
	os << '(';
	for(size_t i = 0; i < Size - 1; i++) {
		os << vector[i] << ", ";
	}
	os << vector[Size - 1] << ")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const Position& position) {
	os << "(" << double(position.x) << ", " << double(position.y) << ", " << double(position.z) << ")";
	return os;
}

template<typename N, size_t Width, size_t Height>
inline std::ostream& operator<<(std::ostream& os, const Matrix<N, Width, Height>& matrix) {
	os << "(";

	for(size_t row = 0; row < Height; row++) {
		for(size_t col = 0; col < Width - 1; col++) {
			os << matrix[row][col] << ", ";
		}
		os << matrix[row][Width - 1] << "; ";
	}
	os << ")";

	return os;
}

template<typename N, size_t Size>
inline std::ostream& operator<<(std::ostream& os, const SymmetricMatrix<N, Size>& matrix) {
	os << "(";

	for(size_t row = 0; row < Size; row++) {
		for(size_t col = 0; col < Size - 1; col++) {
			os << matrix[row][col] << ", ";
		}
		os << matrix[row][Size - 1] << "; ";
	}
	os << ")";

	return os;
}

template<typename N, size_t Size>
inline std::ostream& operator<<(std::ostream& os, const DiagonalMatrix<N, Size>& matrix) {
	os << "Diag(";

	for(size_t i = 0; i < Size; i++) {
		os << matrix[i] << "; ";
	}
	os << ")";

	return os;
}

template<typename N, size_t Width, size_t Height>
inline std::string str(const Matrix<N, Width, Height>& matrix) {
	std::stringstream ss;
	ss.precision(4);
	ss << matrix;
	return ss.str();
}
template<typename N, size_t Size>
inline std::string str(const SymmetricMatrix<N, Size>& matrix) {
	std::stringstream ss;
	ss.precision(4);
	ss << matrix;
	return ss.str();
}

template<typename N, size_t Size>
inline std::ostream& operator<<(std::ostream& os, const EigenValues<N, Size>& v) {
	os << "EigenValues(";
	for(size_t i = 0; i < Size - 1; i++)
		os << v[i] << ", ";

	os << v[Size - 1] << ")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const Rotation& rotation) {
	os << rotation.asRotationMatrix();
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const CFrame& cframe) {
	os << "CFrame(" << cframe.position << ", " << cframe.rotation << ")";
	return os;
}
inline std::ostream& operator<<(std::ostream& os, const GlobalCFrame& cframe) {
	os << "GlobalCFrame(" << cframe.position << ", " << cframe.rotation << ")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const TranslationalMotion& motion) {
	os << "{vel: " << motion.velocity;
	os << ", accel: " << motion.acceleration << "}";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const RotationalMotion& motion) {
	os << "{angularVel: " << motion.angularVelocity;
	os << ", angularAccel: " << motion.angularAcceleration << "}";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const Motion& motion) {
	os << "{vel: " << motion.translation.velocity;
	os << ", angularVel: " << motion.rotation.angularVelocity;
	os << ", accel: " << motion.translation.acceleration;
	os << ", angularAccel: " << motion.rotation.angularAcceleration << "}";

	return os;
}

inline std::ostream& operator<<(std::ostream& os, const RelativeMotion& relMotion) {
	os << "{motion: " << relMotion.relativeMotion;
	os << ", offset: " << relMotion.locationOfRelativeMotion << "}";

	return os;
}

template<typename T>
inline std::string str(const T& obj) {
	std::stringstream ss;
	ss.precision(4);
	ss << obj;
	return ss.str();
}
