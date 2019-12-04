
#include <kcurse_struct.h>
#include <graphics.h>

extern int *kernel_disp_curse_x;
extern int *kernel_disp_curse_y;

void kcurse_init()
{
     /* 640*480 */
     *kernel_disp_curse_x = kcurse_pos_init_x;
     *kernel_disp_curse_y = kcurse_pos_init_y;
}

void clean_kernel_disp_area()
{
     disp_color(kcurse_pos_init_x, kcurse_pos_init_y, 639, 479, 1);/* 1 is blue */
}

curse_pos_stru kcurse_get_pos(int length)
{
               curse_pos_stru pos;

               /* col overflow */
               if ((640-8) <= (*kernel_disp_curse_x + length*8))
               {
                   *kernel_disp_curse_y = *kernel_disp_curse_y + 16;
                   *kernel_disp_curse_x = kcurse_pos_init_x;

                   pos.x = *kernel_disp_curse_x;

                   /* used */
                   *kernel_disp_curse_x = *kernel_disp_curse_x + 8*length;
               }
               else
               {
                   pos.x = *kernel_disp_curse_x;

                   /* next x pos update */
                   *kernel_disp_curse_x = *kernel_disp_curse_x + 8*length;
               }
               /* leave out */
               pos.y = *kernel_disp_curse_y;

               /* bottom */
               if ((480-16) <= *kernel_disp_curse_y)
               {
                   *kernel_disp_curse_y = kcurse_pos_init_y;
                   *kernel_disp_curse_x = kcurse_pos_init_x;

                   pos.x = *kernel_disp_curse_x;
                   pos.y = *kernel_disp_curse_y;

                   /* update x */
                   *kernel_disp_curse_x = *kernel_disp_curse_x + 8*length;

                   /* clean */
                   clean_kernel_disp_area();
               }

               return pos;
}

