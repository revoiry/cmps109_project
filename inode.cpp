// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $

#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "inode.h"
#include "util.h"
#include "commands.h"


int inode::next_inode_nr {1};

inode::inode(inode_t init_type):
   inode_nr (next_inode_nr++), type (init_type){
   switch (type) {
      case PLAIN_INODE:
           contents = make_shared<plain_file>();
           break;
      case DIR_INODE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) {
   plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
   if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
   return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
   directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
   if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
   return dirptr;
}


size_t plain_file::size() const {
   size_t size {0};
   size = data.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   for(auto it = words.begin()+2; it!=words.end();it++){
     data.push_back(*it);
   }
   DEBUGF ('i', words);
}

size_t directory::size() const {
   size_t size {0};
   size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   //check dirents contains the dir/file
   if(dirents.find(filename)!=dirents.end()){
        inode_ptr temp = dirents[filename];
        if(temp->get_type()==PLAIN_INODE){
            //delete from map
            dirents.erase(filename);
            return;
        }
        if(temp->get_type()==DIR_INODE){
            //valid dir
            if(filename!="."&&filename!=".."){
                //not an empty dir
                if(directory_ptr_of(temp->get_contents())->dirents.size()==2){
                    //delete from map
                    dirents.erase(filename);
                    return;
                }
            }
            //through exp
            throw ysh_exit_exn();
            return;
        }
    }
   else {
        //through exp
        throw ysh_exit_exn();
        return;
   }

    DEBUGF ('i', filename);
}

// set and get function add
//added for inode
inode_t inode::get_type(){
    return type;
}
file_base_ptr inode::get_contents(){
    return contents;
}

//for inode_state
inode_ptr inode_state::get_cwd(){
    return cwd;
}

inode_ptr inode_state::get_root(){
    return root;
}

void inode_state::set_cwd(inode_ptr cwd_new){
    cwd = cwd_new;
}

void inode_state::pwd_clean(){
    pwd.clear();
}

void inode_state::pwd_pop(){
    pwd.pop_back();
}

int inode_state::pwd_size(){
    return pwd.size();
}

void inode_state::pwd_push(string &dir){
    pwd.push_back(dir);
}

vector<string> inode_state::get_pwd(){
    return pwd;
}
void inode_state::set_prompt(string p){
    prompt = p;

}

string inode_state::get_prompt(){
    return prompt;
}


// added for directory
void directory:: set_map (const string& name, inode_ptr& ptr){
    //dirents[name] = ptr;
    dirents.insert(std::pair<const string&, inode_ptr&>(name,ptr));
}

void directory::map_erase(string s){
    dirents.erase(s);
}


map<string,inode_ptr> directory::get_map (){
    return dirents;
}

inode_state::inode_state() {
    //new a inode for root
    //inode root_node(DIR_INODE);
    /*root_node = make_shared<inode>;
    root = (inode_ptr)&root_node;
    cwd = (inode_ptr)&root_node;*/
    root = make_shared<inode>(DIR_INODE);
    cwd = root;

    root->contents = make_shared<directory>();
    directory_ptr_of(root->contents)->set_map(".",root);
    directory_ptr_of(root->contents)->set_map("..",root);

    DEBUGF ('i',root->get_contents());
    DEBUGF ('i',root->get_inode_nr());
    DEBUGF ('i',directory_ptr_of(root->contents)->get_map().begin()->first);

   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt <<"\"");
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

