/*
 * $Id$
 *
 * File:   testMountPoint.cpp
 * Author: Werner Jaeger
 *
 * Created on Nov 29, 2015, 7:52:10 PM
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

#include "testMountPoint.h"

CPPUNIT_TEST_SUITE_REGISTRATION(testMountPoint);

testMountPoint::testMountPoint() : m_MountInfo(initMountInfo())
{
}

testMountPoint::~testMountPoint()
{
}

void testMountPoint::setUp()
{
}

void testMountPoint::tearDown()
{
}

void testMountPoint::testMountPointInfo()
{
    CPPUNIT_ASSERT(m_MountInfo.mountPoint("/system/bin/adb")->mountPoint() == string("/system"));
    CPPUNIT_ASSERT(m_MountInfo.mountPoint("/sbin/adb")->mountPoint() == string("/"));
}

void testMountPoint::testMountPointOptions()
{
    CPPUNIT_ASSERT(m_MountInfo.isMountedNoexec("/mnt/media_rw/extSdCard"));
    CPPUNIT_ASSERT(m_MountInfo.isMountedRo("/system/bin"));
}

deque<string> testMountPoint::initMountInfo()
{
    deque<string> mountOutput;

    mountOutput.push_back("rootfs on / type rootfs (ro,relatime)");
    mountOutput.push_back("tmpfs on /dev type tmpfs (rw,seclabel,nosuid,relatime,mode=755)");
    mountOutput.push_back("devpts on /dev/pts type devpts (rw,seclabel,relatime,mode=600)");
    mountOutput.push_back("proc on /proc type proc (rw,relatime)");
    mountOutput.push_back("sysfs on /sys type sysfs (rw,seclabel,relatime)");
    mountOutput.push_back("selinuxfs on /sys/fs/selinux type selinuxfs (rw,relatime)");
    mountOutput.push_back("debugfs on /sys/kernel/debug type debugfs (rw,relatime)");
    mountOutput.push_back("none on /acct type cgroup (rw,relatime,cpuacct)");
    mountOutput.push_back("none on /sys/fs/cgroup type tmpfs (rw,seclabel,relatime,mode=750,gid=1000)");
    mountOutput.push_back("tmpfs on /mnt/secure type tmpfs (rw,seclabel,relatime,mode=700)");
    mountOutput.push_back("tmpfs on /mnt/secure/asec type tmpfs (rw,seclabel,relatime,mode=700)");
    mountOutput.push_back("tmpfs on /mnt/asec type tmpfs (rw,seclabel,relatime,mode=755,gid=1000)");
    mountOutput.push_back("tmpfs on /mnt/obb type tmpfs (rw,seclabel,relatime,mode=755,gid=1000)");
    mountOutput.push_back("none on /dev/cpuctl type cgroup (rw,relatime,cpu)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/system on /system type ext4 (ro,seclabel,relatime,data=ordered)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/userdata on /data type ext4 (rw,seclabel,nosuid,nodev,noatime,discard,journal_checksum,journal_async_commit,noauto_da_alloc,data=ordered)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/cache on /cache type ext4 (rw,seclabel,nosuid,nodev,noatime,discard,journal_checksum,journal_async_commit,noauto_da_alloc,errors=panic,data=ordered)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/persist on /persist type ext4 (rw,seclabel,nosuid,nodev,noatime,discard,journal_checksum,journal_async_commit,noauto_da_alloc,data=ordered)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/efs on /efs type ext4 (rw,seclabel,nosuid,nodev,noatime,discard,journal_checksum,journal_async_commit,noauto_da_alloc,data=ordered)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/persdata on /persdata/absolute type ext4 (rw,seclabel,nosuid,nodev,relatime,data=ordered)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/apnhlos on /firmware type vfat (ro,context=u:object_r:firmware:s0,relatime,uid=1000,gid=1000,fmask=0337,dmask=0227,codepage=cp437,iocharset=iso8859-1,shortname=lower,errors=remount-ro)");
    mountOutput.push_back("/dev/block/platform/msm_sdcc.1/by-name/modem on /firmware-modem type vfat (ro,context=u:object_r:firmware:s0,relatime,uid=1000,gid=1000,fmask=0337,dmask=0227,codepage=cp437,iocharset=iso8859-1,shortname=lower,errors=remount-ro)");
    mountOutput.push_back("/dev/block/mmcblk0p25 on /preload type ext4 (ro,seclabel,nosuid,nodev,relatime,data=ordered)");
    mountOutput.push_back("/data/media on /mnt/shell/emulated type sdcardfs (rw,nosuid,nodev,relatime,uid=1023,gid=1023)");
    mountOutput.push_back("/data/knox/sdcard on /mnt/shell/knox-emulated type sdcardfs (rw,nosuid,nodev,relatime,uid=1000,gid=1000)");
    mountOutput.push_back("/dev/block/dm-0 on /mnt/asec/net.halfmobile.scannerpro-1 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-1 on /mnt/asec/com.pathtracker-1 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-2 on /mnt/asec/com.voicetranslator.SpeakAndTranslatePro-23 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-3 on /mnt/asec/kr.aboy.sound-8 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-4 on /mnt/asec/com.realex-2 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-6 on /mnt/asec/com.yuvalluzon.yourmagnifierpro-4 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-7 on /mnt/asec/com.flyersoft.moonreaderp-36 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-8 on /mnt/asec/com.wolfram.android.alpha-6 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-9 on /mnt/asec/com.asrazpaid-3 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/dm-10 on /mnt/asec/com.fiistudio.key.fiinote-2 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");
    mountOutput.push_back("/dev/block/vold/179:64 on /mnt/media_rw/extSdCard type exfat (rw,dirsync,nosuid,nodev,noexec,noatime,nodiratime,uid=1023,gid=1023,fmask=0007,dmask=0007,allow_utime=0020,codepage=cp437,iocharset=utf8,namecase=0,errors=remount-ro)");
    mountOutput.push_back("/dev/block/vold/179:64 on /mnt/secure/asec type exfat (rw,dirsync,nosuid,nodev,noexec,noatime,nodiratime,uid=1023,gid=1023,fmask=0007,dmask=0007,allow_utime=0020,codepage=cp437,iocharset=utf8,namecase=0,errors=remount-ro)");
    mountOutput.push_back("/mnt/media_rw/extSdCard on /storage/extSdCard type sdcardfs (rw,nosuid,nodev,relatime,uid=1023,gid=1023)");
    mountOutput.push_back("/dev/block/dm-11 on /mnt/asec/air.com.llingo.tga_l65_pro-6 type ext4 (ro,dirsync,seclabel,nosuid,nodev,noatime,errors=continue)");

    return(mountOutput);
}
