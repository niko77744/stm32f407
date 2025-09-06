/**
 * @file lv_conf.h
 * Configuration file for v8.4.0
 */

/*
 * Copy this file as `lv_conf.h`
 * 1. simply next to the `lvgl` folder
 * 2. or any other places and
 *    - define `LV_CONF_INCLUDE_SIMPLE`
 *    - add the path as include path
 */

/* clang-format off */
#if 1 /*Set it to "1" to enable content*/

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS ��ɫ����
 *====================*/

/*Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
/* ��ɫ���: 1(ÿ����1�ֽ�), 8(RGB332), 16(RGB565), 32(ARGB8888) */
#define LV_COLOR_DEPTH 16

/*Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI)*/
/* ����2�ֽڵ�RGB565��ɫ�������ʾ��8λ�ӿ�(����SPI) */
#define LV_COLOR_16_SWAP 0

/*Enable features to draw on transparent background.
 *It's required if opa, and transform_* style properties are used.
 *Can be also used if the UI is above another layer, e.g. an OSD menu or video player.*/
/* 1: ������Ļ͸��.
 * ��OSD���������ص���gui������.
 * Ҫ��' LV_COLOR_DEPTH = 32 '��ɫ����Ļ����ʽӦ�ñ��޸�: `style.body.opa = ...`*/
#define LV_COLOR_SCREEN_TRANSP 0

/* Adjust color mix functions rounding. GPUs might calculate color mix (blending) differently.
 * 0: round down, 64: round up from x.75, 128: round up from half, 192: round up from x.25, 254: round up */
#define LV_COLOR_MIX_ROUND_OFS 0

/*Images pixels with this color will not be drawn if they are chroma keyed)*/
/* ���ʹ��ɫ�ȼ������������������ɫ��ͼ������) */
#define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00)         /*pure green*/

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*1: use custom malloc/free, 0: use the built-in `lv_mem_alloc()` and `lv_mem_free()`*/
/* 0: ʹ�����õ� `lv_mem_alloc()` �� `lv_mem_free()`*/
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
    /*Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB)*/
    /* `lv_mem_alloc()`�ɻ�õ��ڴ��С(���ֽ�Ϊ��λ)(>= 2kB) */
    #define LV_MEM_SIZE (48U * 1024U)          /*[bytes]*/

    /*Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too.*/
    #define LV_MEM_ADR 0     /*0: unused*/
    /*Instead of an address give a memory allocator that will be called to get a memory pool for LVGL. E.g. my_malloc*/
    #if LV_MEM_ADR == 0
        #undef LV_MEM_POOL_INCLUDE
        #undef LV_MEM_POOL_ALLOC
    #endif

#else       /*LV_MEM_CUSTOM*/
    #define LV_MEM_CUSTOM_INCLUDE <stdlib.h>   /*Header for the dynamic memory function*/
    #define LV_MEM_CUSTOM_ALLOC   malloc
    #define LV_MEM_CUSTOM_FREE    free
    #define LV_MEM_CUSTOM_REALLOC realloc
#endif     /*LV_MEM_CUSTOM*/

/*Number of the intermediate memory buffer used during rendering and other internal processing mechanisms.
 *You will see an error log message if there wasn't enough buffers. */
/* ����Ⱦ�������ڲ���������ڼ�ʹ�õ��м��ڴ滺������������
 * ���û���㹻�Ļ���������ῴ��һ��������־��Ϣ. */
#define LV_MEM_BUF_MAX_NUM 16

/*Use the standard `memcpy` and `memset` instead of LVGL's own functions. (Might or might not be faster).*/
/* ʹ�ñ�׼�� `memcpy` �� `memset` ����LVGL�Լ��ĺ�����(���ܸ��죬Ҳ���ܲ������) */
#define LV_MEMCPY_MEMSET_STD 0

/*====================
   HAL SETTINGS
 *====================*/

/*Default display refresh period. LVG will redraw changed areas with this period time*/
/* Ĭ�ϵ���ʾˢ�����ڡ�LVGLʹ����������ػ��޸Ĺ������� */
#define LV_DISP_DEF_REFR_PERIOD 30      /*[ms]*/

/*Input device read period in milliseconds*/
/* �����豸�Ķ�ȡ����(�Ժ���Ϊ��λ) */
#define LV_INDEV_DEF_READ_PERIOD 30     /*[ms]*/

/*Use a custom tick source that tells the elapsed time in milliseconds.
 *It removes the need to manually update the tick with `lv_tick_inc()`)*/
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE          "FreeRTOS.h"                /* ϵͳʱ�亯��ͷ */
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR    (xTaskGetTickCount())       /* ����ϵͳ��ǰʱ��ı��ʽ(�Ժ���Ϊ��λ) */
#endif   /*LV_TICK_CUSTOM*/

/*Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 *(Not so important, you can adjust it to modify default sizes and spaces)*/
/* Ĭ��ÿӢ��ĵ����������ڳ�ʼ��Ĭ�ϴ�С������С������С����ʽ��䡣
 * (���Ǻ���Ҫ������Ե��������޸�Ĭ�ϴ�С�Ϳո�) */
#define LV_DPI_DEF 130     /*[px/inch]*/

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/

/*-------------
 * Drawing
 *-----------*/

/*Enable complex draw engine.
 *Required to draw shadow, gradient, rounded corners, circles, arc, skew lines, image transformations or any masks*/
/* ���ø��ӵĻ�������
 * ��Ҫ������Ӱ���ݶȣ�Բ�ǣ�Բ������б�ߣ�ͼ��ת�����κ����� */
#define LV_DRAW_COMPLEX 1
#if LV_DRAW_COMPLEX != 0

    /*Allow buffering some shadow calculation.
    *LV_SHADOW_CACHE_SIZE is the max. shadow size to buffer, where shadow size is `shadow_width + radius`
    *Caching has LV_SHADOW_CACHE_SIZE^2 RAM cost*/
    /* ������һЩ��Ӱ����
     * LV_SHADOW_CACHE_SIZEΪ���Ļ����С�������СΪ `��Ӱ��� + �뾶`
     * ������ LV_SHADOW_CACHE_SIZE^2 ���ڴ濪�� */
    #define LV_SHADOW_CACHE_SIZE 0

    /* Set number of maximally cached circle data.
    * The circumference of 1/4 circle are saved for anti-aliasing
    * radius * 4 bytes are used per circle (the most often used radiuses are saved)
    * 0: to disable caching */
    /* ������󻺴�ѭ�����ݵ�������
     * ����1/4Բ���ܳ����ڿ����
     * �뾶*ÿ��Բʹ��4���ֽ�(������õİ뾶)
     * 0:���û��� */
    #define LV_CIRCLE_CACHE_SIZE 4
#endif /*LV_DRAW_COMPLEX*/

/**
 * "Simple layers" are used when a widget has `style_opa < 255` to buffer the widget into a layer
 * and blend it as an image with the given opacity.
 * Note that `bg_opa`, `text_opa` etc don't require buffering into layer)
 * The widget can be buffered in smaller chunks to avoid using large buffers.
 *
 * - LV_LAYER_SIMPLE_BUF_SIZE: [bytes] the optimal target buffer size. LVGL will try to allocate it
 * - LV_LAYER_SIMPLE_FALLBACK_BUF_SIZE: [bytes]  used if `LV_LAYER_SIMPLE_BUF_SIZE` couldn't be allocated.
 *
 * Both buffer sizes are in bytes.
 * "Transformed layers" (where transform_angle/zoom properties are used) use larger buffers
 * and can't be drawn in chunks. So these settings affects only widgets with opacity.
 */
#define LV_LAYER_SIMPLE_BUF_SIZE          (24 * 1024)
#define LV_LAYER_SIMPLE_FALLBACK_BUF_SIZE (3 * 1024)

/*Default image cache size. Image caching keeps the images opened.
 *If only the built-in image formats are used there is no real advantage of caching. (I.e. if no new image decoder is added)
 *With complex image decoders (e.g. PNG or JPG) caching can save the continuous open/decode of images.
 *However the opened images might consume additional RAM.
 *0: to disable caching*/
/* Ĭ��ͼ�񻺴��С��ͼ�񻺴汣��ͼ��򿪡�
 * ���ֻʹ�����õ�ͼ���ʽ������û�����������ơ�(��û������µ�ͼ�������)
 * ���ӵ�ͼ�������(��PNG��JPG)������Ա���������/�����ͼ��Ȼ�����򿪵�ͼ����ܻ����Ķ����RAM��
 * 0:���û��� */
#define LV_IMG_CACHE_DEF_SIZE 0

/*Number of stops allowed per gradient. Increase this to allow more stops.
 *This adds (sizeof(lv_color_t) + 1) bytes per additional stop*/
/* ÿ���¶�����ͣ������Ŀ���������ֵ���������ͣվ��
 * ÿ�������ֹͣ����(sizeof(lv_color_t) + 1)�ֽ� */
#define LV_GRADIENT_MAX_STOPS 2

/*Default gradient buffer size.
 *When LVGL calculates the gradient "maps" it can save them into a cache to avoid calculating them again.
 *LV_GRAD_CACHE_DEF_SIZE sets the size of this cache in bytes.
 *If the cache is too small the map will be allocated only while it's required for the drawing.
 *0 mean no caching.*/
/* Ĭ���ݶȻ�������С��
 * ��LVGL�����ݶȡ���ͼ���������Խ����Ǳ��浽���棬�Ա����ٴμ������ǡ�
 * LV_GRAD_CACHE_DEF_SIZE���û���Ĵ�С(���ֽ�Ϊ��λ)��
 * �������̫С����ͼֻ������Ҫ���Ƶ�ʱ�򱻷��䡣
 * 0��ʾû�л���*/
#define LV_GRAD_CACHE_DEF_SIZE 0

/*Allow dithering the gradients (to achieve visual smooth color gradients on limited color depth display)
 *LV_DITHER_GRADIENT implies allocating one or two more lines of the object's rendering surface
 *The increase in memory consumption is (32 bits * object width) plus 24 bits * object width if using error diffusion */
/* ����������(�����޵���ɫ�����ʾ��ʵ���Ӿ�ƽ������ɫ����)
 * LV_DITHER_GRADIENT��ζ�ŷ��������Ⱦ�����һ����������
 * �ڴ����ĵ�������(32λ*������)����24λ*���������ʹ�ô�����ɢ */
#define LV_DITHER_GRADIENT 0
#if LV_DITHER_GRADIENT
    /*Add support for error diffusion dithering.
     *Error diffusion dithering gets a much better visual result, but implies more CPU consumption and memory when drawing.
     *The increase in memory consumption is (24 bits * object's width)*/
    /* �����˴�����ɢ������֧�֡�
     * ������ɢ�����õ��˸��õ��Ӿ�Ч�������ڻ���ʱ��ζ�Ÿ����CPU���ڴ����ġ��ڴ���������(24λ*����Ŀ��) */
    #define LV_DITHER_ERROR_DIFFUSION 0
#endif

/*Maximum buffer size to allocate for rotation.
 *Only used if software rotation is enabled in the display driver.*/
/* Ϊ��ת�������󻺳�����С��������ʾ�������������������תʱʹ�� */
#define LV_DISP_ROT_MAX_BUF (10*1024)

/*-------------
 * GPU
 *-----------*/

/*Use Arm's 2D acceleration library Arm-2D */
#define LV_USE_GPU_ARM2D 0

/*Use STM32's DMA2D (aka Chrom Art) GPU*/
/* ʹ��STM32��DMA2D(����Chrom Art) GPU */
#define LV_USE_GPU_STM32_DMA2D 0
#if LV_USE_GPU_STM32_DMA2D
    /*Must be defined to include path of CMSIS header of target processor
    e.g. "stm32f7xx.h" or "stm32f4xx.h"*/
    /* ���붨�����Ŀ�괦������CMSISͷ��·��
       �硣��stm32f769xx.h����stm32f429xx.h��*/
    #define LV_GPU_DMA2D_CMSIS_INCLUDE
#endif

/*Enable RA6M3 G2D GPU*/
#define LV_USE_GPU_RA6M3_G2D 0
#if LV_USE_GPU_RA6M3_G2D
    /*include path of target processor
    e.g. "hal_data.h"*/
    #define LV_GPU_RA6M3_G2D_INCLUDE "hal_data.h"
#endif

/*Use SWM341's DMA2D GPU*/
#define LV_USE_GPU_SWM341_DMA2D 0
#if LV_USE_GPU_SWM341_DMA2D
    #define LV_GPU_SWM341_DMA2D_INCLUDE "SWM341.h"
#endif

/*Use NXP's PXP GPU iMX RTxxx platforms*/
#define LV_USE_GPU_NXP_PXP 0
#if LV_USE_GPU_NXP_PXP
    /*1: Add default bare metal and FreeRTOS interrupt handling routines for PXP (lv_gpu_nxp_pxp_osa.c)
    *   and call lv_gpu_nxp_pxp_init() automatically during lv_init(). Note that symbol SDK_OS_FREE_RTOS
    *   has to be defined in order to use FreeRTOS OSA, otherwise bare-metal implementation is selected.
    *0: lv_gpu_nxp_pxp_init() has to be called manually before lv_init()
    */
    /*1:ΪPXP (lv_gpu_nxp_pxp_osa.c)���Ĭ�ϵ�������FreeRTOS�жϴ�������
     *����lv_init()ʱ�Զ�����lv_gpu_nxp_pxp_init()��ע�����SDK_OS_FREE_RTOS
     *���붨���Ա�ʹ��FreeRTOS OSA������ѡ�������ʵ�֡�
     *0: lv_gpu_nxp_pxp_init()������lv_init()֮ǰ�ֶ�����
    */
    #define LV_USE_GPU_NXP_PXP_AUTO_INIT 0
#endif

/*Use NXP's VG-Lite GPU iMX RTxxx platforms*/
#define LV_USE_GPU_NXP_VG_LITE 0

/*Use SDL renderer API*/
/* ʹ��SDL��Ⱦ��API */
#define LV_USE_GPU_SDL 0
#if LV_USE_GPU_SDL
    #define LV_GPU_SDL_INCLUDE_PATH <SDL2/SDL.h>
    /*Texture cache size, 8MB by default*/
    /* �������С��Ĭ��8MB */
    #define LV_GPU_SDL_LRU_SIZE (1024 * 1024 * 8)
    /*Custom blend mode for mask drawing, disable if you need to link with older SDL2 lib*/
    /* �Զ�����ģʽ���ɰ���ƣ��������Ҫ���Ӿ�SDL2����� */
    #define LV_GPU_SDL_CUSTOM_BLEND_MODE (SDL_VERSION_ATLEAST(2, 0, 6))
#endif

/*-------------
 * Logging ��־
 *-----------*/

/*Enable the log module*/
#define LV_USE_LOG 0
#if LV_USE_LOG

    /*How important log should be added:
    *LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
    *LV_LOG_LEVEL_INFO        Log important events
    *LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
    *LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
    *LV_LOG_LEVEL_USER        Only logs added by the user
    *LV_LOG_LEVEL_NONE        Do not log anything*/
      /*Ӧ����Ӷ���Ҫ����־:
    *LV_LOG_LEVEL_TRACE       ��������־��������ϸ����Ϣ
    *LV_LOG_LEVEL_INFO        ��¼��Ҫ�¼�
    *LV_LOG_LEVEL_WARN        ���������һЩ����Ҫ�����鵫û���������⣬���¼����
    *LV_LOG_LEVEL_ERROR       ֻ����ϵͳ���ܳ��ֹ���ʱ�Ż���ֹؼ�����
    *LV_LOG_LEVEL_USER        ���û��Լ���ӵ���־
    *LV_LOG_LEVEL_NONE        ��Ҫ��¼�κ�����*/
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

    /*1: Print the log with 'printf';
    *0: User need to register a callback with `lv_log_register_print_cb()`*/
    #define LV_LOG_PRINTF 0

    /*Enable/disable LV_LOG_TRACE in modules that produces a huge number of logs*/
    #define LV_LOG_TRACE_MEM        1
    #define LV_LOG_TRACE_TIMER      1
    #define LV_LOG_TRACE_INDEV      1
    #define LV_LOG_TRACE_DISP_REFR  1
    #define LV_LOG_TRACE_EVENT      1
    #define LV_LOG_TRACE_OBJ_CREATE 1
    #define LV_LOG_TRACE_LAYOUT     1
    #define LV_LOG_TRACE_ANIM       1

#endif  /*LV_USE_LOG*/

/*-------------
 * Asserts ����
 *-----------*/

/*Enable asserts if an operation is failed or an invalid data is found.
 *If LV_USE_LOG is enabled an error message will be printed on failure*/
/* �������ʧ�ܻ�����Ч���ݣ������ö��ԡ�
 * ���������LV_USE_LOG��ʧ��ʱ���ӡ������Ϣ*/
#define LV_USE_ASSERT_NULL          1    /* �������Ƿ�ΪNULL��(�ǳ���,�Ƽ�) */
#define LV_USE_ASSERT_MALLOC        1    /* ����ڴ��Ƿ����ɹ���(�ǳ���,�Ƽ�) */
#define LV_USE_ASSERT_STYLE         0    /* �����ʽ�Ƿ���ȷ��ʼ����(�ǳ���,�Ƽ�) */
#define LV_USE_ASSERT_MEM_INTEGRITY 0    /* �ؼ�������ɺ����顰lv_mem���������ԡ�(��)*/
#define LV_USE_ASSERT_OBJ           0    /* ����������ͺʹ���(���磬δɾ��)��(��) */

/*Add a custom handler when assert happens e.g. to restart the MCU*/
#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER while(1);   /*Halt by default*/

/*-------------
 * Others ����
 *-----------*/

/*1: Show CPU usage and FPS count*/
/* 1:��ʾCPUʹ���ʺ�FPS */
#define LV_USE_PERF_MONITOR 0
#if LV_USE_PERF_MONITOR
    #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
#endif

/*1: Show the used memory and the memory fragmentation
 * Requires LV_MEM_CUSTOM = 0*/
/* 1����ʾʹ�õ��ڴ���ڴ���Ƭ
 * Ҫ��LV_MEM_CUSTOM = 0*/
#define LV_USE_MEM_MONITOR 0
#if LV_USE_MEM_MONITOR
    #define LV_USE_MEM_MONITOR_POS LV_ALIGN_BOTTOM_LEFT
#endif

/*1: Draw random colored rectangles over the redrawn areas*/
/* 1:�����»��Ƶ������ϻ�������Ĳ�ɫ���� */
#define LV_USE_REFR_DEBUG 0

/*Change the built in (v)snprintf functions*/
/* �ı����õ�(v)snprintf���� */
#define LV_SPRINTF_CUSTOM 0
#if LV_SPRINTF_CUSTOM
    #define LV_SPRINTF_INCLUDE <stdio.h>
    #define lv_snprintf  snprintf
    #define lv_vsnprintf vsnprintf
#else   /*LV_SPRINTF_CUSTOM*/
    #define LV_SPRINTF_USE_FLOAT 0
#endif  /*LV_SPRINTF_CUSTOM*/

#define LV_USE_USER_DATA 1

/*Garbage Collector settings
 *Used if lvgl is bound to higher level language and the memory is managed by that language*/
/* �����ռ�������
 * ���lvgl�󶨵��߼����ԣ������ڴ��ɸ����Թ���ʱʹ��*/
#define LV_ENABLE_GC 0
#if LV_ENABLE_GC != 0
    #define LV_GC_INCLUDE "gc.h"                           /*Include Garbage Collector related things*/
#endif /*LV_ENABLE_GC*/

/*=====================
 *  COMPILER SETTINGS ����������
 *====================*/

/*For big endian systems set to 1*/
/* ��������Ϊ1�Ĵ����ϵͳ */
#define LV_BIG_ENDIAN_SYSTEM 0

/*Define a custom attribute to `lv_tick_inc` function*/
/* Ϊ' lv_tick_inc '��������һ���Զ������� */
#define LV_ATTRIBUTE_TICK_INC

/*Define a custom attribute to `lv_timer_handler` function*/
/* Ϊ' lv_timer_handler '��������һ���Զ������� */
#define LV_ATTRIBUTE_TIMER_HANDLER

/*Define a custom attribute to `lv_disp_flush_ready` function*/
/* Ϊ' lv_disp_flush_ready '��������һ���Զ������� */
#define LV_ATTRIBUTE_FLUSH_READY

/*Required alignment size for buffers*/
/* ����������Ķ����С */
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1

/*Will be added where memories needs to be aligned (with -Os data might not be aligned to boundary by default).
 * E.g. __attribute__((aligned(4)))*/
/* ������ӵ���Ҫ�����ڴ�ĵط�(Ĭ�������-Os���ݿ��ܲ�����뵽�߽�)��
 * ��__attribute__((����(4))) */
#define LV_ATTRIBUTE_MEM_ALIGN

/*Attribute to mark large constant arrays for example font's bitmaps*/
/* ��������Ǵ��ͳ������飬���������λͼ */
#define LV_ATTRIBUTE_LARGE_CONST

/*Compiler prefix for a big array declaration in RAM*/
/* RAM�д����������ı�����ǰ׺ */
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/*Place performance critical functions into a faster memory (e.g RAM)*/
/* �����ܹؼ����ܷ��������ڴ���(����RAM) */
#define LV_ATTRIBUTE_FAST_MEM

/*Prefix variables that are used in GPU accelerated operations, often these need to be placed in RAM sections that are DMA accessible*/
/* ��GPU���ٲ�����ʹ�õ�ǰ׺������ͨ����Ҫ������DMA�ɷ��ʵ�RAM���� */
#define LV_ATTRIBUTE_DMA

/*Export integer constant to binding. This macro is used with constants in the form of LV_<CONST> that
 *should also appear on LVGL binding API such as Micropython.*/
/* �������ͳ������󶨡��ú���LV_<CONST> that��ʽ�ĳ���һ��ʹ��
 * ҲӦ�ó�����LVGL��API����Micropython��*/
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning /*The default value just prevents GCC warning*/

/*Extend the default -32k..32k coordinate range to -4M..4M by using int32_t for coordinates instead of int16_t*/
/* ��չĬ��ֵ-32k..32k���귶Χ��-4M..ʹ��int32_t������int16_t��Ϊ���� */
#define LV_USE_LARGE_COORD 0

/*==================
 *   FONT USAGE �ֿ�����
 *===================*/

/*Montserrat fonts with ASCII range and some symbols using bpp = 4
 *https://fonts.google.com/specimen/Montserrat*/
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/*Demonstrate special features*/
#define LV_FONT_MONTSERRAT_12_SUBPX      0
#define LV_FONT_MONTSERRAT_28_COMPRESSED 0  /*bpp = 3*/
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0  /* ϣ��������������˹���Լ����ǵĸ�����ʽ */
#define LV_FONT_SIMSUN_16_CJK            0  /*1000 most common CJK radicals*/

/*Pixel perfect monospace fonts*/
/* ���������ĵ��ռ����� */
#define LV_FONT_UNSCII_8  0
#define LV_FONT_UNSCII_16 0

/*Optionally declare custom fonts here.
 *You can use these fonts as default font too and they will be available globally.
 *E.g. #define LV_FONT_CUSTOM_DECLARE   LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2)*/
/* ��ѡ�����Զ������������
 * ��Ҳ����ʹ����Щ������ΪĬ�����壬���ǽ���ȫ����õġ�
 * �硣#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2) */
#define LV_FONT_CUSTOM_DECLARE

/*Always set a default font*/
/* ʼ������Ĭ������ */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*Enable handling large font and/or fonts with a lot of characters.
 *The limit depends on the font size, font face and bpp.
 *Compiler error will be triggered if a font needs it.*/
/* ���ô���������/����д����ַ������塣
 * ����ȡ���������С���������bpp��
 * ���������󽫱����������������Ҫ�� */
#define LV_FONT_FMT_TXT_LARGE 0

/*Enables/disables support for compressed fonts.*/
/* ����/���ö�ѹ�������֧�֡� */
#define LV_USE_FONT_COMPRESSED 0

/*Enable subpixel rendering*/
/* ʹ��������Ⱦ */
#define LV_USE_FONT_SUBPX 0
#if LV_USE_FONT_SUBPX
    /*Set the pixel order of the display. Physical order of RGB channels. Doesn't matter with "normal" fonts.*/
    /* ������ʾ������˳��RGBͨ��������˳���롰�����������޹ء� */
    #define LV_FONT_SUBPX_BGR 0  /*0: RGB; 1:BGR order*/
#endif

/*Enable drawing placeholders when glyph dsc is not found*/
#define LV_USE_FONT_PLACEHOLDER 1

/*=================
 *  TEXT SETTINGS �ı�����
 *=================*/

/**
 * Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
/**
 * Ϊ�ַ���ѡ���ַ�����
 * IDE��༭��Ӧ�þ�����ͬ���ַ�����
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/*Can break (wrap) texts on these chars*/
/* ��������Щ�ַ����ж�(����)�ı� */
#define LV_TXT_BREAK_CHARS " ,.;:-_"

/*If a word is at least this long, will break wherever "prettiest"
 *To disable, set to a value <= 0*/
/* ���һ��������������ô�����ͻ��ڡ��������ĵط�����
 * Ҫ���ã�����һ��ֵ<= 0 */
#define LV_TXT_LINE_BREAK_LONG_LEN 0

/*Minimum number of characters in a long word to put on a line before a break.
 *Depends on LV_TXT_LINE_BREAK_LONG_LEN.*/
/* ��һ���������У���ͣ��֮ǰ����һ�е���С�ַ�����
 * ȡ����LV_TXT_LINE_BREAK_LONG_LEN�� */
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/*Minimum number of characters in a long word to put on a line after a break.
 *Depends on LV_TXT_LINE_BREAK_LONG_LEN.*/
/* ��һ���������У���ͣ�ٺ����һ�е���С�ַ�����
 * ȡ����LV_TXT_LINE_BREAK_LONG_LEN��*/
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/*The control character to use for signalling text recoloring.*/
/* ���������ı�������ɫ�Ŀ����ַ��� */
#define LV_TXT_COLOR_CMD "#"

/* ֧��˫���ı��������ϴ����Һʹ��ҵ�����ı���
 * ��������Unicode˫���㷨���д���:
 * https://www.w3.org/International/articles/inline-bidi-markup/uba-basics*/
#define LV_USE_BIDI 0
#if LV_USE_BIDI
    /* ����Ĭ�Ϸ���֧�ֵ�ֵ:
    *`LV_BASE_DIR_LTR` ������
    *`LV_BASE_DIR_RTL` ���ҵ���
    *`LV_BASE_DIR_AUTO` ����ı��������� */
    #define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#endif

/* ֧�ְ�������/��˹����
 * ����Щ�����У��ַ�Ӧ�ø��������ı��е�λ�ñ��滻Ϊ������ʽ */
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/*==================
 *  WIDGET USAGE
 *================*/

/*Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html*/
/* С�������ĵ�:https://docs.lvgl.io/latest/en/html/widgets/index.html */

#define LV_USE_ARC        1

#define LV_USE_BAR        1

#define LV_USE_BTN        1

#define LV_USE_BTNMATRIX  1

#define LV_USE_CANVAS     1

#define LV_USE_CHECKBOX   1

#define LV_USE_DROPDOWN   1   /*Requires: lv_label*/

#define LV_USE_IMG        1   /*Requires: lv_label*/

#define LV_USE_LABEL      1
#if LV_USE_LABEL
    #define LV_LABEL_TEXT_SELECTION 1 /*Enable selecting text of the label*/  /* ���ñ�ǩ��ѡ���ı�*/
    #define LV_LABEL_LONG_TXT_HINT 1  /*Store some extra info in labels to speed up drawing of very long texts*/ /* �ڱ�ǩ�д洢һЩ�������Ϣ���Լӿ���Ʒǳ������ı� */
#endif

#define LV_USE_LINE       1

#define LV_USE_ROLLER     1   /*Requires: lv_label*/
#if LV_USE_ROLLER
    #define LV_ROLLER_INF_PAGES 7 /*Number of extra "pages" when the roller is infinite*/  /* ����Ͳ����ʱ������ġ�ҳ���� */
#endif

#define LV_USE_SLIDER     1   /*Requires: lv_bar*/

#define LV_USE_SWITCH     1

#define LV_USE_TEXTAREA   1   /*Requires: lv_label*/  /* ����: lv_label*/
#if LV_USE_TEXTAREA != 0
    #define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500    /*ms*/
#endif

#define LV_USE_TABLE      1

/*==================
 * EXTRA COMPONENTS
 *==================*/

/*-----------
 * Widgets
 *----------*/
#define LV_USE_ANIMIMG    1

#define LV_USE_CALENDAR   1
#if LV_USE_CALENDAR
    #define LV_CALENDAR_WEEK_STARTS_MONDAY 0
    #if LV_CALENDAR_WEEK_STARTS_MONDAY
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"}
    #else
        #define LV_CALENDAR_DEFAULT_DAY_NAMES {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"}
    #endif

    #define LV_CALENDAR_DEFAULT_MONTH_NAMES {"January", "February", "March",  "April", "May",  "June", "July", "August", "September", "October", "November", "December"}
    #define LV_USE_CALENDAR_HEADER_ARROW 1
    #define LV_USE_CALENDAR_HEADER_DROPDOWN 1
#endif  /*LV_USE_CALENDAR*/

#define LV_USE_CHART      1

#define LV_USE_COLORWHEEL 1

#define LV_USE_IMGBTN     1

#define LV_USE_KEYBOARD   1

#define LV_USE_LED        1

#define LV_USE_LIST       1

#define LV_USE_MENU       1

#define LV_USE_METER      1

#define LV_USE_MSGBOX     1

#define LV_USE_SPAN       1
#if LV_USE_SPAN
    /*A line text can contain maximum num of span descriptor */
    /* һ�����ı����԰������������span������ */
    #define LV_SPAN_SNIPPET_STACK_SIZE 64
#endif

#define LV_USE_SPINBOX    1

#define LV_USE_SPINNER    1

#define LV_USE_TABVIEW    1

#define LV_USE_TILEVIEW   1

#define LV_USE_WIN        1

/*-----------
 * Themes ����
 *----------*/

/* һ���򵥣�����ӡ����̺ͷǳ����������� */
/*A simple, impressive and very complete theme*/
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT

    /*0: Light mode; 1: Dark mode*/
    /* 0:��ģʽ;1:�ڰ���ģʽ */
    #define LV_THEME_DEFAULT_DARK 0

    /*1: Enable grow on press*/
    /* 1:����ʹ������ */
    #define LV_THEME_DEFAULT_GROW 1

    /*Default transition time in [ms]*/
    /* Ĭ��ת��ʱ��[ms] */
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif /*LV_USE_THEME_DEFAULT*/

/*A very simple theme that is a good starting point for a custom theme*/
/* һ���ǳ��򵥵����⣬����һ���Զ��������������� */
#define LV_USE_THEME_BASIC 1

/*A theme designed for monochrome displays*/
/* רΪ��ɫ��ʾ����Ƶ����� */
#define LV_USE_THEME_MONO 1

/*-----------
 * Layouts ����
 *----------*/

/*A layout similar to Flexbox in CSS.*/
/* ������CSS�е�Flexbox���� */
#define LV_USE_FLEX 1

/*A layout similar to Grid in CSS.*/
/* ������CSS�е����񲼾� */
#define LV_USE_GRID 1

/*---------------------
 * 3rd party libraries ��������
 *--------------------*/
/* ����ͨ��api���ļ�ϵͳ�ӿ�
 * Ϊ��API����������������� */

/*File system interfaces for common APIs */

/*API for fopen, fread, etc*/
#define LV_USE_FS_STDIO 0
#if LV_USE_FS_STDIO
    #define LV_FS_STDIO_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_STDIO_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define LV_FS_STDIO_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for open, read, etc*/
#define LV_USE_FS_POSIX 0
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_POSIX_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define LV_FS_POSIX_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for CreateFile, ReadFile, etc*/
#define LV_USE_FS_WIN32 0
#if LV_USE_FS_WIN32
    #define LV_FS_WIN32_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_WIN32_PATH ""         /*Set the working directory. File/directory paths will be appended to it.*/
    #define LV_FS_WIN32_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for FATFS (needs to be added separately). Uses f_open, f_read, etc*/
#define LV_USE_FS_FATFS 0
#if LV_USE_FS_FATFS
    #define LV_FS_FATFS_LETTER '\0'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_FATFS_CACHE_SIZE 0    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

/*API for LittleFS (library needs to be added separately). Uses lfs_file_open, lfs_file_read, etc*/
#define LV_USE_FS_LITTLEFS 0
#if LV_USE_FS_LITTLEFS
    #define LV_FS_LITTLEFS_LETTER '\0'      /* ����һ���ɷ����������Ĵ�д��ĸ(���硣��A��) */
    #define LV_FS_LITTLEFS_CACHE_SIZE 0    /* >0��lv_fs_read()�л�������ֽ��� */
#endif

/*PNG decoder library PNG�������� */
#define LV_USE_PNG 0

/*BMP decoder library BMP��������*/
#define LV_USE_BMP 0

/* JPG +�ָ�JPG�������⡣
 * Split JPG��ΪǶ��ʽϵͳ�Ż����Զ����ʽ */
#define LV_USE_SJPG 0

/*GIF decoder library GIF�������� */
#define LV_USE_GIF 0

/*QR code library QR�������� */
#define LV_USE_QRCODE 0

/*FreeType library*/
#define LV_USE_FREETYPE 0
#if LV_USE_FREETYPE
    /* FreeType���ڻ����ַ�[bytes]���ڴ�(-1:û�л���) */
    #define LV_FREETYPE_CACHE_SIZE (16 * 1024)
    #if LV_FREETYPE_CACHE_SIZE >= 0
        /* 1:λͼcacheʹ��sbit cache, 0:λͼcacheʹ��ͼ��cache
         * sbit����:����С��λͼ(�����С< 256)�������ڴ�Ч�ʸ���
         *��������С>= 256����������Ϊͼ�񻺴� */
        #define LV_FREETYPE_SBIT_CACHE 0
        /* ���������ʵ������Ĵ򿪵�FT_Face/FT_Size��������������*��
           (0:ʹ��ϵͳĬ��ֵ) */
        #define LV_FREETYPE_CACHE_FT_FACES 0
        #define LV_FREETYPE_CACHE_FT_SIZES 0
    #endif
#endif

/*Tiny TTF library*/
#define LV_USE_TINY_TTF 0
#if LV_USE_TINY_TTF
    /*Load TTF data from files*/
    #define LV_TINY_TTF_FILE_SUPPORT 0
#endif

/*Rlottie library Rlottie �� */
#define LV_USE_RLOTTIE 0

/* FFmpeg���ͼ�����Ͳ�����Ƶ
 * ֧��������Ҫ��ͼ���ʽ�����Բ���������ͼ����������� */
/*FFmpeg library for image decoding and playing videos
 *Supports all major image formats so do not enable other image decoder with it*/
#define LV_USE_FFMPEG 0
#if LV_USE_FFMPEG
    /*Dump input information to stderr*/
    #define LV_FFMPEG_DUMP_FORMAT 0
#endif

/*-----------
 * Others
 *----------*/

/*1: Enable API to take snapshot for object  1:����API�Զ�����п��� */
#define LV_USE_SNAPSHOT 0

/*1: Enable Monkey test 1:ʹ��Monkey���� */
#define LV_USE_MONKEY 0

/*1: Enable grid navigation 1:�������񵼺� */
#define LV_USE_GRIDNAV 0

/*1: Enable lv_obj fragment*/
#define LV_USE_FRAGMENT 0

/*1: Support using images as font in label or span widgets */
#define LV_USE_IMGFONT 0

/*1: Enable a published subscriber based messaging system */
#define LV_USE_MSG 0

/*1: Enable Pinyin input method*/
/*Requires: lv_keyboard*/
#define LV_USE_IME_PINYIN 0
#if LV_USE_IME_PINYIN
    /*1: Use default thesaurus*/
    /*If you do not use the default thesaurus, be sure to use `lv_ime_pinyin` after setting the thesauruss*/
    #define LV_IME_PINYIN_USE_DEFAULT_DICT 1
    /*Set the maximum number of candidate panels that can be displayed*/
    /*This needs to be adjusted according to the size of the screen*/
    #define LV_IME_PINYIN_CAND_TEXT_NUM 6

    /*Use 9 key input(k9)*/
    #define LV_IME_PINYIN_USE_K9_MODE      1
    #if LV_IME_PINYIN_USE_K9_MODE == 1
        #define LV_IME_PINYIN_K9_CAND_TEXT_NUM 3
    #endif // LV_IME_PINYIN_USE_K9_MODE
#endif

/*==================
* EXAMPLES
*==================*/

/*Enable the examples to be built with the library �����ÿ⹹��ʾ�� */
#define LV_BUILD_EXAMPLES 1

/*===================
 * DEMO USAGE ��ʾʹ��
 ====================*/

/*Show some widget. It might be required to increase `LV_MEM_SIZE` ��ʾ��һЩ������������Ҫ���ӡ�LV_MEM_SIZE�� */
#define LV_USE_DEMO_WIDGETS 0
#if LV_USE_DEMO_WIDGETS
#define LV_DEMO_WIDGETS_SLIDESHOW 0
#endif

/*Demonstrate the usage of encoder and keyboard ��ʾ�������ͼ��̵��÷� */
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0

/*Benchmark your system ��׼ϵͳ */
#define LV_USE_DEMO_BENCHMARK 0
#if LV_USE_DEMO_BENCHMARK
/*Use RGB565A8 images with 16 bit color depth instead of ARGB8565*/
#define LV_DEMO_BENCHMARK_RGB565A8 0
#endif

/*Stress test for LVGL LVGLѹ������ */
#define LV_USE_DEMO_STRESS 1

/*Music player demo ���ֲ���������ʾ*/
#define LV_USE_DEMO_MUSIC 0
#if LV_USE_DEMO_MUSIC
    #define LV_DEMO_MUSIC_SQUARE    0
    #define LV_DEMO_MUSIC_LANDSCAPE 0
    #define LV_DEMO_MUSIC_ROUND     0
    #define LV_DEMO_MUSIC_LARGE     0
    #define LV_DEMO_MUSIC_AUTO_PLAY 0
#endif

/*--END OF LV_CONF_H--*/

#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/
