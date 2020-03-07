/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2019-2020, Ondrej Čerman
 */

#ifndef _FSUTILS_H_
#define _FSUTILS_H_

#include <File.h>
#include <fs_attr.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <StatusBar.h>
#include <String.h>
#include <Window.h>

class FSUtils{

	public:
		static bool CopyFileContents(BFile *srcfile, BFile *dstfile, size_t blksize, BWindow *w, BStatusBar *sb);
		static void CopyStats(struct stat *srcstat, BNode *dst);
		static bool CopyAttr(BNode *srcnode, BNode *destnode);
		static bool CopyAttr(const char *srcfilename, const char *dstfilename);

	private:

};

#endif
