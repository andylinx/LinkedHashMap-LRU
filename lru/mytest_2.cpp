#include "src.hpp"
#include <iostream>
#include <cassert>
#include <string>
using namespace sjtu;
int main(){

    double_list<int> list;
    list.insert_head(10);
    list.insert_head(11);
    for(auto it = list.begin();it!=list.end();it++){
        std::cout<<*it<<std::endl;
    }
    
    linked_hashmap <int,int> G;
    G.insert(pair<int,int>(1,2));

    linked_hashmap <int,int> :: iterator it = G.begin();
    linked_hashmap <int,int> :: iterator itt(it);
    if(it.id->begin() == itt.pos)
        printf("yes\n");
    return 0;
}