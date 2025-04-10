#include "Render/Component/QGaussianSplattingPointCloudRenderComponent.h"
#include "Utils/MathUtils.h"
#include "QEngineObjectManager.h"

QGaussianSplattingPointCloudRenderComponent::QGaussianSplattingPointCloudRenderComponent() {
}

void QGaussianSplattingPointCloudRenderComponent::setGaussianSplattingPointCloud(QSharedPointer<QGaussianSplattingPointCloud> val) {
	mGaussianSplattingPointCloud = val;
	if (mGaussianSplattingPointCloud) {
		mSigRebuildResource.request();
	}
}

void QGaussianSplattingPointCloudRenderComponent::onPreRenderTick(QRhiCommandBuffer* cmdBuffer)
{
	onCpuSort(cmdBuffer);
}

void QGaussianSplattingPointCloudRenderComponent::onRebuildResource() {
	if (mGaussianSplattingPointCloud.isNull())
		return;

	std::vector<float> verts = { 
		-2, -2,
		 2, -2,
		 2, 2, 
		-2, 2 
	};

	mQuadBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(float) * verts.size()));
	mQuadBuffer->create();

	mGSBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::VertexBuffer, sizeof(QGaussianSplattingPoint) * mGaussianSplattingPointCloud->mPoints.size()));
	mGSBuffer->create();

	if (!mRenderProxy) {
		mRenderProxy = newPrimitiveRenderProxy();
		mRenderProxy->setTopology(QRhiGraphicsPipeline::Topology::TriangleFan);
		QVector<QRhiGraphicsPipeline::TargetBlend> blendState(getColorAttachmentCount());
		for (auto& state : blendState) {
			state.enable = true; 
		}
		mRenderProxy->setBlendStates(blendState);
		mRenderProxy->setDepthTest(false);

		mRenderProxy->addUniformBlock(QRhiShaderStage::Vertex, "UBO")
			->addParam("M", QGenericMatrix<4, 4, float>())
			->addParam("V", QGenericMatrix<4, 4, float>())
			->addParam("P", QGenericMatrix<4, 4, float>())
			->addParam("ViewSize", QVector2D(800, 600));

		mRenderProxy->setInputBindings({
			QRhiVertexInputBindingEx(mQuadBuffer.get(), sizeof(float) * 2),
			QRhiVertexInputBindingEx(mGSBuffer.get(), sizeof(QGaussianSplattingPoint), 0, QRhiVertexInputBinding::PerInstance),
		});

		mRenderProxy->setInputAttribute({
			QRhiVertexInputAttributeEx("inQuadPosition",0, 0, QRhiVertexInputAttribute::Float2, 0),
			QRhiVertexInputAttributeEx("inGsPosition"  ,1, 1, QRhiVertexInputAttribute::Float4, offsetof(QGaussianSplattingPoint, Position)),
			QRhiVertexInputAttributeEx("inGsColor"	   ,1, 2, QRhiVertexInputAttribute::Float4, offsetof(QGaussianSplattingPoint, Color)),
			QRhiVertexInputAttributeEx("inGsSigma"	   ,1, 3, QRhiVertexInputAttribute::Float4, offsetof(QGaussianSplattingPoint, Sigma) + 0 * sizeof(float), 0),
			QRhiVertexInputAttributeEx("inGsSigma"	   ,1, 4, QRhiVertexInputAttribute::Float4, offsetof(QGaussianSplattingPoint, Sigma) + 4 * sizeof(float), 1),
			QRhiVertexInputAttributeEx("inGsSigma"	   ,1, 5, QRhiVertexInputAttribute::Float4, offsetof(QGaussianSplattingPoint, Sigma) + 8 * sizeof(float), 2),
			QRhiVertexInputAttributeEx("inGsSigma"	   ,1, 6, QRhiVertexInputAttribute::Float4, offsetof(QGaussianSplattingPoint, Sigma) + 12 * sizeof(float), 3),
		});

		mRenderProxy->setShaderMainCode(QRhiShaderStage::Vertex, R"(
			layout(location = 0) out vec4 vColor;
			layout(location = 1) out vec2 vQuadPosition;

			vec4 get_basis(mat2 sigma) {
				float a = sigma[0][0];
				float b = sigma[0][1];
				float c = sigma[1][0];
				float d = sigma[1][1];

				float tr = a + d;
				float det = a * d - b * c;

				// eigenvalues
				float s = sqrt((tr * tr) - (4 * det));
				float lambda1 = 0.5 * (tr + s);
				float lambda2 = 0.5 * (tr - s);

				// eigenvectors
				const float epsilon = 0.00001;

				vec2 e1 = vec2(1, 0);
				if (abs(c) > epsilon) {
					e1 = vec2(lambda1 - d, c);
				} else if (abs(b) > epsilon) {
					e1 = vec2(b, lambda1 - a);
				}
				e1 = normalize(e1);

				vec2 e2 = vec2(e1.y, -e1.x);

				const float max_size = 32 * 2048;
				lambda1 = min(max_size, lambda1);
				lambda2 = min(max_size, lambda2);

				// basis vectors
				vec2 b1 = sqrt(2 * lambda1) * e1;
				vec2 b2 = sqrt(2 * lambda2) * e2;

				return vec4(b1, b2);
			}

			void main(){
				vec4 u = UBO.V * inGsPosition;
				u /= u.w;
				float focal = UBO.P[0][0] * UBO.ViewSize.x * 0.5f;
				mat3 jacobian = mat3(
						focal/u.z, 0,     -(focal * u.x)/(u.z * u.z),
						0,     focal/u.z, -(focal * u.y)/(u.z * u.z),
						0,     0,     0
				);

				// Calculate 2D covariance matrix
				mat3 t = jacobian * mat3(UBO.V);
				mat3 sigma_prime = t * mat3(inGsSigma) * transpose(t);
				mat2 sigma2 = mat2(sigma_prime);  // take upper left

				// Get basis vectors of the splatted 2D Gaussian
				vec4 bases = get_basis(sigma2);
				vec2 b1 = bases.xy;
				vec2 b2 = bases.zw;

				// Position in screen space
				vec4 pos2d = UBO.P * u;
				vec2 center = pos2d.xy / pos2d.w;

				gl_Position = vec4(center
						+ (inQuadPosition.x * b1) / (0.5 * UBO.ViewSize)
						+ (inQuadPosition.y * b2) / (0.5 * UBO.ViewSize),
						-1, 1);

				gl_Position.z = pos2d.z / pos2d.w;

				vColor = inGsColor;
				vQuadPosition = inQuadPosition;
			}
		)");
		mRenderProxy->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
			layout(location = 0) in vec4 vColor;
			layout(location = 1) in vec2 vQuadPosition;
			void main(){
				int f = 1;
				float A = -dot(f * vQuadPosition, f * vQuadPosition);
				if (A < -4.0) discard;
				float B = exp(A) * vColor.a;
				BaseColor = B * vec4(vColor.rgb, 1);
			})")
			.toLocal8Bit()
		);
		mRenderProxy->setOnUpload([this, verts](QRhiResourceUpdateBatch* batch) {
			batch->uploadStaticBuffer(mQuadBuffer.get(), verts.data());
		});

		mRenderProxy->setOnUpdate([this](QRhiResourceUpdateBatch* batch, const QPrimitiveRenderProxy::UniformBlocks& blocks, const QPrimitiveRenderProxy::UpdateContext& ctx) {
			auto size = getPixelSize();
			blocks["UBO"]->setParamValue("M", QVariant::fromValue(getModelMatrix().toGenericMatrix<4, 4>()));
			blocks["UBO"]->setParamValue("V", QVariant::fromValue(ctx.viewMatrix.toGenericMatrix<4, 4>()));
			blocks["UBO"]->setParamValue("P", QVariant::fromValue(ctx.projectionMatrix.toGenericMatrix<4, 4>()));
			blocks["UBO"]->setParamValue("ViewSize", QVector2D(size.width(), size.height()));
		});

		mRenderProxy->setOnDraw([this](QRhiCommandBuffer* cmdBuffer) {
			const QRhiCommandBuffer::VertexInput vertexBindings[] = {
				{ mQuadBuffer.get(), 0 },
				{ mGSBuffer.get(), 0 },
			};
			cmdBuffer->setVertexInput(0, 2, vertexBindings);
			cmdBuffer->draw(4, mGaussianSplattingPointCloud->mPoints.size());
		});
	}
}

void QGaussianSplattingPointCloudRenderComponent::onCpuSort(QRhiCommandBuffer* cmdBuffer)
{
	auto startTime = std::chrono::system_clock::now();
	QVector3D camPos = mRenderer->getCamera()->getPosition();;
	const size_t bucketCount = 65535;
	const size_t pointCount = mGaussianSplattingPointCloud->mPoints.size();
	std::vector<size_t> count(bucketCount + 1, 0);
	std::vector<size_t> distances{};
	distances.reserve(pointCount);

	std::vector<int> sortedIndices(pointCount, 0);

	float maxDist = 1.2f * mGaussianSplattingPointCloud->mBounds.first.distanceToPoint(mGaussianSplattingPointCloud->mBounds.second);

	maxDist *= maxDist;

	for (auto const& point : mGaussianSplattingPointCloud->mPoints) {
		auto v = -camPos - QVector3D(point.Position);
		float d = v.x() * v.x() + v.y() * v.y() + v.z() * v.z();    // dot product
		float d_normalized = bucketCount * d / maxDist;  // between 0 and n_buckets
		size_t dist = qMin(d_normalized, (float)bucketCount - 1);
		++count[dist];
		distances.push_back(dist);
	}

	for (int i = 1; i < count.size(); ++i) {
		count[i] = count[i] + count[i - 1];
	}

	for (int i = pointCount - 1; i >= 0; --i) {
		size_t j = distances[i];
		--count[j];
		sortedIndices[count[j]] = i;
	}

	auto endTime = std::chrono::system_clock::now();
	std::chrono::duration<double> durationSec = endTime - startTime;
	qDebug() << "Sorting cost " << durationSec.count() << "s";

	QVector<QGaussianSplattingPoint> sortedPoints;
	sortedPoints.resize(pointCount);
	for (int i = 0; i < pointCount; i++) {
		sortedPoints[i] = mGaussianSplattingPointCloud->mPoints[sortedIndices[i]];
	}
	mGaussianSplattingPointCloud->mPoints = sortedPoints;
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mGSBuffer.get(), 0, sizeof(QGaussianSplattingPoint) * mGaussianSplattingPointCloud->mPoints.size(), mGaussianSplattingPointCloud->mPoints.data());
	cmdBuffer->resourceUpdate(batch);
}

QSharedPointer<QGaussianSplattingPointCloud> QGaussianSplattingPointCloudRenderComponent::getGaussianSplattingPointCloud() const
{
	return mGaussianSplattingPointCloud;
}

QENGINE_REGISTER_CLASS(QGaussianSplattingPointCloudRenderComponent)