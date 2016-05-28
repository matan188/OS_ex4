/*
 * CachingFileSystem.cpp
 *
 *  Author: Netanel Zakay, HUJI, 67808  (Operating Systems 2015-2016).
 */

#define FUSE_USE_VERSION 26

#include <iostream>
#include <fuse.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <limits.h>
#include <errno.h>
#include <cstdlib>
#include <dirent.h>
#include <cstring>
#include <map>
#include <cmath>
#include <vector>

#include "CDE.h"
#include "LRUStack.h"
#include "CountChain.h"

using namespace std;

#define CMAX 3 // the maximum count list value
#define USAGE_ERROR "Usage: CachingFileSystem rootdir mountdir numberOfBlocks fOld fNew\n"
#define FILE_LOCATION ".filesystem.log"

struct fuse_operations caching_oper;

struct {
    char* rootDir;
} typedef user_data;

static char* rootDir;
static int blockSize;
static map<pair<string, int>, CDE*> cacheMap;
static LRUStack lru;
static CountChain countChain(CMAX);
static int numberOfBlocks;

//TODO change log filename (remember the DOT)
//TODO clear debug prints
//TODO -Wextra to makefile
//TODO rename a directory
//TODO cache by realpath?

std::ofstream logFile;
std::stringstream logBuffer;

static int getBlockSize() {
    struct stat fi;
    stat("/tmp", &fi);
    return (int) fi.st_blksize;
}

static void buildPath(char fpath[PATH_MAX], const char *path) {
    strcpy(fpath, rootDir);
    strncat(fpath, path, PATH_MAX);
}

bool isInputParamsValid(int argc, char* argv[]) {
    // Usage error
    bool usage_error = false;
    if(argc != 6) {
        usage_error = true;
        //cout << "1" << endl;
    } else {
        struct stat sb;
        // fetch params
        char* rootdir = argv[1];
        char* mountdir = argv[2];
        int numberOfBlocks = atoi(argv[3]);
        double fOld = atof(argv[4]);
        double fNew = atof(argv[5]);

        // check params are valid
        if(fOld <= 0 || fOld >= 1 || fNew <= 0 || fNew >= 1 || fOld + fNew > 1) {
            usage_error = true;
            //cout << "2" << endl;
        } else if(numberOfBlocks < 0) {
            usage_error = true;
            //cout << "3" << endl;
        } else if(stat(rootdir, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
            usage_error = true;
            //cout << "4" << endl;
        } else if(stat(mountdir, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
            usage_error = true;
            //cout << "5" << endl;
        }
    }
    if(usage_error) {
        cout << USAGE_ERROR << endl;
    }
    return !usage_error;
}

void sysError(std::string errFunc) {
    std::cerr << "System Error: " <<
    errFunc << " failed." << std::endl;
    exit(1);
}

void writeToLog(string msg) {
    logFile.open(FILE_LOCATION, std::ios_base::app);
    if(logFile.fail()) {
        sysError("open");
    }

    time_t t;
    if((int) time(&t) < 0) {
        sysError("time");
    }

    //cout << ">> " << msg << endl;

    logFile << t << " " << msg << "\n";
    logFile.close();
    if(logFile.fail()) {
        sysError("close");
    }
}

int calcNumberOfBlocks(int size, int offset) {
    int n = 0;
    if(size == 0) {
        return 0;
    }
    if(offset % blockSize != 0) {
        n += 1;
        size -= blockSize - (offset % blockSize);
    }
    n += ceil(size / (double) blockSize);
    return n;
}

void removeFromCache() {
    //cout << "01" << endl;
    CDE * cde = countChain.getItemToRemove();
    //cout << "02" << endl;
    if(cde == nullptr) {
       cde = lru.getTail();
    }
    //cout << "03" << endl;
    lru.remove(cde);
    countChain.remove(cde);
    string fileName = cde->getFileName();
    int blockId = cde->getBlockId();
    //cout << "04" << endl;
    cacheMap.erase({fileName, blockId});
    delete cde;
}

/****************************************************************************/

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int caching_getattr(const char *path, struct stat *statbuf){
    writeToLog("getattr");
    char fpath[PATH_MAX];
    buildPath(fpath, path);
    int ret = stat(fpath, statbuf);
    //cout << statbuf->st_size << endl;
    if(ret != 0) {
        return -errno;
    }
	return ret;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int caching_fgetattr(const char *path, struct stat *statbuf, 
					struct fuse_file_info *fi){
    writeToLog("fgetattr");
    int ret = fstat((int) fi->fh, statbuf);
    if(ret != 0) {
        return -errno;
    }
    return ret;
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int caching_access(const char *path, int mask)
{
    writeToLog("access");
    char fpath[PATH_MAX];
    buildPath(fpath, path);

    int ret = access(fpath, mask);
    if(ret != 0) {
        return -errno;
    }
    return ret;
}


/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * initialize an arbitrary filehandle (fh) in the fuse_file_info 
 * structure, which will be passed to all file operations.

 * pay attention that the max allowed path is PATH_MAX (in limits.h).
 * if the path is longer, return error.

 * Changed in version 2.2
 */
int caching_open(const char *path, struct fuse_file_info *fi){
    writeToLog("open");
    fi->direct_io = 1;
    char fpath[PATH_MAX];
    buildPath(fpath, path);
    // if path too long
    if(sizeof(*fpath) > PATH_MAX) {
        return -ENAMETOOLONG;
    }

    // check the access is valid (read only)
    if((fi->flags & 3) != O_RDONLY) {
        return -EACCES;
    }

    int ret = open(fpath, (O_RDONLY | O_DIRECT | O_SYNC));
    if(ret == -1) {
        return -errno;
    }
    fi->fh = (uint64_t) ret;
    return 0;
}


/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error. For example, if you receive size=100, offest=0,
 * but the size of the file is 10, you will init only the first 
   ten bytes in the buff and return the number 10.
   
   In order to read a file from the disk, 
   we strongly advise you to use "pread" rather than "read".
   Pay attention, in pread the offset is valid as long it is 
   a multipication of the block size.
   More specifically, pread returns 0 for negative offset 
   and an offset after the end of the file
   (as long as the the rest of the requirements are fulfiiled).
   You are suppose to preserve this behavior also in your implementation.

 * Changed in version 2.2
 */
int caching_read(const char *path, char *buf, size_t size, 
				off_t offset, struct fuse_file_info *fi){
    writeToLog("read");
    //cout << "size: " << size << endl;
    //cout << "offset: " << offset << endl;
    int currentBlock = (int) offset / blockSize;
    CDE * cde;
    ssize_t b = 0;
    string fileName = string(path);

    //TODO what if offset + size > file_size?

    //cout << "<filename: "<< fileName << ", currentBlock: " << currentBlock << ">" << endl;
    if(cacheMap.count({fileName, currentBlock}) > 0) {
        // cache hit
        //cout << "<cache hit>" << endl;
        cde = cacheMap[{fileName, currentBlock}];
        //cout << "01" << endl;
        /*
        if(currentBlock * blockSize + cde->getSize() <= offset) {
            cout << "RETURN: 0" << endl;
            return 0;
        }
        */

        int readSize = 0;
        int x = blockSize * currentBlock;
        if((size_t) offset >= cde->getSize() + x) {
            //cout << "RETURN: 0" << endl;
            return 0;
        } else if(offset + size < cde->getSize() + x) {
            readSize = (int) size;
        } else {
            readSize = (cde->getSize() + x) - (int) offset;
        }

        int inBlockOffset = (int) offset - currentBlock * blockSize;
        /*
        if(size > blockSize - inBlockOffset) {
            size = (size_t) blockSize - inBlockOffset;
        }
        */
        strncpy(buf, cde->getData() + inBlockOffset, (size_t) readSize);
        countChain.increment(cde);
        lru.reinsert(cde);
        //cout << "RETURN: " << readSize << endl;
        return (int) readSize;
    } else {
        // cache miss
        //cout << "<cache miss>" << endl;
        char* blockData = (char *) aligned_alloc(blockSize, blockSize * sizeof(char));
        b = pread((int) fi->fh, (void *) blockData, (size_t) blockSize, offset);
        //cout << "bytes read: " << b << endl;
        if(b < 0) {
            cout << "errno: " << errno << endl;
        } else if(b == 0) {
            free(blockData);
            return 0;
        }

        cacheMap[{fileName, currentBlock}] = new CDE(currentBlock, fileName, b, blockData);

        CDE* cde = cacheMap[{fileName, currentBlock}];

        free(blockData);


        // add the new cde (which has count of 1 to CountChain[0]
        countChain.insert(cde, 1);

        if((int) lru.getSize() < numberOfBlocks) {
            // there is empty place in cache
            lru.insert(cde);
            cde->setIsNew(true);
        } else {
            // cache is full, use replacement policy
            if(numberOfBlocks != 0) {
                removeFromCache();
                lru.insert(cde);
            }
        }
        strncpy(buf, cde->getData(), (size_t) b);
        //cout << "RETURN: " << b << endl;
        return (int) b;
    }
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int caching_flush(const char *path, struct fuse_file_info *fi)
{
    writeToLog("flush");
    return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int caching_release(const char *path, struct fuse_file_info *fi){
    writeToLog("release");
    int ret = close(fi->fh);
    if(ret != 0) {
        return -errno;
    }
    return ret;
}

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int caching_opendir(const char *path, struct fuse_file_info *fi){
    writeToLog("opendir");

    DIR *dp;
    int ret = 0;

    char fpath[PATH_MAX];
    buildPath(fpath, path);

    dp = opendir(fpath);
    if(dp == NULL) {
        ret = -errno;
    } else {
        fi->fh = (intptr_t) dp;
    }
    return ret;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * Introduced in version 2.3
 */
int caching_readdir(const char *path, void *buf, 
					fuse_fill_dir_t filler, 
					off_t offset, struct fuse_file_info *fi){
    writeToLog("readdir");
    int ret = 0;
    DIR *dp;
    struct dirent *de;

    dp = (DIR *) fi->fh;
    de = readdir(dp);

    if(de == 0) {
        return -errno;
    }

    do {
        if(filler(buf, de->d_name, NULL, 0) != 0) {
            return -ENOMEM;
        }
    } while((de = readdir(dp)) != NULL);

	return ret;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int caching_releasedir(const char *path, struct fuse_file_info *fi){
    writeToLog("releasedir");
    int ret = closedir((DIR *) fi->fh);
    if(ret != 0) {
        return -errno;
    }
    return ret;
}

/** Rename a file */
int caching_rename(const char *path, const char *newpath){
    writeToLog("rename");

    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    buildPath(fpath, path);
    buildPath(fnewpath, newpath);

    int ret = rename(fpath, fnewpath);
    if(ret != 0) {
        return -errno;
    }

    // rename block in cache
    //lru.printLru();
    CDE * cde = lru.getHead();
    while(cde != nullptr) {
        string fileName = cde->getFileName();
        int pos = fileName.find(string(path));
        //cout << "fileName: " << fileName << " pos: " << pos << endl;
        if(pos == 0 && (fileName[string(path).length() - 1] == '/' || fileName.length() == string(path).length())) {

            string suffix = fileName.substr(string(path).length(), fileName.length());

            string realNewPath = string(newpath) + suffix;
            //cout << "%%%%%% realNewPath: " << realNewPath << endl;
            // block was in cache. need to be renamed
            cde->setFileName(realNewPath);
            cacheMap[{realNewPath, cde->getBlockId()}] = cde;
            cacheMap.erase({string(path), cde->getBlockId()});
        }
        cde = cde->getNext();
    }
    //lru.printLru();
    /*
    for(int i = 0; i < blocksInFile; ++i) {
        // rename block if it's in cache
        if(cacheMap.count({path, i}) > 0) {
            // block was in cache. need to be renamed
            CDE * cde = cacheMap[{path, i}];
            cde->setFileName(string(newpath));
            cacheMap[{newpath, i}] = cde;
            cacheMap.erase({path, i});
        }
    }
     */

    return ret;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 
If a failure occurs in this function, do nothing (absorb the failure 
and don't report it). 
For your task, the function needs to return NULL always 
(if you do something else, be sure to use the fuse_context correctly).
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *caching_init(struct fuse_conn_info *conn){
	return NULL;
}


/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
  
If a failure occurs in this function, do nothing 
(absorb the failure and don't report it). 
 
 * Introduced in version 2.3
 */
void caching_destroy(void *userdata){
}


/**
 * Ioctl from the FUSE sepc:
 * flags will have FUSE_IOCTL_COMPAT set for 32bit ioctls in
 * 64bit environment.  The size and direction of data is
 * determined by _IOC_*() decoding of cmd.  For _IOC_NONE,
 * data will be NULL, for _IOC_WRITE data is out area, for
 * _IOC_READ in area and if both are set in/out area.  In all
 * non-NULL cases, the area is of _IOC_SIZE(cmd) bytes.
 *
 * However, in our case, this function only needs to print 
 cache table to the log file .
 * 
 * Introduced in version 2.8
 */
int caching_ioctl (const char *, int cmd, void *arg,
		struct fuse_file_info *, unsigned int flags, void *data){
	return 0;
}


// Initialise the operations. 
// You are not supposed to change this function.
void init_caching_oper()
{

	caching_oper.getattr = caching_getattr;
	caching_oper.access = caching_access;
	caching_oper.open = caching_open;
	caching_oper.read = caching_read;
	caching_oper.flush = caching_flush;
	caching_oper.release = caching_release;
	caching_oper.opendir = caching_opendir;
	caching_oper.readdir = caching_readdir;
	caching_oper.releasedir = caching_releasedir;
	caching_oper.rename = caching_rename;
	caching_oper.init = caching_init;
	caching_oper.destroy = caching_destroy;
	caching_oper.ioctl = caching_ioctl;
	caching_oper.fgetattr = caching_fgetattr;


	caching_oper.readlink = NULL;
	caching_oper.getdir = NULL;
	caching_oper.mknod = NULL;
	caching_oper.mkdir = NULL;
	caching_oper.unlink = NULL;
	caching_oper.rmdir = NULL;
	caching_oper.symlink = NULL;
	caching_oper.link = NULL;
	caching_oper.chmod = NULL;
	caching_oper.chown = NULL;
	caching_oper.truncate = NULL;
	caching_oper.utime = NULL;
	caching_oper.write = NULL;
	caching_oper.statfs = NULL;
	caching_oper.fsync = NULL;
	caching_oper.setxattr = NULL;
	caching_oper.getxattr = NULL;
	caching_oper.listxattr = NULL;
	caching_oper.removexattr = NULL;
	caching_oper.fsyncdir = NULL;
	caching_oper.create = NULL;
	caching_oper.ftruncate = NULL;
}


int main(int argc, char* argv[]){

    // Check input parameters
    if(!isInputParamsValid(argc, argv)) {
        exit(0);
    }

	init_caching_oper();


    blockSize = getBlockSize();
    rootDir = realpath(argv[1], NULL);


    numberOfBlocks = atoi(argv[3]);

    lru.setNewIndex((int) (atof(argv[5]) * numberOfBlocks));
    lru.setOldIndex(numberOfBlocks - (int) (atof(argv[4]) * numberOfBlocks) + 1);


    // arrange args for fuse_main call
	argv[1] = argv[2];
	for (int i = 2; i< (argc - 1); i++){
		argv[i] = NULL;
	}
	argv[2] = (char*) "-s";
    //argv[3] = (char*) "-f";
	argc = 3;


	int fuse_stat = fuse_main(argc, argv, &caching_oper, NULL);


    free(rootDir);
	return fuse_stat;
}