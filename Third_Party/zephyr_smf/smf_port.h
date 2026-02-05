#ifndef SM_PORT_H
#define SM_PORT_H

#include <cstdint>
#include <stdbool.h>
#include <stdio.h>

// 启用层次状态机
#define CONFIG_SMF_ANCESTOR_SUPPORT 1

// 启用初始转换支持
#define CONFIG_SMF_INITIAL_TRANSITION 0

#define LOG_ERR(fmt, ...)                                        \
    do                                                           \
    {                                                            \
        fprintf(stderr, "[SMF ERROR] " fmt "\n", ##__VA_ARGS__); \
    } while (0)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Used internally by COND_CODE_1 and COND_CODE_0. */
#define Z_COND_CODE_1(_flag, _if_1_code, _else_code) \
    __COND_CODE(_XXXX##_flag, _if_1_code, _else_code)
#define __COND_CODE(one_or_two_args, _if_code, _else_code) \
    __GET_ARG2_DEBRACKET(one_or_two_args _if_code, _else_code)

/* Gets second argument and removes brackets around that argument. It
 * is expected that the parameter is provided in brackets/parentheses.
 */
#define __GET_ARG2_DEBRACKET(ignore_this, val, ...) __DEBRACKET val

/* Used to remove brackets from around a single argument. */
#define __DEBRACKET(...) __VA_ARGS__
/**
 * @brief Insert code depending on whether @p _flag expands to 1 or not.
 *
 * This relies on similar tricks as IS_ENABLED(), but as the result of
 * @p _flag expansion, results in either @p _if_1_code or @p
 * _else_code is expanded.
 *
 * To prevent the preprocessor from treating commas as argument
 * separators, the @p _if_1_code and @p _else_code expressions must be
 * inside brackets/parentheses: <tt>()</tt>. These are stripped away
 * during macro expansion.
 *
 * Example:
 *
 *     COND_CODE_1(CONFIG_FLAG, (uint32_t x;), (there_is_no_flag();))
 *
 * If @p CONFIG_FLAG is defined to 1, this expands to:
 *
 *     uint32_t x;
 *
 * It expands to <tt>there_is_no_flag();</tt> otherwise.
 *
 * This could be used as an alternative to:
 *
 *     #if defined(CONFIG_FLAG) && (CONFIG_FLAG == 1)
 *     #define MAYBE_DECLARE(x) uint32_t xtoolchain.hutil_internal
 *     #else
 *     #define MAYBE_DECLARE(x) there_is_no_flag()
 *     #endif
 *
 *     MAYBE_DECLARE(x);
 *
 * However, the advantage of COND_CODE_1() is that code is resolved in
 * place where it is used, while the @p \#if method defines @p
 * MAYBE_DECLARE on two lines and requires it to be invoked again on a
 * separate line. This makes COND_CODE_1() more concise and also
 * sometimes more useful when used within another macro's expansion.
 *
 * @note @p _flag can be the result of preprocessor expansion, e.g.
 *	 an expression involving <tt>NUM_VA_ARGS_LESS_1(...)</tt>.
 *	 However, @p _if_1_code is only expanded if @p _flag expands
 *	 to the integer literal 1. Integer expressions that evaluate
 *	 to 1, e.g. after doing some arithmetic, will not work.
 *
 * @param _flag evaluated flag
 * @param _if_1_code result if @p _flag expands to 1; must be in parentheses
 * @param _else_code result otherwise; must be in parentheses
 */
#define COND_CODE_1(_flag, _if_1_code, _else_code) \
    Z_COND_CODE_1(_flag, _if_1_code, _else_code)

/**
 * @brief Insert code if @p _flag is defined and equals 1.
 *
 * Like COND_CODE_1(), this expands to @p _code if @p _flag is defined to 1;
 * it expands to nothing otherwise.
 *
 * Example:
 *
 *     IF_ENABLED(CONFIG_FLAG, (uint32_t foo;))
 *
 * If @p CONFIG_FLAG is defined to 1, this expands to:
 *
 *     uint32_t foo;
 *
 * and to nothing otherwise.
 *
 * It can be considered as a more compact alternative to:
 *
 *     #if defined(CONFIG_FLAG) && (CONFIG_FLAG == 1)
 *     uint32_t foo;
 *     #endif
 *
 * @param _flag evaluated flag
 * @param _code result if @p _flag expands to 1; must be in parentheses
 */
#define IF_ENABLED(_flag, _code) \
    COND_CODE_1(_flag, _code, ())

#define LOG_MODULE_REGISTER(name) // No-op for this simplified example

#endif // SM_PORT_H
