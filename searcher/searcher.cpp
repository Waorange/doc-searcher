#include "searcher.h"
#include "../common/util.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <unordered_map>


namespace searcher
{
    //构建正排索引和倒排索引
    bool Index::Build(const std::string & input_path)
    {
        std::ifstream input_file(input_path.c_str());
        if(!input_file.is_open()){
            std::cout << "open file error file_path:" 
                << input_path << std::endl;
            return false;
        }
        std::string line;
        uint32_t count = 0;
        while(std::getline(input_file, line))
        {
            //构建正排索引, 并反序列化字符串
            const DocInfo *doc_info = BuildForword(line);
            
            //更新倒排索引数据
            BuildInverted(doc_info);
            ++count;
            if(count % 500 == 0) 
            {
                std::cout << "Build finish: " << count <<std::endl;
            }   
        }
        return true;
    }


    const DocInfo * Index::BuildForword(const std::string &line)
    {
        //存储切分结果
        std::vector<std::string> tokens; 

        //对字符串进行切分
        StringUtil::Split(line, &tokens, "\3");
        if(tokens.size() != 3){
            return nullptr;
        }
        
        //然后构造DocInfo对象，将其插入正排索引中
        DocInfo doc_info;
        doc_info.doc_id = forword_index_.size();
        doc_info.title = tokens[0];
        doc_info.content = tokens[1];
        doc_info.url = tokens[2];

        forword_index_.push_back(doc_info);
        return &forword_index_.back();
    }
    
    void Index::BuildInverted(const DocInfo *doc_info)
    {
        //对返回的文件进行分词
        std::vector<std::string> title_tokens;
        CutWord(doc_info->title, &title_tokens);
        std::vector<std::string> content_tokens;
        CutWord(doc_info->content, &content_tokens);

        //通过分词的结果进行统计词出现次数
        struct WordCnt{
            uint32_t title_cnt;  //记录标题中词出现的次数
            uint32_t content_cnt;  //记录正文中词出现的频率
        };
        
        //统计出现的次数
        std::unordered_map<std::string, WordCnt> word_cnt;

        for(auto & word : title_tokens)
        {
            //将所有的词转换为小写
            boost::to_lower(word);
            if(word == " ")
            {
                continue;
            }
            ++word_cnt[word].title_cnt;
        }
        for(auto & word : content_tokens)
        {
            //将所有的词转换为小写
            boost::to_lower(word);
            if(word == " ")
            {
                continue;
            }
            ++word_cnt[word].content_cnt;
        }
        
        //构建Weigth对象计算词的权重
        for(const auto & word_pair : word_cnt)
        {
            Weigth word_weigth;
            word_weigth.doc_id = doc_info->doc_id;
            word_weigth.word =  word_pair.first;
            //简单计算权重 标题出现一次权重大一点
            word_weigth.weigth = word_pair.second.title_cnt * 10
                + word_pair.second.content_cnt;

            //更新倒排索引
            //如果存在则返回对应value的地址，不存在则构造对象
            InvertedList * invertedlist \
                = &inverted_index_[word_pair.first];
            invertedlist->push_back(word_weigth);
        }
    }

    void Index::CutWord(const std::string & str, 
            std::vector<std::string> * words)
    {
        jieba.CutForSearch(str, *words);
    }

    //通过文件编号查正排索引获取文件
    DocInfo * Index::GetDocInfo(uint64_t doc_id)
    {
        if(doc_id < forword_index_.size())
        {
            return &forword_index_[doc_id];
        }
        return nullptr;
    }
    //通过词查倒排索引获取倒排拉链
    InvertedList * Index::GetInvertedList(const std::string & word)
    {
        auto ret = inverted_index_.find(word);
        if(ret == inverted_index_.end()){
            return NULL;
        }

        return &ret->second; 
    }

    ///////////////////////////////////////////////////////////////
    //Searcher类定义

    //初始化Index对象构造索引
    bool Searcher::Init(const std::string & file_path)
    {
        std::cout << "Init Index" << std::endl;
        bool ret = index_->Build(file_path);
        if(!ret){
            return false;
        }
        std::cout << "Init finish" << std::endl;
        return true;
    }
    
    //通过关键词进行查询
    bool Searcher::Search(const std::string & query, std::string * result_string)
    {
        //对查询的字符串进行分词
        std::vector<std::string> keywords;
        index_->CutWord(query, &keywords);

        //通过分词结果查倒排索引, 获取倒排拉链
        InvertedList all_token_result;
        for(std::string & keyword : keywords)
        {
            boost::to_lower(keyword);
            if(keyword == " ")
            {
                continue;
            }
            std::cout << "开始查找: "<< keyword<<std::endl;
            InvertedList * invertedlist = index_->GetInvertedList(keyword);
            if(invertedlist == nullptr){
                continue;
            }
            //将获取的倒拉链结果统一放到all_token_result中
            MergeInvertedList(*invertedlist, all_token_result);
//          std::cout << "合并后" <<std::endl;
//          for(auto it : all_token_result)
//          {
//              std::cout << it.doc_id << " ";
//          }
            std::cout << std::endl;
//          all_token_result.insert(all_token_result.end(),
//                  invertedlist->begin(), invertedlist->end());
        }
        

        //对返回的结果按照一定规律进行排序
//      std::sort(all_token_result.begin(), all_token_result.end(),
//              [](const Weigth & w1, const Weigth & w2)
//              {
//                  return w1.weigth > w2.weigth;
//              });
        all_token_result.sort([](const Weigth & w1, const Weigth &w2)
                {
                    return w1.weigth > w2.weigth; 
                });
        //将总的拉链构造成JSON对象返回
        Json::Value results;  //表示所有搜索结果的JSON对象
        for(const auto & weigth: all_token_result)
        {
            //通过正排索引获取DocInfo对象指针
            const auto *doc_info = index_->GetDocInfo(weigth.doc_id);
            if(doc_info == nullptr){
                std::cout << "Index error" << std::endl;
                return false;
            }
//          std::cout << doc_info->title << std::endl;
//          std::cout << ProduceDesc(weigth.word, doc_info->content) <<std::endl;
//          std::cout << doc_info->url << std::endl;

            Json::Value result; //表示一个结果的JSON对象
            result["title"] = doc_info->title;
            result["desc"] = ProduceDesc(weigth.word, doc_info->content);
            result["url"] = doc_info->url;
            results.append(result);
        }
        //将JSON对象装换为字符串
        Json::FastWriter writer;
        *result_string = writer.write(results);
        return true;
    }

    //通过正文生成简单描述
    const std::string Searcher::ProduceDesc(const std::string & keyword
            , const std::string & content)
    {
        size_t pos = content.find(keyword);
        //如果没有找到表示正文中没有关键词
        //就用文件前一小部分生成描述
        if(pos == std::string::npos)
        {
            if(content.size() <= 100){
                return content;
            }
            else{
                return content.substr(0, 100) + "...";
            }
        }
        
        size_t beg = pos >= 50 ? pos - 50 : 0;
        //如果后面长度不足50则剩余内容全部添加到描述
        //如果后面内容够50则只截取50字符进行添加
        if(content.size() < pos + 50)
        {
            return content.substr(beg);   
        }
        else
        {
            return content.substr(beg, 100) + "...";
        }
        return content;    
    }
    void Searcher::MergeInvertedList(const InvertedList & invertedlist 
           , InvertedList & all_token_result)
    { 
//      std::cout << "合并前" <<std::endl;
//      for(auto it : invertedlist)
//      {
//          std::cout << it.doc_id << " ";
//      }
//      std::cout << std::endl;
//      for(auto it : all_token_result)
//      {
//          std::cout << it.doc_id << " ";
//      }
        std::cout << std::endl;
        //合并有序链表因为文档是有序生成的
        //所以倒排拉链中的序号本来就是有序的
        auto inverl = invertedlist.begin();
        auto result = all_token_result.begin();
        auto endinv = invertedlist.end();
        auto endres = all_token_result.end();
        while(inverl != endinv && result != endres)
        {
            if(inverl->doc_id < result->doc_id)
            {
                all_token_result.insert(result, *inverl);
                ++inverl;
            }
            else if(inverl->doc_id == result->doc_id)
            {
                //当需要查找的不同关键词在同一个文档中出现
                //提升其搜索权值
                result->weigth = 3 * (result->weigth + inverl->weigth);
                ++inverl;
            }
            else
            {
                ++result;
            }
        }
        if(inverl == endinv)
        {
            return;
        }
        else
        {
            all_token_result.insert(result, inverl, endinv);
        }   
    }
}
