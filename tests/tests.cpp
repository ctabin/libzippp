
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

#include <assert.h>
#include <string.h>
#include <iostream>
#include <cstdlib>
#include <string>

#include "libzippp.h"

using namespace std;
using namespace libzippp;

void test1() {
    cout << "Running test 1...";
    
    ZipArchive z1("test.zip");
    assert(!z1.isOpen());
    assert(!z1.isMutable());
    z1.open(ZipArchive::WRITE);
    assert(z1.isOpen());
    assert(z1.isMutable());
    bool result = z1.addEntry("folder/subfolder/finalfolder/");
    assert(result);
    z1.close();
    assert(!z1.isOpen());
    assert(!z1.isMutable());
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::READ_ONLY);
    assert(z2.isOpen());
    assert(!z2.isMutable());
    int nbEntries = z2.getNbEntries();
    assert(nbEntries==3);
    assert(z2.hasEntry("folder/"));
    assert(z2.hasEntry("folder/subfolder/"));
    assert(z2.hasEntry("folder/subfolder/finalfolder/"));
    z2.close();
    assert(z2.unlink());
    
    cout << " done." << endl;
}

void test2() {
    cout << "Running test 2...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::READ_ONLY);
    assert(z2.getNbEntries()==1);
    assert(z2.hasEntry("somedata"));
    
    ZipEntry entry = z2.getEntry("somedata");
    assert(!entry.isNull());
    
    string data = entry.readAsText();
    int clen = data.size();
    assert(clen==len);
    assert(strncmp(txtFile, data.c_str(), len)==0);
    
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test3() {
    cout << "Running test 3...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolder/data.txt", txtFile, len);
    assert(z1.addEntry("somedata/"));
    assert(z1.addEntry("in/"));
    assert(z1.addEntry("in/subfolder/"));
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::READ_ONLY);
    assert(z2.getNbEntries()==6);
    assert(z2.hasEntry("somedata/in/subfolder/data.txt"));
    
    ZipEntry entry = z2.getEntry("somedata/in/subfolder/data.txt");
    assert(!entry.isNull());
    
    string data = entry.readAsText();
    int clen = data.size();
    assert(clen==len);
    assert(strncmp(txtFile, data.c_str(), len)==0);
    
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test4() {
    cout << "Running test 4...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/test.txt", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    assert(z2.getNbEntries()==2);
    
    ZipEntry d = z2.getEntry("somedata/test.txt");
    assert(!d.isNull() && d.isFile());
    assert(z2.deleteEntry(d)==1);
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test5() {
    cout << "Running test 5...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolders/test.txt", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    assert(z2.getNbEntries()==4);
    
    ZipEntry d = z2.getEntry("somedata/in/");
    assert(!d.isNull() && d.isDirectory());
    assert(z2.deleteEntry(d)==3);
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test6() {
    cout << "Running test 6...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolders/test.txt", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    assert(z2.getNbEntries()==4);
    
    ZipEntry d = z2.getEntry("somedata/in/");
    assert(!d.isNull() && d.isDirectory());
    assert(z2.renameEntry(d, "somedata/out/")==3);
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test7() {
    cout << "Running test 7...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolders/test.txt", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    assert(z2.getNbEntries()==4);
    
    ZipEntry d = z2.getEntry("somedata/in/");
    assert(!d.isNull() && d.isDirectory());
    assert(z2.renameEntry(d, "somedata/in/subfolder/")==3);
    z2.close();
    
    ZipArchive z3("test.zip");
    z3.open(ZipArchive::READ_ONLY);
    assert(z3.getNbEntries()==5);
    z3.close();
    z3.unlink();
    
    cout << " done." << endl;
}

void test8() {
    cout << "Running test 8...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolders/test.txt", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    assert(z2.getNbEntries()==4);
    
    ZipEntry d = z2.getEntry("somedata/in/");
    assert(!d.isNull() && d.isDirectory());
    assert(z2.renameEntry(d, "newdata/out/subfolders/")==3);
    z2.close();
    
    ZipArchive z3("test.zip");
    z3.open(ZipArchive::READ_ONLY);
    assert(z3.getNbEntries()==6);
    z3.close();
    z3.unlink();
    
    cout << " done." << endl;
}

void test9() {
    cout << "Running test 9...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolders/test.txt", txtFile, len);
    z1.addData("somedata/out/subfolders/other.txt", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    assert(z2.getNbEntries()==7);
    
    ZipEntry d = z2.getEntry("somedata/in/");
    assert(!d.isNull() && d.isDirectory());
    assert(z2.renameEntry(d, "root/")==3);
    z2.close();
    
    ZipArchive z3("test.zip");
    z3.open(ZipArchive::READ_ONLY);
    assert(z3.getNbEntries()==7);
    z3.close();
    z3.unlink();
    
    cout << " done." << endl;
}

void test10() {
    cout << "Running test 10...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolders/test.txt", txtFile, len);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    assert(z2.getNbEntries()==4);
    
    ZipEntry d = z2.getEntry("somedata/in/");
    assert(!d.isNull() && d.isDirectory());
    assert(z2.renameEntry(d, "newdata/out/subfolders/")==3);
    z2.discard();
    
    ZipArchive z3("test.zip");
    z3.open(ZipArchive::READ_ONLY);
    assert(z3.getNbEntries()==4);
    z3.close();
    z3.unlink();
    
    cout << " done." << endl;
}

void test11() {
    cout << "Running test 11...";
    
    string c = "a basic comment";
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addEntry("test/");
    z1.setComment(c);
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::READ_ONLY);
    string str = z2.getComment();
    assert(str == c);
    z2.close();
    
    ZipArchive z3("test.zip");
    z3.open(ZipArchive::WRITE);
    z3.removeComment();
    z3.close();
    
    ZipArchive z4("test.zip");
    z4.open(ZipArchive::READ_ONLY);
    string str2 = z4.getComment();
    assert(str2.empty());
    z4.close();
    z4.unlink();
    
    cout << " done." << endl;
}

void test12() {
    cout << "Running test 12...";
    
    string c = "a basic comment";
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addEntry("test/");
    z1.addData("file/data.txt", c.c_str(), c.length());
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    z2.addEntry("content/new/");
    z2.addData("newfile.txt", c.c_str(), c.length());
    assert(z2.getNbEntries(ZipArchive::CURRENT)==6);
    assert(z2.getNbEntries(ZipArchive::ORIGINAL)==3);
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test13() {
    cout << "Running test 13...";
    
    string c = "some example of text";
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addEntry("test/");
    z1.addData("file/data.txt", c.c_str(), c.length());
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    z2.renameEntry(z2.getEntry("file/data.txt"), "file/subdir/data.txt");
    assert(z2.getNbEntries(ZipArchive::CURRENT)==4);
    assert(z2.getNbEntries(ZipArchive::ORIGINAL)==3);
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test14() {
    cout << "Running test 14...";
    
    string c = "some example of text";
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addEntry("test/");
    z1.addData("file/data.txt", c.c_str(), c.length());
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::WRITE);
    z2.renameEntry(z2.getEntry("file/data.txt"), "content/data/file.txt");
    assert(z2.getNbEntries(ZipArchive::CURRENT)==5);
    assert(z2.getNbEntries(ZipArchive::ORIGINAL)==3);
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test15() {
    cout << "Running test 15...";
    
    const char* txtFile = "this is some data";
    int len = strlen(txtFile);
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addData("somedata/in/subfolder/data.txt", txtFile, len);
    assert(z1.addEntry("somedata/"));
    assert(z1.addEntry("in/"));
    assert(z1.addEntry("in/subfolder/"));
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::READ_ONLY);
    assert(z2.getNbEntries()==6);
    assert(z2.hasEntry("somedata/in/subfolder/data.txt"));
    
    char* data = (char*)z2.readEntry("somedata/in/subfolder/data.txt", true);
    int clen = strlen(data);
    assert(clen==len);
    assert(strncmp(txtFile, data, len)==0);
    
    delete[] data;
    
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test16() {
    cout << "Running test 16...";
    
    string c = "some example of text";
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    z1.addEntry("dir/");
    z1.addData("file.txt", c.c_str(), c.length());
    
    ZipEntry e1 = z1.getEntry("dir/");
    ZipEntry e2 = z1.getEntry("file.txt");
    assert(e1.setComment("commentDir"));
    assert(e2.setComment("commentFile"));
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::READ_ONLY);
    ZipEntry e12 = z2.getEntry("dir/");
    ZipEntry e22 = z2.getEntry("file.txt");
    assert(e12.getComment() == "commentDir");
    assert(e22.getComment() == "commentFile");
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

void test17() {
    cout << "Running test 17...";
    
    ZipArchive z1("test.zip");
    assert(z1.open(ZipArchive::READ_ONLY) == false);
    void* nil1 = z1.readEntry("an/absent/file.txt", true);
    void* nil2 = z1.readEntry("an/absent/file.txt", true);
    assert(nil1 == NULL);
    assert(nil2 == NULL);
    z1.close();
    z1.unlink();
    
    cout << " done." << endl;
}

void test18() {
    cout << "Running test 18...";
    
    string entry1("a/földér/with/sûbâènts/");
    string entry2("fïle/iñ/sûbfôlder/dàtÄ.txt");
    string text("File wiôth sömè tîxt");
    
    ZipArchive z1("test.zip");
    z1.open(ZipArchive::WRITE);
    assert(z1.addEntry(entry1));
    assert(z1.addData(entry2, text.c_str(), text.length()));
    z1.close();
    
    ZipArchive z2("test.zip");
    z2.open(ZipArchive::READ_ONLY);
    ZipEntry e1 = z2.getEntry(entry1);
    assert(!e1.isNull());
    assert(e1.getName() == entry1);
    
    ZipEntry e2 = z2.getEntry(entry2);
    assert(!e2.isNull());
    assert(e2.getName() == entry2);
    assert(e2.readAsText() == text);
    
    z2.close();
    z2.unlink();
    
    cout << " done." << endl;
}

int main(int argc, char** argv) {
    test1();  test2();  test3();  test4();  test5();
    test6();  test7();  test8();  test9();  test10();
    test11(); test12(); test13(); test14(); test15();
    test16(); test17(); test18();
}


