///////////////////////////////////////////////////////
//用于html文件的解析
//讲html的标签去掉，并且将其标题, 内容，和url
//组织成行文件进行输出

//代码中输入参数用const &表示
//      输出参数用 * 表示
//      输入输出参数用 & 表示
////////////////////////////////////////////////////////
#include<iostream>
#include<string>
#include<vector>
#include<boost/filesystem/operations.hpp>
#include<boost/filesystem/path.hpp>
#include"../common/util.hpp"

//讲html文件解析后组织成下面的结构
struct DocInfo{ 
    std::string title;
    std::string content;
    std::string url;
};

std::string g_input_path = "../data/input/html";
std::string g_output_path = "../data/temp/raw_input";

//boost库文件系统命名空间
namespace fs = boost::filesystem;

//用于枚举文件将一个文件中的所有文件遍历
//将其路径添加到vector中
bool EnumFile(const std::string& file_path, 
        std::vector<std::string>* File_List)
{
    fs::path root_path(file_path);
    
    //文件不存在
    if(!fs::exists(root_path))
    {
        std::cout<<"file path error"<<std::endl;
        return false;
    }
    
    //使用迭代器的方式进行遍历文件
    fs::recursive_directory_iterator end_iter;
    fs::recursive_directory_iterator iter(root_path);

    for(;iter!=end_iter;iter++)
    {
        if(fs::is_regular_file(*iter))
        {
            if(iter->path().extension() == ".html")
            {
                File_List->push_back(iter->path().string());
            }
        }
    }
    return true;
}

//将文件的标题从html文件中提取出来
bool ParseTitle(const std::string& file_text, std::string* title)
{
    size_t begin = file_text.find("<title>");
    if(begin == std::string::npos){
        return false;
    }
    begin += strlen("<title>");

    size_t end = file_text.find("</title>");
    if(end == std::string::npos){
        return false;
    }
    if(end < begin){
        std::cout << "end > begin" << std::endl;
        return false;
    }

    *title = file_text.substr(begin, end-begin);
    return true;
}

//去掉html文件的标签将内容提取出来
bool ParseConten(const std::string& file_text, std::string* content)
{
    //用于标记标签状态
    //如果为真表示当前处于标签内，如果为假表示当前处于内容中
    bool tag_flag = false;
    size_t size = file_text.size();
    size_t pos = file_text.find("</title>");
    if(pos == std::string::npos)
    {
        pos = 0;
    }
    else
    {
        pos += strlen("</title>");
    }
    for(size_t i = pos; i < size; ++i)
    {
        if(!tag_flag)
        {
            if(file_text[i] != '<'){
                if(file_text[i] == '\n'){
                    content->push_back(' ');
                }
                else{
                    content->push_back(file_text[i]);
                }
            }
            else{
                tag_flag = true;
            }
        }
        else
        {
            if(file_text[i] == '>'){
                tag_flag = false;
            }
        }
    }
    return true;
}

bool ParseUrl(const std::string &path, std::string* url)
{
    //和boost官网上的帮助文档url进行对应
    //url前缀有统一的格式
    const std::string prefix = "https://www.boost.org/doc/libs/1_69_0/doc/html";
    
    //根据文件路径提供utl的后面的路径
    int pos = path.find(g_input_path);
    if(pos == std::string::npos){
        return false;
    }

    pos += g_input_path.size();
    *url = prefix + path.substr(pos);
    return true;
}


bool Parse(std::string path, DocInfo* doc)
{
    //将html文件读取出来存入string 
    std::string file_text;
    bool ret = FileUtil::Read(path, &file_text);
    if(!ret){
        std::cout << "read html file error" <<std::endl;
        return false;
    }
    
    //解析html文件标题
    ret = ParseTitle(file_text, &(doc->title));
    if(!ret){
        std::cout<<"ParseTitle error"<<std::endl;
        return false;
    }

    //解析html文件内容
    ret = ParseConten(file_text, &(doc->content));
    if(!ret){
        std::cout<<"ParseConten error"<<std::endl;
        return false;
    }
    
    //解析url
    ret = ParseUrl(path, &(doc->url));
    if(!ret){
        std::cout<<"ParseURL error"<<std::endl;
        return false;
    }
    return true;
}

bool WriteOutPut(const DocInfo & doc_info, 
        std::ofstream & output_file)
{
    const std::string write_string = doc_info.title
        +'\3' + doc_info.content
        +'\3' + doc_info.url + '\n';
    output_file.write(write_string.c_str(), write_string.size());
    return true;
}

int main()
{
    bool ret = false;
    std::vector<std::string> FileList;
    
    //遍历该目录找整个目录中所有文件
    ret = EnumFile(g_input_path, &FileList);
    if(!ret){
        std::cout<< "EnumFile Error!" << std::endl;
        return 1;
    }
    
    //打开输出文件用于存储行文本文件
    std::ofstream output_file (g_output_path);
    if(!output_file.is_open()){
        std::cout << "output file open error file_path:"\
            << g_output_path << std::endl;
    }

    for( const auto& path : FileList)
    {
        //建立DocInfo存储解析的内容
        DocInfo doc_info;
        ret = Parse(path, &doc_info);
        if(!ret){
            std::cout<< "Parse Error! file_path:"
               << path << std::endl;
            continue;
        }
        ret = WriteOutPut(doc_info, output_file);
        if(!ret){
            std::cout << "WriteOutPut Error" << std::endl;
            return 1;
        }
    }
    return 0;
}
