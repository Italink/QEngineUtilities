#include "QGaussianSplattingPointCloud.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

const float SH_0 = 0.28209479177387814f;

QVector<QGaussianSplattingPoint> parseSplatFromStream(std::istream& in)
{
	QVector<QGaussianSplattingPoint>  result;

	if (!in.good()) {
		qDebug("[ERROR] Unable to read from input stream.");
		return result;
	}

	std::string line;
	std::getline(in, line);
	if (line != "ply") {
		qDebug("[ERROR] Input data is not a .ply file.");
		return result;
	}

	std::getline(in, line);
	if (line != "format binary_little_endian 1.0") {
		qDebug("[ERROR] Unsupported .ply format.");
		return result;
	}

	std::getline(in, line);
	if (line.find("element vertex ") != 0) {
		qDebug("[ERROR] Missing vertex count.");
		return result;
	}

	int numPoints = std::stoi(line.substr(std::strlen("element vertex ")));

	if (numPoints <= 0 || numPoints > 10 * 1024 * 1024) {
		qDebug("[ERROR] Invalid vertex count: %d", numPoints);
		return result;
	}

	qDebug("[LOG] Loading %d points", numPoints);
	std::unordered_map<std::string, int> fields; // name -> index
	for (int i = 0;; i++) {
		if (!std::getline(in, line)) {
			qDebug("[ERROR] Unexpected end of header.");
			return result;
		}

		if (line == "end_header")
			break;

		if (line.find("property float ") != 0) {
			qDebug("[ERROR] Unsupported property data type: %s", line.c_str());
			return result;
		}
		std::string name = line.substr(std::strlen("property float "));
		fields[name] = i;
	}

	// Returns the index for a given field name, ensuring the name exists.
	const auto index = [&fields](const std::string& name) {
		const auto& itr = fields.find(name);
		if (itr == fields.end()) {
			qDebug("[ERROR] Missing field: %s", name.c_str());
			return -1;
		}
		return itr->second;
		};

	const std::vector<int> positionIdx = { index("x"), index("y"), index("z") };
	const std::vector<int> scaleIdx = { index("scale_0"), index("scale_1"),
									   index("scale_2") };
	const std::vector<int> rotIdx = { index("rot_1"), index("rot_2"),
									 index("rot_3"), index("rot_0") };
	const std::vector<int> alphaIdx = { index("opacity") };
	const std::vector<int> colorIdx = { index("f_dc_0"), index("f_dc_1"),
									   index("f_dc_2") };

	// Check that only valid indices were returned.
	auto checkIndices = [&](const std::vector<int>& idxVec) -> bool {
		for (auto idx : idxVec) {
			if (idx < 0) {
				return false;
			}
		}
		return true;
		};

	if (!checkIndices(positionIdx) || !checkIndices(scaleIdx) ||
		!checkIndices(rotIdx) || !checkIndices(alphaIdx) ||
		!checkIndices(colorIdx)) {
		return result;
	}

	// Spherical harmonics are optional and variable in size (depending on degree)
	std::vector<int> shIdx;
	for (int i = 0; i < 45; i++) {
		const auto& itr = fields.find("f_rest_" + std::to_string(i));
		if (itr == fields.end())
			break;
		shIdx.push_back(itr->second);
	}
	const int shDim = static_cast<int>(shIdx.size() / 3);

	// If spherical harmonics fields are present, ensure they are complete
	if (shIdx.size() % 3 != 0) {
		qDebug("[ERROR] Incomplete spherical harmonics fields.");
		return result;
	}

	std::vector<float> values;
	values.resize(numPoints * fields.size());

	in.read(reinterpret_cast<char*>(values.data()),
		values.size() * sizeof(float));
	if (!in.good()) {
		qDebug("[ERROR] Unable to load data from input stream.");
		return result;
	}

	result.resize(numPoints);



	for (size_t i = 0; i < static_cast<size_t>(numPoints); i++) {
		size_t vertexOffset = i * fields.size();
		QGaussianSplattingPoint& point = result[i];

		// Position
		point.Position.setX(values[vertexOffset + positionIdx[0]]);
		point.Position.setY(values[vertexOffset + positionIdx[1]]);
		point.Position.setZ(values[vertexOffset + positionIdx[2]]);
		point.Position.setW(1.0f);

		// Color
		point.Color.setX(values[vertexOffset + colorIdx[0]]);
		point.Color.setY(values[vertexOffset + colorIdx[1]]);
		point.Color.setZ(values[vertexOffset + colorIdx[2]]);
		point.Color = SH_0 * point.Color + QVector4D(0.5f, 0.5f, 0.5f, 0.5f) ;
		
		float alpha = values[vertexOffset + alphaIdx[0]];
		point.Color.setW(1.0f / (1.0f + qExp(-alpha)));

		QVector3D scale = QVector3D(
			qExp(values[vertexOffset + scaleIdx[0]]),
			qExp(values[vertexOffset + scaleIdx[1]]),
			qExp(values[vertexOffset + scaleIdx[2]])
		);
		QMatrix4x4 scaleMat = QMatrix4x4();
		scaleMat.scale(scale);

		QQuaternion rot = QQuaternion(
			values[vertexOffset + rotIdx[3]],
			values[vertexOffset + rotIdx[0]],
			values[vertexOffset + rotIdx[1]],
			values[vertexOffset + rotIdx[2]]
		);
		QMatrix4x4 rotMat = QMatrix4x4(rot.toRotationMatrix());
		
		QMatrix4x4 rotScale = rotMat * scaleMat;

		point.Sigma = (rotScale * rotScale.transposed()).toGenericMatrix<4, 4>();
	}
	return result;
}

QSharedPointer<QGaussianSplattingPointCloud> QGaussianSplattingPointCloud::CreateFromFile(const QString& inFilePath)
{
	std::ifstream in(inFilePath.toStdString(), std::ios::binary);
	if (!in.is_open()) {
		qDebug("[ERROR] Unable to open: %s", inFilePath.toStdString().c_str());
		return {};
	}
	QVector<QGaussianSplattingPoint> points = parseSplatFromStream(in);
	in.close();
	QSharedPointer<QGaussianSplattingPointCloud> pointCloud = QSharedPointer<QGaussianSplattingPointCloud>::create();
	pointCloud->mPoints = points;
	pointCloud->mBounds = {
		QVector3D(FLT_MAX, FLT_MAX,FLT_MAX),
		QVector3D(FLT_MIN, FLT_MIN,FLT_MIN),
	};
	for (const auto& point : points) {
		pointCloud->mBounds.first.setX(qMin(pointCloud->mBounds.first.x(), point.Position.x()));
		pointCloud->mBounds.second.setX(qMax(pointCloud->mBounds.second.x(), point.Position.x()));

		pointCloud->mBounds.first.setY(qMin(pointCloud->mBounds.first.y(), point.Position.y()));
		pointCloud->mBounds.second.setY(qMax(pointCloud->mBounds.second.y(), point.Position.y()));

		pointCloud->mBounds.first.setZ(qMin(pointCloud->mBounds.first.z(), point.Position.z()));
		pointCloud->mBounds.second.setZ(qMax(pointCloud->mBounds.second.z(), point.Position.z()));
	}
	return pointCloud;
}

