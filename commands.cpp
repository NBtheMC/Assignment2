// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

#include "util.h"
#include "commands.h"
#include "debug.h"
#include "iomanip"

command_hash cmd_hash {
   {"cat"   , fn_cat    },
   {"cd"    , fn_cd     },
   {"echo"  , fn_echo   },
   {"exit"  , fn_exit   },
   {"ls"    , fn_ls     },
   {"lsr"   , fn_lsr    },
   {"make"  , fn_make   },
   {"mkdir" , fn_mkdir  },
   {"prompt", fn_prompt },
   {"pwd"   , fn_pwd    },
   {"rm"    , fn_rm     },
   {"rmr"   , fn_rmr    },
   {"#"     , fn_nothing},


};

inode_ptr findNode( inode_state& state, string path);
void preExitClear(inode_ptr& node);
int stringToInt(string str);


command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec filenames;
   for(long unsigned int i = 1; i < words.size(); i++){
      filenames.push_back(words[i]);
   }


   inode_ptr targetNode;
   for(auto filename : filenames){
      if(filename.find("/") != string::npos){
         size_t lastSlash = filename.find_last_of("/");
         string path = filename.substr(0,lastSlash+1);
         filename = filename.substr(lastSlash+1);
         targetNode = findNode(state,path);
      }else{
         targetNode = state.getCwd();
      }

      if (targetNode->getContents()->getdirents().count(filename) == 0)
      {
         throw command_error (filename + ": no such file");
      }

      auto fileMapObj = 
         targetNode->getContents()->getdirents().find(filename);
      auto filePtr = fileMapObj->second;
      auto data = filePtr->getContents()->readfile();
 
      for(auto word : data){
         cout << word << " ";
      }
      cout << endl;
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   state.changeCwd(findNode(state,words[1])); 
   

   if(words[1].at(0) == '/'){
      state.getCwdPath().clear();
   }


   wordvec parsedPath = split(words[1],"/");
   for(auto dir : parsedPath){ 
      if(dir == ".."){
         state.getCwdPath().pop_back();
      }
      else if(dir == "."){
         
      }
      else {
         state.getCwdPath().push_back(dir);  
      }

   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   //set all content pointers to null
   preExitClear(state.getRoot());
   if(words.size() > 1){
      exec::status(stringToInt(words[1]));
   }else{
      exec::status(0);
   }
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   inode_ptr currentDir;
   if(words.size() > 1){
      currentDir = findNode(state, words[1]);
   }
   else{
      currentDir = state.getCwd();
   }
   if(currentDir  == state.getRoot()){
      cout << "/:" << endl;
   }
   else{
      cout  << currentDir->getPath() <<":"<<endl;
   }
   
   for( auto mapObj : currentDir->getContents()->getdirents()){
      auto inodePtr = mapObj.second;
      cout << setw(6)<< inodePtr->get_inode_nr() 
         << setw(6)
         << inodePtr->getContents()->size() 
         << "  " << mapObj.first;
      if(currentDir->getContents()->fileType() == "directory")  {
         cout << "/" ;
      } 
      cout << endl;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   inode_ptr currentDir;
   if(words.size() > 1){
      currentDir = findNode(state, words[1]);
   }
   else{
      currentDir = state.getCwd();
   }
   if(currentDir == state.getRoot()){
      cout << "/:" << endl;
   } else{
      cout  << currentDir->getPath() <<":"<<endl;
   }
   auto wordCopy = words;

   for( auto mapObj : currentDir->getContents()->getdirents()){
      if(mapObj.second->getContents()->fileType() == "file"){
         auto inodePtr = mapObj.second;
         cout << setw(6)<< inodePtr->get_inode_nr()
            << setw(6)
            << inodePtr->getContents()->size()
            << "  " << mapObj.first
            << endl;
       }else{
          wordCopy = words;

          if(mapObj.first == "." || mapObj.first == ".."){
             
            auto inodePtr = mapObj.second;
            cout << setw(6)<< inodePtr->get_inode_nr()
               << setw(6)
               << inodePtr->getContents()->size()
               << "  " << mapObj.first << "/"
               << endl;

          }else{
              if(words.size() > 1){
                wordCopy[1] +="/";
                wordCopy[1] += mapObj.first;
             }else{
                wordCopy.push_back(mapObj.first);
             }
             fn_lsr(state,wordCopy);
          }
       }
   }



}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string filename = "";
   wordvec fileContents;
   filename = words[1];
   inode_ptr targetNode; 
   if(filename.find("/") != string::npos){
      size_t lastSlash = filename.find_last_of("/");
      string path = filename.substr(0,lastSlash+1);
      filename = filename.substr(lastSlash+1);
      targetNode = findNode(state,path);
   }
   else{
      targetNode = state.getCwd();
   }
   
   for(long unsigned int pos = 2; pos < words.size(); pos++){
      fileContents.push_back(words[pos]);
   }
   auto file = targetNode->getContents()->mkfile(filename);
   file->getContents()->writefile(fileContents);
   file->setPath(targetNode->getPath()+"/"+filename);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   //split vector between slashes
   
   string dirname = words[1];

   inode_ptr targetNode;
   if(dirname.find("/") != string::npos){
      size_t lastSlash = dirname.find_last_of("/");
      string path = dirname.substr(0,lastSlash+1);
      dirname = dirname.substr(lastSlash+1);
      targetNode = findNode(state,path);
   }else{
      targetNode = state.getCwd();
   }
   //dont make directory named . or ..
   if(dirname == "." || dirname == ".."){
      return;
   }
   //only make if target does not have same name directory
   auto nodeToEnter = targetNode->getContents()->getdirents().find(dirname);
   if(nodeToEnter == targetNode->getContents()->getdirents().end()){
      auto dir = targetNode->getContents()->mkdir(dirname);
      auto insertPair = pair<string,inode_ptr>("..", state.getCwd());
      dir->getContents()->getdirents().insert(insertPair);
      
      dir->setPath(targetNode->getPath()+"/"+dirname);

   }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string temp = "";
   bool passedCmd = false;
   for(auto word : words){
      if(passedCmd){
         temp += word;
         temp += " ";
      }else{
         passedCmd = true;
      }
   }
   state.changePrompt(temp);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   for(auto dir : state.getCwdPath()){
      cout << "/" << dir;
   }
   cout<<endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string filename = words[1];

   inode_ptr targetNode;
   if(filename.find("/") != string::npos){
      size_t lastSlash = filename.find_last_of("/");
      string path = filename.substr(0,lastSlash+1);
      filename = filename.substr(lastSlash+1);
      targetNode = findNode(state,path);
   }else{
      targetNode = state.getCwd();
   }

   targetNode->getContents()->remove(filename);
}

void fn_rmr (inode_state& state, const wordvec& words){
   inode_ptr dir = findNode(state, words[1]);

   wordvec parsedPath = split(words[1],"/");
   string dirname = parsedPath.back();
   inode_ptr parentDir;
   string parentPath = "";
   if(parsedPath.size() > 1){
      parsedPath.pop_back();
      for(auto word : parsedPath){
         parentPath += word;
      }
      parentDir = findNode(state, parentPath);
   }else{
      parentDir = state.getCwd();
   }
   parentDir->getContents()->remove(dirname);
   auto dirents = dir->getContents()->getdirents();

   if(dirents.size() <= 2){
      dir->getContents()->getdirents().clear();
      dir->getContents() = nullptr;
   }else{
      for(auto entryPair : dirents){
         if(entryPair.first != "." && entryPair.first != ".."){
            preExitClear(entryPair.second);
            dir->getContents()->getdirents().erase(entryPair.first);
         }
      }

      dir->getContents()->getdirents().clear();
      dir->getContents() = nullptr;
   }

}

void fn_nothing (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

inode_ptr findNode( inode_state& state, string path){
   inode_ptr currDir;
   if(path.at(0) == '/'){
      currDir = state.getRoot();
   }else{
      currDir = state.getCwd();
   }
   wordvec parsedPath = split(path,"/");
   map<string,inode_ptr> dirents;
   for(auto word: parsedPath){
      dirents = currDir->getContents()->getdirents();

      if(dirents.count(word) == 0){
         throw command_error (word + ": no such directory");
      }

      currDir = dirents.find(word)->second;
   }
   return currDir;
   
}

void preExitClear(inode_ptr& node){
   if(node->getContents()->fileType() == "file"){
      node->getContents() = nullptr;
      return;
   }
   auto dirents = node->getContents()->getdirents();

   if(dirents.size() <= 2){
      node->getContents()->getdirents().clear();
      //delete empty directory
      node->getContents() = nullptr;
   }else{
      for(auto entryPair : dirents){
         if(entryPair.first != "." && entryPair.first != ".."){
            preExitClear(entryPair.second);
            node->getContents()->getdirents().erase(entryPair.first);
         }
      }

      node->getContents()->getdirents().clear();
      //delete empty directory
      node->getContents() = nullptr;
   }
}




int stringToInt(string str){
   int result = 0;
   for(auto digit : str){
      if(isdigit(digit)){
         result = result * 10 + digit - '0';
      }else{
         return 127;
      }
   }
   return result;
}

