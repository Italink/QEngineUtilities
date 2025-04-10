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
		-1, -1,
		 1, -1,
		 1, 1,
		-1, 1
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
			state.srcColor = QRhiGraphicsPipeline::SrcAlpha;
			state.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
		}
		mRenderProxy->setDepthTest(false);
		mRenderProxy->setBlendStates(blendState);

		mRenderProxy->addUniformBlock(QRhiShaderStage::Vertex, "UBO")
			->addParam("M", QGenericMatrix<4, 4, float>())
			->addParam("V", QGenericMatrix<4, 4, float>())
			->addParam("VInv", QGenericMatrix<4, 4, float>())
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

			const float sqrt8 = sqrt(8.0);

			void main(){
				vec4 PosInWorld = UBO.M * inGsPosition;
				vec4 PosInView = UBO.V * PosInWorld;
				PosInView /= PosInView.w;
				vec4 PosInClip = UBO.P * PosInView;
				PosInClip = PosInClip / PosInClip.w;

				vec2 Focal = vec2(UBO.P[0][0], UBO.P[1][1]) * UBO.ViewSize * 0.5f;

				mat3 J = mat3(
					Focal.x / PosInView.z, 0,     -(Focal.x * PosInView.x)/(PosInView.z * PosInView.z),
					0,     Focal.y / PosInView.z, -(Focal.y * PosInView.y)/(PosInView.z * PosInView.z),
					0,     0,     0
				);

				// Concatenate the projection approximation with the model-view transformation
				mat3 MatViewToLocal =  mat3(transpose(UBO.M) * UBO.VInv);
				mat3 W = MatViewToLocal;
				mat3 T = W * J;
				
				mat3 Vrk = mat3(inGsSigma);

				// Transform the 3D covariance matrix (Vrk) to compute the 2D covariance matrix
				mat3 Cov2Dm = transpose(T) * Vrk * T;

				Cov2Dm[0][0] += 0.3f;
                Cov2Dm[1][1] += 0.3f;

				// We are interested in the upper-left 2x2 portion of the projected 3D covariance matrix because
				// we only care about the X and Y values. We want the X-diagonal, Cov2Dm[0][0],
				// the Y-diagonal, Cov2Dm[1][1], and the correlation between the two Cov2Dm[0][1]. We don't
				// need Cov2Dm[1][0] because it is a symetric matrix.
				vec3 Cov2Dv = vec3(Cov2Dm[0][0], Cov2Dm[0][1], Cov2Dm[1][1]);

				// We now need to solve for the eigen-values and eigen vectors of the 2D covariance matrix
				// so that we can determine the 2D Basis for the splat. This is done using the method described
				// here: https://people.math.harvard.edu/~knill/teaching/math21b2004/exhibits/2dmatrices/index.html
				// After calculating the eigen-values and eigen-vectors, we calculate the Basis for rendering the splat
				// by normalizing the eigen-vectors and then multiplying them by (sqrt(8) * sqrt(eigen-value)), which is
				// equal to scaling them by sqrt(8) standard deviations.
				//
				// This is a different approach than in the original work at INRIA. In that work they compute the
				// max extents of the projected splat in screen space to form a screen-space aligned bounding rectangle
				// which forms the geometry that is actually rasterized. The dimensions of that bounding box are 3.0
				// times the square root of the maximum eigen-value, or 3 standard deviations. They then use the inverse
				// 2D covariance matrix (called 'conic') in the CUDA rendering thread to determine fragment opacity by
				// calculating the full gaussian: exp(-0.5 * (X - mean) * conic * (X - mean)) * splat opacity
				float a = Cov2Dv.x;
				float d = Cov2Dv.z;
				float b = Cov2Dv.y;
				float D = a * d - b * b;
				float Trace = a + d;
				float TraceOver2 = 0.5 * Trace;
				float Term2 = sqrt(max(0.1f, TraceOver2 * TraceOver2 - D));
				float EigenValue1 = TraceOver2 + Term2;
				float EigenValue2 = TraceOver2 - Term2;

				if (EigenValue2 <= 0.0){
					gl_Position = vec4(0);
					return;
				}

				vec2 EigenVector1 = normalize(vec2(b, EigenValue1 - a));
				// since the eigen vectors are orthogonal, we derive the second one from the first
				vec2 EigenVector2 = vec2(EigenVector1.y, -EigenVector1.x);

				// We use sqrt(8) standard deviations instead of 3 to eliminate more of the splat with a very low opacity.
				vec2 BasisVector1 = EigenVector1 * min(sqrt8 * sqrt(EigenValue1), 1024.0);
				vec2 BasisVector2 = EigenVector2 * min(sqrt8 * sqrt(EigenValue2), 1024.0);
				
				vec2 NdcOffset = vec2(inQuadPosition.x * BasisVector1 + inQuadPosition.y * BasisVector2) / UBO.ViewSize * 2.0;

				gl_Position = vec4(PosInClip.xy + NdcOffset, PosInClip.z, 1);

				vColor = inGsColor;
				vQuadPosition = inQuadPosition * sqrt8;
			}
		)");

		mRenderProxy->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
			layout(location = 0) in vec4 vColor;
			layout(location = 1) in vec2 vQuadPosition;
			void main(){
				float A = dot(vQuadPosition, vQuadPosition);
                // Since the positional data in vPosition has been scaled by sqrt(8), the squared result will be
                // scaled by a factor of 8. If the squared result is larger than 8, it means it is outside the ellipse
                // defined by the rectangle formed by vPosition. It also means it's farther
                // away than sqrt(8) standard deviations from the mean.
                if (A > 8.0) discard;
                vec3 color = vColor.rgb;

                // Since the rendered splat is scaled by sqrt(8), the inverse covariance matrix that is part of
                // the gaussian formula becomes the identity matrix. We're then left with (X - mean) * (X - mean),
                // and since 'mean' is zero, we have X * X, which is the same as A:
                float opacity = exp(-0.5 * A) * vColor.a;

                BaseColor = vec4(color.rgb, opacity);
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
			blocks["UBO"]->setParamValue("VInv", QVariant::fromValue(ctx.viewMatrix.inverted().toGenericMatrix<4, 4>()));
			blocks["UBO"]->setParamValue("P", QVariant::fromValue(ctx.projectionMatrixWithCorr.toGenericMatrix<4, 4>()));
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

	QMatrix4x4 M = getModelMatrix();
	for (auto const& point : mGaussianSplattingPointCloud->mPoints) {
		QVector3D worldPos = M * QVector3D(point.Position);
		auto v = -camPos - worldPos;
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