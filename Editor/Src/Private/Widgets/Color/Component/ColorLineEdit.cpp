/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 * \copyright Copyright (C) 2017 caryoscelus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QApplication>
#include <QPainter>
#include <QStyleOptionFrame>
#include "ColorLineEdit.hpp"
#include "ColorNames.hpp"
#include "ColorUtils.hpp"


class ColorLineEdit::Private
{
public:
	QColor color;
	bool show_alpha = false;
	bool preview_color = false;
	QBrush background;

	bool customAlpha()
	{
		return preview_color && show_alpha && color.alpha() < 255;
	}

	void setPalette(const QColor& color, ColorLineEdit* parent)
	{
		if (preview_color)
		{
			QColor bg = customAlpha() ? Qt::transparent : color;
			QColor text = utils::color_lumaF(color) > 0.5 || color.alphaF() < 0.2 ? Qt::black : Qt::white;
			parent->setStyleSheet(
				QString("background-color: %1; color: %2;")
				.arg(bg.name()).arg(text.name())
			);
		}
	}
};

ColorLineEdit::ColorLineEdit(QWidget* parent)
	: QLineEdit(parent), p(new Private)
{
	p->background.setTexture(QPixmap(QStringLiteral(":/Resources/Icons/alphaback")));
	SetColor(Qt::white);
	/// \todo determine if having this connection might be useful
	/*connect(this, &QLineEdit::textChanged, [this](const QString& text){
		QColor color = p->colorFromString(text);
		if ( color.isValid() )
			Q_EMIT colorChanged(color);
	});*/
	connect(this, &QLineEdit::textEdited, [this](const QString& text) {
		QColor color = colorFromString(text, p->show_alpha);
		if (color.isValid())
		{
			p->color = color;
			p->setPalette(color, this);
			Q_EMIT colorEdited(color);
			Q_EMIT OnColorChanged(color);
		}
	});
	connect(this, &QLineEdit::editingFinished, [this]() {
		QColor color = colorFromString(text(), p->show_alpha);
		if (color.isValid())
		{
			p->color = color;
			Q_EMIT colorEditingFinished(color);
			Q_EMIT OnColorChanged(color);
		}
		else
		{
			setText(stringFromColor(p->color, p->show_alpha));
			Q_EMIT colorEditingFinished(p->color);
			Q_EMIT OnColorChanged(color);
		}
		p->setPalette(p->color, this);
	});
}

ColorLineEdit::~ColorLineEdit()
{
	delete p;
}

QColor ColorLineEdit::color() const
{
	return p->color;
}

void ColorLineEdit::SetColor(const QColor& color)
{
	if (color != p->color)
	{
		p->color = color;
		p->setPalette(p->color, this);
		setText(stringFromColor(p->color, p->show_alpha));
		Q_EMIT OnColorChanged(p->color);
	}
}

void ColorLineEdit::setShowAlpha(bool showAlpha)
{
	if (p->show_alpha != showAlpha)
	{
		p->show_alpha = showAlpha;
		p->setPalette(p->color, this);
		setText(stringFromColor(p->color, p->show_alpha));
		Q_EMIT showAlphaChanged(p->show_alpha);
	}
}

bool ColorLineEdit::showAlpha() const
{
	return p->show_alpha;
}

void ColorLineEdit::dragEnterEvent(QDragEnterEvent* event)
{
	if (isReadOnly())
		return;

	if (event->mimeData()->hasColor() ||
		(event->mimeData()->hasText() &&
		colorFromString(event->mimeData()->text(), p->show_alpha).isValid()))
	{
		event->acceptProposedAction();
	}
}

void ColorLineEdit::dropEvent(QDropEvent* event)
{
	if (isReadOnly())
		return;

	if (event->mimeData()->hasColor())
	{
		SetColor(event->mimeData()->colorData().value<QColor>());
		event->accept();
	}
	else if (event->mimeData()->hasText())
	{
		QColor col = colorFromString(event->mimeData()->text(), p->show_alpha);
		if (col.isValid())
		{
			SetColor(col);
			event->accept();
		}
	}
}

bool ColorLineEdit::previewColor() const
{
	return p->preview_color;
}

void ColorLineEdit::setPreviewColor(bool previewColor)
{
	if (previewColor != p->preview_color)
	{
		p->preview_color = previewColor;

		if (p->preview_color)
			p->setPalette(p->color, this);
		else
			setPalette(QApplication::palette());

		Q_EMIT previewColorChanged(p->preview_color);
	}
}

void ColorLineEdit::paintEvent(QPaintEvent* event)
{
	if (p->customAlpha())
	{
		QPainter painter(this);
		QStyleOptionFrame panel;
		initStyleOption(&panel);
		QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, nullptr);
		painter.fillRect(r, p->background);
		painter.fillRect(r, p->color);
	}

	QLineEdit::paintEvent(event);
}
