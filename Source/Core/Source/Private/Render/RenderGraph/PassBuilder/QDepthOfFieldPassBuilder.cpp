#include "QDepthOfFieldPassBuilder.h"
#include "IRenderer.h"

QDepthOfFieldPassBuilder::QDepthOfFieldPassBuilder()
{
    mDepthOfFieldFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (binding = 0) uniform sampler2D uFocus;
		layout (binding = 1) uniform sampler2D uPosition;
		layout (binding = 2) uniform Params{
            mat4 VP;
			float focus;
			float aperture;
			float focalLength;
			int apertureBlades;
			float bokehSqueeze;
			float bokehSqueezeFalloff;
			float aspectRatio;
			int iterations;
		}params;

		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;

		const float PI = 3.1415926f;
        const float PI_OVER_2 = 1.5707963f;
        const float PI_OVER_4 = 0.785398f;
        const float EPSILON = 0.000001f;

		/**
         * Projects coordinates on a unit square from -1.0 to 1.0 onto a circle
         * Ported GLSL version of
         * http://www.adriancourreges.com/blog/2018/12/02/ue4-optimized-post-effects/
         * Based on Shirley��s concentric mapping
         */

        vec2 UnitSquareToUnitDiskPolar(float a, float b) {
            float radius;
            float angle;
            if (abs(a) > abs(b)) { // First region (left and right quadrants of the disk)
                radius = a;
                angle = b / (a + EPSILON) * PI_OVER_4;
            } else { // Second region (top and botom quadrants of the disk)
                radius = b;
                angle = PI_OVER_2 - (a / (b + EPSILON) * PI_OVER_4);
            }
            if (radius < 0) { // Always keep radius positive
                radius *= -1.0f;
                angle += PI;
            }
            return vec2(radius, angle);
        }

		 vec2 SquareToDiskMapping(float a, float b) {
            vec2 PolarCoord = UnitSquareToUnitDiskPolar(a, b);
            return vec2(PolarCoord.x * cos(PolarCoord.y), PolarCoord.x * sin(PolarCoord.y));
        }

        vec2 SquareToPolygonMapping(float a, float b, float edgeCount, float shapeRotation) {
            vec2 PolarCoord = UnitSquareToUnitDiskPolar(a, b); // (radius, angle)

            // Re-scale radius to match a polygon shape
            PolarCoord.x *= cos(PI / edgeCount) /
                cos(PolarCoord.y - (2.0f * PI / edgeCount) * floor((edgeCount * PolarCoord.y + PI) / 2.0f / PI));

            // Apply a rotation to the polygon shape
            PolarCoord.y += shapeRotation;

            return vec2(PolarCoord.x * cos(PolarCoord.y), PolarCoord.x * sin(PolarCoord.y));
        }

        /**
         * 2D Rotation matrix from a radian angle
         */
        mat2 rotate2d(float angle) {
            return mat2(
                cos(angle), -sin(angle),
                sin(angle),  cos(angle)
            );
        }


        float getBlurSize(float depth) {
            return abs((params.focalLength * (params.focus - depth)) /(depth * (params.focus - params.focalLength)) ) * (params.focalLength / params.aperture) * 1000.0;
        }

        void main() {
            vec2 pixelSize = 1.0 / textureSize(uFocus, 0);  

            vec3 color = texture(uFocus, vUV).rgb;
            
            if (params.iterations == 0) {
                outFragColor = vec4(color,1.0f);
                return;
            }

            /**
             * Vector pointing away from the center
             */
            vec2 fromCenter = (vec2(0.5, 0.5) - vUV) * vec2(params.aspectRatio, 1.0);
            /**
             * Calc squeeze distortion strength
             */
            float squeezeStrength = pow(length(fromCenter), params.bokehSqueezeFalloff) * params.bokehSqueeze;

            /**
             * This will cause the 'cats eye bokeh' at the edges of the
             * screen by squeezing it based on the direction to the center
             */
            float angle = acos(dot(vec2(1.0, 0.0), normalize(fromCenter)));

            /**
             * flip the lower half, so it bends symmetrically
             */
            if (fromCenter.y > 0.0) { angle = -angle; }

            /**
             * Build a matrix to squeeze in a single step and
             * also scale according to the focus scale
             */
            mat2 squeeze = rotate2d(angle) * mat2(1.0 + squeezeStrength, 0.0, 0.0, 1.0) * rotate2d(-angle);
            squeeze = squeeze * mat2(
                pixelSize.x, 0.0,
                0.0, pixelSize.y
            );

            float centerDepth = (params.VP * texture(uPosition, vUV)).z;
            float centerBlur = getBlurSize(centerDepth);

            /**
             * Handle the iterations 
             */
            int iterationsX = int(floor(sqrt(float(params.iterations))));
            float stepSize = 2.0f / float(iterationsX);
            float steps = 1.0;

            for (float x = -1.0; x <= 1.0; x += stepSize) {
                for (float y = -1.0; y <= 1.0; y += stepSize) {
                    /**
                     * This is the offset we'll use to get the depth and color
                     * It's already in the bokeh shape and squeezed
                     */
                    vec2 offset = SquareToPolygonMapping(x, y, float(params.apertureBlades), 0.0);
                    offset *= (params.focalLength / params.aperture) * 50.0;
                    offset *= squeeze;

                    float sampleDepth = (params.VP * texture(uPosition, vUV + offset)).z;
                    float sampleBlur = getBlurSize(sampleDepth);

                    if (sampleDepth > centerDepth) {
                        sampleBlur = clamp(sampleBlur, 0.0, centerBlur * 2.0);
                    }

                    sampleBlur = clamp(sampleBlur, 0.0, 1.0);

                    /**
                     * Based on that we'll move the sample point closer to the center.
                     */
                    vec3 sampleColor = texture(uFocus, vUV + offset * sampleBlur).rgb;

                    color += sampleColor;
                    steps += 1.0;
                }
            }
            color /= steps;
            outFragColor = vec4(color,1.0f);
        }
	)");
}

void QDepthOfFieldPassBuilder::setup(QRenderGraphBuilder& builder)
{
    mRenderer = builder.renderer();
	builder.setupTexture(mRT.colorAttachment, "DepthOfFieldTexture", QRhiTexture::RGBA8 , mInput._BaseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "DepthOfFieldRenderTarget", { mRT.colorAttachment.get() });
    
    builder.setupBuffer(mUniformBuffer, "DepthOfFieldUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock));
    
    builder.setupSampler(mSampler, "DepthOfFieldSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupShaderResourceBindings(mBindings, "DepthOfFieldBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, mInput._BaseColorTexture.get(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage, mInput._PositionTexture.get(), mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer (2,QRhiShaderResourceBinding::FragmentStage, mUniformBuffer.get()),
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.fullScreenVS() },
		{ QRhiShaderStage::Fragment, mDepthOfFieldFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "DepthOfFieldPipeline", PSO);
	mOutput.DepthOfFieldResult = mRT.colorAttachment;
}

void QDepthOfFieldPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
    QMatrix4x4 VP = mRenderer->getCamera()->getProjectionMatrixWithCorr(cmdBuffer->rhi()) * mRenderer->getCamera()->getViewMatrix();
    mParams.VP = VP.toGenericMatrix<4,4>();
	mParams.aperture = mInput._Aperture;
	mParams.apertureBlades = mInput._ApertureBlades;
	mParams.bokehSqueeze = mInput._BokehSqueeze;
	mParams.bokehSqueezeFalloff = mInput._BokehSqueezeFalloff;
	mParams.focalLength = mInput._FocalLength;
	mParams.focus = mInput._Focus;
	mParams.iterations = mInput._Iterations;
    QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &mParams);
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
