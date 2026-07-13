/**
 * stdint.h — C51 适配
 *
 * Keil C51 不自带 <stdint.h>，但 u8g2 依赖它。
 * 此文件提供标准整数类型定义，供 u8g2 编译使用。
 *
 * 注意: WS51F6240.h 中也定义了 uint8_t/uint16_t/uint32_t，
 * 但该文件不会被 u8g2 的 .c 文件直接 include。
 * 为避免重复定义警告，此处使用 #ifndef 保护。
 */
#ifndef _STDINT_H_
#define _STDINT_H_

#ifndef _UINT8_T_DEFINED
#define _UINT8_T_DEFINED
typedef unsigned char  uint8_t;
#endif

#ifndef _UINT16_T_DEFINED
#define _UINT16_T_DEFINED
typedef unsigned int   uint16_t;
#endif

#ifndef _UINT32_T_DEFINED
#define _UINT32_T_DEFINED
typedef unsigned long  uint32_t;
#endif

#ifndef _INT8_T_DEFINED
#define _INT8_T_DEFINED
typedef signed char    int8_t;
#endif

#ifndef _INT16_T_DEFINED
#define _INT16_T_DEFINED
typedef signed int     int16_t;
#endif

#ifndef _INT32_T_DEFINED
#define _INT32_T_DEFINED
typedef signed long    int32_t;
#endif

/* C51 指针: 通用指针 3 字节 (generic pointer) */
#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
typedef unsigned int   uintptr_t;
#endif

/* 其他可能用到的宏 */
#define INT8_MIN    (-128)
#define INT8_MAX    127
#define UINT8_MAX   255
#define INT16_MIN   (-32768)
#define INT16_MAX   32767
#define UINT16_MAX  65535
#define INT32_MIN   (-2147483647L - 1L)
#define INT32_MAX   2147483647L
#define UINT32_MAX  4294967295UL

#endif /* _STDINT_H_ */
