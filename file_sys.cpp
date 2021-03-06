// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>
using namespace std;

#include "debug.h"
#include "file_sys.h"

size_t inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

// inode state =====================================================

inode_state::inode_state() {
   //initializing root of tree
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   cwd = root;
   //two new pointers in map (".",root) and ("..",root)
   root->contents->getdirents()
      .insert(pair<string,inode_ptr>(".",root));
   root->contents->getdirents()
      .insert(pair<string,inode_ptr>("..",root));
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}


void inode_state::changePrompt(const string str){
   prompt_ = str;
}


//inode ============================================================
inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}

// File inode

size_t plain_file::size() const {
   size_t size = 0;
   if(data.size() == 0){
      return 0;
   }
   for(auto word : data){
      size += word.size();
   }
   if(size>0){
      //add in the spaces
      size += data.size()-1;
   }
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data.clear();
   data = words;
}

//Directory inode

size_t directory::size() const {
   return dirents.size();
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   dirents.erase(filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   inode_ptr dir = make_shared<inode>(file_type::DIRECTORY_TYPE);
   //insert dot into new directory
   (dir->getContents())->getdirents()
      .insert(pair<string,inode_ptr>(".",dir));
   dirents.insert(pair<string,inode_ptr>(dirname, dir));  
   return dir;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   
   inode_ptr file = make_shared<inode>(file_type::PLAIN_TYPE);
   dirents.insert(pair<string,inode_ptr>(filename, file));
   return file;
}

