#ifndef __SEARCHER_H__
#define __SEARCHER_H__


#include <cppjieba/Jieba.hpp>

namespace searchar
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

typedef std::vector<Weigth> InvertedList;

//用于构建索引
class Index
{
    Index()
        :jieba(DICT_PATH,
                HMM_PATH,
                USER_DICT_PATH,
                IDF_PATH,
                STOP_WORD_PATH)
    {}
    //构建正排索引和倒排索引
    bool Build();

    //通过文件编号查正排索引获取文件
    DocInfo * GetDocInfo(uint64_t doc_id);
    //通过词查倒排索引获取倒排拉链
    InvertedList * GetInvertedList(const std::string & word);

private:
    //正排索引 通过文件编号获取文件内容
    std::vector<DocInfo> forword_index_;
    
    //倒排索引 通过关键词获取文件编号
    std::unordered_map<std::string, InvertedList> inverted_index_;
    
    //结巴分词，用于分词
    cppjieba::Jieba jieba;
};



}

#endif
