
#ifndef LIBZIPPP_H
#define	LIBZIPPP_H

#include <cstdio>
#include <string>
#include <vector>

#include <zip.h>
#include <errno.h>

#define DIRECTORY_SEPARATOR '/'
#define IS_DIRECTORY(str) (str.length()>0 && str[str.length()-1]==DIRECTORY_SEPARATOR)

// documentation
// http://www.nih.at/libzip/libzip.html
// http://slash.developpez.com/tutoriels/c/utilisation-libzip/

using namespace std;

namespace libzippp {
    class ZipFile;
    
    /**
     * Represents an entry in a zip file.
     * This class is meant to be used by the ZipFile class.
     */
    class ZipEntry {
    friend class ZipFile;
    public:
        /**
         * Creates a new null-ZipEntry. Only a ZipFile will create a valid ZipEntry
         * usable to read and modify an archive.
         */
        ZipEntry(void) : zipFile(NULL) {}
        virtual ~ZipEntry(void) {}
        
        /**
         * Returns the name of the entry.
         */
        string getName(void) const { return name; }
        
        /**
         * Returns the index of the file in the zip.
         */
        int getIndex(void) const { return index; }
        
        /**
         * Returns the timestamp of the entry.
         */
        time_t getDate(void) const { return time; }
        
        /**
         * Returns the compression method.
         */
        int getMethod(void) const { return method; }
        
        /**
         * Returns the size of the file (not deflated).
         */
        int getSize(void) const { return size; }
        
        /**
         * Returns the size of the deflated file.
         */
        int getDeflatedSize(void) const { return sizeComp; }
        
        /**
         * Returns the CRC of the file.
         */
        int getCRC(void) const { return crc; }
        
        /**
         * Returns true if the entry is a directory.
         */
        bool isDirectory(void) const { return IS_DIRECTORY(name); }
        
        /**
         * Returns true if the entry is a file.
         */
        bool isFile(void) const { return !isDirectory(); }
        
        /**
         * Returns true if this entry is null (means no more entry is available).
         */
        bool isNull(void) const { return zipFile==NULL; }
        
        /**
         * Returns the comment of the entry.
         */
        string getComment(void) const { return comment; }
        
    private:
        const ZipFile* zipFile;
        string name;
        int index;
        time_t time;
        int method;
        int size;
        int sizeComp;
        int crc;
        string comment;
        
        ZipEntry(const ZipFile* zipFile, const string& name, int index, time_t time, int method, int size, int sizeComp, int crc, const string& comment) : 
                zipFile(zipFile), name(name), index(index), time(time), method(method), size(size), sizeComp(sizeComp), crc(crc), comment(comment) {}
    };
    
    /**
     * Represents a ZipFile. This class provides useful methods to handle an archive
     * content. It is wrapper around ziplib.
     */
    class ZipFile {
    public:
        
        /**
         * Defines how the zip file must be open.
         * NOT_OPEN is a special mode where the file is not open.
         * READ_ONLY is the basic mode to only read the archive.
         * WRITE will append to an existing archive or create a new one if it does not exist
         * NEW will create a new archive or erase all the data if a previous one exists
         */
        enum OpenMode {
            NOT_OPEN,
            READ_ONLY,
            WRITE,
            NEW
        };
        
        /**
         * Creates a new ZipFile with the given path. If the password is defined, it
         * will be used to read encrypted archive. It won't affect the files added
         * to the archive.
         * 
         * http://nih.at/listarchive/libzip-discuss/msg00219.html
         */
        ZipFile(const string& zipPath, const string& password="");
        virtual ~ZipFile(void); //commit all the changes if open
        
        /**
         * Return the path of the ZipFile.
         */
        string getPath(void) const { return path; }
        
        /**
         * Open the ZipFile in read mode. This method will return true if the operation
         * is successful, false otherwise. If the OpenMode is NOT_OPEN an invalid_argument
         * will be thrown.
         */
        bool open(OpenMode mode=READ_ONLY, bool checkConsistency=false);
        
        /**
         * Closes the ZipFile and releases all the resources held by it. If the ZipFile was
         * not open previously, this method does nothing. If the archive was open in modification
         * and some were done, they will be committed.
         */
        void close(void);
        
        /**
         * Closes the ZipFile and releases all the resources held by it. If the ZipFile was
         * not open previously, this method does nothing. If the archive was open in modification
         * and some were done, they will be rollbacked.
         */
        void discard(void);
        
        /**
         * Deletes the file denoted by the path. If the ZipFile is open, all the changes will
         * be discarded and the file removed.
         */
        bool unlink(void);
        
        /**
         * Returns true if the ZipFile is currently open.
         */
        bool isOpen(void) const { return zipHandle!=NULL; }
        
        /**
         * Returns true if the ZipFile is open and mutable.
         */
        bool isMutable(void) const { return isOpen() && mode!=NOT_OPEN && mode!=READ_ONLY; }
        
        /**
         * Returns true if the ZipFile is encrypted. This method returns true only if
         * a password has been set in the constructor.
         */
        bool isEncrypted(void) const { return !password.empty(); }
        
        /**
         * Defines the comment of the archive. In order to set the comment, the archive
         * must have been open in WRITE or NEW mode. If the archive is not open, the getComment
         * method will return an empty string.
         */
        string getComment(void) const;
        bool setComment(const string& comment) const;
        
        /**
         * Removes the comment of the archive, if any. The archive must have been open
         * in WRITE or NEW mode.
         */
        bool removeComment(void) const { setComment(string()); }
        
        /**
         * Returns the number of entries in this zip file (folders are included).
         * The zip file must be open otherwise -1 will be returned.
         */
        int getNbEntries(void) const;
        
        /**
         * Returns all the entries of the ZipFile.
         * The zip file must be open otherwise an empty vector will be returned.
         */
        vector<ZipEntry> getEntries(void) const;
        
        /**
         * Return true if an entry with the specified name exists. If no such entry exists,
         * then false will be returned. If a directory is searched, the name must end with a '/' !
         * The zip file must be open otherwise false will be returned.
         */
        bool hasEntry(const string& name, bool excludeDirectories=false, bool caseSensitive=true) const;
        
        /**
         * Return the ZipEntry for the specified entry name. If no such entry exists,
         * then a null-ZiPEntry will be returned. If a directory is searched, the name
         * must end with a '/' !
         * The zip file must be open otherwise a null-ZipEntry will be returned.
         */
        ZipEntry getEntry(const string& name, bool excludeDirectories=false, bool caseSensitive=true) const;
        
        /**
         * Return the ZipEntry for the specified index. If the index is out of range,
         * then a null-ZipEntry will be returned.
         * The zip file must be open otherwise a null-ZipEntry will be returned.
         */
        ZipEntry getEntry(int index) const;
        
        /**
         * Read the specified ZipEntry of the ZipFile and returns its content within
         * a char array. If there is an error while reading the entry, then null will be returned.
         * The data must be deleted by the developer once not used anymore.
         * The zip file must be open otherwise null will be returned. If the ZipEntry was not
         * created by this ZipFile, null will be returned.
         */
        void* readEntry(const ZipEntry& zipEntry) const;
        
        /**
         * Deletes the specified entry from the zip file. If the entry is a folder, all its
         * subentries will be removed. This method returns the number of entries removed.
         * If the open mode does not allow a deletion, this method will return  -1. If an
         * error occurs during deletion, this method will return -2.
         * If the ZipFile is not open or the entry was not edited by this ZipFile or is a null-ZipEntry,
         * then -3 will be returned.
         */
        int deleteEntry(const ZipEntry& entry) const;
        
        /**
         * Renames the entry with the specified newName. The method returns the number of entries
         * that have been renamed, 0 if the new name is invalid, -1 if the mode doesn't allow modification 
         * or -2 if an error occurred. 
         * If the entry is a directory, a '/' will automatically be append at the end of newName if the 
         * latter hasn't it already. All the files in the folder will be moved.
         * If the ZipFile is not open or the entry was not edited by this ZipFile or is a null-ZipEntry,
         * then -3 will be returned.
         */
        int renameEntry(const ZipEntry& entry, const string& newName) const;
        
        /**
         * Add the specified file in the archive with the given entry. If the entry already exists,
         * it will be replaced. This method returns true if the file has been added successfully. 
         * If the entryName specifies folders that doesn't exist in the archive, they will be automatically created.
         * If the entryName denotes a directory, this method returns false.
         * The zip file must be open otherwise false will be returned.
         */
        bool addFile(const string& entryName, const string& file) const;
        
        /**
         * Add the given data to the specified entry name in the archive. If the entry already exists,
         * its content will be erased. 
         * If the entryName specifies folders that doesn't exist in the archive, they will be automatically created.
         * If the entryName denotes a directory, this method returns false.
         * If the zip file is not open, this method returns false.
         */
        bool addData(const string& entryName, const void* data, uint length, bool freeData=false) const;
        
        /**
         * Add the specified directory to the ZipFile. All the hierarchy will be created.
         * If the ZipFile is not open or the entryName is not a directory, this method will
         * returns false. If the entry already exists, this method returns also true.
         */
        bool addDirectory(const string& entryName) const;
        
        /**
         * Returns the mode in which the file has been open.
         * If the archive is not open, then NOT_OPEN will be returned.
         */
        OpenMode getMode(void) const { return mode; }
        
    private:
        string path;
        zip* zipHandle;
        OpenMode mode;
        int openflag;
        string password;
        
        //generic method to create ZipEntry
        ZipEntry createEntry(struct zip_stat* stat) const;
        
        //prevent copy across functions
        ZipFile(const ZipFile& zf);
        ZipFile& operator=(const ZipFile&);
    };
}

#endif

