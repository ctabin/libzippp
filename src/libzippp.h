
#ifndef LIBZIPPP_H
#define LIBZIPPP_H

/*
  libzippp.h -- exported declarations.
  Copyright (C) 2013 Cédric Tabin

  This file is part of libzippp, a library that wraps libzip for manipulating easily
  ZIP files in C++.
  The author can be contacted on http://www.astorm.ch/blog/index.php?contact

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _WIN32
#include <cstdint>
#endif
#include <cstdio>
#include <string>
#include <vector>
#include <functional>

//defined in libzip
struct zip;
struct zip_source;

#define LIBZIPPP_ENTRY_PATH_SEPARATOR '/'
#define LIBZIPPP_ENTRY_IS_DIRECTORY(str) ((str).length()>0 && (str)[(str).length()-1]==LIBZIPPP_ENTRY_PATH_SEPARATOR)
#define LIBZIPPP_DEFAULT_CHUNK_SIZE 524288
#define LIBZIPPP_DEFAULT_PROGRESSION_PRECISION 0.5

//libzip documentation
//- http://www.nih.at/libzip/libzip.html
//- http://slash.developpez.com/tutoriels/c/utilisation-libzip/

#ifdef _WIN32
        typedef long long libzippp_int64;
        typedef unsigned char libzippp_uint8;
        typedef unsigned long long libzippp_uint64;
        typedef unsigned int libzippp_uint32;
        typedef unsigned short libzippp_uint16;
        //special declarations for windows to use libzippp from a DLL
        #define LIBZIPPP_SHARED_LIBRARY_EXPORT __declspec(dllexport)
        #define LIBZIPPP_SHARED_LIBRARY_IMPORT
#else
        //standard ISO c++ does not support long long
        typedef std::int64_t libzippp_int64;
        typedef std::uint8_t libzippp_uint8;
        typedef std::uint64_t libzippp_uint64;
        typedef std::uint32_t libzippp_uint32;
        typedef std::uint16_t libzippp_uint16;

        #define LIBZIPPP_SHARED_LIBRARY_EXPORT
        #define LIBZIPPP_SHARED_LIBRARY_IMPORT
#endif

#ifdef LIBZIPPP_EXPORTS
        #define LIBZIPPP_INTERNAL
        #define LIBZIPPP_API LIBZIPPP_SHARED_LIBRARY_EXPORT
#else
        #define LIBZIPPP_API LIBZIPPP_SHARED_LIBRARY_IMPORT
#endif

// special return code for libzippp
#define LIBZIPPP_OK 0
#define LIBZIPPP_ERROR_NOT_OPEN -1
#define LIBZIPPP_ERROR_NOT_ALLOWED -2
#define LIBZIPPP_ERROR_INVALID_ENTRY -3
#define LIBZIPPP_ERROR_INVALID_PARAMETER -4
#define LIBZIPPP_ERROR_MEMORY_ALLOCATION -16
#define LIBZIPPP_ERROR_FOPEN_FAILURE -25
#define LIBZIPPP_ERROR_FREAD_FAILURE -26
#define LIBZIPPP_ERROR_OWRITE_FAILURE -35
#define LIBZIPPP_ERROR_OWRITE_INDEX_FAILURE -36
#define LIBZIPPP_ERROR_HANDLE_FAILURE -37
#define LIBZIPPP_ERROR_UNKNOWN -99

/**
 * User-defined error-handler.
 * See https://libzip.org/documentation/zip_error_system_type.html
 * 
 * The default handler just print the error by the following way:
 *   fprintf(stderr, message.c_str(), strerror.c_str());
 * 
 * Parameters are:
 * - message: A message, with possible placeholders (%s) for printf.
 * - strerror: A description of the error message, issued from libzip (see zip_error_strerror).
 * - zip_error_code: the libzip error code.
 * - system_error_code: the system error code.
 */
typedef void ErrorHandlerCallback(const std::string& message,
                                  const std::string& strerror,
                                  int zip_error_code,
                                  int system_error_code);

namespace libzippp {
    class ZipEntry;
    class ZipProgressListener;

    /**
     * Compression algorithm to use.
     * See https://libzip.org/documentation/zip_set_file_compression.html
     */
    enum CompressionMethod {
      DEFAULT = 0,
      STORE,
      DEFLATE,
#ifdef ZIP_CM_BZIP2
      BZIP2,
#endif
#ifdef ZIP_CM_XZ
      XZ,
#endif
#ifdef ZIP_CM_ZSTD
      ZSTD
#endif
     };

// BZIP2
#ifdef ZIP_CM_BZIP2
#define LIBZIPPP_USE_BZIP2
#endif

// XZ
#ifdef ZIP_CM_XZ
#define LIBZIPPP_USE_XZ
#endif

// ZSTD
#ifdef ZIP_CM_ZSTD
#define LIBZIPPP_USE_ZSTD
#endif

    /**
     * Represents a ZIP archive. This class provides useful methods to handle an archive
     * content. It is simply a wrapper around libzip.
     */
    class LIBZIPPP_API ZipArchive {
    public:

        /**
         * Defines how the zip file must be open.
         * NotOpen is a special mode where the file is not open.
         * ReadOnly is the basic mode to only read the archive.
         * Write will append to an existing archive or create a new one if it does not exist.
         * New will create a new archive or erase all the data if a previous one exists.
         */
        enum OpenMode {
            NotOpen,
            ReadOnly,
            Write,
            New
        };
        
        /**
         * Defines how the reading of the data should be made in the archive.
         * Original will read the data of the original archive file, without any change.
         * Current will read the current content of the archive.
         */
        enum State {
            Original,
            Current
        };

        /**
         * Defines encryption methods to be used, when writing, by the underlying libzip library.
         * These algorithms map to the types defined in libzip,
         * with the addition of a "ZIP_" prefix. For details see:
         * https://libzip.org/documentation/zip_file_set_encryption.html
         * None will use no encryption.
         * Aes128 will use Winzip AES-128 encryption.
         * Aes192 will use Winzip AES-192 encryption.
         * Aes256 will use Winzip AES-256 encryption.
         * TradPkware will use  Traditional PKWare encryption. Do not use this method, it is not secure. It is only provided for backwards compatibility.
         */
        enum Encryption {
            None,
#ifdef LIBZIPPP_WITH_ENCRYPTION
            Aes128,
            Aes192,
            Aes256,
            TradPkware
#endif
        };
        
        /**
         * Creates a new ZipArchive with the given path. If the password is defined, it
         * will be used to read/write an encrypted archive. It won't affect the files added
         * to the archive.
         * 
         * http://nih.at/listarchive/libzip-discuss/msg00219.html
         *
         * The arguments are:
         * - The zip file to be read/written.
         * - The password to be used to encrypt/decrypt each file within the zip file.
         * - The algorithm to be used by libzip when writing a zip file. The defined algorithm will use the password for each file within the zip file.
         */
        explicit ZipArchive(const std::string& zipPath, const std::string& password="", Encryption encryptionMethod=Encryption::None);
        
        /**
         * Commit all the changes of the archive and deletes the pointer.
         * It is recommended to use ZipArchive::free instead of deleting directly the pointer
         * especially if the ZipArchive was created with the ZipArchive::fromSource or ZipArchive::fromBuffer
         * methods.
         */
        virtual ~ZipArchive(void);
        
        /**
         * Creates a new ZipArchive with the given source. The archive will directly
         * be open with the given mode. If the archive fails to be open or
         * if the consistency check fails, this method will return null and the source
         * is left untouched.
         * 
         * Use ZipArchive::free to delete the returned pointer.
         */
        static ZipArchive* fromSource(zip_source* source, OpenMode mode=ReadOnly, bool checkConsistency=false, const std::string& password="", Encryption encryptionMethod=Encryption::None);
        
        /**
         * Creates a new ZipArchive from the specified data. The archive will
         * directly be open in ReadOnly mode. If the archive fails to be open or
         * if the consistency check fails, this method will return null.
         * The data pointer must remain valid while the ZipArchive is alive.
         * The data pointer won't be freed by the ZipArchive and won't be modified,
         * even when ZipArchive::close is used.
         * 
         * Use ZipArchive::free to delete the returned pointer.
         */
        static ZipArchive* fromBuffer(const void* data, libzippp_uint32 size, bool checkConsistency=false, const std::string& password="", Encryption encryptionMethod=Encryption::None);
        
        /**
         * Creates a new ZipArchive from the specified data. The archive will
         * directly be open with the given mode. If the archive fails to be open or
         * if the consistency check fails, this method will return null.
         * The data pointer must remain valid while the ZipArchive is alive.
         * The data pointer won't be freed by the ZipArchive.
         * 
         * If the mode is New or Write, then the data pointer will be updated when the ZipArchive is
         * closed and its new length will be available through the ZipArchive::getBufferLength method.
         * 
         * WARNING: do NOT use dynamicly allocated memory with the 'new' keyword as data, because since
         * the data array might be extended by realloc, it *might* not be compatible with C++ 'new' keyword.
         * Use the standard malloc/calloc instead.
         * 
         * Use ZipArchive::free to delete the returned pointer.
         */
        static ZipArchive* fromWritableBuffer(void** data, libzippp_uint32 size, OpenMode mode=Write, bool checkConsistency=false, const std::string& password="", Encryption encryptionMethod=Encryption::None);
        
        /**
         * Deletes a ZipArchive.
         * In order to ensure boundaries between DLLs it is recommended to always use this function
         * in order to delete a ZipArchive pointer.
         * This will commit all the pending changes to the archive.
         */
        static void free(ZipArchive* archive);
        
        /**
         * Returns the buffer length of the buffer when the fromBuffer method has been used to create
         * the archive. Otherwise, this method returns 0.
         * When the archive has been closed, this value will contains the new length of the buffer.
         */
        inline libzippp_uint64 getBufferLength(void) const { return bufferLength; }
        
        /**
         * Returns the path of the ZipArchive.
         */
        inline std::string getPath(void) const { return path; }
        
        /**
         * Open the ZipArchive with the given mode. This method will return true if the operation
         * is successful, false otherwise. If the OpenMode is NOT_OPEN an invalid_argument
         * will be thrown. If the archive is already open, this method returns true only if the
         * mode is the same.
         */
        bool open(OpenMode mode=ReadOnly, bool checkConsistency=false);

        /**
         * Closes the ZipArchive and releases all the resources held by it. If the ZipArchive was
         * not open previously, this method does nothing. If the archive was open in modification
         * and some were done, they will be committed.
         * This method returns LIBZIPPP_OK if the archive was successfully closed, otherwise it 
         * returns a LIBZIPPP error code. The error is dispatched to ErrorHandlerCallback.
         * While being closed, all the registered ZipProgressListener instances will be invoked on
         * a regular basis, depending on the progression precision.
         * In some cases (when the archive is created with fromWritableBuffer), the archive is still
         * being closed, even if an error code is returned.
         */
        int close(void);
        
        /**
         * Closes the ZipArchive and releases all the resources held by it. If the ZipArchive was
         * not open previously, this method does nothing. If the archive was open in modification
         * and some were done, they will be rollbacked.
         */
        void discard(void);
        
        /**
         * Deletes the file denoted by the path. If the ZipArchive is open, all the changes will
         * be discarded and the file removed.
         */
        bool unlink(void);
        //bool delete(void) { return unlink(); } //delete is a reserved keyword
        
        /**
         * Returns true if the ZipArchive is currently open.
         */
        inline bool isOpen(void) const { return zipHandle!=nullptr; }
        
        /**
         * Returns true if the ZipArchive is open and mutable.
         */
        inline bool isMutable(void) const { return isOpen() && mode!=NotOpen && mode!=ReadOnly; }
        
        /**
         * Returns true if the ZipArchive is encrypted. This method returns true only if
         * a password has been set in the constructor.
         */
        inline bool isEncrypted(void) const { return !password.empty(); }
        
        /**
         * Defines the comment of the archive. In order to set the comment, the archive
         * must have been open in WRITE or NEW mode. If the archive is not open, the getComment
         * method will return an empty string.
         */
        std::string getComment(State state=Current) const;
        bool setComment(const std::string& comment) const;
        
        /**
         * Removes the comment of the archive, if any. The archive must have been open
         * in WRITE or NEW mode.
         */
        inline bool removeComment(void) const { return setComment(std::string()); }
        
        /**
         * Returns the number of entries in this zip file (folders are included).
         * The zip file must be open otherwise LIBZIPPP_ERROR_NOT_OPEN will be returned. 
         * If the state is Original, then the number entries of the original archive are returned.
         * Any change will not be considered.
         * Note also that the deleted entries does not affect the result of this method
         * with the Current state. For instance, if there are 3 entries and you delete one,
         * this method will still return 3. However, if you add one entry, it will return
         * 4 with the state Current and 3 with the state Original.
         * If you wanna know the "real" entries effectively in the archive, you might use
         * the getEntries method.
         */
        libzippp_int64 getNbEntries(State state=Current) const;
        inline libzippp_int64 getEntriesCount(State state=Current) const { return getNbEntries(state); }
        //libzippp_int64 size(State state=Current) const { return getNbEntries(state); } //not clear enough => could be the size of the file instead...

        /**
         * Returns all the entries of the ZipArchive. If the state is Original, then
         * returns the entries in the original archive, any change will not be considered.
         * The zip file must be open otherwise an empty vector will be returned.
         */
        std::vector<ZipEntry> getEntries(State state=Current) const;
        
        /**
         * Returns true if an entry with the specified name exists. If no such entry exists,
         * then false will be returned. If a directory is searched, the name must end with a '/' !
         * The zip file must be open otherwise false will be returned.
         */
        bool hasEntry(const std::string& name, bool excludeDirectories=false, bool caseSensitive=true, State state=Current) const;
        
        /**
         * Returns the ZipEntry for the specified entry name. If no such entry exists,
         * then a null-ZiPEntry will be returned. If a directory is searched, the name
         * must end with a '/' !
         * The zip file must be open otherwise a null-ZipEntry will be returned.
         */
        ZipEntry getEntry(const std::string& name, bool excludeDirectories=false, bool caseSensitive=true, State state=Current) const;
        
        /**
         * Returns the ZipEntry for the specified index. If the index is out of range,
         * then a null-ZipEntry will be returned.
         * The zip file must be open otherwise a null-ZipEntry will be returned.
         */
        ZipEntry getEntry(libzippp_int64 index, State state=Current) const;
        
        /**
         * Defines the comment of the entry. If the ZipArchive is not open or the
         * entry is not linked to this archive, then an empty string or false will 
         * be returned.
         */
        std::string getEntryComment(const ZipEntry& entry, State state=Current) const;
        bool setEntryComment(const ZipEntry& entry, const std::string& comment) const;
        
        /**
         * Defines the compression method of an entry. If the ZipArchive is not open
         * or the entry is not linked to this archive, false will be returned.
         **/
        bool setEntryCompressionConfig(ZipEntry& entry, CompressionMethod compMethod=CompressionMethod::DEFAULT, libzippp_uint32 compLevel=0) const;
        
        /**
         * Reads the specified ZipEntry of the ZipArchive and returns its content within
         * a char array. If there is an error while reading the entry, then null will be returned.
         * The data must be deleted by the developer once not used anymore. If the asText
         * is set to true, then the returned void* will be ended by a '\0' (hence the size of
         * the returned array will be zipEntry.getSize()+1 or size+1 if the latter is specified).
         * The zip file must be open otherwise null will be returned. If the ZipEntry was not
         * created by this ZipArchive, null will be returned.
         */
        void* readEntry(const ZipEntry& zipEntry, bool asText=false, State state=Current, libzippp_uint64 size=0) const;
        
        /**
         * Reads the specified ZipEntry of the ZipArchive and returns its content within
         * a char array. If there is an error while reading the entry, then null will be returned.
         * The data must be deleted by the developer once not used anymore. If the asText
         * is set to true, then the returned void* will be ended by a '\0' (hence the size of
         * the returned array will be zipEntry.getSize()+1 or size+1 if the latter is specified).
         * The zip file must be open otherwise null will be returned. If the ZipEntry was not
         * created by this ZipArchive, null will be returned. If the zipEntry does not exist,
         * this method returns nullptr:
         */
        void* readEntry(const std::string& zipEntry, bool asText=false, State state=Current, libzippp_uint64 size=0) const;
        
        /**
         * Reads the specified ZipEntry of the ZipArchive and inserts its content in the provided reference to an already
         * opened std::ofstream, gradually, with chunks of size "chunksize" to reduce memory usage when dealing with big files.
         * The method returns LIBZIPPP_OK if the extraction has succeeded with no problem, LIBZIPPP_ERROR_INVALID_PARAMETER if the 
         * ofstream is not open, LIBZIPPP_ERROR_NOT_OPEN if the archive is not open, LIBZIPPP_ERROR_INVALID_ENTRY if the zipEntry 
         * doesn't belong to the archive, LIBZIPPP_ERROR_FOPEN_FAILURE if zip_fopen_index() has failed, LIBZIPPP_ERROR_MEMORY_ALLOCATION if 
         * a memory allocation has failed, LIBZIPPP_ERROR_FREAD_FAILURE if zip_fread() didn't succeed to read data, 
         * LIBZIPPP_ERROR_OWRITE_INDEX_FAILURE if the last ofstream operation has failed, LIBZIPPP_ERROR_OWRITE_FAILURE if fread() didn't 
         * return the exact amount of requested bytes and -9 if the amount of extracted bytes didn't match the size of the file (unknown error).
         * If the provided chunk size is zero, it will be defaulted to LIBZIPPP_DEFAULT_CHUNK_SIZE (512KB).
         * The method doesn't close the ofstream after the extraction.
         */
        int readEntry(const ZipEntry& zipEntry, std::ostream& ofOutput, State state=Current, libzippp_uint64 chunksize=LIBZIPPP_DEFAULT_CHUNK_SIZE) const;
        
        /**
         * Reads the specified ZipEntry of the ZipArchive and invokes the output function with its content, gradually, with chunks of
         * size "chunksize" to reduce memory usage when dealing with big files.
         * The method returns LIBZIPPP_OK if the extraction has succeeded with no problem, LIBZIPPP_ERROR_INVALID_PARAMETER if the 
         * ofstream is not open, LIBZIPPP_ERROR_NOT_OPEN if the archive is not open, LIBZIPPP_ERROR_INVALID_ENTRY if the zipEntry 
         * doesn't belong to the archive, LIBZIPPP_ERROR_FOPEN_FAILURE if zip_fopen_index() has failed, LIBZIPPP_ERROR_MEMORY_ALLOCATION if 
         * a memory allocation has failed, LIBZIPPP_ERROR_FREAD_FAILURE if zip_fread() didn't succeed to read data, 
         * LIBZIPPP_ERROR_OWRITE_INDEX_FAILURE if the last ofstream operation has failed, LIBZIPPP_ERROR_OWRITE_FAILURE if fread() didn't 
         * return the exact amount of requested bytes and -9 if the amount of extracted bytes didn't match the size of the file (unknown error).
         * If the provided chunk size is zero, it will be defaulted to LIBZIPPP_DEFAULT_CHUNK_SIZE (512KB).
         * The method doesn't close the ofstream after the extraction.
         */
        int readEntry(const ZipEntry& zipEntry, std::function<bool(const void*,libzippp_uint64)> output, State state=Current, libzippp_uint64 chunksize=LIBZIPPP_DEFAULT_CHUNK_SIZE) const;

        /**
         * Deletes the specified entry from the zip file. If the entry is a folder, all its
         * subentries will be removed. This method returns the number of entries removed.
         * If the open mode does not allow a deletion, this method will return LIBZIPPP_ERROR_NOT_ALLOWED. 
         * If the ZipArchive is not open, LIBZIPPP_ERROR_NOT_OPEN will be returned. If the entry is not handled 
         * by this ZipArchive or is a null-ZipEntry, then LIBZIPPP_ERROR_INVALID_ENTRY will be returned.
         * If an error occurs during deletion, this method will return LIBZIPPP_ERROR_UNKNOWN.
         * Note that this method does not affect the result returned by getNbEntries !
         */
        int deleteEntry(const ZipEntry& entry) const;
        
        /**
         * Deletes the specified entry from the zip file. If the entry is a folder, all its
         * subentries will be removed. This method returns the number of entries removed.
         * If the open mode does not allow a deletion, this method will return LIBZIPPP_ERROR_NOT_ALLOWED. 
         * If the ZipArchive is not open, LIBZIPPP_ERROR_NOT_OPEN will be returned. If the entry is not handled 
         * by this ZipArchive or is a null-ZipEntry, then LIBZIPPP_ERROR_INVALID_ENTRY will be returned.
         * If an error occurs during deletion, this method will return LIBZIPPP_ERROR_UNKNOWN.
         * If the entry does not exist, this method returns LIBZIPPP_ERROR_INVALID_PARAMETER.
         * Note that this method does not affect the result returned by getNbEntries !
         */
        int deleteEntry(const std::string& entry) const;
        
        /**
         * Renames the entry with the specified newName. The method returns the number of entries
         * that have been renamed, LIBZIPPP_ERROR_INVALID_PARAMETER if the new name is invalid, 
         * LIBZIPPP_ERROR_NOT_ALLOWED if the mode doesn't allow modification or LIBZIPPP_ERROR_UNKNOWN if an error 
         * occurred. If the entry is a directory, a '/' will automatically be appended at the end of newName if the 
         * latter hasn't it already. All the files in the folder will be moved.
         * If the ZipArchive is not open or the entry was not edited by this ZipArchive or is a null-ZipEntry,
         * then LIBZIPPP_ERROR_INVALID_ENTRY will be returned.
         */
        int renameEntry(const ZipEntry& entry, const std::string& newName) const;
        
        /**
         * Renames the entry with the specified newName. The method returns the number of entries
         * that have been renamed, LIBZIPPP_ERROR_INVALID_PARAMETER if the new name is invalid, 
         * LIBZIPPP_ERROR_NOT_ALLOWED if the mode doesn't allow modification or LIBZIPPP_ERROR_UNKNOWN if an error 
         * occurred. If the entry is a directory, a '/' will automatically be appended at the end of newName if the 
         * latter hasn't it already. All the files in the folder will be moved.
         * If the ZipArchive is not open or the entry was not edited by this ZipArchive or is a null-ZipEntry,
         * then LIBZIPPP_ERROR_INVALID_ENTRY will be returned. If the entry does not exist, this method returns LIBZIPPP_ERROR_INVALID_PARAMETER.
         */
        int renameEntry(const std::string& entry, const std::string& newName) const;
        
        /**
         * Adds the specified file in the archive with the given entry. If the entry already exists,
         * it will be replaced. This method returns true if the file has been added successfully. 
         * If the entryName contains folders that don't exist in the archive, they will be automatically created.
         * If the entryName denotes a directory, this method returns false.
         * The zip file must be open otherwise false will be returned.
         */
        bool addFile(const std::string& entryName, const std::string& file) const;
        
        /**
         * Adds the given data to the specified entry name in the archive. If the entry already exists,
         * its content will be erased. 
         * If the entryName contains folders that don't exist in the archive, they will be automatically created.
         * If the entryName denotes a directory, this method returns false.
         * If the zip file is not open, this method returns false.
         */
        bool addData(const std::string& entryName, const void* data, libzippp_uint64 length, bool freeData=false) const;
        
        /**
         * Adds the given data to the specified entry name in the archive. If the entry already exists,
         * its content will be erased. 
         * If the entryName contains folders that don't exist in the archive, they will be automatically created.
         * If the entryName denotes a directory, this method returns false.
         * If the zip file is not open, this method returns false.
         */
        bool addData(const std::string& entryName, const std::basic_string<libzippp_uint8> data) const;
        
        /**
         * Adds the specified entry to the ZipArchive. All the needed hierarchy will be created.
         * The entryName must be a directory (end with '/').
         * If the ZipArchive is not open or the entryName is not a directory, this method will
         * returns false. If the entry already exists, this method returns true.
         * This method will only add the specified entry. The 'real' directory may exist or not.
         * If the directory exists, the files in it won't be added to the archive.
         */
        bool addEntry(const std::string& entryName) const;
        
        /**
         * Returns the mode in which the file has been open.
         * If the archive is not open, then NOT_OPEN will be returned.
         */
        inline OpenMode getMode(void) const { return mode; }

        /**
         * Returns the underlying libzip handle used by this ZipArchive.
         * This value will be set only when the ZipArchive is open.
         */
        inline zip* getZipHandle(void) const { return zipHandle; }

        /**
         * Returns the underlying libzip source used by this ZipArchive.
         * This value will be available only when the archive has been created with ZipArchive::fromBuffer.
         */
        inline zip_source* getZipSource(void) const { return zipSource; }
        
        /**
         * Registers the specified listener to be notified when the ZipArchive is closed.
         * A listener can be added even if the ZipArchive is not yet open.
         */
        inline void addProgressListener(ZipProgressListener* listener) { listeners.push_back(listener); }
        
        /**
         * Removes the specified listener.
         */
        void removeProgressListener(ZipProgressListener* listener);
        
        /**
         * Returns all the listeners registered to this ZipArchive.
         */
        inline std::vector<ZipProgressListener*> getProgressListeners(void) const { return listeners; }

        /**
         * Defines the progress precision (defaults to 0.5).
         * This value is between 0.00 and 1.00 and determines how often the
         * progression callback will be invoked while the ZipArchive is being closed.
         * 
         * From the libzip documentation: The precision argument is a double in the range from 0.00 to 1.00
         * that defines the smallest change for which the callback should be called (to avoid too frequent calls).
         */
        inline double getProgressPrecision(void) const { return progressPrecision; }
        void setProgressPrecision(double p) { progressPrecision = p; }

        /**
         * Defines the error handler callback to notify in case of error while handling
         * the underlying zip file.
         */
        void setErrorHandlerCallback(ErrorHandlerCallback* callback) {
           errorHandlingCallback = callback;
        }

        /**
         * Defines the compression method to used for the newly created ZipEntry.
         */
        void setCompressionMethod(CompressionMethod comp);
        CompressionMethod getCompressionMethod(void) const;

        /**
         * Defines the compression level to use. By default this value is zero to use the default behaviour of libzip.
         * Otherwise, this value should be between 1 and 9, 1 being the fastest compression and 9 the best.
         * For ZSTD, possible values are defined by ZSTD_minCLevel and ZSTD_maxCLevel.
         */
        inline void setCompressionLevel(libzippp_uint32 level) { this->compressionLevel = level; }
        inline libzippp_uint32 getCompressionLevel(void) const { return compressionLevel; }

    private:
        std::string path;
        zip* zipHandle;
        zip_source* zipSource;
        OpenMode mode;
        std::string password;
        int encryptionMethod;
        std::vector<ZipProgressListener*> listeners;
        double progressPrecision;
        
        void** bufferData;
        libzippp_uint64 bufferLength;

        bool useArchiveCompressionMethod;
        libzippp_uint16 compressionMethod;
        libzippp_uint32 compressionLevel;

        // User-defined error handler
        ErrorHandlerCallback* errorHandlingCallback;
        
        //open from in-memory data
        bool openBuffer(void** buffer, libzippp_uint32 sz, OpenMode mode=ReadOnly, bool checkConsistency=false);
        bool openSource(zip_source* source, OpenMode mode=ReadOnly, bool checkConsistency=false);
        
        //generic method to create ZipEntry
        ZipEntry createEntry(struct zip_stat* stat) const;
        
        //prevent copy across functions
        ZipArchive(const ZipArchive& zf);
        ZipArchive& operator=(const ZipArchive&);
    };
    
    /**
     * Implementation of a progression listener that will be notified when the
     * ZipArchive is being closed and changes are being committed.
     */
    class LIBZIPPP_API ZipProgressListener {
    public:
    
        /**
         * This method is invoked while the changes are being committed during
         * the closing of the ZipArchive.
         * The value p is a double between 0 and 1, representing the overall progression.
         * The frequency of invocation of this method depends of the precision.
         *
         * Note that libzippp enforces the first invokation to be with a p-value of zero
         * and the last invokation to be with a p-value of 1. Hence, it might be possible
         * to receive multiple invokations with the same p-value, depending on the precsion
         * set in libzip.
         */
        virtual void progression(double p) = 0;
        
        /**
         * This method is invoked during zip/unzip operations.
         * Define this function to be able to stop a long zip/unzip operation.
         * If this function return 1 the operation is cancelled.
         * If this function return 0 the operation will continue.
         */
        virtual int cancel(void) = 0;
    };
    
    /**
     * Represents an entry in a zip file.
     * This class is meant to be used by the ZipArchive class.
     */
    class LIBZIPPP_API ZipEntry {
    friend class ZipArchive;
    public:
        /**
         * Creates a new null-ZipEntry. Only a ZipArchive will create a valid ZipEntry
         * usable to read and modify an archive.
         */
        explicit ZipEntry(void);
        virtual ~ZipEntry(void) {}
        
        /**
         * Returns the name of the entry.
         */
        inline std::string getName(void) const { return name; }
        
        /**
         * Returns the index of the file in the zip.
         */
        inline libzippp_uint64 getIndex(void) const { return index; }
        
        /**
         * Returns the timestamp of the entry.
         */
        inline time_t getDate(void) const { return time; }
        
        /**
         * Defines the compression method to be used. By default, ZIP_CM_DEFAULT.
         * Can be one of ZIP_CM_DEFAULT,ZIP_CM_STORE,ZIP_CM_BZIP2,ZIP_CM_DEFLATE,ZIP_CM_XZ or ZIP_CM_ZSTD.
         */
        CompressionMethod getCompressionMethod(void) const;
        bool setCompressionMethod(CompressionMethod compMethod);
        
        /**
         * Defines the compression level to use. By default this value is zero to use the default behaviour of libzip.
         * Otherwise, this value should be between 1 and 9, 1 being the fastest compression and 9 the best.
         * For ZSTD, possible values are defined by ZSTD_minCLevel and ZSTD_maxCLevel.
         */
        inline libzippp_uint32 getCompressionLevel(void) const { return compressionLevel; }
        bool setCompressionLevel(libzippp_uint32 level);
        
        /**
         * Returns the encryption method.
         * Can be one of ZIP_EM_NONE,ZIP_EM_AES_128,ZIP_EM_AES_192,ZIP_EM_AES_256 or ZIP_EM_TRAD_PKWARE.
         */
        inline libzippp_uint16 getEncryptionMethod(void) const { return encryptionMethod; }
        
        /**
         * Returns the size of the file (uncompressed).
         */
        inline libzippp_uint64 getSize(void) const { return size; }
        
        /**
         * Returns the size of the deflated file (compressed).
         */
        inline libzippp_uint64 getDeflatedSize(void) const { return sizeComp; }
        
        /**
         * Returns the CRC of the file.
         */
        inline int getCRC(void) const { return crc; }
        
        /**
         * Returns true if the entry is a directory.
         */
        inline bool isDirectory(void) const { return LIBZIPPP_ENTRY_IS_DIRECTORY(name); }
        
        /**
         * Returns true if the entry is a file.
         */
        inline bool isFile(void) const { return !isDirectory(); }
        
        /**
         * Returns true if this entry is null (means no more entry is available).
         */
        inline bool isNull(void) const { return zipFile==nullptr; }
        
        /**
         * Defines the comment of the entry. In order to call either one of those
         * methods, the corresponding ZipArchive must be open otherwise an empty string
         * or false will be returned. Those methods are wrappers around ZipArchive::getEntryComment
         * and ZipArchive::setEntryComment.
         */
        std::string getComment(void) const;
        bool setComment(const std::string& str) const;
        
        /**
         * Reads the content of this ZipEntry as text.
         * The returned string will be of size `getSize()` if the latter is not specified or too big. 
         * If the ZipArchive is not open, this method returns an
         * empty string. This method is a wrapper around ZipArchive::readEntry(...).
         */
        std::string readAsText(ZipArchive::State state=ZipArchive::Current, libzippp_uint64 size=0) const;
        
        /**
         * Reads the content of this ZipEntry as binary. 
         * The returned void* will be of size `getSize()` if the latter is not specified or too big.
         * If the ZipArchive is not open, this method returns nullptr.
         * The data must be deleted by the developer once not used anymore.
         * This method is a wrapper around ZipArchive::readEntry(...).
         */
        libzippp_uint8* readAsBinary(ZipArchive::State state=ZipArchive::Current, libzippp_uint64 size=0) const;

        /**
         * Reads the content of this ZipEntry as binary string. 
         * The returned string will be of size `getSize()` if the latter is not specified or too big.
         * If the ZipArchive is not open, this method returns nullptr.
         * This method is a wrapper around ZipArchive::readEntry(...).
         */
        std::basic_string<libzippp_uint8> readAsBinaryString(ZipArchive::State state=ZipArchive::Current, libzippp_uint64 size=0) const;

        /**
         * Reads the specified ZipEntry of the ZipArchive and inserts its content in the provided reference to an already
         * opened std::ofstream, gradually, with chunks of size "chunksize" to reduce memory usage when dealing with big files.
         * The method returns LIBZIPPP_OK if the extraction has succeeded with no problems, LIBZIPPP_ERROR_INVALID_PARAMETER if the 
         * ofstream is not open, LIBZIPPP_ERROR_NOT_OPEN if the archive is not open, LIBZIPPP_ERROR_INVALID_ENTRY if the zipEntry 
         * doesn't belong to the archive, LIBZIPPP_ERROR_FOPEN_FAILURE if zip_fopen_index() has failed, LIBZIPPP_ERROR_MEMORY_ALLOCATION if 
         * a memory allocation has failed, LIBZIPPP_ERROR_FREAD_FAILURE if zip_fread() didn't succeed to read data, 
         * LIBZIPPP_ERROR_OWRITE_INDEX_FAILURE if the last ofstream operation has failed, LIBZIPPP_ERROR_OWRITE_FAILURE if fread() didn't 
         * return the exact amount of requested bytes and -9 if the amount of extracted bytes didn't match the size of the file (unknown error).
         * If the provided chunk size is zero, it will be defaulted to LIBZIPPP_DEFAULT_CHUNK_SIZE (512KB).
         * The method doesn't close the ofstream after the extraction.
         */
        int readContent(std::ostream& ofOutput, ZipArchive::State state=ZipArchive::Current, libzippp_uint64 chunksize=LIBZIPPP_DEFAULT_CHUNK_SIZE) const;
        
    private:
        const ZipArchive* zipFile;
        std::string name;
        libzippp_uint64 index;
        time_t time;
        libzippp_uint16 compressionMethod;
        libzippp_uint32 compressionLevel;
        libzippp_uint16 encryptionMethod;
        libzippp_uint64 size;
        libzippp_uint64 sizeComp;
        int crc;
        
        ZipEntry(const ZipArchive* zipFile, const std::string& name, libzippp_uint64 index, time_t time, libzippp_uint16 compMethod, libzippp_uint32 compLevel, libzippp_uint16 encMethod, libzippp_uint64 size, libzippp_uint64 sizeComp, int crc) : 
                zipFile(zipFile), name(name), index(index), time(time), compressionMethod(compMethod), compressionLevel(compLevel), encryptionMethod(encMethod), size(size), sizeComp(sizeComp), crc(crc) {}
    };
}

#endif

