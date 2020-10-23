// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

#include "commands.h"
#include "debug.h"

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
   for(int i = 1; i < words.size(); i++){
      filenames.push_back(words[i]);
   }

   auto fileMapObj = state.getCwd()->getContents()->getdirents().find(filenames[0]);
   auto filePtr = fileMapObj->second;
   auto data = filePtr->getContents()->readfile();

   for(auto filename : filenames){
      fileMapObj = state.getCwd()->getContents()->getdirents().find(filename);
      filePtr = fileMapObj->second;
      data = filePtr->getContents()->readfile();

      for(auto word : data){
         cout << word << " ";
      }
      cout << endl;
   }
   cout << endl;
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   for( auto mapObj : state.getCwd()->getContents()->getdirents()){
      auto inodePtr = mapObj.second;
      cout << inodePtr->get_inode_nr() << " " << inodePtr->getContents()->size() << " "<< mapObj.first << endl;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string filename = "";
   wordvec fileContents;
   filename = words[1];
   for(long unsigned int pos = 2; pos < words.size(); pos++){
      fileContents.push_back(words[pos]);
   }
   auto file = state.getCwd()->getContents()->mkfile(filename);
   file->getContents()->writefile(fileContents);
}

void fn_mkdir (inode_state& state, const wordvec& words){
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
   if(state.getCwd() == state.getRoot()){
      cout<<"in root!" <<endl;
   }
   //cout << state.getCwd();
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string filename = words[1];
   
   state.getCwd()->getContents()->remove(filename);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_nothing (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}


