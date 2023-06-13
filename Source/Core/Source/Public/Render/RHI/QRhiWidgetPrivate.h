// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RHIWIDGET_P_H
#define RHIWIDGET_P_H

#include "QRhiWidget.h"

#include <private/qwidget_p.h>
#include <private/qbackingstorerhisupport_p.h>

class QRhiWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QRhiWidget)
public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	TextureData texture() const override {
		TextureData td;
		if (!textureInvalid)
			td.textureLeft = t;
		return td;
	}
#else
	QRhiTexture* texture() const override {
		return textureInvalid ? nullptr : t;
	}
#endif
    QPlatformBackingStoreRhiConfig rhiConfig() const override;

    void ensureRhi();
    void ensureTexture();

    QRhi *rhi = nullptr;
    QRhiTexture *t = nullptr;
    bool noSize = false;
    QPlatformBackingStoreRhiConfig config;
    QRhiTexture::Format format = QRhiTexture::RGBA8;
    QSize explicitSize;
    QBackingStoreRhiSupport offscreenRenderer;
    bool textureInvalid = false;
};

#endif
