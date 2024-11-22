#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include "vk_mem_alloc_6_8_0.h"
#elif QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#include "vk_mem_alloc_6_7_0.h"
#else
#error This Qt Version Not Supported
#endif