#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "main.h"

typedef enum
{
    btn_id_0 = 1, /* Button 1 */
    btn_id_1 = 2, /* Button 2 */
    btn_id_2 = 3, /* Button 3 */
    btn_id_3 = 4, /* Button 4 */
    max_btn_num,
} btn_id_e;

void buttons_init(void);

#endif /* __BUTTON_H__ */
