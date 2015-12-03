/*
 * $Id$
 *
 * File:   main.cpp
 * Author: Werner Jaeger
 *
 * Created on November 17, 2015, 12:14 PM
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
#include "adbncfs.h"

/**
 * Main struct for FUSE interface.
 */
static struct fuse_operations adbfs_oper;

/*
 *
 */
int main(const int argc, char** const argv)
{
    int iRes(initAdbncFs(argc, argv));

    if (iRes == 0)
    {
        ::memset(&adbfs_oper, 0, sizeof(adbfs_oper));
        adbfs_oper.statfs= adbnc_statfs;
        adbfs_oper.opendir= adbnc_opendir;
        adbfs_oper.readdir= adbnc_readdir;
        adbfs_oper.releasedir= adbnc_releasedir;
        adbfs_oper.getattr= adbnc_getattr;
        adbfs_oper.access= adbnc_access;
        adbfs_oper.open= adbnc_open;
        adbfs_oper.flush = adbnc_flush;
        adbfs_oper.fsync = adbnc_fsync;
        adbfs_oper.release = adbnc_release;
        adbfs_oper.read= adbnc_read;
        adbfs_oper.write = adbnc_write;
        adbfs_oper.utimens = adbnc_utimens;
        adbfs_oper.truncate = adbnc_truncate;
        adbfs_oper.mknod = adbnc_mknod;
        adbfs_oper.mkdir = adbnc_mkdir;
        adbfs_oper.rename = adbnc_rename;
        adbfs_oper.rmdir = adbnc_rmdir;
        adbfs_oper.unlink = adbnc_unlink;
        adbfs_oper.readlink = adbnc_readlink;
        adbfs_oper.init = adbnc_init;
        adbfs_oper.destroy = adbnc_destroy;
        iRes = fuse_main(argc, argv, &adbfs_oper, NULL);
    }

    if (iRes)
        adbnc_destroy(NULL);

    return(iRes);
}

