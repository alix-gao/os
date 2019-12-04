/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : make_symbol.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include "stdio.h"
#include "stdlib.h"

#define FUNC_NAME_LEN 0x100

FILE *fpsrc;
FILE *fpdst;

int main()
{
    char onec;
    char name[FUNC_NAME_LEN];
    int i = 0;
    char head[] = "char symbol[] =\n{";
    char tail[] = "\n};\n";

    /* init global variable */
    fpsrc = fopen("symbol.txt", "rt+");
    if (NULL == fpsrc)
    {
        printf("cannot open file symbol.txt\n");
        exit(0);
    }

    fpdst = fopen("symbol.c", "wt");
    if (NULL == fpdst)
    {
        printf("cannot open file symbol.c\n");
        exit(0);
    }

    /* 生成extern */
    while (!feof(fpsrc))
    {
        fread(&onec, sizeof(char), 1, fpsrc);

        if ('_' == onec)
        {
            onec = '\n';
            fwrite(&onec, sizeof(char), 1, fpdst);

            onec = 0x22;
            fwrite(&onec, sizeof(char), 1, fpdst);

            i = 0;

            while (1)
            {
                fread(&onec, sizeof(char), 1, fpsrc);
                name[i] = onec;
                i++;

                if ((' ' == onec) || ('@' == onec))
                {
                    onec = 0x22;
                    fwrite(&onec, sizeof(char), 1, fpdst);

                    onec = ',';
                    fwrite(&onec, sizeof(char), 1, fpdst);

                    fwrite(name, (i-1)*sizeof(char), 1, fpdst);

                    onec = ',';
                    fwrite(&onec, sizeof(char), 1, fpdst);
                    break;
                }

                fwrite(&onec, sizeof(char), 1, fpdst);
            }
        }
    }

    /* write .text */
    fseek(fpsrc, 0l, 0);
    fseek(fpdst, 0l, 0);

    /* 写头部 */
    fwrite(head, sizeof(head), 1, fpdst);

    while (!feof(fpsrc))
    {
        fread(&onec, sizeof(char), 1, fpsrc);

        if ('_' == onec)
        {
            onec = '\n';
            fwrite(&onec, sizeof(char), 1, fpdst);

            onec = 0x22;
            fwrite(&onec, sizeof(char), 1, fpdst);

            i = 0;

            while (1)
            {
                fread(&onec, sizeof(char), 1, fpsrc);
                name[i] = onec;
                i++;

                if ((' ' == onec) || ('@' == onec))
                {
                    onec = 0x22;
                    fwrite(&onec, sizeof(char), 1, fpdst);

                    onec = ',';
                    fwrite(&onec, sizeof(char), 1, fpdst);

                    fwrite(name, (i-1)*sizeof(char), 1, fpdst);

                    onec = ',';
                    fwrite(&onec, sizeof(char), 1, fpdst);
                    break;
                }

                fwrite(&onec, sizeof(char), 1, fpdst);
            }
        }
    }

    /* 写尾部 */
    fwrite(tail, sizeof(tail), 1, fpdst);

    fclose(fpsrc);
    fclose(fpdst);

    //system("pause");
}

