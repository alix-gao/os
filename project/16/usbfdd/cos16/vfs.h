/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : vfs.h
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/

#ifndef __VFS_H__
#define __VFS_H__

/****************************************************************
 include head file
 ****************************************************************/

/****************************************************************
 macro define
 ****************************************************************/

/****************************************************************
 enum define
 ****************************************************************/

/****************************************************************
 struct define
 ****************************************************************/

/****************************************************************
 extern function
 ****************************************************************/
VOID init_vfs(VOID);
WORD load_file(BYTE *file_name, DWORD addr);
WORD change_directory(BYTE *dir_name);
VOID list_vfs(VOID);

#endif

