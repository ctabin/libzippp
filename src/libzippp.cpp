#include <iostream>
#include "libzippp.h"

using namespace libzippp;
using namespace std;

ZipFile::ZipFile(const string& zipPath, const string& password) : path(zipPath), zipHandle(NULL), mode(NOT_OPEN), openflag(ZIP_FL_UNCHANGED), password(password) {
}

ZipFile::~ZipFile(void) { 
    close(); /* discard ??? */ 
}

bool ZipFile::open(OpenMode om, bool checkConsistency) {
    int zipFlag = 0;
    if (om==READ_ONLY) { zipFlag = 0; }
    else if (om==WRITE) { zipFlag = ZIP_CREATE; }
    else if (om==NEW) { zipFlag = ZIP_CREATE | ZIP_TRUNCATE; }
    else { return false; }
    
    if (checkConsistency) {
        zipFlag = zipFlag | ZIP_CHECKCONS;
    }
    
    int errorFlag = 0;
    zipHandle = zip_open(path.c_str(), zipFlag, &errorFlag);
    
    //error during opening of the file
    if(errorFlag!=ZIP_ER_OK) {
        /*char* errorStr = new char[256];
        zip_error_to_str(errorStr, 255, errorFlag, errno);
        errorStr[255] = '\0';
        cout << "Error: " << errorStr << endl;*/
        
        zipHandle = NULL;
        return false;
    }
    
    if (zipHandle!=NULL) {
        if (isEncrypted()) {
            int result = zip_set_default_password(zipHandle, password.c_str());
            if (result!=0) { 
                close();
                return false;
            }
        }
        
        mode = om;
        return true;
    }
    
    return false;
}

void ZipFile::close(void) {
    if (zipHandle) {
        zip_close(zipHandle);
        zipHandle = NULL;
        mode = NOT_OPEN;
    }
}

void ZipFile::discard(void) {
    if (zipHandle) {
        zip_discard(zipHandle);
        zipHandle = NULL;
        mode = NOT_OPEN;
    }
}

bool ZipFile::unlink(void) {
    if (isOpen()) { discard(); }
    int result = remove(path.c_str());
    return result==0;
}

string ZipFile::getComment(void) const {
    if (!isOpen()) { return string(); }
    
    int length = 0;
    const char* comment = zip_get_archive_comment(zipHandle, &length, ZIP_FL_ENC_GUESS);
    if (comment==NULL) { return string(); }
    return string(comment, length);
}

bool ZipFile::setComment(const string& comment) const {
    if (!isOpen()) { return false; }
    
    int size = comment.size();
    const char* data = comment.c_str();
    int result = zip_set_archive_comment(zipHandle, data, size);
    return result==0;
}

int ZipFile::getNbEntries(void) const {
    if (!isOpen()) { return -1; }
    return zip_get_num_entries(zipHandle, openflag);
}

ZipEntry ZipFile::createEntry(struct zip_stat* stat) const {
    string name(stat->name);
    int index = stat->index;
    int size = stat->size;
    int method = stat->comp_method;
    int sizeComp = stat->comp_size;
    int crc = stat->crc;
    time_t time = stat->mtime;

    uint clen;
    const char* com = zip_file_get_comment(zipHandle, index, &clen, ZIP_FL_ENC_GUESS);
    string comment = com==NULL ? string() : string(com, clen);
    
    return ZipEntry(this, name, index, time, method, size, sizeComp, crc, comment);
}

vector<ZipEntry> ZipFile::getEntries(void) const {
    if (!isOpen()) { return vector<ZipEntry>(); }
    
    struct zip_stat stat;
    zip_stat_init(&stat);

    vector<ZipEntry> entries;
    int nbEntries = getNbEntries();
    for(int i=0 ; i<nbEntries ; ++i) {
        int result = zip_stat_index(zipHandle, i, openflag, &stat);
        if (result==0) {
            ZipEntry entry = createEntry(&stat);
            entries.push_back(entry);
        } else {
            //TODO handle read error => crash ?
        }
    }
    return entries;
}

bool ZipFile::hasEntry(const string& name, bool excludeDirectories, bool caseSensitive) const {
    if (!isOpen()) { return false; }
    
    int flags = ZIP_FL_ENC_GUESS;
    if (excludeDirectories) { flags = flags | ZIP_FL_NODIR; }
    if (!caseSensitive) { flags = flags | ZIP_FL_NOCASE; }
    
    int index = zip_name_locate(zipHandle, name.c_str(), flags);
    if (index>=0) {
        return true;
    }
    return false;
}

ZipEntry ZipFile::getEntry(const string& name, bool excludeDirectories, bool caseSensitive) const {
    if (isOpen()) {
        int flags = ZIP_FL_ENC_GUESS;
        if (excludeDirectories) { flags = flags | ZIP_FL_NODIR; }
        if (!caseSensitive) { flags = flags | ZIP_FL_NOCASE; }

        int index = zip_name_locate(zipHandle, name.c_str(), flags);
        if (index>=0) {
            return getEntry(index);
        } else {
            //name not found
        }
    }
    return ZipEntry();
}
        
ZipEntry ZipFile::getEntry(int index) const {
    if (isOpen()) {
        struct zip_stat stat;
        zip_stat_init(&stat);
        int result = zip_stat_index(zipHandle, index, openflag, &stat);
        if (result==0) {
            return createEntry(&stat);
        } else {
            //index not found / invalid index
        }
    }
    return ZipEntry();
}

void* ZipFile::readEntry(const ZipEntry& zipEntry) const {
    if (!isOpen()) { return NULL; }
    if (zipEntry.zipFile!=this) { return NULL; }
    
    struct zip_file* zipFile = zip_fopen_index(zipHandle, zipEntry.getIndex(), openflag);
    if (zipFile) {
        int size = zipEntry.getSize();
        
        char* data = new char[size];
        int result = zip_fread(zipFile, data, size);
        zip_fclose(zipFile);
        
        if (result==size) {
            return data;
        } else {
            delete data;
            //unexpected number of bytes read => crash ?
        }
    } else {
        //unable to read the entry => crash ?
    }     
    
    return NULL;
}

int ZipFile::deleteEntry(const ZipEntry& entry) const {
    if (!isOpen()) { return -3; }
    if (entry.zipFile!=this) { return -3; }
    if (mode==READ_ONLY) { return -1; } //deletion not allowed
    
    if (entry.isFile()) {
        int result = zip_delete(zipHandle, entry.getIndex());
        if (result==0) { return 1; }
        return -2; //unable to delete the entry
    } else {
        int counter = 0;
        vector<ZipEntry> allEntries = getEntries();
        vector<ZipEntry>::const_iterator eit;
        for(eit=allEntries.begin() ; eit!=allEntries.end() ; ++eit) {
            ZipEntry ze = *eit;
            int startPosition = ze.getName().find(entry.getName());
            if (startPosition==0) {
                int result = zip_delete(zipHandle, ze.getIndex());
                if (result==0) { ++counter; }
                else { return -2; } //unable to remove the current entry
            }
        }
        return counter;
    }
}

int ZipFile::renameEntry(const ZipEntry& entry, const string& newName) const {
    if (!isOpen()) { return -3; }
    if (entry.zipFile!=this) { return -3; }
    if (mode==READ_ONLY) { return -1; } //renaming not allowed
    if (newName.length()==0) { return 0; }
    if (newName==entry.getName()) { return 0; }
    
    if (entry.isFile()) {
        if (IS_DIRECTORY(newName)) { return 0; } //invalid new name
        
        int lastSlash = newName.rfind(DIRECTORY_SEPARATOR);
        if (lastSlash!=1) { 
            bool dadded = addDirectory(newName.substr(0, lastSlash+1)); 
            if (!dadded) { return 0; } //the hierarchy hasn't been created
        }
        
        int result = zip_file_rename(zipHandle, entry.getIndex(), newName.c_str(), ZIP_FL_ENC_GUESS);
        if (result==0) { return 1; }
        return 0; //renaming was not possible (entry already exists ?)
    } else {
        if (!IS_DIRECTORY(newName)) { return 0; } //invalid new name
        
        int parentSlash = newName.rfind(DIRECTORY_SEPARATOR, newName.length()-2);
        if (parentSlash!=-1) { //updates the dir hierarchy
            string parent = newName.substr(0, parentSlash+1);
            bool dadded = addDirectory(parent);
            if (!dadded) { return 0; }
        }
        
        int counter = 0;
        string originalName = entry.getName();
        vector<ZipEntry> allEntries = getEntries();
        vector<ZipEntry>::const_iterator eit;
        for(eit=allEntries.begin() ; eit!=allEntries.end() ; ++eit) {
            ZipEntry ze = *eit;
            string currentName = ze.getName();
            
            int startPosition = currentName.find(originalName);
            if (startPosition==0) {
                if (currentName == originalName) {
                    int result = zip_file_rename(zipHandle, entry.getIndex(), newName.c_str(), ZIP_FL_ENC_GUESS);
                    if (result==0) { ++counter; }
                    else { return -2;  } //unable to rename the folder
                } else  {
                    string targetName = currentName.replace(0, originalName.length(), newName);
                    int result = zip_file_rename(zipHandle, ze.getIndex(), targetName.c_str(), ZIP_FL_ENC_GUESS);
                    if (result==0) { ++counter; }
                    else { return -2; } //unable to rename a sub-entry
                }
            } else {
                //file not affected by the renaming
            }
        }
        
        /*
         * Special case for moving a directory a/x to a/x/y to avoid to lose
         * the a/x path in the archive.
         */
        bool newNameIsInsideCurrent = (newName.find(entry.getName())==0);
        if (newNameIsInsideCurrent) {
            bool dadded = addDirectory(newName);
            if (!dadded) { return 0; }
        }
        
        return counter;
    }
}

bool ZipFile::addFile(const string& entryName, const string& file) const {
    if (!isOpen()) { return false; }
    if (mode==READ_ONLY) { return false; } //adding not allowed
    if (IS_DIRECTORY(entryName)) { return false; }
    
    int lastSlash = entryName.rfind(DIRECTORY_SEPARATOR);
    if (lastSlash!=-1) { //creates the needed parent directories
        string dirEntry = entryName.substr(0, lastSlash+1);
        bool dadded = addDirectory(dirEntry);
        if (!dadded) { return false; }
    }
    
    zip_source* source = zip_source_file(zipHandle, file.c_str(), 0, file.size());
    if (source!=NULL) {
        int result = zip_file_add(zipHandle, entryName.c_str(), source, ZIP_FL_OVERWRITE);
        if (result>=0) { return true; } 
        else { zip_source_free(source); } //unable to add the file
    } else {
        //unable to create the zip_source
    }
    return false;
}

bool ZipFile::addData(const string& entryName, const void* data, uint length, bool freeData) const {
    if (!isOpen()) { return false; }
    if (mode==READ_ONLY) { return false; } //adding not allowed
    if (IS_DIRECTORY(entryName)) { return false; }
    
    int lastSlash = entryName.rfind(DIRECTORY_SEPARATOR);
    if (lastSlash!=-1) { //creates the needed parent directories
        string dirEntry = entryName.substr(0, lastSlash+1);
        bool dadded = addDirectory(dirEntry);
        if (!dadded) { return false; }
    }
    
    zip_source* source = zip_source_buffer(zipHandle, data, length, freeData);
    if (source!=NULL) {
        int result = zip_file_add(zipHandle, entryName.c_str(), source, ZIP_FL_OVERWRITE);
        if (result>=0) { return true; } 
        else { zip_source_free(source); } //unable to add the file
    } else {
        //unable to create the zip_source
    }
    return false;
}

bool ZipFile::addDirectory(const string& entryName) const {
    if (!isOpen()) { return false; }
    if (mode==READ_ONLY) { return false; } //adding not allowed
    if (!IS_DIRECTORY(entryName)) { return false; }
    
    int nextSlash = entryName.find(DIRECTORY_SEPARATOR);
    while(nextSlash!=-1) {
        string pathToCreate = entryName.substr(0, nextSlash+1);
        if (!hasEntry(pathToCreate)) {
            int result = zip_dir_add(zipHandle, pathToCreate.c_str(), ZIP_FL_ENC_GUESS);
            if (result==-1) { return false; }
        }
        nextSlash = entryName.find(DIRECTORY_SEPARATOR, nextSlash+1);
    }
    
    return true;
}