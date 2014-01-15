#ifndef WEIBO_TOKENIZER_TOKENIZER_DEFINE_H
#define WEIBO_TOKENIZER_TOKENIZER_DEFINE_H

#include <inttypes.h>

#ifndef RELEASE_C
#ifdef __cplusplus
namespace weibo
{
namespace tokenizer
{
#endif
#endif

/*
 *  supported encode type
 */
enum ENCODE_TYPE {UTF_8, GBK, UNICODE};

/*
 *  supportted segmentor type, such as: word segmentor 、char segmentor
 */
enum TOKENIZER_TYPE {DEFAULT_TOKENIZER};

typedef uint16_t WORD_LEN_TYPE; 

/*
 *  granularity of token
 */
typedef uint32_t GRAN_TYPE;

static const GRAN_TYPE NORMAL_WORD = 0x00000001;
static const GRAN_TYPE SMALL_WORD = 0x00000002;
static const GRAN_TYPE PHRASE_WORD = 0x00000004;

/*
 *  categories of dynamically recognized words
 */
typedef uint32_t PROPERTY_TYPE;

static const PROPERTY_TYPE PROPERTY_DEF = 0x00;  //unknown
static const PROPERTY_TYPE PROPERTY_DFA = 0x01;  //DFA
static const PROPERTY_TYPE PROPERTY_NUM = 0x10;   //数字，不包含中文
static const PROPERTY_TYPE PROPERTY_LET = 0x20; //纯字母串
static const PROPERTY_TYPE PROPERTY_LETNUM = 0x40; //数字-字母混合串
static const PROPERTY_TYPE PROPERTY_CHN_NUM = 0x80;   //中文数字
static const PROPERTY_TYPE PROPERTY_ORD_NUM = 0x100;   //序数词
static const PROPERTY_TYPE PROPERTY_DATE = 0x200;  //日期
static const PROPERTY_TYPE PROPERTY_TIME = 0x400;  //时间
static const PROPERTY_TYPE PROPERTY_TEL = 0x800;   //电话号码
static const PROPERTY_TYPE PROPERTY_EMAIL = 0x1000; //Email地址
static const PROPERTY_TYPE PROPERTY_URL = 0x2000;   //URL地址
static const PROPERTY_TYPE PROPERTY_IP = 0x4000;    //IP地址
static const PROPERTY_TYPE PROPERTY_PUNC = 0x8000;    //标点符号
static const PROPERTY_TYPE PROPERTY_PER_NAME = 0x100000;  //人名
static const PROPERTY_TYPE PROPERTY_ORG = 0x200000;   //机构名
static const PROPERTY_TYPE PROPERTY_PLACE = 0x400000; //地名
static const PROPERTY_TYPE PROPERTY_PER_NAME_CHN = 0x1000000; //中国人名
static const PROPERTY_TYPE PROPERTY_PER_NAME_EUR = 0x2000000; //欧美人名

/*
 *  defination of pos tags
 */
typedef uint8_t POS_TAG_TYPE;

static const POS_TAG_TYPE POS_TAG_DEF = 0; //unknown
static const POS_TAG_TYPE POS_TAG_LD = 130; //副词功能习用语
static const POS_TAG_TYPE POS_TAG_LB = 155; //区别词功能成语
static const POS_TAG_TYPE POS_TAG_LA = 134; //形容词功能习用语
static const POS_TAG_TYPE POS_TAG_LN = 133; //名词功能习用语
static const POS_TAG_TYPE POS_TAG_TT = 142; //专名时间词
static const POS_TAG_TYPE POS_TAG_VTN = 10; //
static const POS_TAG_TYPE POS_TAG_LV = 116; //动词功能习用语
static const POS_TAG_TYPE POS_TAG_TG = 158; //时语素
static const POS_TAG_TYPE POS_TAG_DF = 144; //否定副词
static const POS_TAG_TYPE POS_TAG_DG = 161; //副语素
static const POS_TAG_TYPE POS_TAG_DC = 145; //程度副词
static const POS_TAG_TYPE POS_TAG_YG = 11; //语气词语素
static const POS_TAG_TYPE POS_TAG_D = 124; //副词
static const POS_TAG_TYPE POS_TAG_QR = 168; //容器量词
static const POS_TAG_TYPE POS_TAG_QT = 162; //时量词
static const POS_TAG_TYPE POS_TAG_QV = 171; //动量词
static const POS_TAG_TYPE POS_TAG_H = 176; //前接成分
static const POS_TAG_TYPE POS_TAG_QZ = 12; //种类量词
static const POS_TAG_TYPE POS_TAG_L = 118; //习用语
static const POS_TAG_TYPE POS_TAG_P = 148; //介词
static const POS_TAG_TYPE POS_TAG_QC = 173; //成形量词
static const POS_TAG_TYPE POS_TAG_QB = 156; //不定量词
static const POS_TAG_TYPE POS_TAG_QE = 151; //个体量词
static const POS_TAG_TYPE POS_TAG_QD = 139; //度量词
static const POS_TAG_TYPE POS_TAG_QG = 177; //量语素
static const POS_TAG_TYPE POS_TAG_QJ = 13; //集体量词
static const POS_TAG_TYPE POS_TAG_QL = 14; //倍率量词
static const POS_TAG_TYPE POS_TAG_EG = 166; //叹语素
static const POS_TAG_TYPE POS_TAG_ZG = 175; //状态词语素
static const POS_TAG_TYPE POS_TAG_WD = 15; //逗号
static const POS_TAG_TYPE POS_TAG_RR = 152; //人称代词
static const POS_TAG_TYPE POS_TAG_RY = 157; //疑问代词
static const POS_TAG_TYPE POS_TAG_RG = 170; //代语素
static const POS_TAG_TYPE POS_TAG_MQ = 117; //数量词
static const POS_TAG_TYPE POS_TAG_WM = 16; //冒号
static const POS_TAG_TYPE POS_TAG_T = 101; //时间词
static const POS_TAG_TYPE POS_TAG_WF = 17; //分号
static const POS_TAG_TYPE POS_TAG_VX = 154; //形式动词
static const POS_TAG_TYPE POS_TAG_BG = 172; //区别语素
static const POS_TAG_TYPE POS_TAG_JV = 143; //动词功能简称
static const POS_TAG_TYPE POS_TAG_WJ = 18; //句号
static const POS_TAG_TYPE POS_TAG_WW = 19; //问号
static const POS_TAG_TYPE POS_TAG_JN = 123; //名词功能简称
static const POS_TAG_TYPE POS_TAG_WT = 20; //叹号
static const POS_TAG_TYPE POS_TAG_WS = 21; //省略号
static const POS_TAG_TYPE POS_TAG_WP = 22; //破折号
static const POS_TAG_TYPE POS_TAG_JD = 23; //副词功能简称
static const POS_TAG_TYPE POS_TAG_JB = 141; //区别词功能简称
static const POS_TAG_TYPE POS_TAG_C = 132; //连词
static const POS_TAG_TYPE POS_TAG_F = 135; //方位词
static const POS_TAG_TYPE POS_TAG_K = 159; //后接成分
static const POS_TAG_TYPE POS_TAG_WU = 24; //顿号
static const POS_TAG_TYPE POS_TAG_O = 137; //拟声词
static const POS_TAG_TYPE POS_TAG_S = 115; //处所词
static const POS_TAG_TYPE POS_TAG_W = 25; //标点符号
static const POS_TAG_TYPE POS_TAG_NRG = 128; //名
static const POS_TAG_TYPE POS_TAG_NRF = 136; //姓
static const POS_TAG_TYPE POS_TAG_IV = 120; //动词功能成语
static const POS_TAG_TYPE POS_TAG_MG = 167; //数语素
static const POS_TAG_TYPE POS_TAG_B = 107; //区别词
static const POS_TAG_TYPE POS_TAG_UZ = 26; //助词“着”
static const POS_TAG_TYPE POS_TAG_J = 104; //简称略语
static const POS_TAG_TYPE POS_TAG_US = 27; //助词“所”
static const POS_TAG_TYPE POS_TAG_N = 100; //名词
static const POS_TAG_TYPE POS_TAG_UL = 28; //助词“了”
static const POS_TAG_TYPE POS_TAG_UO = 29; //助词“过”
static const POS_TAG_TYPE POS_TAG_R = 126; //代词
static const POS_TAG_TYPE POS_TAG_UI = 30; //助词“地”
static const POS_TAG_TYPE POS_TAG_V = 103; //动词
static const POS_TAG_TYPE POS_TAG_UE = 31; //助词“得”
static const POS_TAG_TYPE POS_TAG_UD = 174; //助词“的”
static const POS_TAG_TYPE POS_TAG_UG = 169; //助语素
static const POS_TAG_TYPE POS_TAG_Z = 106; //状态词
static const POS_TAG_TYPE POS_TAG_VD = 33; //副动词
static const POS_TAG_TYPE POS_TAG_AD = 164; //副形词
static const POS_TAG_TYPE POS_TAG_AG = 138; //形语素
static const POS_TAG_TYPE POS_TAG_VG = 150; //动语素
static const POS_TAG_TYPE POS_TAG_VI = 131; //不及物动词
static const POS_TAG_TYPE POS_TAG_VL = 146; //联系动词
static const POS_TAG_TYPE POS_TAG_VN = 127; //名动词
static const POS_TAG_TYPE POS_TAG_AN = 163; //名形词
static const POS_TAG_TYPE POS_TAG_WKY = 34; //右括号
static const POS_TAG_TYPE POS_TAG_VQ = 35; //趋向动词
static const POS_TAG_TYPE POS_TAG_WKZ = 36; //左括号
static const POS_TAG_TYPE POS_TAG_VU = 165; //助动词
static const POS_TAG_TYPE POS_TAG_IN = 121; //名词功能成语
static const POS_TAG_TYPE POS_TAG_IA = 122; //形容词功能成语
static const POS_TAG_TYPE POS_TAG_IB = 129; //区别词功能成语
static const POS_TAG_TYPE POS_TAG_ID = 125; //副词功能成语
static const POS_TAG_TYPE POS_TAG_NG = 111; //名语素
static const POS_TAG_TYPE POS_TAG_NX = 112; //非汉字串
static const POS_TAG_TYPE POS_TAG_NZ = 109; //其他专名
static const POS_TAG_TYPE POS_TAG_RYW = 160; //谓词性疑问代词
static const POS_TAG_TYPE POS_TAG_NR = 108; //人名
static const POS_TAG_TYPE POS_TAG_NS = 113; //地名
static const POS_TAG_TYPE POS_TAG_NT = 140; //机构团体
static const POS_TAG_TYPE POS_TAG_A = 110; //形容词
static const POS_TAG_TYPE POS_TAG_E = 153; //叹词
static const POS_TAG_TYPE POS_TAG_I = 119; //成语
static const POS_TAG_TYPE POS_TAG_M = 102; //数词
static const POS_TAG_TYPE POS_TAG_WYZ = 37; //左引号
static const POS_TAG_TYPE POS_TAG_WYY = 38; //右引号
static const POS_TAG_TYPE POS_TAG_Q = 114; //量词
static const POS_TAG_TYPE POS_TAG_U = 149; //助词
static const POS_TAG_TYPE POS_TAG_Y = 147; //语气词

/*
 *  definition of semantic tags
 */
typedef uint16_t SEMANTIC_TAG_TYPE;

static const SEMANTIC_TAG_TYPE SEM_TAG_DEF = 0; //未知标签
static const SEMANTIC_TAG_TYPE SEM_TAG_XXC = 192; //序数词
static const SEMANTIC_TAG_TYPE SEM_TAG_SZ = 109; //数字词
static const SEMANTIC_TAG_TYPE SEM_TAG_RQ = 101; //日期词
static const SEMANTIC_TAG_TYPE SEM_TAG_SJ = 102; //时间词
static const SEMANTIC_TAG_TYPE SEM_TAG_BQFH = 125; //表情符号
static const SEMANTIC_TAG_TYPE SEM_TAG_BDFH = 193; //标点符号
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYSY_FG = 158; //专业术语-法规类
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_ZYC = 165; //专有名词-中药材
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_YQ = 177; //专有名词-乐器
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_TY = 176; //专有名词-体育
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_DW = 164; //专有名词-动物
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_QG = 180; //专有名词-器官
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_TW = 140; //专有名词-天文
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_ZW = 151; //专有名词-植物
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_WQ = 136; //专有名词-武器
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_MZ = 166; //专有名词-民族
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_QX = 173; //专有名词-气象
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_JB = 175; //专有名词-疾病
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_JR = 168; //专有名词-节假日
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_YP = 156; //专有名词-药品
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_CY = 171; //专有名词-菜肴
static const SEMANTIC_TAG_TYPE SEM_TAG_ZYMC_YY = 184; //专有名词-语言
static const SEMANTIC_TAG_TYPE SEM_TAG_CP_PP = 105; //产品-品牌
static const SEMANTIC_TAG_TYPE SEM_TAG_XSC = 132; //产品类型修饰词
static const SEMANTIC_TAG_TYPE SEM_TAG_CPLX_FH = 142; //产品类型-复合
static const SEMANTIC_TAG_TYPE SEM_TAG_SXC = 130; //产品类型属性词
static const SEMANTIC_TAG_TYPE SEM_TAG_CPLX_JD = 106; //产品类型-简单
static const SEMANTIC_TAG_TYPE SEM_TAG_CPLX_TC = 128; //产品类型-统称
static const SEMANTIC_TAG_TYPE SEM_TAG_PER_YDNHZ = 120; //人名-不带后缀乐队组合
static const SEMANTIC_TAG_TYPE SEM_TAG_PER_CHN = 153; //人名-中国人名
static const SEMANTIC_TAG_TYPE SEM_TAG_PER_YDHZ = 187; //人名-带后缀乐队组合
static const SEMANTIC_TAG_TYPE SEM_TAG_PER_JPN = 152; //人名-日本人名
static const SEMANTIC_TAG_TYPE SEM_TAG_PER_OM = 137; //人名-欧美人名
static const SEMANTIC_TAG_TYPE SEM_TAG_PER_EN = 127; //人名-英文人名
static const SEMANTIC_TAG_TYPE SEM_TAG_TY_FHWHZ = 183; //体育-复合无后缀球队
static const SEMANTIC_TAG_TYPE SEM_TAG_TY_FHYHZ = 188; //体育-复合有后缀球队
static const SEMANTIC_TAG_TYPE SEM_TAG_TY_DLWHZ = 124; //体育-独立无后缀球队
static const SEMANTIC_TAG_TYPE SEM_TAG_TY_DLYHZ = 178; //体育-独立有后缀球队
static const SEMANTIC_TAG_TYPE SEM_TAG_TY_CD = 179; //体育-车队
static const SEMANTIC_TAG_TYPE SEM_TAG_CXC = 182; //促销词
static const SEMANTIC_TAG_TYPE SEM_TAG_QH_GD = 181; //区划-国家及地区
static const SEMANTIC_TAG_TYPE SEM_TAG_QH_WHZ = 155; //区划-无后缀区划
static const SEMANTIC_TAG_TYPE SEM_TAG_QH_YBH = 161; //区划-有不可去后缀区划
static const SEMANTIC_TAG_TYPE SEM_TAG_QH_YKH = 154; //区划-有可去后缀区划
static const SEMANTIC_TAG_TYPE SEM_TAG_QH_FZ = 186; //区划-泛指区划
static const SEMANTIC_TAG_TYPE SEM_TAG_CH_YL = 162; //吃喝玩乐-娱乐休闲
static const SEMANTIC_TAG_TYPE SEM_TAG_CH_FW = 167; //吃喝玩乐-服务场所
static const SEMANTIC_TAG_TYPE SEM_TAG_JB_ZW = 144; //基本词-中文
static const SEMANTIC_TAG_TYPE SEM_TAG_JB_ZWCZ = 145; //基本词-中文词组
static const SEMANTIC_TAG_TYPE SEM_TAG_JB_ZY = 104; //基本词-中英混合
static const SEMANTIC_TAG_TYPE SEM_TAG_JB_ZYCZ = 103; //基本词-中英混合词组
static const SEMANTIC_TAG_TYPE SEM_TAG_JB_YW = 123; //基本词-字母词
static const SEMANTIC_TAG_TYPE SEM_TAG_FDC_SY = 163; //房地产-商用
static const SEMANTIC_TAG_TYPE SEM_TAG_FDC_MY = 147; //房地产-民用
static const SEMANTIC_TAG_TYPE SEM_TAG_WT_FLASH = 117; //文体娱乐类-flash作品
static const SEMANTIC_TAG_TYPE SEM_TAG_WT_SW = 107; //文体娱乐类-书文课程类
static const SEMANTIC_TAG_TYPE SEM_TAG_WT_YS = 100; //文体娱乐类-影视类
static const SEMANTIC_TAG_TYPE SEM_TAG_WT_GQ = 108; //文体娱乐类-戏剧歌曲类
static const SEMANTIC_TAG_TYPE SEM_TAG_WT_BZ = 116; //文体娱乐类-报纸杂志类
static const SEMANTIC_TAG_TYPE SEM_TAG_WT_YX = 110; //文体娱乐类-游戏类
static const SEMANTIC_TAG_TYPE SEM_TAG_JG_WQH = 185; //机构-无区划机构名
static const SEMANTIC_TAG_TYPE SEM_TAG_JG_NTZ = 172; //机构-无特指机构名
static const SEMANTIC_TAG_TYPE SEM_TAG_JG_YTZ = 190; //机构-有特指机构名
static const SEMANTIC_TAG_TYPE SEM_TAG_JG_BC = 174; //机构-机构半称
static const SEMANTIC_TAG_TYPE SEM_TAG_JG_TZ = 115; //机构-机构特指
static const SEMANTIC_TAG_TYPE SEM_TAG_ZL_SS = 131; //杂类-赛事
static const SEMANTIC_TAG_TYPE SEM_TAG_SY_GYC = 160; //熟语-古语词
static const SEMANTIC_TAG_TYPE SEM_TAG_SY_GYY = 149; //熟语-惯用语
static const SEMANTIC_TAG_TYPE SEM_TAG_SY_CY = 146; //熟语-成语
static const SEMANTIC_TAG_TYPE SEM_TAG_TSSP_SC = 133; //特殊商品-收藏
static const SEMANTIC_TAG_TYPE SEM_TAG_TSSP_FW = 134; //特殊商品-服务
static const SEMANTIC_TAG_TYPE SEM_TAG_TSSP_XN = 143; //特殊商品-虚拟
static const SEMANTIC_TAG_TYPE SEM_TAG_SL_CHN = 150; //缩略-中文缩略
static const SEMANTIC_TAG_TYPE SEM_TAG_SL_CHNQC = 169; //缩略-中文缩略全称
static const SEMANTIC_TAG_TYPE SEM_TAG_SL_ENG = 129; //缩略-英文缩略
static const SEMANTIC_TAG_TYPE SEM_TAG_SL_ENGCHN = 148; //缩略-英文缩略的中文全称
static const SEMANTIC_TAG_TYPE SEM_TAG_WZ_BHZ = 112; //网站-不带网站后缀
static const SEMANTIC_TAG_TYPE SEM_TAG_WZ_QT = 119; //网站-其他
static const SEMANTIC_TAG_TYPE SEM_TAG_WZ_YM = 126; //网站-域名类
static const SEMANTIC_TAG_TYPE SEM_TAG_WZ_FH = 141; //网站-复合
static const SEMANTIC_TAG_TYPE SEM_TAG_WZ_DHZ = 111; //网站-带网站后缀
static const SEMANTIC_TAG_TYPE SEM_TAG_WZ_JG = 135; //网站-机构名
static const SEMANTIC_TAG_TYPE SEM_TAG_WZ_PD = 139; //网站-频道名
static const SEMANTIC_TAG_TYPE SEM_TAG_SQ_ZW = 159; //色情词汇-中文
static const SEMANTIC_TAG_TYPE SEM_TAG_SQ_ZWCZ = 191; //色情词汇-中文词组
static const SEMANTIC_TAG_TYPE SEM_TAG_SQ_ZY = 114; //色情词汇-中英混合
static const SEMANTIC_TAG_TYPE SEM_TAG_SQ_ZYCZ = 113; //色情词汇-中英混合词组
static const SEMANTIC_TAG_TYPE SEM_TAG_CJ_JJMC = 170; //财经-基金名称
static const SEMANTIC_TAG_TYPE SEM_TAG_CJ_CP = 118; //财经-彩票
static const SEMANTIC_TAG_TYPE SEM_TAG_CJ_GPMC = 138; //财经-股票名称
static const SEMANTIC_TAG_TYPE SEM_TAG_RJ_CH = 121; //软件-纯软件名
static const SEMANTIC_TAG_TYPE SEM_TAG_CW = 189; //错误词
static const SEMANTIC_TAG_TYPE SEM_TAG_DM_QT = 157; //非区划地名-其它
static const SEMANTIC_TAG_TYPE SEM_TAG_DM_JD = 122; //非区划地名-景点

#ifndef RELEASE_C
#ifdef __cplusplus
} // end of namespace tokenizer
} // end of namespace weibo
#endif
#endif

#endif // WEIBO_TOKENIZER_WEIBO_TOKENIZER_DEFINE_H
