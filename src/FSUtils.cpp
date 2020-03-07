/*
 * Copyright 2002-2020. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 *	2019-2020, Ondrej Čerman
 */

#include "FSUtils.h"
#include <Directory.h>
#include <File.h>
#include <fs_attr.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <StatusBar.h>
#include <String.h>
#include <Volume.h>
#include <Window.h>

////////////////////////////////////////////////////////////////////////
bool FSUtils::CopyFileContents(BFile *srcfile, BFile *dstfile, size_t blksize, BWindow *w, BStatusBar *sb)
////////////////////////////////////////////////////////////////////////
{
	ssize_t len;

	unsigned char *buf = new unsigned char[blksize];
	if (!buf)
		return false;

	while (true)
	{
		len = srcfile->Read(buf, blksize);
		if (len>0)
		{
			dstfile->Write(buf, len);
			if (w != NULL && sb != NULL){
				w->Lock();
				sb->Update(len);
				w->Unlock();
			}
		}
		else if (len<0) // error
		{
			delete [] buf;
			return false;
		}
		else	// No more bytes to copy, we are done...
			break;
	}

	delete [] buf;
	return true;
}


////////////////////////////////////////////////////////////////////////
void FSUtils::CopyStats(struct stat *srcstat, BNode *dst)
////////////////////////////////////////////////////////////////////////
{
	dst->SetPermissions(srcstat->st_mode);
	dst->SetOwner(srcstat->st_uid);
	dst->SetGroup(srcstat->st_gid);
	dst->SetModificationTime(srcstat->st_mtime);
	dst->SetCreationTime(srcstat->st_crtime);
}

////////////////////////////////////////////////////////////////////////
bool FSUtils::CopyAttr(BNode *srcnode, BNode *dstnode)
////////////////////////////////////////////////////////////////////////
{
	char attrname[B_ATTR_NAME_LENGTH];
	attr_info attrinfo;
	ssize_t len = 0;	// ennyit olvasott a ReadAttr()
	unsigned char *buf;

	while (srcnode->GetNextAttrName(attrname) == B_OK)
	{
		if (srcnode->GetAttrInfo(attrname, &attrinfo) != B_OK)
			continue;	// skip current attr

		buf = new unsigned char[attrinfo.size];
		if (buf)
		{
			len = srcnode->ReadAttr(attrname, attrinfo.type, 0, buf, attrinfo.size);

			if (len>0)
				dstnode->WriteAttr(attrname, attrinfo.type, 0, buf, attrinfo.size);

			delete [] buf;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////
bool FSUtils::CopyAttr(const char *srcfilename, const char *dstfilename)
////////////////////////////////////////////////////////////////////////
{
	BNode srcnode(srcfilename);
	BNode dstnode(dstfilename);
	return CopyAttr(&srcnode, &dstnode);
}

////////////////////////////////////////////////////////////////////////
bool FSUtils::IsDirReadOnly(const char *destination)
////////////////////////////////////////////////////////////////////////
{
	struct stat statbuf;
	BDirectory dir(destination);
	BVolume volume;

	if (dir.InitCheck()!=B_OK)
		return false;

	if (dir.GetStatFor(destination, &statbuf)!=B_OK)
		return false;

	volume.SetTo(statbuf.st_dev);
	if (volume.IsReadOnly())
		return true;

	return false;	// Not read only
}


////////////////////////////////////////////////////////////////////////
bool FSUtils::IsRecursiveCopyMove(const char *source, const char *destination)
////////////////////////////////////////////////////////////////////////
{
	BEntry src(source);
	BEntry dst(destination);

	if (src == dst)
		return true;

	while ((dst.GetParent(&dst)) == B_OK)
	{
		if (src == dst)
			return true;
	}

	return false;
}
