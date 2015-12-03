/*
 * $Id$
 *
 * File:   adbncfs.h
 * Author: Werner Jaeger
 *
 * Created on November 17, 2015, 1:28 PM
 *
 * Copyright 2015 Werner Jaeger.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADBNCFS_H
#define ADBNCFS_H

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <map>

int adbnc_statfs(const char *pcPath, struct statvfs* pFst);
int adbnc_getattr(const char *pcPath, struct stat *oStatBuf);
int adbnc_open(const char *pcPath, struct fuse_file_info *pFi);
int adbnc_opendir(const char *pcPath, struct fuse_file_info *pFi);
int adbnc_readdir(const char *pcPath, void *vpBuf, fuse_fill_dir_t filler, off_t iOffset, struct fuse_file_info *pFi);
int adbnc_releasedir(const char *pcPath, struct fuse_file_info *pFi);
int adbnc_readlink(const char *pcPath, char *pcBuf, size_t iSize);
int adbnc_access(const char *pcPath, int iMask);
int adbnc_flush(const char *pcPath, struct fuse_file_info *pFi);
int adbnc_release(const char *pcPath, struct fuse_file_info *pFi);
int adbnc_read(const char *pcPath, char *pcBuf, size_t iSize, off_t iOffset, struct fuse_file_info *pFi);
int adbnc_write(const char *pcPath, const char *pcBuf, size_t iSize, off_t iOffset, struct fuse_file_info *pFi);
int adbnc_utimens(const char *pcPath, const struct timespec ts[2]);
int adbnc_truncate(const char *pcPath, off_t iSize);
int adbnc_mknod(const char *pcPath, mode_t mode, dev_t rdev);
int adbnc_mkdir(const char *pcPath, mode_t mode);
int adbnc_rename(const char *pcFrom, const char *pcTo);
int adbnc_rmdir(const char *pcPath);
int adbnc_unlink(const char *pcPath);
int adbnc_fsync(const char* pcPath, int iIsdatasync, struct fuse_file_info* pFi);
void*adbnc_init(struct fuse_conn_info *pConn);
void adbnc_destroy(void* private_data);
int initAdbncFs(const int argc, char** const argv);

#endif /* ADBNCFS_H */

