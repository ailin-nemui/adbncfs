/*
 * $Id$
 *
 * File:   adbncfs.cpp
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
#include <sstream>

#include <sys/statvfs.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include "adbncfs.h"
#include "fileInfoCache.h"
#include "spawn.h"
#include "userInfo.h"
#include "mountInfo.h"

#define DBG(line) if (fDebug) cout << "--*-- " << line << endl
#define INF(line) cout << "--*-- " << line << endl
#define ERR(line) cerr << "--*-- " << line << endl

/**
 * If we run CppUnit test for adbnc we want to be able to test static methods.
 */
#ifdef ADBNCTEST
#define static
#endif

using namespace std;

/** Local and remote adb forward port */
static const int iForwardPort(4444);

static const char* pcDone = "---eoc---";    // end of command marker

/** Template used to makeTempDir() */
static const char* pcTempDirTemplate = "/tmp/adbncfs-XXXXXX";

/**
 * Big Hack, don't now how to figure out wher sdcard on a phone is mounted.
 *
 * @todo figure out how to get this information from the device.
 */
static const char* pcSdCardMountEntry = "sdcardfs on /storage/emulated/legacy type sdcardfs (rw,nosuid,nodev,relatime,uid=1023,gid=1023)";

/**
 * Path to the temporary directory used by the instance of this application
 * initialized in makeTempDir().
 */
static string strTempDirPath;

/** Debug mode as set in initAdbncFs() */
static bool fDebug(false);

/** Pointer to the netcat process initialized in initNetCat() */
static Spawn* pNetCat = NULL;

static FileCache fileCache;
static FileStatus fileStatus;

/**
 * Map to store directory listing retrieved with adbnc_opendir() from android
 * device. Key is the pathname to the directory.
 */
static map<string, deque<string> > openDirs;

/** Pointer to user info instance initialized in queryUserInfo() */
static UserInfo* pUserInfo = NULL;

/** Pointer to mount info instance initialized in queryMountInfo() */
static MountInfo* pMountInfo = NULL;

/** Mutex to synchronize thread access to execCommandViaNetCat() */
static pthread_mutex_t ncCmdMutex;

/** Mutex to synchronize thread access to adbnc_open() */
static pthread_mutex_t openMutex;

/**
 * Mutex to synchronize thread access to adbnc_opendir() and adbnc_releasedir()
 */
static pthread_mutex_t inReleaseDirMutex;

/**
 * Condition to block adbnc_opendir() until adbnc_releasedir() is finished.
 */
static pthread_cond_t inReleaseDirCond;

/** Waited for condition variable */
static bool fInReleaseDirCond(false);

/**
 * Segmentation fault handler.
 *
 * Is installed in initAdbncFs()
 *
 * @param iSig is supposed to be SIGSEGV.
 * @see initAdbncFs
 */
static void sig11Handler(int iSig)
{
    void *pvArray[10];

    // get void*'s for all entries on the stack
    const int iSize(::backtrace(pvArray, sizeof(*pvArray)));

    // print out all the frames to stderr
    ::fprintf(stderr, "Error: signal %d:\n", iSig);
    ::backtrace_symbols_fd(pvArray, iSize, 2);
    adbnc_destroy(NULL);
    ::exit(1);
}

/**
 * Spawns a netcat process on the local host with the local forward port.
 *
 * Writes the used start command to stdout.
 *
 * @return a reference to the spawned netcat process is returned.
 */
static Spawn& initNetCat()
{
    if (!pNetCat)
    {
        ostringstream strForward;
        strForward << iForwardPort;
        const char* const argv[] = { "nc", "localhost", strForward.str().c_str(), NULL };

        cout << "--*-- " << "spawn: ";
        for (int i = 0; argv[i]; i++)
        {
            cout << argv[i];
            if (argv[i+1])
                cout << ", ";
        }
        cout << endl;

        pNetCat = new Spawn(argv, false, true);
    }

    return(*pNetCat);
}

/**
 * Kills the netcat process spawned in initNetCat().
 */
static void destroyNetCat()
{
    if (pNetCat)
    {
        pNetCat->sendEof();
        INF("Waiting to terminate netcat...");
        INF("Status: " << pNetCat->wait());
        delete pNetCat;
        pNetCat = NULL;
    }
}

/**
 * Replace all instances of string strFind with string strReplace in the given
 * string strSource, which is modified in-place.
 *
 * The work proceeds down the string and skips over the text
 * introduced by the replacement.
 *
 * @param strSource the string to be modified.
 * @param strFind the pattern to be replaced.
 * @param strReplace the replacement for strFind.
 */
static void stringReplacer(string& strSource, const string& strFind, const string& strReplace)
{
    const int iLen(strFind.length());
    size_t j(0);
    for (; (j = strSource.find(strFind, j)) != string::npos ;)
    {
	strSource.replace(j, iLen, strReplace);
        j += strReplace.length();
    }
}

/**
 * Converts the given android path to a path on the local host.
 *
 * @param strPath the path as used on the android device.
 *
 * @return the path used on the local host within #strTempDirPath
 *
 * @see #pcTempDirTemplate
 * @see makeTempDir()
 */
static string makeLocalPath(const string& strPath)
{
    string strLocalPath(strTempDirPath);
    string strRemotePath(strPath);
    stringReplacer(strRemotePath, "/", "-");
    strLocalPath.append(strRemotePath);
    return(strLocalPath);
}

/**
 * Returns the parent pathname string of the given strPath.
 *
 * The parent of strPath consists of the pathname's prefix, if any, and
 * each name in the strPath name sequence except for the last.
 *
 * @param strPath the pathname thats parent to retrieve.
 *
 * @return The pathname string of the parent directory named by strPath
 */
static string parent(const string& strPath)
{
    string strParent(strPath);
    const size_t uiLen(strParent.length());

    /* if strPath == / we are done*/
    if (uiLen != 1 || strParent[0] != '/')
    {
        if (uiLen > 0)
        {
            /** strip last slash if there is one */
            if (strParent[uiLen - 1] == '/')
                strParent = strParent.substr(0, uiLen - 1);

            const std::size_t uiPos(strParent.rfind('/'));
            if (uiPos != string::npos)
                strParent = strParent.substr(0, uiPos == 0 ? 1 : uiPos);

            if (strParent == strPath)
                strParent = ".";
        }
        else
            strParent = ".";
    }

    return(strParent);
}

/**
 * Splits the given data string into tokens.
 *
 * The vector returned by this function contains each substring of the data
 * string that is terminated by  @code\t@endcode, or blank by the end of the data string.
 *
 * The substrings in the vector are in the order in which they occur in the
 * data string. If no blank or  @code\t@endcode, is in the data string then the resulting
 * vector has just one element, namely the data string.
 *
 * @param strData the string to split around blanks and tabs.
 *
 * @return a vector of tokens
 */
static vector<string> tokenize(const string& strData)
{
    vector<string> tokens;

    const char* pcDelimiters = " \t";
    const int iLen(strData.length() + 1);

    char acTokens[iLen];
    ::strncpy(acTokens, strData.c_str(), iLen);

    char *pcSave;
    char* pch = ::strtok_r(acTokens, pcDelimiters, &pcSave);
    while (pch != NULL)
    {
        tokens.push_back(pch);
        pch = ::strtok_r(NULL, pcDelimiters, &pcSave);
    }

    return(tokens);
}

/**
 * Tests whether the file or directory denoted by pcName exists on the local
 * host.
 *
 * @param pcName the pathname to test for existence.
 *
 * @return true if and only if the file or directory denoted by pcName exists;
 *         false otherwise
 */
static bool fileExists(const char* pcName)
{
    struct stat buffer;
    return (::stat(pcName, &buffer) == 0);
}

/**
 * Execute the given command string via netcat.
 *
 * @param strCommand the string to be executed as a command.
 *
 * @return the queue of lines written to stdout by the executed command.
 */
static deque<string>execCommandViaNetCat(const string& strCommand)
{
    ::pthread_mutex_lock(&ncCmdMutex);

    DBG("execCommandViaNetCat: " << strCommand);

    pNetCat->outStream() << strCommand << endl << "echo '" << pcDone << "'" << endl;

    deque<string> output;

    string strTmpString;
    while (!getline(pNetCat->inStream(), strTmpString).eof())
    {
        if (strTmpString.find(pcDone) != string::npos)
            break;

        output.push_back(strTmpString);
    }

    if (!output.empty())
        DBG("output: " << output.front());
    else
        DBG("output: EMPTY");

    ::pthread_mutex_unlock(&ncCmdMutex);

    return(output);
}

/**
 * Execute a program.
 *
 * if program could not be started of if no pipe could be created. an error
 * message is printed to stderr and if piError is not null error number is
 * returned in *piError.
 *
 * @param argv an array of pointers to null-terminated strings that represent
 *        the argument list available to the new program. The first argument,
 *        by convention, must point to the filename associated with the file
 *        being executed. The array of pointers must be terminated by a NULL
 *        pointer.
 * @param fUseStdErr if true stderr is redirected instead of stdout.
 *
 * @return the queue of lines written to stdout respectively stderr by the
 *         executed program.
 */
static deque<string> execProg(const char* const argv[], const bool fUseStdErr = false, int* const piError = NULL)
{
    if (fDebug)
    {
        cout << "--*-- ";
        for (int i = 0; argv[i]; i++)
        {
            cerr << argv[i];
            if (argv[i+1])
                cout << " ";
        }
        cout << endl;
    }

    deque<string> output;

    try
    {
        Spawn cmd(argv, fUseStdErr, true);

        string strTmpString;
        while (!getline(cmd.inStream(), strTmpString).eof())
        {
            if (fUseStdErr)
            {
                if (strTmpString.find(": Permission denied") != string::npos)
                {
                    if (piError)
                        *piError = -EACCES;

                    break;
                }
                else if (strTmpString.find("does not exist") != string::npos)
                {
                    if (piError)
                        *piError = -ENOENT;

                    break;
                }
            }

            output.push_back(strTmpString);
        }

        if (!output.empty())
            DBG("output: " << output.front());
        else
            DBG("output: EMPTY");

        cmd.sendEof();

        int iExitCode(cmd.wait());
        if (WIFEXITED(iExitCode))
        {
            iExitCode = WEXITSTATUS(iExitCode);
            if (iExitCode == ENOENT)
            {
                ERR("Error: program \"" << argv[0] << "\" not found");
                if (piError)
                    *piError = iExitCode;
            }
        }
    }
    catch (const runtime_error& error)
    {
        ERR(error.what());
        if (piError)
            *piError = -EIO;
    }

    return(output);
}

/**
 * Execute a shell command on the android device.
 *
 * The given string command is prefixed with "busybox ".
 *
 * @param strCommand the command to execute.
 *
 * @return the queue of lines written to stdout by the executed command.
 *
 * @see execCommandViaNetCat.
 */
static deque<string> adbncShell(const string& strCommand)
{
    string strActualCommand(strCommand);
    strActualCommand.insert(0, "busybox ");
    return(execCommandViaNetCat(strActualCommand));
}

static const string &quotePath(const string &path)
{
    static string buffer;
    string tmp("'");
    for (string::const_iterator i(path.begin()), end(path.end()); i != end; ++i) {
	if (*i == '\'')
	    tmp.append("'\"'\"'");
	else
	    tmp.append(1, *i);
    }
    tmp.append("'");
    buffer = tmp;
    return(buffer);
}

/**
 * Execute an adb push or pull command with given paths.
 *
 * @param fPush true for a push command, false for pull.
 * @param strLocalPath path on local host for push or pull command.
 * @param strRemotePath path on remote device for push or pull command.
   @return 0 if no error, non zero otherwise
 * @see adbncPull.
 * @see adbncPush.
 */
static int adbncPushPullCmd(const bool fPush, const string& strLocalPath, const string& strRemotePath)
{
    string strCmdPath1(quotePath((fPush ? strLocalPath : strRemotePath)));

    string strCmdPath2(quotePath((fPush ? strRemotePath : strLocalPath)));

    const char* argv[5];
    argv[0] = "adb";
    argv[1] = fPush ? "push" : "pull";
    argv[2] = fPush ? strLocalPath.c_str() : strRemotePath.c_str();
    argv[3] = fPush ? strRemotePath.c_str() : strLocalPath.c_str();
    argv[4] = NULL;

    int iRes(adbnc_access(fPush ? parent(strRemotePath).c_str() : strRemotePath.c_str(), fPush ? W_OK : R_OK));
    if (!iRes)
    {
        /*
         * uses stderr instead of stdout (second arg is true) because we need
         * the error messages which adb writes to stderr.
         *
         * Beside error messages adb also writes performance statistics to
         * stderr (e.g 238 KB/s (19074 bytes in 0.078s). This way we always have
         * at least one output line.
         *
         * Exit status is unfortunately not useful, it seems to be 256 always.
         */
        execProg(argv, true, &iRes);
    }

    return(iRes);
}

/**
 * Copy (using adb pull) a file from the Android device to the local
 * host.
 *
 * @param strRemoteSource Android-side file path to copy.
 * @param strLocalDestination local host-side destination path for copy.
 *
 * @return 0 if no error, non zero otherwise.
 *
 * @see adbncPush.
 * @see adbncPushPullCmd.
 *
 * @bug problems with files with ? in filenames (adb bug?)
 */
static int adbncPull(const string& strRemoteSource, const string& strLocalDestination)
{
    return(adbncPushPullCmd(false, strLocalDestination, strRemoteSource));
}

/**
 * Copy (using adb push) a file from the local host to the Android
 * device. Very similar to adbnc_pull().
 *
 * @return 0 if no error, non zero otherwise.
 *
 * @see adbncPull.
 * @see adbncPushPullCmd.
 *
 * @bug problems with files with ? in filenames (adb bug?).
 */
int adbncPush(const string& strLocalSource, const string& strRemoteDestination)
{
    return(adbncPushPullCmd(true, strLocalSource, strRemoteDestination));
}

/**
 * Execute a stat command on android file or directory denoted by pcPath.
 *
 * @param pcPath pathname of file or directory on android device.
 * @param pOutputTokens if not NULL receives the tokenized output of the stat.
 *        command.
 *
 * @return -ENOENT if pcPath does not exists, zero otherwise.
 */
static int doStat(const char *pcPath, vector<string>* pOutputTokens = NULL)
{
    deque<string> output;
    const deque<string>* pOutput(fileCache.getStat(pcPath));

    if (!pOutput)
    {
        string strCommand("stat -t ");
        strCommand.append(quotePath(pcPath));

        output = adbncShell(strCommand);
        fileCache.putStat(pcPath, output);
    }
    else
    {
        // from cache
        output = *pOutput;
        if (!output.empty())
            DBG("from cache " << output.front());
        else
            DBG("from cache EMPTY");
    }

    if (output.empty())
        return -ENOENT;

    if (output.size() > 1)
    {
        deque<string>::iterator it(output.begin());
        while (it != output.end())
            output.front() += *it++;
    }

    if (pOutputTokens)
    {
        *pOutputTokens = tokenize(output.front());
        if (pOutputTokens->size() < 13)
            return -ENOENT;

        while (pOutputTokens->size() > 15)
            pOutputTokens->erase(pOutputTokens->begin());
    }

    return(0);
}

/**
 * Return the command line used to start netcat process on android device.
 *
 * @return nc -ll -p forwardport -e /system/xbin/bash
 * @see androidStartNetcat
 */
static const string androidNetCatStartCommand()
{
    ostringstream strCmdStream;
    strCmdStream << "nc -ll -p " << iForwardPort << " -e /system/xbin/bash";

    return(strCmdStream.str());
}

/**
 * Test if netcat is started on the android device.
 *
 * @return pid of netcat or 0 if not started.
 */
static int androidNetcatStarted()
{
    int iPid(0);

    const char* const argv[] = { "adb", "shell", "su", "-c", "busybox", "ps", "|", "grep", androidNetCatStartCommand().c_str(), NULL };

    deque<string> output(execProg(argv));

    while (output.size() > 0)
    {
        if (output.front().find("grep") == string::npos)
        {
            vector<string> tokens(tokenize(output.front()));
            iPid = stoi(tokens[0].c_str());
            break;
        }
        else
            output.pop_front();
    }

    return(iPid);
}

/**
 * Kills the running netcat process on android device.
 *
 * @see androidStartNetcat.
 */
static void androidKillNetCat()
{
    int iPid(androidNetcatStarted());

    if (iPid > 0)
    {
        const string strPid(to_string(iPid));
        const char* const argv[] = { "adb", "shell", "su", "-c", "busybox", "kill", strPid.c_str(), NULL };
        execProg(argv);
    }

    iPid = androidNetcatStarted();
    if (iPid == 0)
        INF("Netcat successfully stopped on android device");
    else
        INF("Failed to kill NetCat on android device");
}

/**
 * Starts a netcat process on the android device.
 *
 * @return 0 if netcat could successfully be started, 3 otherwise.
 *
 * @see androidNetCatStartCommand.
 */
static int androidStartNetcat()
{
    if (!androidNetcatStarted())
    {
        const char* const argv[] = { "adb", "shell", "su", "-c", "busybox", "nohup", androidNetCatStartCommand().c_str(), "2>/dev/null",  "1>/dev/null", "&", NULL };
        execProg(argv);
    }

    const int iStarted(androidNetcatStarted());
    if (!iStarted)
        INF("error: could not start netcat on android device");
    else
        INF("Netcat successfully started on android device");

    return(iStarted ? 0 : 3);
}

/**
 * Tests if any android device is connected to a local host's usb port.
 *
 * @return 0 if a device is connected; 1 otherwise.
 */
static int isAndroidDeviceConnected()
{
    int iRes(1);

    // check if a android device is connected
    int iError(0);

    const char* const argv[] = { "adb", "devices", NULL };
    deque<string> output(execProg(argv, false, &iError));

    if (!iError)
    {
        if (output.size() > 2)
        {
            const string strDeviceId(output[output.size() - 2].substr(0, 8));

            if (strDeviceId != "List of ")
            {
                INF("Using android device " << strDeviceId);
                iRes = 0;
            } // error message already written by adb
         }  // error message already written by adb
    }

    return(iRes);
}

/**
 * Tests if tcp socket connections from local port to remote port on the android
 * device is in place.
 *
 * @param strForwardArg the local and remote port to be used for testing.
 *        e.g. "tcp:4444 tcp:4444"
 *
 * @return true if and only if adb port forwarding is in place; false otherwise.
 *
 * @see setAndroidPortForwarding
 */
static bool isAndroidPortForwarded(const string& strForwardArg)
{
    bool fRes(false);

    const char* const argv[] = { "adb", "forward", "--list", NULL };
    deque<string> output(execProg(argv));

    for(deque<string>::iterator it = output.begin(); it != output.end(); ++it)
    {
        if((*it).find(strForwardArg) != string::npos)
        {
            fRes = true;
            break;
        }
    }

    return(fRes);
}

/**
 * Enable adb port forwarding for our local and remote port.
 *
 * @return 0 if port forwarding could be enabled, 2 otherwise.
 */
static int setAndroidPortForwarding()
{
    int iRes(2);

    ostringstream strForwardPort;
    strForwardPort << "tcp:" << iForwardPort;

    ostringstream strForwardArg;
    strForwardArg << strForwardPort.str() << " " << strForwardPort.str();

    if (!isAndroidPortForwarded(strForwardArg.str()))
    {
        const char* const argv[] = { "adb", "forward", strForwardPort.str().c_str(), strForwardPort.str().c_str(), NULL };
        execProg(argv);
    }

    if (isAndroidPortForwarded(strForwardArg.str()))
    {
        INF("Port " << strForwardPort.str() << " successfully forwarded to android device");
        iRes = 0;
    }
    else
        INF("Failed to forward Port " << strForwardPort.str() << " to android device");

    return(iRes);
}

/**
 * Tries to remove android port forwarding.
 *
 * @return true if and only if adb port forwarding for our local and remote
 *         port is removed; false otherwise.
 *
 * @see setAndroidPortForwarding.
 */
static bool removeAndroidPortForwarding()
{
    ostringstream strForwardPort;
    strForwardPort << "tcp:" << iForwardPort;

    ostringstream strForwardArg;
    strForwardArg << strForwardPort.str() << " " << strForwardPort.str();

    if (isAndroidPortForwarded(strForwardArg.str()))
    {
        const char* const argv[] = { "adb", "forward", "--remove", strForwardPort.str().c_str(), NULL };
        execProg(argv);
    }

    bool fRet(!isAndroidPortForwarded(strForwardArg.str()));
    if (fRet)
        INF("Forward port " << strForwardPort.str() << " to android device successfully removed");
    else
        INF("Failed to remove forward port " << strForwardPort.str() << " from android device");

    return(fRet);
}

/**
 * Recursively deletes the temporary directory created in makeTempDir().
 */
static void cleanupTempDir(void)
{
    const char* const argv[] = { "rm", "-rf", strTempDirPath.c_str(), NULL };
    execProg(argv);
}

/**
 * Create a temporary directory and stores the path in #strTempDirPath variable.
 *
 * @return errno if an error occurred or 0 otherwise.
 *
 * @see acTempDirTemplate
 */
static int makeTempDir(void)
{
    char acTempDirTemplate[::strlen(pcTempDirTemplate) + 1];
    ::strncpy(acTempDirTemplate, pcTempDirTemplate, sizeof(acTempDirTemplate));
    const char * pcTempDir = ::mkdtemp(&acTempDirTemplate[0]);

    if (pcTempDir)
    {
        strTempDirPath.assign(pcTempDir);
        strTempDirPath.append("/");
    }

    return(pcTempDir ? 0 : errno);
}

/**
 * Query user information (uid, gid, groups) form android device.
 *
 * A "adb shell busybox id" command is executed and the output is stored in
 * a UserInfo instance for later retrieval. A pointer to the UserInfo object
 * is stored in #pUserInfo.
 *
 * If command failed en error message is written to stdout
 *
 * @return 0 if user information is obtained, 1 otherwise.
 */
static int queryUserInfo()
{
    const char* const argv[] = { "adb", "shell", "busybox id", NULL };
    deque<string> output(execProg(argv));

    int iRes(!(output.size() == 1));
    if (!iRes && output.front().length() > 5 && output.front()[0] == 'u')
        pUserInfo = new UserInfo(output.front().c_str());
    else
        INF("Failed to query user info from device");

    return(iRes);
}

/**
 * Query mount information form android device.
 *
 * A "adb shell busybox mount" command is executed and the output is stored in
 * a MontInfo instance for later retrieval. A pointer to the MontInfo object
 * is stored in #pMountInfo.
 *
 * If command failed en error message is written to stdout
 *
 * @return 0 if mount information is obtained, 1 otherwise.
 */
static int queryMountInfo()
{
    const char* const argv[] = { "adb", "shell", "busybox mount", NULL };
    deque<string> output(execProg(argv));

    int iRes(!(output.size() > 0));
    if (!iRes)
    {
        /*
         * Big Hack, coz. I don't know how to get mount info for sdcard from device.
         *
         * @todo get rid of that
         */
        output.push_back(pcSdCardMountEntry);
        pMountInfo = new MountInfo(output);
    }
    else
        INF("Failed to query mount info from device");

    return(iRes);
}

/**
 * Initialize the file system application
 *
 * - A segmentation fault signal handler() is installed.
 * - makeTempDir() is called.
 * - setAndroidPortForwarding() is called
 * - androidStartNetcat() is called
 * - queryUserInfo() is called
 * - queryMountInfo() is called
 * - initNetCat() is called
 *
 * @return 0 if a device is connected and no other error occurred;
 *         a value != 0 otherwise.
 */
int initAdbncFs(const int argc, char** const argv)
{
    ::signal(SIGSEGV, sig11Handler);   // install our handler

    bool fInitRequired(true);

    // check if debug option is set or -h/--help or -V/--version
    for (int i = 1; i < argc; i++)
    {
        if (::strcmp(argv[i], "-d") == 0)
            fDebug = true;

        if (::strcmp(argv[i], "-h") == 0)
            fInitRequired = false;

        if (::strcmp(argv[i], "--help") == 0)
            fInitRequired = false;

        if (::strcmp(argv[i], "-V") == 0)
            fInitRequired = false;

        if (::strcmp(argv[i], "--version") == 0)
            fInitRequired = false;
    }

    int iRes(0);

    if (fInitRequired)
    {
        iRes =makeTempDir();

        if (!iRes)
        {
            iRes = isAndroidDeviceConnected();

            if (!iRes)
            {
                iRes = setAndroidPortForwarding();

                if (!iRes)
                {
                    iRes = androidStartNetcat();
                    if (!iRes)
                    {
                        iRes = queryUserInfo();
                        if (!iRes)
                        {
                            iRes = queryMountInfo();
                            initNetCat();
                        }
                    }
                }
            }
        }
    }

    return(iRes);
}

/**
 * FUSE callback function to initialize the file system.
 *
 * One-time setup of #cmdMutex, #openMutex, #inReleaseDirMutex and
 * #inReleaseDirCond.
 *
 * @param pConn gives information about what features are supported by FUSE.
 *
 * @return NULL.
 */
void* adbnc_init(struct fuse_conn_info *pConn)
{
    DBG("adbnc_init()");

//    pConn->async_read = 0;
//    pConn->want &= ~ FUSE_CAP_ASYNC_READ; // clear async read flag
    pConn->want |= FUSE_CAP_EXPORT_SUPPORT; // set . and .. not handled by us

    ::pthread_mutex_init(&ncCmdMutex, NULL);
    ::pthread_mutex_init(&openMutex, NULL);
    ::pthread_mutex_init(&inReleaseDirMutex, NULL);
    ::pthread_cond_init (&inReleaseDirCond, NULL);

    return(NULL);
}

/**
 * FUSE callback function, called when the file system exits.
 *
 * - destruction of #cmdMutex, #openMutex, #inReleaseDirMutex and
 *   #inReleaseDirCond.
 * - destroyNetCat()
 * - androidKillNetCat()
 * - removeAndroidPortForwarding()
 * - delete #pMountInfo and pUserInfo
 *
 * @param private_data comes from the return value of adbnc_init().
 */
void adbnc_destroy(void* private_data)
{
    DBG("adbnc_destroy()");

    ::pthread_mutex_destroy(&ncCmdMutex);
    ::pthread_mutex_destroy(&openMutex);
    ::pthread_mutex_destroy(&inReleaseDirMutex);
    ::pthread_cond_destroy(&inReleaseDirCond);

    destroyNetCat();
    androidKillNetCat();
    removeAndroidPortForwarding();
    cleanupTempDir();

    if (pMountInfo)
    {
        delete pMountInfo;
        pMountInfo = NULL;
    }

    if (pUserInfo)
    {
        delete pUserInfo;
        pUserInfo = NULL;
    }
}

/**
 * FUSE callback function to retrieve statistics about the file system.
 *
 * this is how programs like df determine the free space.
 *
 * @param pcPath statistic is retrieved for the file system containing this
 *        path.
 * @param pFst See statvfs(2) for a description of the structure contents.
 *
 * @return zero on success, non zero otherwise.
 */
int adbnc_statfs(const char *pcPath, struct statvfs* pFst)
{
    DBG("adbnc_statfs(" << pcPath << ")");

    ::memset(pFst, 0, sizeof(struct statvfs));

    int iRes(0);

    if (::strcmp(pcPath, "/") != 0)
    {
        // fist get block info
        string strCommand("df -P -B 4096 ");
        strCommand.append(quotePath(pcPath));
        deque<string> output(adbncShell(strCommand));

        iRes = (output.size() > 1 ? 0 : -EIO);
        if (!iRes)
        {
            /*
                Filesystem           4K-blocks      Used Available Use% Mounted on
                tmpfs                   355740        35    355705   0% /dev
            */
            output.pop_front(); // remove header
            vector<string> tokens(tokenize(output.front()));
            if (tokens.size() >= 6)
            {
                try
                {
                    pFst->f_bsize = 4096;
                    pFst->f_blocks = stoul(tokens[1]);      // Total number of blocks on the file system
                    pFst->f_frsize = pFst->f_bsize;         // Fundamental file system block size (fragment size).
                    pFst->f_bfree =  stoul(tokens[3]);      // Total number of free blocks.
                    pFst->f_bavail = pFst->f_bfree;         // Total number of free blocks available to non-privileged processes.
                }
                catch (const exception& e)
                {
                    ERR("Exception thrown in adbnc_statfs(" << pcPath << ")" << ": " << e.what());

                    for (int i = 0; i < tokens.size(); i++)
                        ERR("Token[" << i << "] :" << tokens[i]);

                    iRes = -EIO;
                }
            }
        }

        if (!iRes)
        {
            // now get inode info
            strCommand.assign("df -P -i ");
            strCommand.append(quotePath(pcPath));
            output = adbncShell(strCommand);

            iRes = (output.size() > 1 ? 0 : -EIO);
            if (!iRes)
            {
                /*
                    Filesystem              Inodes      Used Available Capacity Mounted on
                    /dev/block/platform/msm_sdcc.1/by-name/efs                             896        99       797  11% /efs
                */
                output.pop_front(); // remove header
                vector<string> tokens(tokenize(output.front()));
                if (tokens.size() >= 6)
                {
                    try
                    {
                        pFst->f_files = stoul(tokens[1]);   // Total number of file nodes (inodes) on the file system.
                        pFst->f_ffree = stoul(tokens[3]);   // Total number of free file nodes (inodes).
                        pFst->f_favail = pFst->f_ffree;     // Total number of free file nodes (inodes) available to non-privileged processes.
                    }
                    catch (const exception& e)
                    {
                        ERR("Exception thrown in adbnc_statfs(" << pcPath << ")" << ": " << e.what());

                        for (int i = 0; i < tokens.size(); i++)
                            ERR("Token[" << i << "] :" << tokens[i]);

                        iRes = -EIO;
                    }
                }
            }

            // global / guessed info
            pFst->f_namemax = 1024; // Maximum length of a file name (path element).
        }
    }

    return(iRes);
}

/**
 * FUSE callback function to retrieve file attributes.
 *
 * For the given pathname, the elements of the "stat" structure are filled.
 * @param pcPath
 *
 * @param pStatBuf is described in detail in the stat(2) manual page.
 *
 * @return
 */
int adbnc_getattr(const char *pcPath, struct stat *pStatBuf)
{
    DBG("adbnc_getattr(" << pcPath << ")");

    int iRes(0);
    ::memset(pStatBuf, 0, sizeof(struct stat));
    vector<string> tokens;
    iRes = doStat(pcPath, &tokens);

    if (!iRes)
    {
        /*
           stat -t Explained:
           file name (%n)
           total size (%s)
           number of blocks (%b)
           raw mode in hex (%f)
           UID of owner (%u)
           GID of file (%g)
           device number in hex (%D)
           inode number (%i)
           number of hard links (%h)
           major device type in hex (%t)
           minor device type in hex (%T)
           last access time as seconds since the Unix Epoch (%X)
           last modification as seconds since the Unix Epoch (%Y)
           last change as seconds since the Unix Epoch (%Z)
           I/O block size (%o)
           */

        try
        {
            pStatBuf->st_ino = stoul(tokens[7].c_str());    /* inode number */
            const unsigned int uiRawMode(stoul(tokens[3], NULL, 16));
            pStatBuf->st_mode = uiRawMode | 0700;    /* protection */
            pStatBuf->st_nlink = 1;   /* number of hard links */
            pStatBuf->st_uid = stoul(tokens[4].c_str());    /* user ID of owner */
            pStatBuf->st_gid = stoul(tokens[5].c_str());    /* group ID of owner */

            const unsigned int uiDeviceId(stoul(tokens[6], NULL, 16));
            pStatBuf->st_rdev = uiDeviceId;    // device ID (if special file)

            pStatBuf->st_size = stoul(tokens[1].c_str());    /* total size, in bytes */
            pStatBuf->st_blksize = stol(tokens[14].c_str()); /* blocksize for filesystem I/O */
            pStatBuf->st_blocks = stoul(tokens[2].c_str());  /* number of blocks allocated */
            pStatBuf->st_atime = stol(tokens[11].c_str());   /* time of last access */
            pStatBuf->st_mtime = stol(tokens[12].c_str());   /* time of last modification */
            pStatBuf->st_ctime = stol(tokens[13].c_str());   /* time of last status change */
        }
        catch (const exception& e)
        {
            ERR("Exception thrown in adbnc_getattr(" << pcPath << ")" << ": " << e.what());

            for (int i = 0; i < tokens.size(); i++)
                ERR("Token[" << i << "] :" << tokens[i]);

            iRes = -EIO;
        }
    }

    return(iRes);
}

/**
 * FUSE callback to open a file.
 *
 * Calls adbncPull() to copy the file from the android device to local host and
 * opens the file on local host. The file handle obtained from local open is set
 * to pFi->fh;
 *
 * @param pcPath path to the filename to open.
 *
 * @param pFi pFi->fh receives the file handle if file could be opened
 *        successfully otherwise it is set to -1.
 *
 * @return -errno in case of an error, zero otherwise.
 */
int adbnc_open(const char *pcPath, struct fuse_file_info *pFi)
{
    ::pthread_mutex_lock(&openMutex);

    int iRes(0);

    DBG("adbnc_open(" << pcPath << ")");

    string strLocalPath(makeLocalPath(pcPath));

    if (!fileStatus.truncated(pcPath))
    {
        iRes = doStat(pcPath);
        if (!iRes && !fileExists(pcPath))
            iRes = adbncPull(pcPath, strLocalPath);
    }
    else
        fileStatus.truncated(pcPath, false);

    if (!iRes)
    {
        pFi->fh = ::open(strLocalPath.c_str(), pFi->flags);
        if (pFi->fh == -1)
            iRes = -errno;
    }

    ::pthread_mutex_unlock(&openMutex);

    return(iRes);
}

/**
 * FUSE callback to open a directory for reading.
 *
 * Stores directory content retrieved from android device in #openDirs map
 * for retrieval in adbnc_readdir().
 *
 * @param pcPath pathname of the directory to open.
 * @param pFi not used here.
 *
 * @return -EIO if failed to retrieve directory from android device, zero
 *         otherwise.
 */
int adbnc_opendir(const char *pcPath, struct fuse_file_info *pFi)
{
    int iRes(0);

    ::pthread_mutex_lock(&inReleaseDirMutex);

    /* wait until directory is released */
    while (fInReleaseDirCond)
        ::pthread_cond_wait(&inReleaseDirCond, &inReleaseDirMutex);

    DBG("adbnc_opendir(" << pcPath << ")");

    string strCommand("ls -1a ");
    strCommand.append(quotePath(pcPath));
    strCommand.append(" 2>/dev/null");
    deque<string> output(adbncShell(strCommand));

    if (!output.empty())
    {
        const map<string, deque<string> >::iterator it(openDirs.find(pcPath));
        if (it == openDirs.end())
            openDirs.insert(make_pair(pcPath, output));
    }
    else
        iRes = -EIO; // could also be EACCES

    ::pthread_mutex_unlock(&inReleaseDirMutex);

    return(iRes);
}

/**
 * FUSE callback to retrieve directory entries.
 *
 * #openDirs, filled by adbnc_opendir(), is supposed to hold the directory
 * listing.
 *
 * @param pcPath pathname of the directory get the listing from.
 * @param vpBuf buffer where result is returned.
 * @param filler FUSE provided helper function for putting directory entries
 *        into the result buffer.
 * @param iOffset start reading form this directory entry.
 * @param pFi not used here.
 *
 * @return -EBADF if directory listing was not found in #openDirs, zero
 *         otherwise.
 *
 */
int adbnc_readdir(const char *pcPath, void *vpBuf, fuse_fill_dir_t filler, off_t iOffset, struct fuse_file_info *pFi)
{
    int iRes(0);

    DBG("adbnc_readdir(" << pcPath << ")");

    const map<string, deque<string> >::const_iterator it(openDirs.find(pcPath));
    if (it != openDirs.end())
    {
        int iNumDirectoryEntries(it->second.size());
        for (int i(0); i < iNumDirectoryEntries; i++)
        {
            // Skip dot and dot-dot entries
            if (i < 2)
                continue;

            /* Skip this entry if we weren't asked for it */
            if (i < iOffset)
                continue;

            DBG("entry: " << it->second[i]);

            string strFullEntryPath(pcPath);
            if (strFullEntryPath != "/")
                strFullEntryPath.append("/");

            strFullEntryPath.append(it->second[i]);

            /* Skip this entry if file no longer exists */
            struct stat statBuf;
            if (adbnc_getattr(strFullEntryPath.c_str(), &statBuf) != 0)
                continue;

            /* Add this to our response until we are asked to stop */
            if (filler(vpBuf, it->second[i].c_str(), &statBuf, i+1))
                break;
        }
        /* All done because we were asked to stop or because we finished */
    }
    else
        iRes = -EBADF;

    return(iRes);
}

/**
 * FUSE callback to release given directory.
 *
 * Erases the directory listing entry in #openDirs.
 *
 * @param pcPath pathname to the directory to release.
 * @param pFi not used here.
 *
 * @return always zero;
 */
int adbnc_releasedir(const char *pcPath, struct fuse_file_info *pFi)
{
    int iRes(0);

    ::pthread_mutex_lock(&inReleaseDirMutex);

    fInReleaseDirCond = true;

    DBG("adbnc_releasedir(" << pcPath << ")");

    openDirs.erase(pcPath);

    /* signal waiting thread directory is released */
    ::pthread_cond_signal(&inReleaseDirCond);

    fInReleaseDirCond = false;

    ::pthread_mutex_unlock(&inReleaseDirMutex);

    return(iRes);
}

/**
 * FUSE callback function to resolve a link.
 *
 * If path is a symbolic link, fills pcBuf with its target, up to iSize.
 *
 * @param pcPath pathname of the link.
 * @param pcBuf receives the target name of the link.
 * @param iSize size of pcBuf.
 *
 * @return -ENOENT if pathname does not exist on android device, -ENOSYS if
 *         provided pcBuf is to small for the target name, zero otherwise.
 */
int adbnc_readlink(const char *pcPath, char *pcBuf, size_t iSize)
{
    DBG("adbnc_readlink(" << pcPath << ")");

    deque<string> output;
    const deque<string>* pOutput(fileCache.getReadLink(pcPath));

    if (!pOutput)
    {
        string strCommand("readlink -f ");
        strCommand.append(quotePath(pcPath));

        output = adbncShell(strCommand);

        fileCache.putReadLink(pcPath, output);
    }
    else
    {
        // from cache
        output = *pOutput;
        if (!output.empty())
            DBG("from cache " << output.front());
        else
            DBG("from cache EMPTY");
    }

    if(output.empty())
       return -ENOENT;

    string strRes(output.front());
    if (strRes[0] == '/')
    {
        int iPos(0);
        int iDepth(-1);
        while (pcPath[iPos] != '\0')
        {
            if (pcPath[iPos++] == '/')
                iDepth++;
        }

        strRes.erase(0, 1);
        while (iDepth > 0)
        {
            string strDotDot("..");
            strDotDot.append("/");
            strRes.insert(0, strDotDot);
            iDepth--;
        }
    }

    size_t iMySize(strRes.size());
    if (iMySize >= iSize)
       return -ENOSYS;

    ::memcpy(pcBuf, strRes.c_str(), iMySize + 1);
    return(0);
}

/**
 * FUSE callback to check whether file pcPath can be accessed.
 *
 * No need to follow links, pcPath never points to a link.
 *
 * @param pcPath pathname for that access is tested.
 * @param iMask specifies the accessibility check(s) to be performed, and is
 *        either the value F_OK, or a mask consisting of the bitwise OR of one
 *        or more of R_OK, W_OK, and X_OK. F_OK tests for the existence of the
 *        file. R_OK, W_OK, and X_OK test whether the file exists and grants
 *        read, write, and execute permissions, respectively.
 *
 * @return it returns -ENOENT if the path doesn't exist, -EACCESS if the
 *         requested permission isn't available, or 0 for success.
 */
int adbnc_access(const char *pcPath, int iMask)
{
    DBG("adbnc_access(" << pcPath << ")");

    /* Does it exist */
    vector<string> tokens;
    int iRes(doStat(pcPath, &tokens));
    if (iRes && iMask == F_OK)
        iRes = -ENOENT;

    if (!iRes && (iMask != F_OK))
    {
        unsigned int uiRawMode(0);
        int iUid(0);
        int iGid(0);

        try
        {
            /* Has it the right permission ?*/
            uiRawMode = stoul(tokens[3], NULL, 16);
            iUid = stoi(tokens[4].c_str());
            iGid = stoi(tokens[5].c_str());
        }
        catch (const exception& e)
        {
            ERR("Exception thrown in adbnc_access(" << pcPath << ")" << ": " << e.what());

            for (int i = 0; i < tokens.size(); i++)
                ERR("Token[" << i << "] :" << tokens[i]);

            iRes = -EIO;
        }

        if (!iRes && pUserInfo)
        {
            iRes = -pUserInfo->access(iUid, iGid, uiRawMode, iMask);
            if (!iRes && pMountInfo)
            {
                if (iMask & W_OK)
                {
                    if (pMountInfo->isMountedRo(pcPath))
                        iRes = -EACCES; // we are asked for write access but mounted ro
                }

                if (!iRes && (iMask & X_OK))
                {
                    if (pMountInfo->isMountedNoexec(pcPath))
                        iRes = -EACCES; // we are asked for execute access but mounted noexec
                }
            }
        }
    }

    return(iRes);
}

/**
 * FUSE callback function called on each close so that the file system has a
 * chance to report delayed errors.
 *
 * Important: I noticed flush calls for already closed files which causes
 * fsync() to return a EBADF, we'll ignore this error.
 *
 * @param pcPath path to the file to be flushed.
 * @param pFi pFi-fd the file descriptor of the file to flush.
 * @return 0 if success -errno otherwise but never EBADF.
 */
int adbnc_flush(const char *pcPath, struct fuse_file_info *pFi)
{
    int iRes(0);

    DBG("adbnc_flush(" << pcPath << ")");

    iRes = ::fsync(pFi->fh);
    if (!iRes || errno == EBADF)
    {
        string strLocalPath(makeLocalPath(pcPath));
        int iFlags(pFi->flags);

        DBG("flag is: " << iFlags);

        if (fileStatus.pendingOpen(pcPath))
            fileCache.invalidate(pcPath);

        iRes = fileStatus.flush(pcPath, strLocalPath);
    }
    else
        iRes = -errno;

    return(iRes);
}

int adbnc_fsync(const char* pcPath, int iIsdatasync, struct fuse_file_info* pFi)
{
    DBG("adbnc_fsync(" << pcPath << ")");

    int iRes(::fsync(pFi->fh));
    if (!iRes)
    {
        if (fileStatus.pendingOpen(pcPath))
        {
            iRes = adbncPush(makeLocalPath(pcPath), pcPath);
            fileStatus.pendingOpen(pcPath, false, false);
            fileCache.invalidate(pcPath);
        }
    }
    else
        iRes = -errno;

    return(iRes);
}

/**
 * FUSE callback called when FUSE is completely done with a file.
 *
 * Closes the file handle.
 *
 * @param pcPath path of the filename close.
 * @param pFi pFi->fh the file handle of the file to close
 *        as provided by adbnc_open().
 *
 * @return 0 if successfully closed, -EBADF if file handle is invalid or
 *         -errno i case of an close error.
 */
int adbnc_release(const char *pcPath, struct fuse_file_info *pFi)
{
    DBG("adbnc_release(" << pcPath << ")");

    return(fileStatus.release(pcPath, pFi->fh));
}

/**
 * FUSE callback to read iSize bytes from the given file into the buffer pcBuf,
 * beginning at iOffset bytes into the file.
 *
 * @param pcPath path of the filename to read from.
 * @param pcBuf receives the read bytes.
 * @param iSize number of bytes to read.
 * @param iOffset start reading at this offset.
 * @param pFi pFi->fh the file handle of the file to read from.
 *        as provided by adbnc_open().
 *
 * @return Returns the number of bytes transferred or -errno in
 *         case of an error.
 */
int adbnc_read(const char *pcPath, char *pcBuf, size_t iSize, off_t iOffset, struct fuse_file_info *pFi)
{
    int iRes(-EBADF);

    DBG("adbnc_read(" << pcPath << ")");

    fileStatus.pendingOpen(pcPath, true, false);

    if (pFi->fh != -1)
    {
        iRes = ::pread(pFi->fh, pcBuf, iSize, iOffset);
        if (iRes == -1)
            iRes = -errno;
    }

    return(iRes);
}

int adbnc_write(const char *pcPath, const char *pcBuf, size_t iSize, off_t iOffset, struct fuse_file_info *pFi)
{
    DBG("adbnc_write(" << pcPath << ")");

    fileStatus.pendingOpen(pcPath, true, true);

    int iRes(::pwrite(pFi->fh, pcBuf, iSize, iOffset));

    return(iRes == -1 ? -errno : iRes);
}

int adbnc_utimens(const char *pcPath, const struct timespec ts[2])
{
    DBG("adbnc_utimens(" << pcPath << ")");

    fileCache.invalidate(pcPath);

    string command("touch ");
    command.append(quotePath(pcPath));

    adbncShell(command);

    return(0);
}

int adbnc_truncate(const char *pcPath, off_t iSize)
{
    DBG("adbnc_truncate(" << pcPath << ")");

    int iRes(doStat(pcPath));
    if (!iRes)
    {
        const string strLocalPath(makeLocalPath(pcPath));

        iRes = ::truncate(strLocalPath.c_str(), iSize);
        if (!iRes)
        {
            DBG("truncate[path=" << strLocalPath << "][size=" << iSize << "]");

            fileStatus.truncated(pcPath, true);
            fileCache.invalidate(pcPath);
        }
        else
            iRes = -errno;
    }

    return(iRes);
}

int adbnc_mknod(const char *pcPath, mode_t mode, dev_t rdev)
{
    DBG("adbnc_mknod(" << pcPath << ")");

    const string strLocalPath(makeLocalPath(pcPath));

    DBG("mknod for " << strLocalPath);

    int iRes(::mknod(strLocalPath.c_str(), mode, rdev));
    if (!iRes)
    {
        iRes = adbncPush(strLocalPath, pcPath);
        if (!iRes)
            adbncShell("sync");

        fileCache.invalidate(pcPath);
    }
    else
        iRes = -errno;

    return(iRes);
}

int adbnc_mkdir(const char *pcPath, mode_t mode)
{
    DBG("adbnc_mkdir(" << pcPath << ")");

    fileCache.invalidate(pcPath);
    string strCommand("mkdir ");
    strCommand.append(quotePath(pcPath));

    DBG("Making directory " << pcPath);

    adbncShell(strCommand);
    return(0);
}

int adbnc_rename(const char *pcFrom, const char *pcTo)
{
    DBG("adbnc_rename(" << pcFrom << ", " << pcTo << ")");

    string strCommand("mv ");
    strCommand.append(quotePath(pcFrom));
    strCommand.append(" ");
    strCommand.append(quotePath(pcTo));

    DBG("Renaming " << pcFrom << " to " << pcTo);

    adbncShell(strCommand);

    // invalidate to cache I don't check here if from File is different to toFile
    fileCache.invalidate(pcTo);

    // from file no longer exists -> invalidate in cache
    fileCache.invalidate(pcFrom);

    if (fileStatus.pendingOpen(pcFrom))
    {
        // transfer existing pending open to renamed
        fileStatus.pendingOpen(pcTo, makeLocalPath(pcFrom));
    }

    return(0);
}

int adbnc_rmdir(const char *pcPath)
{
    DBG("adbnc_rmdir(" << pcPath << ")");

    fileCache.invalidate(pcPath);
    string strCommand("rmdir ");
    strCommand.append(quotePath(pcPath));

    DBG("Removing directory " << pcPath);

    adbncShell(strCommand);

    return(0);
}

/**
 * FUSE callback function to remove (delete) the given file, symbolic link,
 * hard link, or special node.
 *
 * Note that if you support hard links, unlink only deletes the data when the
 * last hard link is removed. See unlink(2) for details.
 *
 * @param pcPath oath to filename to unlink.
 *
 * @return 0 if success, -errno otherwise.
 */
int adbnc_unlink(const char *pcPath)
{
    DBG("adbnc_unlink(" << pcPath << ")");

    fileCache.invalidate(pcPath);

    string strCommand("rm ");
    strCommand.append(quotePath(pcPath));

    DBG("Deleting " << pcPath);

    ::unlink(makeLocalPath(pcPath).c_str());
    adbncShell(strCommand);

    return(0);
}

