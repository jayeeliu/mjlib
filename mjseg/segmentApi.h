#ifndef _H_H_SEGMENTAPI_H_H_
#define _H_H_SEGMENTAPI_H_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tokenizer_define.h"

// 每个词条最大子粒度数量
#define MAX_CHILD_NUM 256

// 词性标签名字最长长度
#ifndef MAX_POSTAG_NAME_LEN
#define MAX_POSTAG_NAME_LEN 32
#endif
// 语义标签名字最长长度
#ifndef MAX_SEMTAG_NAME_LEN
#define MAX_SEMTAG_NAME_LEN 1024
#endif
// 每个词条最多可以具有的语义标签数量
#ifndef MAX_WORD_SEMTAG_COUNT
#define MAX_WORD_SEMTAG_COUNT 64
#endif

typedef struct SEGMENTHANDLE_*  PSEGMENTHANDLE;
typedef struct SEGPTHREADRESULTHANDLE_* PSEGPTHREADRESULTHANDLE; 

typedef struct SEG_WORD_ITEM 
{
    unsigned int wordPos;       // Position in source string 在串中的位置
    unsigned long long wordID;  // WORDID
    unsigned char wordLen;      // Length of this word
    unsigned char wordCat;      // 词的固有属性：例如中文、英文、数字等属性

    PROPERTY_TYPE property;
    unsigned int weight;        // 词权重,计算方式不同于idf
    POS_TAG_TYPE postag_id;      // 词性ID
    unsigned int semtag_size;   // 语义标签ID数量
    SEMANTIC_TAG_TYPE semtag_id[MAX_WORD_SEMTAG_COUNT]; // 标签ID
    
    unsigned char idf;
    unsigned wordvalue;
} SEG_WORD_ITEM_T;

typedef struct SEG_PHRASE_ITEM 
{
    SEG_WORD_ITEM_T* word;      //指向长词的指针
    unsigned short childnum;    //小词个数
    unsigned int childwordPos[MAX_CHILD_NUM];   //小词wordpos列表
    unsigned int childwordLen[MAX_CHILD_NUM];   //小词wordlen列表
} SEG_PHRASE_ITEM_T;

typedef struct SEG_APP_ITEM 
{
    unsigned int appwordPos;    //应用层wordpos
    unsigned int appwordLen;    //应用层wordlen
} SEG_APP_ITEM_T;

typedef struct SEG_WORD_SEGMENT 
{
    int word_num;               //分词结果数
    SEG_WORD_ITEM_T* wordlist;  //分词结果列表
    int phrase_num;             //长词结果数
    SEG_PHRASE_ITEM_T* phraselist;  //长词结果列表
    int app_num;                    //应用层长词结果数
    SEG_APP_ITEM_T* applist;        //应用层长词结果列表
} SEG_WORD_SEGMENT_T;

/*
 *  @brief  initialize one segmentor
 *
 *  @param config config data file
 *  @param opt  没用
 *
 *  @return NULL on failure,segmentor on success
 */
PSEGMENTHANDLE initSegHandle(const char* config, int opt);

/*
 *  @brief  free one segmentor
 *
 *  @return 1 on failure, 0 on success
 */
int freeSegHandle(PSEGMENTHANDLE segHandle);

/*
 *  @brief  initialize one resultHandle,safe for one pthread
 *
 *  @return NULL on failure, resultHandle on success
 */
PSEGPTHREADRESULTHANDLE initPthreadSegResultHandle();

/*
 *  @brief  free one resultHandle
 *
 *  @return 1 on failure,0 on success
 */
int freePthreadSegResultHandle(PSEGPTHREADRESULTHANDLE resultHandle);

/*
 *  @brief  load user's dicts, DO NOTHING NOW !! 
 *          modify the config file to use user dict!!
 *
 *  @param segHandle    segmentor
 *  @param userDicPath  one file or dir with files
 *
 *  @return 1 on failure,0 on success
 */
int loadUserDics(PSEGMENTHANDLE segHandle,const char* userDicPath);

/*
 *  @brief  segment 
 *
 *  @pata segHandle segmentor
 *  @para src   input string
 *  @para srclen    length of @src
 *  @para charset   0->gbk,1->utf-8,
 *  @para ParticleSize  0->long words; 
 *                      1->short words; 
 *                      2->long words & short words
 *
 *  @return 1 on failure,0 on success
 */
int segment(PSEGMENTHANDLE segHandle,
                const char* src,
                int srclen,
                char charset,
                char ParticleSize,
                PSEGPTHREADRESULTHANDLE resultHandle, 
                SEG_WORD_SEGMENT_T** word_segment);

/*
 *  @brief  get the count of words, DO NOTHING!! just return 0!
 *
 *  @param resultHandle resultHandle
 *
 *  @return count
 */
int getWordCount(PSEGPTHREADRESULTHANDLE resultHandle);

/*
 *  @brief  get one word, DO NOTHING!! just return 0!
 *
 *  @para segHandle
 *  @pata resultHandle  resultHandle
 *  @para index index of word
 *  @para word  mem buffer(user define)
 *  @para wordId
 *
 *  @return 0 on success,1 on failure
 */
int getWordInfo(PSEGMENTHANDLE segHandle,
                PSEGPTHREADRESULTHANDLE resultHandle,
                int index,
                char* word,
                int* wordId);

/*
 *  @brief  DO NOTHING! just return 0!
 */
int getWordAtt(PSEGMENTHANDLE segHandle, const char* word);

/*
 *  @brief  get POS tag name by POS tag id
 *
 *  @param name buf to store the name
 *  @param len  buf length
 *  @param enc  encode type of tag name
 *
 *  @return 0 if success, <0 else
 */
int getPosNameById(PSEGMENTHANDLE segHandle, 
                    POS_TAG_TYPE postag_id, 
                    char* name, int len, enum ENCODE_TYPE enc);

/*
 *  @brief  get semantic-tag name by semantic-tag id
 *
 *  @param name buf to store the name
 *  @param len  buf length
 *  @param enc  encode type of tag name
 *
 *  @return 0 if success, <0 else
 */
int getSemNameById(PSEGMENTHANDLE segHandle, 
                    SEMANTIC_TAG_TYPE semtag_id, 
                    char* name, int len, enum ENCODE_TYPE enc);

#ifdef __cplusplus
}
#endif

#endif  // header protector
