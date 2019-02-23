#ifndef __SEARCHER_H__
#define __SEARCHER_H__


#include <cppjieba/Jieba.hpp>
#include <jsoncpp/json/json.h>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>

namespace searcher
{

const char* const DICT_PATH = "../dict/jieba.dict.utf8";
const char* const HMM_PATH = "../dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../dict/user.dict.utf8";
const char* const IDF_PATH = "../dict/idf.utf8";
const char* const STOP_WORD_PATH = "../dict/stop_words.utf8";

struct Weigth{
    //文件编号
    uint64_t doc_id;

    //搜索权重
    uint32_t weigth;
    
    std::string word;
};

//每个需要搜索的文件
struct DocInfo{
    uint64_t doc_id;
    std::string title;
    std::string content;
    std::string url;
};

typedef std::list<Weigth> InvertedList;

//用于构建索引
class Index
{
public:
    Index()
        :jieba(DICT_PATH,
                HMM_PATH,
                USER_DICT_PATH,
                IDF_PATH,
                STOP_WORD_PATH)
    {}
    //构建正排索引和倒排索引
    bool Build(const std::string & input_path);

    //通过文件编号查正排索引获取文件
    DocInfo * GetDocInfo(uint64_t doc_id);
    
    //通过词查倒排索引获取倒排拉链
    InvertedList * GetInvertedList(const std::string & word);
    
    //分词
    void CutWord(const std::string & str, \
            std::vector<std::string> * words);
private:
    const DocInfo *BuildForword(const std::string &line);
    void BuildInverted(const DocInfo * doc_info);

private:

    //正排索引 通过文件编号获取文件内容
    std::vector<DocInfo> forword_index_;
    
    //倒排索引 通过关键词获取文件编号
    std::unordered_map<std::string, InvertedList> inverted_index_;
    
    //结巴分词，用于分词
    cppjieba::Jieba jieba;
};

class Searcher
{
public:
    Searcher()
        :index_(new Index)
    {}
    ~Searcher()
    {
        if(index_ != nullptr){
            delete index_;
        }
    }
    //初始化Index对象构造索引
    bool Init(const std::string & file_path);
    
    //通过关键词进行查询
    bool Search(const std::string & query, std::string * result);

    //生成文章关于搜索内容的摘要
    static const std::string ProduceDesc(const std::string & keyword, 
          const std::string & content);
    
    //用于合并搜索出的倒排拉链
    static void MergeInvertedList(const InvertedList & invertedlist 
           ,InvertedList &  all_token_result);

private:
    Index * index_;
};



}

#endif
