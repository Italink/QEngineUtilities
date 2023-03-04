#include "QRhiGraphicsPipelineBuilder.h"
#include "Render/IRenderPass.h"

void QRhiGraphicsPipelineBuilder::setPolygonModeOverride(QRhiGraphicsPipeline::PolygonMode inMode) {
	if (PolygonModeOverride != inMode) {
		PolygonModeOverride = inMode;
		for (auto ins : Instances) {
			ins->sigRebuild.request();
		}
	}
}

void QRhiGraphicsPipelineBuilder::clearPolygonModeOverride() {
	if (PolygonModeOverride != -1) {
		PolygonModeOverride = -1;
		for (auto ins : Instances) {
			ins->sigRebuild.request();
		}
	}
}

void QRhiGraphicsPipelineBuilder::setShaderMainCode(QRhiShaderStage::Type inStage, QByteArray inCode) {
	mStageInfos[inStage].MainCode = inCode;
}

QByteArray QRhiGraphicsPipelineBuilder::getInputFormatTypeName(QRhiVertexInputAttribute::Format inFormat) {
	switch (inFormat) {
	case QRhiVertexInputAttribute::Float4:
		return "vec4";
	case QRhiVertexInputAttribute::Float3:
		return "vec3";
	case QRhiVertexInputAttribute::Float2:
		return "vec2";
	case QRhiVertexInputAttribute::Float:
		return "float";
	case QRhiVertexInputAttribute::UNormByte4:
		return "vec4";
	case QRhiVertexInputAttribute::UNormByte2:
		return "vec4";
	case QRhiVertexInputAttribute::UNormByte:
		return "vec4";
	case QRhiVertexInputAttribute::UInt4:
		return "uvec4";
	case QRhiVertexInputAttribute::UInt3:
		return "uvec3";
	case QRhiVertexInputAttribute::UInt2:
		return "uvec2";
	case QRhiVertexInputAttribute::UInt:
		return "uint";
	case QRhiVertexInputAttribute::SInt4:
		return "ivec4";
	case QRhiVertexInputAttribute::SInt3:
		return "ivec3";
	case QRhiVertexInputAttribute::SInt2:
		return "ivec2";
	case QRhiVertexInputAttribute::SInt:
		return "int";
	default:
		break;
	}
	return "unknown";
}

QByteArray QRhiGraphicsPipelineBuilder::getOutputFormatTypeName(QRhiTexture::Format inFormat) {
	switch (inFormat) {
	case QRhiTexture::RGBA8:
	case QRhiTexture::BGRA8:
	case QRhiTexture::RGBA16F:
	case QRhiTexture::RGBA32F:
	case QRhiTexture::RGB10A2:
		return "vec4";
	case QRhiTexture::R8:
	case QRhiTexture::R16:
	case QRhiTexture::RED_OR_ALPHA8:
	case QRhiTexture::R16F:
	case QRhiTexture::R32F:
		return "float";
	case QRhiTexture::RG8:
	case QRhiTexture::RG16:
		return "vec2";
	}
	return "unknown";
}

QSize QRhiGraphicsPipelineBuilder::resolveCubeImageFaceSize(const QImage& inImage) {
	if (inImage.width() * 3 == 4 * inImage.height())
		return QSize(inImage.width() / 4, inImage.width() / 4);
	else if (inImage.width() * 4 == 3 * inImage.height())
		return QSize(inImage.width() / 3, inImage.width() / 3);
	else
		return QSize();
}

std::array<QImage, 6> QRhiGraphicsPipelineBuilder::resolveCubeSubImages(const QImage& inImage) {
	const QSize& faceSize = resolveCubeImageFaceSize(inImage);
	QImage vaildImage = inImage.convertToFormat(QImage::Format::Format_RGBA8888);
	if (faceSize.isEmpty()) {
		return { vaildImage ,vaildImage ,vaildImage ,vaildImage ,vaildImage ,vaildImage };
	}
	std::array<QImage, 6> subImages;
	subImages[0] = vaildImage.copy(QRect(QPoint(2 * faceSize.width(), faceSize.width()), faceSize));
	subImages[1] = vaildImage.copy(QRect(QPoint(0, faceSize.width()), faceSize));

	subImages[2] = vaildImage.copy(QRect(QPoint(faceSize.width(), 0), faceSize));
	subImages[3] = vaildImage.copy(QRect(QPoint(faceSize.width(), faceSize.width() * 2), faceSize));

	subImages[4] = vaildImage.copy(QRect(QPoint(faceSize.width(), faceSize.width()), faceSize));
	subImages[5] = vaildImage.copy(QRect(QPoint(3 * faceSize.width(), faceSize.width()), faceSize));
	return subImages;
}

void QRhiGraphicsPipelineBuilder::setInputAttribute(QVector<QRhiVertexInputAttributeEx> inInputAttributes) {
	mInputAttributes = inInputAttributes;
	mVertexInputLayout.setAttributes(inInputAttributes.begin(), inInputAttributes.end());
}

void QRhiGraphicsPipelineBuilder::setInputBindings(QVector<QRhiVertexInputBindingEx> inInputBindings) {
	mInputBindings = inInputBindings;
	mVertexInputLayout.setBindings(mInputBindings.begin(), mInputBindings.end());
}

QRhiUniformBlock* QRhiGraphicsPipelineBuilder::addUniformBlock(QRhiShaderStage::Type inStage, const QString& inName) {
	QSharedPointer<QRhiUniformBlock> unifromBlock = QSharedPointer<QRhiUniformBlock>::create(inStage);
	unifromBlock->setObjectName(inName);
	addUniformBlock(inStage, unifromBlock);
	return unifromBlock.get();
}

void QRhiGraphicsPipelineBuilder::addUniformBlock(QRhiShaderStage::Type inStage, QSharedPointer<QRhiUniformBlock> inUniformBlock) {
	mStageInfos[inStage].mUniformBlocks << inUniformBlock;
	mUniformMap[inUniformBlock->objectName()] = inUniformBlock.get();
}

QRhiUniformBlock* QRhiGraphicsPipelineBuilder::getUniformBlock(const QString& inName) {
	return mUniformMap.value(inName);
}

void QRhiGraphicsPipelineBuilder::addTexture(QRhiShaderStage::Type inStage, TextureInfo::EType inType, const QString& inName, const QImage& inImage, QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW) {
	QSharedPointer<TextureInfo> textureInfo = QSharedPointer<TextureInfo>::create();
	textureInfo->Name = inName;
	textureInfo->Type = inType;
	textureInfo->Image = inImage;
	textureInfo->MagFilter = magFilter;
	textureInfo->MinFilter = minFilter;
	textureInfo->MipmapMode = mipmapMode;
	textureInfo->AddressU = addressU;
	textureInfo->AddressV = addressV;
	textureInfo->AddressW = addressW;
	mStageInfos[inStage].mTextureInfos << textureInfo;
	mTextureMap[inName] = textureInfo.get();
}

void QRhiGraphicsPipelineBuilder::setTexture(const QString& inName, const QImage& inImage) {
	for (auto& stageInfo : mStageInfos) {
		for (auto& textureInfo : stageInfo.mTextureInfos) {
			if (textureInfo->Name == inName) {
				textureInfo->Image = inImage;
				textureInfo->sigUpdate.request();
				return;
			}
		}
	}
	qWarning() << inName << " Not exist!";
}

QVector<QRhiCommandBuffer::VertexInput> QRhiGraphicsPipelineBuilder::getVertexInputs() {
	QVector<QRhiCommandBuffer::VertexInput> vertexInputs;
	for (auto& input : mInputBindings) {
		vertexInputs << QRhiCommandBuffer::VertexInput{ input.mBuffer,input.mOffset };
	}
	return vertexInputs;
}

QRhiShaderResourceBindings* QRhiGraphicsPipelineBuilder::getShaderResourceBindings() {
	return mShaderBindings.get();
}

void QRhiGraphicsPipelineBuilder::create(IRenderComponent* inRenderComponent) {
	mBlendStates.resize(inRenderComponent->sceneRenderPass()->getRenderTargetSlots().size());
	QRhiEx* rhi = inRenderComponent->sceneRenderPass()->getRenderer()->getRhi();
	mPipeline.reset(rhi->newGraphicsPipeline());
	mPipeline->setTopology(mTopology);
	mPipeline->setCullMode(mCullMode);
	mPipeline->setFrontFace(mFrontFace);
	mPipeline->setLineWidth(mLineWidth);
	mPipeline->setTargetBlends(mBlendStates.begin(), mBlendStates.end());
	mPipeline->setDepthTest(bEnableDepthTest);
	mPipeline->setDepthWrite(bEnableDepthWrite);
	mPipeline->setDepthOp(mDepthTestOp);
	mPipeline->setStencilTest(bEnableStencilTest);
	mPipeline->setStencilFront(mStencilFrontOp);
	mPipeline->setStencilBack(mStencilBackOp);
	mPipeline->setStencilReadMask(mStencilReadMask);
	mPipeline->setStencilWriteMask(mStencilWriteMask);
	mPipeline->setSampleCount(inRenderComponent->sceneRenderPass()->getSampleCount());
	mPipeline->setDepthBias(mDepthBias);
	mPipeline->setSlopeScaledDepthBias(mSlopeScaledDepthBias);
	mPipeline->setPatchControlPointCount(mPatchControlPointCount);
	mPipeline->setPolygonMode(PolygonModeOverride == -1 ? mPolygonMode : (QRhiGraphicsPipeline::PolygonMode)PolygonModeOverride);
	mPipeline->setVertexInputLayout(mVertexInputLayout);
	mPipeline->setRenderPassDescriptor(inRenderComponent->sceneRenderPass()->getRenderPassDescriptor());

	recreateShaderBindings(inRenderComponent, rhi);

	QVector<QRhiShaderStage> stages;
	for (const auto& stage : mStageInfos.asKeyValueRange()) {
		QShader shader = QRhiEx::newShaderFromCode((QShader::Stage)stage.first, stage.second.VersionCode + stage.second.DefineCode + stage.second.MainCode);
		stages << QRhiShaderStage(stage.first, shader);
	}
	mPipeline->setShaderStages(stages.begin(), stages.end());
	mPipeline->setShaderResourceBindings(mShaderBindings.get());
	mPipeline->create();
	for (const auto& stage : mStageInfos) {
		for (const auto& uniformBlock : stage.mUniformBlocks) {
			uniformBlock->sigRecreateBuffer.receive();
		}
	}
}

void QRhiGraphicsPipelineBuilder::update(QRhiResourceUpdateBatch* batch) {
	for (const auto& stage : mStageInfos) {
		for (const auto& textureInfo : stage.mTextureInfos) {
			if (textureInfo->sigUpdate.receive()) {
				if (textureInfo->Type == TextureInfo::Texture2D) {
					batch->uploadTexture(textureInfo->Texture.get(), textureInfo->Image.convertedTo(QImage::Format::Format_RGBA8888));
				}
				else if(textureInfo->Type == TextureInfo::Cube){
					std::array<QImage, 6> subImgaes = resolveCubeSubImages(textureInfo->Image);
					QRhiTextureSubresourceUploadDescription subresDesc[6];
					for (int i = 0; i < std::size(subImgaes); i++) {
						subresDesc[i].setImage(subImgaes[i]);
					}
					QRhiTextureUploadDescription desc({
									  { 0, 0, subresDesc[0] },  // +X
									  { 1, 0, subresDesc[1] },  // -X
									  { 2, 0, subresDesc[2] },  // +Y
									  { 3, 0, subresDesc[3] },  // -Y
									  { 4, 0, subresDesc[4] },  // +Z
									  { 5, 0, subresDesc[5] }   // -Z
					});
					batch->uploadTexture(textureInfo->Texture.get(), desc);
					batch->generateMips(textureInfo->Texture.get());
				}
			}
		}
		for (const auto& uniformBlock : stage.mUniformBlocks) {
			if (uniformBlock->sigRecreateBuffer.receive()) {
				sigRebuild.request();
				break;
			}
			uniformBlock->updateResource(batch);
		}
	}
}

void QRhiGraphicsPipelineBuilder::recreateShaderBindings(IRenderComponent* inRenderComponent, QRhiEx* inRhi) {
	for (auto& stage : mStageInfos) {
		stage.DefineCode = QByteArray();
	}
	QString vertexInputCode;
	for (auto& input : mInputAttributes) {
		vertexInputCode += QString::asprintf("layout(location = %d) in %s %s;\n", input.location(), getInputFormatTypeName(input.format()).data(), input.mName.toLocal8Bit().data());
	}
	vertexInputCode += "out gl_PerVertex { vec4 gl_Position;}; \n";
	mStageInfos[QRhiShaderStage::Vertex].DefineCode = vertexInputCode.toLocal8Bit();

	QVector<QRhiShaderResourceBinding> bindings;
	int bindingOffset = 0;
	for (const auto& stage : mStageInfos.asKeyValueRange()) {
		QString uniformDefineCode;
		for (auto& textureInfo : stage.second.mTextureInfos) {
			for (const auto& sampler : mSamplerList) {
				if (sampler->magFilter() == textureInfo->MagFilter
					&& sampler->minFilter() == textureInfo->MinFilter
					&& sampler->mipmapMode() == textureInfo->MipmapMode
					&& sampler->addressU() == textureInfo->AddressU
					&& sampler->addressV() == textureInfo->AddressV
					&& sampler->addressW() == textureInfo->AddressW
					) {
					textureInfo->Sampler = sampler;
					break;
				}
			}
			if (textureInfo->Sampler.isNull()) {
				textureInfo->Sampler.reset(inRhi->newSampler(textureInfo->MagFilter, textureInfo->MinFilter, textureInfo->MipmapMode, textureInfo->AddressU, textureInfo->AddressV, textureInfo->AddressW));
				mSamplerList << textureInfo->Sampler;
				textureInfo->Sampler->create();
			}

			if (textureInfo->Type == TextureInfo::Texture2D) {
				textureInfo->Texture.reset(inRhi->newTexture(QRhiTexture::RGBA8, textureInfo->Image.size(), 1));
				textureInfo->Texture->create();
				bindings << QRhiShaderResourceBinding::sampledTexture(bindingOffset, (QRhiShaderResourceBinding::StageFlag)(1 << (int)stage.first), textureInfo->Texture.get(), textureInfo->Sampler.get());
				uniformDefineCode += QString("layout(binding =  %1) uniform sampler2D %2;\n").arg(bindingOffset).arg(textureInfo->Name);
				bindingOffset++;
			}
			else if (textureInfo->Type == TextureInfo::Cube) {
				textureInfo->Texture.reset(inRhi->newTexture(QRhiTexture::RGBA8, resolveCubeImageFaceSize(textureInfo->Image), 1,
					QRhiTexture::CubeMap
					| QRhiTexture::MipMapped
					| QRhiTexture::UsedWithGenerateMips));
				textureInfo->Texture->create();
				bindings << QRhiShaderResourceBinding::sampledTexture(bindingOffset, (QRhiShaderResourceBinding::StageFlag)(1 << (int)stage.first), textureInfo->Texture.get(), textureInfo->Sampler.get());
				uniformDefineCode += QString("layout(binding =  %1) uniform samplerCube %2;\n").arg(bindingOffset).arg(textureInfo->Name);
				bindingOffset++;
			}
			textureInfo->sigUpdate.request();
		}

		for (const auto& uniformBlock : stage.second.mUniformBlocks) {
			if (!uniformBlock->isEmpty()) {
				uniformBlock->create(inRhi);
				bindings << QRhiShaderResourceBinding::uniformBuffer(bindingOffset, (QRhiShaderResourceBinding::StageFlag)(1 << (int)stage.first), uniformBlock->getUniformBlock());
				uniformDefineCode += QString("layout(binding =  %1) uniform %2Block{\n").arg(bindingOffset).arg(uniformBlock->objectName());
				for (auto& param : uniformBlock->getParamList()) {
					uniformDefineCode += QString("    %1 %2;\n").arg(param->typeName()).arg(param->valueName());
				}
				uniformDefineCode += QString::asprintf("}%s;\n", uniformBlock->objectName().toLocal8Bit().data());
				bindingOffset++;
			}
		}
		stage.second.DefineCode += uniformDefineCode.toLocal8Bit();
	}
	QString fragOutputCode;
	auto targetSlots = inRenderComponent->sceneRenderPass()->getRenderTargetSlots();
	for (int i = 0; i < targetSlots.size(); i++) {
		QByteArray slotType = getOutputFormatTypeName(targetSlots[i].first);
		QByteArray slotName = targetSlots[i].second.toLocal8Bit();
		fragOutputCode += QString::asprintf("layout(location = %d) out %s %s;\n", i, slotType.data(), slotName.data());
	}
	mStageInfos[QRhiShaderStage::Fragment].DefineCode += fragOutputCode.toLocal8Bit();
	mShaderBindings.reset(inRhi->newShaderResourceBindings());
	mShaderBindings->setBindings(bindings.begin(), bindings.end());
	mShaderBindings->create();
}