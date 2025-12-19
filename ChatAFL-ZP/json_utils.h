
#ifndef __JSON_UTILS_H
#define __JSON_UTILS_H

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

#endif // __JSON_UTILS_H
