#ifndef __CHAT_LLM_H
#define __CHAT_LLM_H

#include "klist.h"
#include "kvec.h"
#include "khash.h"
#include <json-c/json.h>

/*
There are 2048 tokens available, around 270 are used for the initial data for the stall prompt
We give at most 400 for the examples and 1300 for the stall prompt
Similarly 1700 is for the example request in the seed enrichment
*/

#define ZHIPU_TOKEN "your_api_token_here"  // 智谱AI的API密钥，请替换为您的实际令牌

#define MAX_PROMPT_LENGTH 2048
#define EXAMPLES_PROMPT_LENGTH 400
#define HISTORY_PROMPT_LENGTH 1300
#define EXAMPLE_SEQUENCE_PROMPT_LENGTH 1700

#define TEMPLATE_CONSISTENCY_COUNT 5

// Maximum amount of retries for the state stall
#define STALL_RETRIES 2

// Maximum amount of tries to get the grammars
#define GRAMMAR_RETRIES 5

// Maximum amount
#define MESSAGE_TYPE_RETRIES 5

//Maximum amount of tries for an enrichment
#define ENRICHMENT_RETRIES 10

// Maximum number of messages to be added
#define MAX_ENRICHMENT_MESSAGE_TYPES 2

// Maximum number of messages to examine for addition
#define MAX_ENRICHMENT_CORPUS_SIZE 10

// Maximum concurrent API calls to Zhipu AI
#define MAX_ZHIPU_CONCURRENT_CALLS 1

// Delay between API calls when rate limited (in seconds)
#define ZHIPU_RATE_LIMIT_DELAY 5

#define PCRE2_CODE_UNIT_WIDTH 8 // Characters are 8 bits
#include <pcre2.h>

// Init KLIST with JSON object
#define __grammar_t_free(x)
#define __rang_t_free(x)
#define __khash_t_free(x) 
KHASH_SET_INIT_STR(strSet);
KLIST_INIT(gram, json_object *, __grammar_t_free)
KLIST_INIT(rang, pcre2_code **, __rang_t_free)
typedef struct
{
    int start;
    int len;
    int mutable;
} range;

typedef kvec_t(range) range_list;
typedef kvec_t(khash_t(strSet)*) message_set_list;

// define one map to save pairs: {key: string, value: int}
KHASH_MAP_INIT_STR(strMap, int)
KHASH_MAP_INIT_STR(field_table, int);
KHASH_INIT(consistency_table, const char *, khash_t(field_table) *, 1, kh_str_hash_func, kh_str_hash_equal);

char *chat_with_llm(char *prompt, char *model, int tries, float temperature);
char *construct_prompt_for_templates(char *protocol_name, char **final_msg);
char *construct_prompt_for_remaining_templates(char *protocol_name, char *templates_prompt, char *templates_answer);
char *construct_prompt_for_protocol_message_types(char *protocol_name);
char *construct_prompt_for_requests_to_states(const char *protocol_name, const char *protocol_state, const char *example_requests);
char *construct_prompt_stall(char *protocol_name, char *examples, char *history);

void extract_message_grammars(char *answers, klist_t(gram) * grammar_set);
char *extract_message_pattern(const char *header_str,
                               khash_t(field_table) * field_table,
                               pcre2_code **patterns,
                               int debug_fd,
                               const char *debug_file_name);
char *extract_stalled_message(char *message, size_t message_len);
char *format_request_message(char *message);


range_list starts_with(char *line, int length, pcre2_code *pattern);
range_list get_mutable_ranges(char *line, int length, int offset, pcre2_code *pattern);
void get_protocol_message_types(char *state_prompt, khash_t(strSet) * message_types);

char *enrich_sequence(char* sequence, khash_t(strSet) *missing_message_types);
khash_t(strSet)* duplicate_hash(khash_t(strSet)* set);
void write_new_seeds(char *enriched_file, char *contents);
char *unescape_string(const char *input);
char *format_string(char *state_string);
message_set_list message_combinations(khash_t(strSet)* sequence, int size);


/**
 * 创建一个安全的JSON字符串，确保所有特殊字符都被正确转义
 * @param input 输入字符串
 * @return 转义后的JSON字符串（需要调用者释放内存）
 */
char* create_safe_json_string(const char* input);

/**
 * 构建智谱AI API请求的JSON对象
 * @param model 模型名称
 * @param prompt 提示词（应为有效的JSON数组字符串）
 * @param max_tokens 最大令牌数
 * @param temperature 温度参数
 * @return JSON对象（需要调用者使用json_object_put释放）
 */
json_object* build_zhipu_request_json(const char* model, const char* prompt, int max_tokens, float temperature);

/**
 * 构建智谱AI API请求的JSON字符串
 * @param model 模型名称
 * @param prompt 提示词（应为有效的JSON数组字符串）
 * @param max_tokens 最大令牌数
 * @param temperature 温度参数
 * @return JSON字符串（需要调用者释放内存）
 */
char* build_zhipu_request_string(const char* model, const char* prompt, int max_tokens, float temperature);

/**
 * 构建消息数组JSON对象
 * @param system_msg 系统消息
 * @param user_msg 用户消息
 * @return 消息数组JSON对象（需要调用者使用json_object_put释放）
 */
json_object* build_message_array(const char* system_msg, const char* user_msg);

/**
 * 构建多轮对话的消息数组JSON对象
 * @param system_msg 系统消息
 * @param first_user_msg 第一个用户消息
 * @param first_assistant_msg 第一个助手消息
 * @param second_user_msg 第二个用户消息
 * @return 消息数组JSON对象（需要调用者使用json_object_put释放）
 */
json_object* build_conversation_array(const char* system_msg,
                                     const char* first_user_msg,
                                     const char* first_assistant_msg,
                                     const char* second_user_msg);

/**
 * 安全地转义字符串，用于构建JSON
 * @param input 输入字符串
 * @return 转义后的字符串（需要调用者释放内存）
 */
char* safe_escape_for_json(const char* input);

#endif // __CHAT_LLM_H
