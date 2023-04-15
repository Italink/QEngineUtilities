#include "QRhiUniformBlock.h"
#include "Render/IRenderComponent.h"
#include "Render/IRenderPass.h"

QRhiUniformBlock::QRhiUniformBlock(QObject* inParent)
	: QObject(inParent)
{
}

void QRhiUniformBlock::setParamValue(const QString& name, QVariant value) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		(*Iter)->setValue(value);
		(*Iter)->sigUpdate.request();
	}
}

void QRhiUniformBlock::removeParam(const QString& name)
{
	auto iter = mParamNameMap.find(name);
	if (iter != mParamNameMap.end()) {
		mParamList.removeOne(*iter);
		mParamNameMap.remove(name);
		sigRecreateBuffer.request();
	}
}

bool QRhiUniformBlock::renameParma(const QString& src, const QString& dst)
{
	if (mParamNameMap.contains(src)) {
		auto param = mParamNameMap.take(src);
		param->mName = dst;
		mParamNameMap[dst] = param;
		sigRecreateBuffer.request();
		return true;
	}
	return false;
}


QString QRhiUniformBlock::getVaildName(QString name) {
	QString newName = name;
	int index = 0;
	while (mParamNameMap.contains(newName)) {
		newName = name + QString::number(index);
	}
	return newName;
}

int align(int size, int alignSize) {
	if (size == alignSize)
		return size;
	return (size + alignSize - 1) & ~(alignSize - 1);
}

void QRhiUniformBlock::updateLayout() {
	mDataByteSize = 0;
	for (int i = 0; i < mParamList.size(); i++) {
		auto& paramDesc = mParamList[i];
		paramDesc->mOffsetInByte = align(mDataByteSize, paramDesc->dataAlignSize());
		paramDesc->mSizeInByte = paramDesc->dataByteSize();
		paramDesc->mSizeInByteAligned = paramDesc->mSizeInByte;	//16字节对齐
		paramDesc->sigUpdate.request();
		mDataByteSize = paramDesc->mOffsetInByte + paramDesc->mSizeInByteAligned;
	}
}

void QRhiUniformBlock::create(QRhiEx* inRhi) {
	updateLayout();
	if (mUniformBlock.isNull() || mUniformBlock->size() != mDataByteSize) {
		mUniformBlock.reset(inRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, mDataByteSize));
		mUniformBlock->create();
		for (auto& dataParam : mParamList) {
			dataParam->sigRecreate.ensure();
			dataParam->sigUpdate.request();
		}
	}
}

void QRhiUniformBlock::updateResource(QRhiResourceUpdateBatch* batch) {
	for (auto& dataParam : mParamList) {
		if (dataParam->sigRecreate.ensure()) {
			sigRecreateBuffer.request();
			return;
		}
		if (dataParam->sigUpdate.ensure()) {
			batch->updateDynamicBuffer(mUniformBlock.get(), dataParam->mOffsetInByte, dataParam->mSizeInByte, dataParam->dataPtr());
		}
	}
}

QSharedPointer<UniformParamDescBase> QRhiUniformBlock::getParamDesc(const QString& inName) {
	return mParamNameMap.value(inName);
}

QByteArray QRhiUniformBlock::createDefineCode(int inBindingOffset) {
	if (mParamList.isEmpty())
		return "";
	QString defineCode = QString("layout(binding =  %1) uniform %2Block{\n").arg(inBindingOffset).arg(objectName());
	for (auto& param : mParamList) {
		defineCode += QString("    %1 %2;\n").arg(param->typeName()).arg(param->valueName());
	}
	defineCode += QString::asprintf("}%s;\n", objectName().toLocal8Bit().data());
	return defineCode.toLocal8Bit();
}
