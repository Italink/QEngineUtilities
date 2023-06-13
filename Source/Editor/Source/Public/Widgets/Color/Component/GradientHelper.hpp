#ifndef GRADIENT_HELPER_HPP
#define GRADIENT_HELPER_HPP

#include <QGradient>

inline QColor blendColors(const QColor& a, const QColor& b, qreal ratio)
{
	return QColor::fromRgbF(
		a.redF() * (1 - ratio) + b.redF() * ratio,
		a.greenF() * (1 - ratio) + b.greenF() * ratio,
		a.blueF() * (1 - ratio) + b.blueF() * ratio,
		a.alphaF() * (1 - ratio) + b.alphaF() * ratio
	);
}

/**
 * \brief Get an insertion point in the gradient
 * \param gradient  Gradient stops to look into (must be properly set up)
 * \param factor    Value in [0, 1] to get the color for
 * \return A pair whose first element is the index to insert the new value at, and a GradientStop
 */
inline QPair<int, QGradientStop> Q_DECL_EXPORT gradientBlendedColorInsert(const QGradientStops& gradient, qreal factor)
{
	if (gradient.empty())
		return { 0, {0, QColor()} };

	if (gradient.size() == 1 || factor <= 0)
		return { 0, gradient.front() };

	int i = 0;
	QGradientStop s1 = gradient.front();
	s1.first = 0;
	for (auto s2 : gradient)
	{
		if (factor < s2.first)
		{
			qreal ratio = (factor - s1.first) / (s2.first - s1.first);
			return { i, {factor, blendColors(s1.second, s2.second, ratio)} };
		}
		s1 = s2;
		++i;
	}

	return { gradient.size(), gradient.back() };
}

/**
 * \brief Returns a color in the gradient
 * \param gradient  Gradient stops to look into (must be properly set up)
 * \param factor    Value in [0, 1] to get the color for
 */
inline QColor Q_DECL_EXPORT gradientBlendedColor(const QGradientStops& gradient, qreal factor)
{
	return gradientBlendedColorInsert(gradient, factor).second.second;
}

/**
 * \brief Returns a color in the gradient
 * \param gradient  Gradient to look into
 * \param factor    Value in [0, 1] to get the color for
 */
inline QColor Q_DECL_EXPORT gradientBlendedColor(const QGradient& gradient, qreal factor)
{
	return gradientBlendedColor(gradient.stops(), factor);
}

#endif // GRADIENT_HELPER_HPP