#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"
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
	mStageInfos[inStage].mainCode = inCode;
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

void QRhiGraphicsPipelineBuilder::setInputAttribute(QVector<QRhiVertexInputAttributeEx> inInputAttributes) {
	mInputAttributes = inInputAttributes;
	mVertexInputLayout.setAttributes(inInputAttributes.begin(), inInputAttributes.end());
}

void QRhiGraphicsPipelineBuilder::setInputBindings(QVector<QRhiVertexInputBindingEx> inInputBindings) {
	mInputBindings = inInputBindings;
	mVertexInputLayout.setBindings(mInputBindings.begin(), mInputBindings.end());
}

QRhiUniformBlock* QRhiGraphicsPipelineBuilder::addUniformBlock(QRhiShaderStage::Type inStage, const QString& inName) {
	QSharedPointer<QRhiUniformBlock> unifromBlock = QSharedPointer<QRhiUniformBlock>::create();
	unifromBlock->setObjectName(inName);
	addUniformBlock(inStage, unifromBlock);
	return unifromBlock.get();
}

void QRhiGraphicsPipelineBuilder::addUniformBlock(QRhiShaderStage::Type inStage, QSharedPointer<QRhiUniformBlock> inUniformBlock) {
	mStageInfos[inStage].uniformBlocks << inUniformBlock;
	mUniformMap[inUniformBlock->objectName()] = inUniformBlock.get();
}

QRhiUniformBlock* QRhiGraphicsPipelineBuilder::getUniformBlock(const QString& inName) {
	return mUniformMap.value(inName);
}

void QRhiGraphicsPipelineBuilder::addMaterial(QSharedPointer<QRhiMaterialDesc> inDesc) {
	if (inDesc) {
		addUniformBlock(QRhiShaderStage::Type::Fragment, inDesc->uniformBlock);
		for (auto desc : inDesc->textureDescs) {
			addTextureDesc(QRhiShaderStage::Type::Fragment, desc);
		}
	}
}

void QRhiGraphicsPipelineBuilder::addTexture2D(QRhiShaderStage::Type inStage, const QString& inName, const QImage& inImage, QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW) {
	QSharedPointer<QRhiTextureDesc> textureInfo = QSharedPointer<QRhiTextureDesc>::create();
	textureInfo->Name = inName;
	textureInfo->ImageCache = inImage.convertedTo(QImage::Format_RGBA8888);
	textureInfo->Size = inImage.size();
	textureInfo->Format = QRhiTexture::RGBA8;
	textureInfo->GlslTypeName = "sampler2D";
	textureInfo->MagFilter = magFilter;
	textureInfo->MinFilter = minFilter;
	textureInfo->MipmapMode = mipmapMode;
	textureInfo->AddressU = addressU;
	textureInfo->AddressV = addressV;
	textureInfo->AddressW = addressW;
	textureInfo->UploadDesc = QRhiTextureUploadDescription({
		QRhiTextureUploadEntry(0,0,QRhiTextureSubresourceUploadDescription(textureInfo->ImageCache))
	});
	addTextureDesc(inStage, textureInfo);
}

void QRhiGraphicsPipelineBuilder::addTexture2D(QRhiShaderStage::Type inStage, const QString& inName, QRhiTexture::Format inFormat, const QSize& inSize, const QByteArray& inData, QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW) {
	QSharedPointer<QRhiTextureDesc> textureInfo = QSharedPointer<QRhiTextureDesc>::create();
	textureInfo->Name = inName;
	textureInfo->Size = inSize;
	textureInfo->Format = inFormat;
	textureInfo->GlslTypeName = "sampler2D";
	textureInfo->MagFilter = magFilter;
	textureInfo->MinFilter = minFilter;
	textureInfo->MipmapMode = mipmapMode;
	textureInfo->AddressU = addressU;
	textureInfo->AddressV = addressV;
	textureInfo->AddressW = addressW;
	textureInfo->UploadDesc = QRhiTextureUploadDescription({
		QRhiTextureUploadEntry(0,0,QRhiTextureSubresourceUploadDescription(inData))
	});
	addTextureDesc(inStage, textureInfo);
}

void QRhiGraphicsPipelineBuilder::addCubeMap(QRhiShaderStage::Type inStage, const QString& inName, QRhiTexture::Format inFormat, const QSize& inSize, const QVector<QByteArray>& inData, QRhiSampler::Filter magFilter /*= QRhiSampler::Filter::Linear*/, QRhiSampler::Filter minFilter /*= QRhiSampler::Filter::Nearest*/, QRhiSampler::Filter mipmapMode /*= QRhiSampler::Filter::Linear*/, QRhiSampler::AddressMode addressU /*= QRhiSampler::AddressMode::Repeat*/, QRhiSampler::AddressMode addressV /*= QRhiSampler::AddressMode::Repeat*/, QRhiSampler::AddressMode addressW /*= QRhiSampler::AddressMode::Repeat*/) {
	QSharedPointer<QRhiTextureDesc> textureInfo = QSharedPointer<QRhiTextureDesc>::create();
	textureInfo->Name = inName;
	textureInfo->Size = inSize;
	textureInfo->Format = inFormat;
	textureInfo->Flags = QRhiTexture::CubeMap | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips| QRhiTexture::UsedWithLoadStore;
	textureInfo->GlslTypeName = "samplerCube";
	textureInfo->MagFilter = magFilter;
	textureInfo->MinFilter = minFilter;
	textureInfo->MipmapMode = mipmapMode;
	textureInfo->AddressU = addressU;
	textureInfo->AddressV = addressV;
	textureInfo->AddressW = addressW;
	if (inData.size() == 6) {
		QRhiTextureSubresourceUploadDescription subresDesc[6];
		for (int i = 0; i < 6; i++) {
			subresDesc[i].setData(inData[i]);
		}
		textureInfo->UploadDesc = QRhiTextureUploadDescription({
			{ 0, 0, subresDesc[0] },  // +X
			{ 1, 0, subresDesc[1] },  // -X
			{ 2, 0, subresDesc[2] },  // +Y
			{ 3, 0, subresDesc[3] },  // -Y
			{ 4, 0, subresDesc[4] },  // +Z
			{ 5, 0, subresDesc[5] }   // -Z
		});
	}
	addTextureDesc(inStage, textureInfo);
}

void QRhiGraphicsPipelineBuilder::addTextureDesc(QRhiShaderStage::Type inStage, QSharedPointer<QRhiTextureDesc> inTexture) {
	mStageInfos[inStage].textureDescs << inTexture;
	mTextureMap[inTexture->Name] = inTexture.get();
}

void QRhiGraphicsPipelineBuilder::setTexture(const QString& inName, const QImage& inImage) {
	for (auto& stageInfo : mStageInfos) {
		for (auto& textureInfo : stageInfo.textureDescs) {
			if (textureInfo->Name == inName) {
				textureInfo->UploadDesc.setEntries({
					QRhiTextureUploadEntry(0,0,QRhiTextureSubresourceUploadDescription(inImage))
				});
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
	mBlendStates.resize(inRenderComponent->getBasePass()->getRenderTargetColorAttachments().size());
	QRhiEx* rhi = inRenderComponent->getBasePass()->getRenderer()->getRhi();
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
	mPipeline->setSampleCount(inRenderComponent->getBasePass()->getSampleCount());
	mPipeline->setDepthBias(mDepthBias);
	mPipeline->setSlopeScaledDepthBias(mSlopeScaledDepthBias);
	mPipeline->setPatchControlPointCount(mPatchControlPointCount);
	mPipeline->setPolygonMode(PolygonModeOverride == -1 ? mPolygonMode : (QRhiGraphicsPipeline::PolygonMode)PolygonModeOverride);
	mPipeline->setVertexInputLayout(mVertexInputLayout);
	mPipeline->setRenderPassDescriptor(inRenderComponent->getBasePass()->getRenderPassDescriptor());

	recreateShaderBindings(inRenderComponent, rhi);

	QVector<QRhiShaderStage> stages;
	for (const auto& stage : mStageInfos.asKeyValueRange()) {
		QShader shader = rhi->newShaderFromCode((QShader::Stage)stage.first, stage.second.versionCode + stage.second.defineCode + stage.second.mainCode);
		stages << QRhiShaderStage(stage.first, shader);
	}
	mPipeline->setShaderStages(stages.begin(), stages.end());
	mPipeline->setShaderResourceBindings(mShaderBindings.get());
	mPipeline->create();
	for (const auto& stage : mStageInfos) {
		for (const auto& uniformBlock : stage.uniformBlocks) {
			uniformBlock->sigRecreateBuffer.ensure();
		}
	}
	sigRebuild.ensure();
}

void QRhiGraphicsPipelineBuilder::update(QRhiResourceUpdateBatch* batch) {
	for (const auto& stage : mStageInfos) {
		for (const auto& textureInfo : stage.textureDescs) {
			if (textureInfo->sigUpdate.ensure()) {
				batch->uploadTexture(textureInfo->Texture.get(), textureInfo->UploadDesc);
			}
		}
		for (const auto& uniformBlock : stage.uniformBlocks) {
			if (uniformBlock->sigRecreateBuffer.ensure()) {
				sigRebuild.request();
				break;
			}
			uniformBlock->updateResource(batch);
		}
	}
}

void QRhiGraphicsPipelineBuilder::recreateShaderBindings(IRenderComponent* inRenderComponent, QRhiEx* inRhi) {
	for (auto& stage : mStageInfos) {
		stage.defineCode = QByteArray();
	}
	QString vertexInputCode;
	for (auto& input : mInputAttributes) {
		if (input.matrixSlice() == -1) {
			vertexInputCode += QString::asprintf("layout(location = %d) in %s %s;\n", input.location(), getInputFormatTypeName(input.format()).data(), input.mName.toLocal8Bit().data());
		}
		int matrixRows = 0;
		for (auto& item : mInputAttributes) {
			if (item.mName == input.mName) {
				matrixRows++;
			}
		}
		if (input.matrixSlice() == 0 && matrixRows == 4) {
			vertexInputCode += QString::asprintf("layout(location = %d) in mat4 %s;\n", input.location(), input.mName.toLocal8Bit().data());
		}
	}
	vertexInputCode += "out gl_PerVertex { vec4 gl_Position;}; \n";
	mStageInfos[QRhiShaderStage::Vertex].defineCode = vertexInputCode.toLocal8Bit();

	QVector<QRhiShaderResourceBinding> bindings;
	int bindingOffset = 0;
	for (const auto& stage : mStageInfos.asKeyValueRange()) {
		QString uniformDefineCode;
		for (auto& textureDesc : stage.second.textureDescs) {
			for (const auto& sampler : mSamplerList) {
				if (sampler->magFilter() == textureDesc->MagFilter
					&& sampler->minFilter() == textureDesc->MinFilter
					&& sampler->mipmapMode() == textureDesc->MipmapMode
					&& sampler->addressU() == textureDesc->AddressU
					&& sampler->addressV() == textureDesc->AddressV
					&& sampler->addressW() == textureDesc->AddressW
					) {
					textureDesc->Sampler = sampler;
					break;
				}
			}
			if (textureDesc->Sampler.isNull()) {
				textureDesc->Sampler.reset(inRhi->newSampler(textureDesc->MagFilter, textureDesc->MinFilter, textureDesc->MipmapMode, textureDesc->AddressU, textureDesc->AddressV, textureDesc->AddressW));
				mSamplerList << textureDesc->Sampler;
				textureDesc->Sampler->create();
			}
			if (textureDesc->Texture.isNull()
				|| textureDesc->Texture->format() != textureDesc->Format 
				|| textureDesc->Texture->sampleCount() != 1 
				|| textureDesc->Texture->flags() != textureDesc->Flags) {
				textureDesc->Texture.reset(inRhi->newTexture(textureDesc->Format, textureDesc->Size, 1, textureDesc->Flags));
				textureDesc->Texture->create();
			}
			bindings << QRhiShaderResourceBinding::sampledTexture(bindingOffset, (QRhiShaderResourceBinding::StageFlag)(1 << (int)stage.first), textureDesc->Texture.get(), textureDesc->Sampler.get());
			uniformDefineCode += QString("layout(binding =  %1) uniform %2 u%3;\n")
				.arg(bindingOffset)
				.arg(textureDesc->GlslTypeName)
				.arg(textureDesc->Name);
			bindingOffset++;
			textureDesc->sigUpdate.request();
		}
		for (const auto& uniformBlock : stage.second.uniformBlocks) {
			if (!uniformBlock->isEmpty()) {
				uniformBlock->create(inRhi);
				bindings << QRhiShaderResourceBinding::uniformBuffer(bindingOffset, (QRhiShaderResourceBinding::StageFlag)(1 << (int)stage.first), uniformBlock->getUniformBlock());
				uniformDefineCode += uniformBlock->createDefineCode(bindingOffset);
				bindingOffset++;
			}
		}
		stage.second.defineCode += uniformDefineCode.toLocal8Bit();
	}
	QString fragOutputCode;
	auto targetSlots = inRenderComponent->getBasePass()->getRenderTargetColorAttachments();
	for (int i = 0; i < targetSlots.size(); i++) {
		QByteArray slotType = getOutputFormatTypeName(targetSlots[i].first);
		QByteArray slotName = targetSlots[i].second.toLocal8Bit();
		fragOutputCode += QString::asprintf("layout(location = %d) out %s %s;\n", i, slotType.data(), slotName.data());
	}
	mStageInfos[QRhiShaderStage::Fragment].defineCode += fragOutputCode.toLocal8Bit();
	mShaderBindings.reset(inRhi->newShaderResourceBindings());
	mShaderBindings->setBindings(bindings.begin(), bindings.end());
	mShaderBindings->create();
}