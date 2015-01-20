// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $

#include "commands.h"
#include "debug.h"

commands::commands(): map ({
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },

}){}

command_fn commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   command_map::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      exit_status::set(1);
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}

//a bfs for inode tree
//return the inode pointer to a specific dir
inode_ptr bfs(inode_state& state, string& dir){
    if(dir.size()==0){
        //cout<<"size =0"<<endl;
        return state.get_cwd();
    }
    //cout<<"size != 0"<<endl;
    wordvec dirs = split(dir,"/");
    bool abs_dir = false;
    if(dir.at(0)=='/')
        abs_dir = true;
    inode_ptr target=(abs_dir?state.get_root():state.get_cwd());
    for(auto it=dirs.begin();it!=dirs.end();it++){
        if(it==dirs.end()-1){
            //check if it is a valid value
            //if(directory_ptr_of(target->get_contents())->get_map().find(*it)==(directory_ptr_of(target->get_contents())->get_map().end())
                //return nullptr;
            target = (directory_ptr_of(target->get_contents())->get_map())[*it];
            //cout<<"target:  "<<target<<endl;
        }
        else{
            target = directory_ptr_of(target->get_contents())->get_map()[*it];
            if(target==nullptr)
                return nullptr;
            if(target->get_type()==PLAIN_INODE){
                //through exp;
                return nullptr;
            }
        }
    }
    return target;
}


//return the path(string) to a specific inode pointer
/*void dfs_helper(inode_ptr root, inode_ptr target,vector<string>& res){
    if(root==target)
        return;

    directory_ptr cur_dir = directory_ptr_of(root->get_contents());
    int map_size = cur_dir->get_map().size();
    auto it =(cur_dir->get_map()).begin();
    for(int i=0;i<map_size;i++){

    }


}

string dfs(inode_state& state){
    vector<string> res;
    string result = "/";
    dfs_helper(state.get_root(),state.get_cwd(),res);
    for(int i=0;i<res.size();i++){
        result=result+res[i]+"/";
    }
    return result;
}

*/




void fn_cat (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    inode_ptr target;
    string final_dir;
    if(words.size()<2){
         exit_status::set(1);
         throw yshell_exn (words[0]+" missing operand");
         return;
    }
    final_dir = simplifyPath(words[1]);
    target = bfs(state, final_dir);
    DEBUGF('y',final_dir);
    if(target==nullptr){
         exit_status::set(1);
         throw yshell_exn ("no such file or directory");
         return;
    }
    else if (target->get_type()==DIR_INODE) {
         exit_status::set(1);
         throw yshell_exn (words[1]+" is a directory");
         return;
    }
    wordvec read_w = plain_file_ptr_of(target->get_contents())->readfile();
    string res;
    for(size_t i=0;i<read_w.size();i++){
        res+=read_w[i]+" ";
    }
    cout<<res<<endl;
    return;
}

void fn_cd (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    inode_ptr ptr;
    string final_dir;
    if(words.size()<2){
        final_dir = simplifyPath(".");
        exit_status::set(1);
        throw yshell_exn (words[0]+" missing operand");
    }
    final_dir = simplifyPath(words[1]);
    ptr = bfs(state, final_dir);
    if(ptr==nullptr){
        exit_status::set(1);
        throw yshell_exn (words[0]+" no such directory");
        return;
    }
    state.set_cwd(ptr);

    //deal with pwd vector
    if(words[1][0]=='/')
        state.pwd_clean();
    wordvec dirs = split(words[1],"/");
    for(size_t i=0;i<dirs.size();i++){
        if(dirs[1]=="..")
            if(state.pwd_size()!=0)
                state.pwd_pop();
            else
                continue;
        else
            state.pwd_push(dirs[i]);
    }
}

void fn_echo (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    string res=" ";
    for(size_t i=1;i<words.size();i++){
        res+=words[i]+" ";
    }
    cout<<res<<endl;
}

void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit_exn();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr target;
   string final_dir;
   if(words.size()<2)
        final_dir = simplifyPath(".");
   else final_dir = simplifyPath(words[1]);
   target = bfs(state, final_dir);
   DEBUGF('y',final_dir);
   if(target==nullptr){
        exit_status::set(1);
        throw yshell_exn (words[0]+": no such file or directory");
   }
   else
       cout<<final_dir<<":"<<endl;
   //iterate the dirents in target
   directory_ptr cur_dir = directory_ptr_of(target->get_contents());
   int map_size = cur_dir->get_map().size();
   auto it =(cur_dir->get_map()).begin();
   for(int i=0;i<map_size;i++){
        inode_ptr temp = it->second;
        int temp_size;
        string dir="";
        if(temp->get_type()==PLAIN_INODE)
            temp_size = plain_file_ptr_of(temp->get_contents())->size();
        else {
            temp_size = directory_ptr_of(temp->get_contents())->size();
            dir="/";
        }
        string name;
        if(it->first=="."||it->first=="..")
            dir="";
        cout<<temp->get_inode_nr()<<"      "<<temp_size<<"      "<<it->first+dir<<"\n"<<endl;
       it++;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    inode_ptr target;
    string final_dir;
    if(words.size()<2)
         final_dir = simplifyPath(".");
    else final_dir = simplifyPath(words[1]);
    target = bfs(state, final_dir);
    DEBUGF('y',final_dir);
    if(target==nullptr){
         throw yshell_exn (words[0]+": no such file or directory");
    }
    else
       cout<<final_dir<<"/:"<<endl;
    vector<string> next_level;
    directory_ptr cur_dir = directory_ptr_of(target->get_contents());
    int map_size = cur_dir->get_map().size();
    auto it =(cur_dir->get_map()).begin();
    for(int i=0;i<map_size;i++){
        inode_ptr temp = it->second;
        int temp_size;
        string dir="";
        if(temp->get_type()==PLAIN_INODE)
            temp_size = plain_file_ptr_of(temp->get_contents())->size();
        else {
            temp_size = directory_ptr_of(temp->get_contents())->size();
            dir="/";
            if(it->first!="."&&it->first!="..")
                next_level.push_back(final_dir+"/"+it->first);
        }
        string name;
        if(it->first=="."||it->first=="..")
            dir="";
        cout<<temp->get_inode_nr()<<"      "<<temp_size<<"      "<<it->first+dir<<"\n"<<endl;
        it++;
    }
    for(size_t i=0;i<next_level.size();i++){
        vector<string> n_dir;
        n_dir.push_back("lsr");
        n_dir.push_back(next_level[i]);
        fn_lsr (state, n_dir);
    }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()<2){
        exit_status::set(1);
        throw yshell_exn (words[0]+": missing operand");
    }

    string path;
    string file_name;
    string final_dir = simplifyPath(words[1]);
    inode_ptr target;
    std::size_t found0 = final_dir.find("/");
    if (found0==std::string::npos){
        target=state.get_cwd();
        file_name = final_dir;
    }
    else{
        unsigned found1 = final_dir.find_last_of("/\\");
        path = final_dir.substr(0,found1);
        file_name = final_dir.substr(found1+1);
        target = bfs(state, path);
    }
    if(target==nullptr){
        exit_status::set(1);
        throw yshell_exn (words[0]+": no such directory: "+path);
        return;
    }
    inode_ptr c_dir = target;
    if(directory_ptr_of(c_dir->get_contents())->get_map().count(file_name)==0){
        DEBUGF('y',"creating a new file");
        inode_ptr new_file = make_shared<inode>(PLAIN_INODE);
        new_file->get_contents() = make_shared<plain_file>();
        directory_ptr_of(c_dir->get_contents())->set_map(file_name,new_file);
        plain_file_ptr_of(new_file->get_contents())->writefile(words);
        return;
    }
    else{
        inode_ptr victim = directory_ptr_of(c_dir->get_contents())->get_map()[file_name];
        if(victim ->get_type()==DIR_INODE){
            exit_status::set(1);
            throw yshell_exn (file_name+": is a directory");
        }
        else{
            plain_file_ptr_of(victim->get_contents())->writefile(words);
            return;
        }
    }

   /*int size_map = directory_ptr_of(c_dir->get_contents())->get_map().size();
   cout<<directory_ptr_of(c_dir->get_contents())->get_map().size()<<endl;
   auto it =(directory_ptr_of(c_dir->get_contents())->get_map()).begin();
   for(int i=0;i<size_map;++i){
       cout<<"it map"<<it->first<<endl;
       ++it;
    }*/

}

void fn_mkdir (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    if(words.size()<1){
        exit_status::set(1);
        throw yshell_exn (words[0]+": missing operand");
    }
    string path;
    string file_name;
    string final_dir = simplifyPath(words[1]);
    inode_ptr target;
    std::size_t found0 = final_dir.find("/");
    if (found0==std::string::npos){
        target=state.get_cwd();
        file_name = final_dir;
    }
    else{
        unsigned found1 = final_dir.find_last_of("/\\");
        path = final_dir.substr(0,found1);
        file_name = final_dir.substr(found1+1);
        target = bfs(state, path);
    }
    if(target==nullptr){
        exit_status::set(1);
        throw yshell_exn (words[0]+": no such directory: "+path);
        return;
    }
    inode_ptr c_dir = target;
    inode_ptr new_file = make_shared<inode>(DIR_INODE);
    new_file->get_contents() = make_shared<directory>();
    directory_ptr_of(c_dir->get_contents())->set_map(file_name,new_file);

    directory_ptr_of(new_file->get_contents())->set_map("..",c_dir);
    directory_ptr_of(new_file->get_contents())->set_map(".",new_file);

}

void fn_prompt (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    if(words.size()<1){
        exit_status::set(1);
        throw yshell_exn (words[0]+": missing operand");
    }
    string res;
    for(size_t i=1;i<words.size();i++){
        res+=words[i]+" ";
    }
    state.set_prompt(res);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   vector<string> cur_dir;
   cur_dir = state.get_pwd();
   string res="/";
   for(size_t i=0;i<cur_dir.size();i++){
       res+=cur_dir[i]+"/";
   }
    cout<<res<<endl;
}

void fn_rm (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);

    string final_dir;
    string file_name;
    string path;

    inode_ptr target;
    inode_ptr victim;
    if(words.size()<1){
         exit_status::set(1);
         throw yshell_exn (words[0]+": missing operand");
         return;
    }
    if(words[1]==".."||words[1]=="."){
         exit_status::set(1);
         throw yshell_exn ("you cannot remove "+words[1]);
         return;
    }
    final_dir = simplifyPath(words[1]);
    std::size_t found0 = final_dir.find("/");
    if (found0==std::string::npos){
        target=state.get_cwd();
        file_name = final_dir;
    }
    else{
        unsigned found1 = final_dir.find_last_of("/\\");
        path = final_dir.substr(0,found1);
        file_name = final_dir.substr(found1+1);
        target = bfs(state, path);
    }
    //from target delete victim
    DEBUGF('y',file_name);
    victim = directory_ptr_of(target->get_contents())->get_map()[file_name];
    if(victim==nullptr){
        exit_status::set(1);
        throw yshell_exn (words[0]+": no such file or directory");
        return;
    }
    if(victim->get_type()==PLAIN_INODE){
        DEBUGF('y',directory_ptr_of(target->get_contents())->get_map().size());
        directory_ptr_of(target->get_contents())->map_erase(file_name);
        DEBUGF('y',directory_ptr_of(target->get_contents())->get_map().size());
        return;
    }
    else{
        DEBUGF('y',directory_ptr_of(victim->get_contents())->get_map().size());
        if(directory_ptr_of(victim->get_contents())->get_map().size()>2){
             exit_status::set(1);
            throw yshell_exn (words[0]+": not an empty directory");
            return;
        }
        else {
            DEBUGF('y',directory_ptr_of(target->get_contents())->get_map().size());
            directory_ptr_of(target->get_contents())->map_erase(file_name);
            DEBUGF('y',directory_ptr_of(target->get_contents())->get_map().size());
            return;
        }
    }

}

void fn_rmr (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    inode_ptr target;
    string final_dir;
    string file_name;
    string path;
    inode_ptr victim;
    if(words.size()<1){
        exit_status::set(1);
        throw yshell_exn (words[0]+": missing operand");
        return;
    }
    if(words[1]==".."||words[1]=="."){
        exit_status::set(1);
        throw yshell_exn ("you cannot remove "+words[1]);
        return;
    }
    final_dir = simplifyPath(words[1]);
    std::size_t found0 = final_dir.find("/");
    if (found0==std::string::npos){
        target=state.get_cwd();
        file_name = final_dir;
    }
    else{
        unsigned found1 = final_dir.find_last_of("/\\");
        path = final_dir.substr(0,found1);
        file_name = final_dir.substr(found1+1);
        target = bfs(state, path);
    }
    //from target delete victim
    DEBUGF('y',file_name);
    //vector<string> next_level;
    victim = directory_ptr_of(target->get_contents())->get_map()[file_name];
    if(victim==nullptr){
        exit_status::set(1);
        throw yshell_exn (words[0]+": no such file or directory");
        return;
    }
    if(victim->get_type()==PLAIN_INODE){
        //DEBUGF('y',directory_ptr_of(target->get_contents())->get_map().size());
        directory_ptr_of(target->get_contents())->map_erase(file_name);
        //DEBUGF('y',directory_ptr_of(target->get_contents())->get_map().size());
        return;
    }
    else{
        DEBUGF('y',"the dir:"<<file_name<<" size is"<<directory_ptr_of(victim->get_contents())->get_map().size());
        if(directory_ptr_of(victim->get_contents())->get_map().size()>2){
            //iterate delete all in the map
            int map_size = (directory_ptr_of(victim->get_contents())->get_map()).size();
            auto it =(directory_ptr_of(victim->get_contents())->get_map()).begin();
            for(int i=0;i<map_size;i++){
                DEBUGF('y',"in for :the dir:"<<file_name<<" size is"<<directory_ptr_of(victim->get_contents())->get_map().size());
                if(it->first!="."&&it->first!=".."){
                    wordvec next_level;
                    next_level.push_back("rmr");
                    next_level.push_back(final_dir+"/"+it->first);
                    DEBUGF('y',"deleting :"<<it->first);
                    fn_rmr (state, next_level);
                    DEBUGF('y',"deleted");
                    it++;
                    map_size--;
                }
            }
        }
        DEBUGF('y',"the dir:"<<file_name<<" size is"<<directory_ptr_of(victim->get_contents())->get_map().size());

        if(directory_ptr_of(victim->get_contents())->get_map().size()==2){
            //delete the victim directory_ptr
            DEBUGF('y',"deleting an empty dir "<<directory_ptr_of(target->get_contents())->get_map().size());
            directory_ptr_of(target->get_contents())->map_erase(file_name);
            DEBUGF('y',"dir removed"<<directory_ptr_of(target->get_contents())->get_map().size());
            return;
        }
    }

}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

