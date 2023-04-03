#include "QStaticMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"
#include "AssetUtils.h"
#include <QDir>
#include <QQueue>
#include <QFontMetrics>
#include <QPainterPath>
#include <QPainter>
#include "Utils/EarCut.h"

QSharedPointer<QStaticMesh> QStaticMesh::CreateFromFile(const QString& inFilePath) {
	QSharedPointer<QStaticMesh> staticMesh;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inFilePath.toUtf8().constData(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	if (!scene) {
		return staticMesh;
	}
	staticMesh = QSharedPointer<QStaticMesh>::create();
	const QVector<QMap<QString, QVariant>>& matertialPropertiesList = AssetUtils::loadMaterialPropertiesList(scene, inFilePath);
	QQueue<QPair<aiNode*, aiMatrix4x4>> qNode;
	qNode.push_back({ scene->mRootNode ,aiMatrix4x4() });
	while (!qNode.isEmpty()) {
		QPair<aiNode*, aiMatrix4x4> node = qNode.takeFirst();
		for (unsigned int i = 0; i < node.first->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node.first->mMeshes[i]];
			SubMeshInfo meshInfo;
			meshInfo.verticesOffset = staticMesh->mVertices.size();
			meshInfo.verticesRange = mesh->mNumVertices;
			meshInfo.localTransfrom = AssetUtils::converter(node.second);
			meshInfo.materialProperties = matertialPropertiesList[mesh->mMaterialIndex];
			staticMesh->mVertices.resize(meshInfo.verticesOffset + meshInfo.verticesRange);
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				QStaticMesh::Vertex& vertex = staticMesh->mVertices[meshInfo.verticesOffset + i];
				vertex.position = AssetUtils::converter(mesh->mVertices[i]);
				if (mesh->mNormals)
					vertex.normal = AssetUtils::converter(mesh->mNormals[i]);
				if (mesh->mTextureCoords[0]) {
					vertex.uv.setX(mesh->mTextureCoords[0][i].x);
					vertex.uv.setY(mesh->mTextureCoords[0][i].y);
				}
				if (mesh->mTangents)
					vertex.tangent = AssetUtils::converter(mesh->mTangents[i]);
				if (mesh->mBitangents)
					vertex.bitangent = AssetUtils::converter(mesh->mBitangents[i]);
			}
			meshInfo.indicesOffset = staticMesh->mIndices.size();
			meshInfo.indicesRange = 0;
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					staticMesh->mIndices.push_back(face.mIndices[j]);
					meshInfo.indicesRange++;
				}
			}
			staticMesh->mSubmeshes << meshInfo;
		}
		for (unsigned int i = 0; i < node.first->mNumChildren; i++) {
			qNode.push_back({ node.first->mChildren[i] ,node.second * node.first->mChildren[i]->mTransformation });
		}
	}
	return staticMesh;
}

QSharedPointer<QStaticMesh> QStaticMesh::CreateFromText(const QString& inText, const QFont& inFont, QColor inColor /*= Qt::white*/, Qt::Orientation inOrientation /*= Qt::Horizontal*/, int inSpacing /*= 2*/, bool bUseTexture) {
	QSharedPointer<QStaticMesh> staticMesh = QSharedPointer<QStaticMesh>::create();
	QVector<Vertex>& vertices = staticMesh->mVertices;
	QVector<Index>& indices = staticMesh->mIndices;
	QFontMetrics fontMetrics(inFont);
	QPainterPath fontPath;
	SubMeshInfo submesh;
	QSize textSize;
	if (inOrientation == Qt::Orientation::Horizontal) {
		textSize = { 0,fontMetrics.height() };
		for (int i = 0; i < inText.size(); i++) {
			fontPath.addText(textSize.width(), fontMetrics.ascent(), inFont, inText[i]);
			textSize.setWidth(textSize.width() + inSpacing + fontMetrics.horizontalAdvance(inText[i]));
		}
	}
	else {
		textSize = { 0,fontMetrics.ascent() };
		for (int i = 0; i < inText.size(); i++) {
			fontPath.addText(0, textSize.height(), inFont, inText[i]);
			textSize.setWidth(qMax(textSize.width(), fontMetrics.horizontalAdvance(inText[i])));
			textSize.setHeight(textSize.height() + inSpacing + fontMetrics.height());
		}
	}
	if (bUseTexture) {
		QImage image(textSize, QImage::Format_RGBA8888);
		image.fill(Qt::transparent);
		QPainter painter(&image);
		painter.fillPath(fontPath, inColor);
		painter.end();
		submesh.materialProperties["BaseColor"] = image;

		Vertex vertex;

		float rwidth = textSize.width()/2;
		float rheight = textSize.height()/2;

		vertex.position = QVector3D(-rwidth, -rheight, 0);
		vertex.uv = QVector2D(0, 1);
		vertices << vertex;

		vertex.position = QVector3D(rwidth, -rheight, 0);
		vertex.uv = QVector2D(1, 1);
		vertices << vertex;

		vertex.position = QVector3D(rwidth, rheight, 0);
		vertex.uv = QVector2D(1, 0);
		vertices << vertex;

		vertex.position = QVector3D(-rwidth, rheight, 0);
		vertex.uv = QVector2D(0, 0);
		vertices << vertex;

		indices << 0 << 1 << 2 << 0 << 2 << 3;
	}
	else {
		const QList<QPolygonF>& polygonList = fontPath.toSubpathPolygons();
		QList<QList<QPolygonF>> polygons;
		QList<QPolygonF> holes;
		for (const auto& polygon : polygonList) {
			if (mapbox::detail::PolygonIsClockwise(polygon))
				polygons << QList<QPolygonF>{polygon};
			else
				holes << polygon;
		}
		for (const auto& hole : holes) {
			for (auto& polygon : polygons) {
				if (polygon.first().containsPoint(hole.first(), Qt::FillRule::WindingFill)) {
					polygon << hole;
				}
			}
		}
		Index offset = 0;
		for (const auto& polygonSet : polygons) {
			for (auto& polygon : polygonSet) {
				for (auto& point : polygon) {
					Vertex vertex;
					vertex.position = QVector3D(point.x(), point.y(), 0);
					vertices.push_back(vertex);
				}
			}
			auto localIndices = mapbox::earcut<Index>(polygonSet);
			for (auto it : localIndices) {
				indices << it + offset;
			}
			offset = vertices.size();
		}

		for (auto& vertex : vertices) {
			vertex.position.setY(textSize.height() - vertex.position.y());
			vertex.position -= QVector3D(textSize.width() / 2.0f, textSize.height() / 2.0f, 0.0f);
		}
		submesh.materialProperties["BaseColor"] = inColor;
	}

	for (auto& vertex : vertices) {
		vertex.normal = QVector3D(0, 0, 1);
		vertex.tangent = QVector3D(0, 1, 0);
		vertex.bitangent = QVector3D(1, 0, 0);
	}
	submesh.indicesOffset = 0;
	submesh.verticesOffset = 0;
	submesh.indicesRange = indices.size();
	submesh.verticesRange = vertices.size();
	staticMesh->mSubmeshes << submesh;
	return staticMesh;
}

