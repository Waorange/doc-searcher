#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <string>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>

class FileUtil
{
public:
    static bool Read(const std::string &file_path, std::string *content)
    {
        std::ifstream file(file_path.c_str());
        if(!file.is_open()){
            return false;
        }

        //按行读取内容
        std::string line;
        while(std::getline(file, line)){
            *content += line + "\n";
        }
        file.close();
        return true;
    }
};

class StringUtil
{
public:
    static void Split(const std::string & input,
            std::vector<std::string> *output, const std::string & split_ch)
    {
        //采用不压缩的方式进行切分
        boost::split(*output, input, boost::is_any_of(split_ch), 
                boost::token_compress_off); 
    }  
};

#endif

