#ifndef DebugUtils_h__
#define DebugUtils_h__

namespace DebugUtils {
	inline QVector4D convertIdToVec4(uint32_t inId) {
		uint32_t r = (inId & 0x000000FF) >> 0;
		uint32_t g = (inId & 0x0000FF00) >> 8;
		uint32_t b = (inId & 0x00FF0000) >> 16;
		uint32_t a = (inId & 0xFF000000) >> 24;
		return QVector4D(r, g, b, a) / 255.0f;;
	}
	inline QString convertIdToVec4Code(uint32_t inId) {
		QVector4D ID = convertIdToVec4(inId);
		return QString("vec4(%1,%2,%3,%4)").arg(ID.x()).arg(ID.y()).arg(ID.z()).arg(ID.w());
	}
}

#define QLogWarn(Message) qWarning()<< __TIME__ << "|" << __FILE__ << "|" << __LINE__ << "|" << __FUNCTION__ << ":" << Message;


#endif // DebugUtils_h__
