
#include "typedef.h"
#include "fat32.h"

VOID init_vfs(VOID)
{
    init_fat32();
}

WORD load_file(BYTE *file_name, DWORD addr)
{
    if (NULL != file_name) {
        return fat32_load_file(file_name, addr);
    }
    return FAIL;
}

WORD change_directory(BYTE *dir_name)
{
    if (NULL != dir_name) {
        return fat32_change_directory(dir_name);
    }
    return FAIL;
}

VOID list_vfs(VOID)
{
    list_fat32();
}

