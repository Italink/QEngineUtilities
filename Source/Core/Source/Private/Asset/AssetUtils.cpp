#include "Asset/AssetUtils.h"
#include "QImage"

QMatrix4x4 AssetUtils::converter(const aiMatrix4x4& aiMat4) {
	QMatrix4x4 mat4;
	const ai_real* dataPtr = (const ai_real*)&aiMat4;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			mat4(i, j) = *(dataPtr + (4 * i) + j);
		}
	}
	return mat4;
}

QVector3D AssetUtils::converter(const aiVector3D& aiVec3) {
	return QVector3D(aiVec3.x, aiVec3.y, aiVec3.z);
}

QByteArray AssetUtils::loadHdr(const QString& fn, QSize* size) {
	QFile f(fn);
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning("Failed to open %s", qPrintable(fn));
		return QByteArray();
	}

	char sig[256];
	f.read(sig, 11);
	if (strncmp(sig, "#?RADIANCE\n", 11))
		return QByteArray();

	QByteArray buf = f.readAll();
	const char* p = buf.constData();
	const char* pEnd = p + buf.size();

	// Process lines until the empty one.
	QByteArray line;
	while (p < pEnd) {
		char c = *p++;
		if (c == '\n') {
			if (line.isEmpty())
				break;
			if (line.startsWith(QByteArrayLiteral("FORMAT="))) {
				const QByteArray format = line.mid(7).trimmed();
				if (format != QByteArrayLiteral("32-bit_rle_rgbe")) {
					qWarning("HDR format '%s' is not supported", format.constData());
					return QByteArray();
				}
			}
			line.clear();
		}
		else {
			line.append(c);
		}
	}
	if (p == pEnd) {
		qWarning("Malformed HDR image data at property strings");
		return QByteArray();
	}

	// Get the resolution string.
	while (p < pEnd) {
		char c = *p++;
		if (c == '\n')
			break;
		line.append(c);
	}
	if (p == pEnd) {
		qWarning("Malformed HDR image data at resolution string");
		return QByteArray();
	}

	int w = 0, h = 0;
	// We only care about the standard orientation.
	if (!sscanf(line.constData(), "-Y %d +X %d", &h, &w)) {
		qWarning("Unsupported HDR resolution string '%s'", line.constData());
		return QByteArray();
	}
	if (w <= 0 || h <= 0) {
		qWarning("Invalid HDR resolution");
		return QByteArray();
	}

	// output is RGBA32F
	const int blockSize = 4 * sizeof(float);
	QByteArray data;
	data.resize(w * h * blockSize);

	typedef unsigned char RGBE[4];
	RGBE* scanline = new RGBE[w];

	for (int y = 0; y < h; ++y) {
		if (pEnd - p < 4) {
			qWarning("Unexpected end of HDR data");
			delete[] scanline;
			return QByteArray();
		}

		scanline[0][0] = *p++;
		scanline[0][1] = *p++;
		scanline[0][2] = *p++;
		scanline[0][3] = *p++;

		if (scanline[0][0] == 2 && scanline[0][1] == 2 && scanline[0][2] < 128) {
			// new rle, the first pixel was a dummy
			for (int channel = 0; channel < 4; ++channel) {
				for (int x = 0; x < w && p < pEnd; ) {
					unsigned char c = *p++;
					if (c > 128) { // run
						if (p < pEnd) {
							int repCount = c & 127;
							c = *p++;
							while (repCount--)
								scanline[x++][channel] = c;
						}
					}
					else { // not a run
						while (c-- && p < pEnd)
							scanline[x++][channel] = *p++;
					}
				}
			}
		}
		else {
			// old rle
			scanline[0][0] = 2;
			int bitshift = 0;
			int x = 1;
			while (x < w && pEnd - p >= 4) {
				scanline[x][0] = *p++;
				scanline[x][1] = *p++;
				scanline[x][2] = *p++;
				scanline[x][3] = *p++;

				if (scanline[x][0] == 1 && scanline[x][1] == 1 && scanline[x][2] == 1) { // run
					int repCount = scanline[x][3] << bitshift;
					while (repCount--) {
						memcpy(scanline[x], scanline[x - 1], 4);
						++x;
					}
					bitshift += 8;
				}
				else { // not a run
					++x;
					bitshift = 0;
				}
			}
		}

		// adjust for -Y orientation
		float* fp = reinterpret_cast<float*>(data.data() + (h - 1 - y) * blockSize * w);
		for (int x = 0; x < w; ++x) {
			float d = qPow(2.0f, float(scanline[x][3]) - 128.0f);
			// r, g, b, a
			*fp++ = scanline[x][0] / 256.0f * d;
			*fp++ = scanline[x][1] / 256.0f * d;
			*fp++ = scanline[x][2] / 256.0f * d;
			*fp++ = 1.0f;
		}
	}

	delete[] scanline;

	if (size)
		*size = QSize(w, h);

	return data;
}

QSize AssetUtils::resolveCubeImageFaceSize(const QImage& inImage) {
	if (inImage.width() * 3 == 4 * inImage.height())
		return QSize(inImage.width() / 4, inImage.width() / 4);
	else if (inImage.width() * 4 == 3 * inImage.height())
		return QSize(inImage.width() / 3, inImage.width() / 3);
	else
		return QSize();
}

std::array<QImage, 6> AssetUtils::resolveCubeSubImages(const QImage& inImage) {
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
