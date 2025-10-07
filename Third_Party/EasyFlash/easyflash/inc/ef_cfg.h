/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the configure head file for this library.
 * Created on: 2015-07-14
 */
#include "main.h"
#ifndef EF_CFG_H_
#define EF_CFG_H_

/* using ENV function, default is NG (Next Generation) mode start from V4.0 */
#define EF_USING_ENV

#ifdef EF_USING_ENV // 当 ENV 初始化时，如果检测到产品存储的版本号与设定版本号不一致，会自动追加默认环境变量集合中新增的环境变量。该功能非常适用于经常升级的产品中，当产品功能变更时，有可能会新增环境变量，此时只需要增大当前设定的 ENV 版本号，下次固件升级后，新增的环境变量将会自动追加上去。
/* Auto update ENV to latest default when current ENV version number is changed. */
#define EF_ENV_AUTO_UPDATE
/**
 * ENV version number defined by user.
 * Please change it when your firmware add a new ENV to default_env_set.
 */
#define EF_ENV_VER_NUM 0 /* @note you must define it for a value, such as 0 */ // 该配置依赖于 EF_ENV_AUTO_UPDATE 配置。设置的环境变量版本号为整形数值，可以从 0 开始。如果在默认环境变量表中增加了环境变量，此时需要对该配置进行修改（通常加 1 ）。

/* MCU Endian Configuration, default is Little Endian Order. */
/* #define EF_BIG_ENDIAN  */

#endif /* EF_USING_ENV */

/* using IAP function */
#define EF_USING_IAP

/* using save log function */
#define EF_USING_LOG

/* The minimum size of flash erasure. May be a flash sector size. */
#define EF_ERASE_MIN_SIZE USER_ERASE_MIN_SIEZ /* @note you must define it for a value */

/* the flash write granularity, unit: bit
 * only support 1(nor flash)/ 8(stm32f4)/ 32(stm32f1) */
#define EF_WRITE_GRAN USER_WRITE_GRAN /* @note you must define it for a value */

/*
 *
 * This all Backup Area Flash storage index. All used flash area configure is under here.
 * |----------------------------|   Storage Size
 * | Environment variables area |   ENV area size @see ENV_AREA_SIZE
 * |----------------------------|
 * |      Saved log area        |   Log area size @see LOG_AREA_SIZE
 * |----------------------------|
 * |(IAP)Downloaded application |   IAP already downloaded application, unfixed size
 * |----------------------------|
 *
 * @note all area sizes must be aligned with EF_ERASE_MIN_SIZE
 *
 * The EasyFlash add the NG (Next Generation) mode start from V4.0. All old mode before V4.0, called LEGACY mode.
 *
 * - NG (Next Generation) mode is default mode from V4.0. It's easy to settings, only defined the ENV_AREA_SIZE.
 * - The LEGACY mode has been DEPRECATED. It is NOT RECOMMENDED to continue using.
 *   Beacuse it will use ram to buffer the ENV and spend more flash erase times.
 *   If you want use it please using the V3.X version.
 */

/* backup area start address 备份区起始地址 备份区共计包含3个区域，依次为：环境变量区(容量：ENV_AREA_SIZE)、日志区(容量：LOG_AREA_SIZE)及在线升级区(容量：备份区剩余空间)。 */
#define EF_START_ADDR USER_START_ADDR /* @note you must define it for a value */

/* ENV area size. It's at least one empty sector for GC. So it's definition must more then or equal 2 flash sector size. */
#define ENV_AREA_SIZE (2 * EF_ERASE_MIN_SIZE) /* 256K */ /* @note you must define it for a value if you used ENV */

/* saved log area size */
#define LOG_AREA_SIZE (2 * EF_ERASE_MIN_SIZE) /* 256K */ /* @note you must define it for a value if you used log */

/* print debug information of flash */
#define PRINT_DEBUG

#endif /* EF_CFG_H_ */
