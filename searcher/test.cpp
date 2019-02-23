#include <iostream>
#include <string>
#include "searcher.h"

int main()
{
    searcher::Searcher search;
    if(!search.Init("../data/temp/raw_input"))
    {
        std::cout << "Init Searcher Error"<< std::endl;
        return 1;
    }

    std::string results;
    search.Search("Postconditions filesystem location", &results);

    
//    std::cout << results << std::endl;
   
  
    return 0;
}
