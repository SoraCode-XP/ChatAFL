
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

/**
 * 创建一个安全的JSON字符串，确保所有特殊字符都被正确转义
 */
char* create_safe_json_string(const char* input) {
    if (!input) {
        return json_object_get_string(json_object_new_string(""));
    }

    // 使用json-c库的内置转义功能
    json_object* jobj = json_object_new_string(input);
    const char* json_str = json_object_get_string(jobj);

    // 创建一个新的副本，因为json_object_put会释放内存
    char* result = strdup(json_str);
    json_object_put(jobj);

    return result;
}

/**
 * 构建智谱AI API请求的JSON对象
 */
json_object* build_zhipu_request_json(const char* model, const char* prompt, int max_tokens, float temperature) {
    // 创建根对象
    json_object* request_obj = json_object_new_object();

    // 添加model字段
    json_object_object_add(request_obj, "model", json_object_new_string(model));

    // 添加messages字段 - 这里假设prompt已经是有效的JSON数组字符串
    json_object* messages_obj = json_tokener_parse(prompt);
    if (!messages_obj) {
        // 如果解析失败，创建一个简单的messages数组
        messages_obj = json_object_new_array();
        json_object* message_obj = json_object_new_object();
        json_object_object_add(message_obj, "role", json_object_new_string("user"));
        json_object_object_add(message_obj, "content", json_object_new_string(prompt));
        json_object_array_add(messages_obj, message_obj);
    }
    json_object_object_add(request_obj, "messages", messages_obj);

    // 添加max_tokens字段
    json_object_object_add(request_obj, "max_tokens", json_object_new_int(max_tokens));

    // 添加temperature字段
    json_object_object_add(request_obj, "temperature", json_object_new_double(temperature));

    return request_obj;
}

/**
 * 构建智谱AI API请求的JSON字符串
 */
char* build_zhipu_request_string(const char* model, const char* prompt, int max_tokens, float temperature) {
    json_object* request_obj = build_zhipu_request_json(model, prompt, max_tokens, temperature);
    const char* json_str = json_object_to_json_string(request_obj);

    // 创建一个新的副本
    char* result = strdup(json_str);
    json_object_put(request_obj);

    return result;
}

/**
 * 构建消息数组JSON对象
 */
json_object* build_message_array(const char* system_msg, const char* user_msg) {
    json_object* messages_array = json_object_new_array();

    // 添加系统消息
    if (system_msg) {
        json_object* sys_msg_obj = json_object_new_object();
        json_object_object_add(sys_msg_obj, "role", json_object_new_string("system"));
        json_object_object_add(sys_msg_obj, "content", json_object_new_string(system_msg));
        json_object_array_add(messages_array, sys_msg_obj);
    }

    // 添加用户消息
    if (user_msg) {
        json_object* user_msg_obj = json_object_new_object();
        json_object_object_add(user_msg_obj, "role", json_object_new_string("user"));
        json_object_object_add(user_msg_obj, "content", json_object_new_string(user_msg));
        json_object_array_add(messages_array, user_msg_obj);
    }

    return messages_array;
}

/**
 * 构建多轮对话的消息数组JSON对象
 */
json_object* build_conversation_array(const char* system_msg, 
                                     const char* first_user_msg, 
                                     const char* first_assistant_msg, 
                                     const char* second_user_msg) {
    json_object* messages_array = json_object_new_array();

    // 添加系统消息
    if (system_msg) {
        json_object* sys_msg_obj = json_object_new_object();
        json_object_object_add(sys_msg_obj, "role", json_object_new_string("system"));
        json_object_object_add(sys_msg_obj, "content", json_object_new_string(system_msg));
        json_object_array_add(messages_array, sys_msg_obj);
    }

    // 添加第一个用户消息
    if (first_user_msg) {
        json_object* user1_msg_obj = json_object_new_object();
        json_object_object_add(user1_msg_obj, "role", json_object_new_string("user"));
        json_object_object_add(user1_msg_obj, "content", json_object_new_string(first_user_msg));
        json_object_array_add(messages_array, user1_msg_obj);
    }

    // 添加第一个助手消息
    if (first_assistant_msg) {
        json_object* assistant1_msg_obj = json_object_new_object();
        json_object_object_add(assistant1_msg_obj, "role", json_object_new_string("assistant"));
        json_object_object_add(assistant1_msg_obj, "content", json_object_new_string(first_assistant_msg));
        json_object_array_add(messages_array, assistant1_msg_obj);
    }

    // 添加第二个用户消息
    if (second_user_msg) {
        json_object* user2_msg_obj = json_object_new_object();
        json_object_object_add(user2_msg_obj, "role", json_object_new_string("user"));
        json_object_object_add(user2_msg_obj, "content", json_object_new_string(second_user_msg));
        json_object_array_add(messages_array, user2_msg_obj);
    }

    return messages_array;
}

/**
 * 安全地转义字符串，用于构建JSON
 */
char* safe_escape_for_json(const char* input) {
    if (!input) return strdup("");

    // 使用json_object_new_string和json_object_get_string来确保正确的转义
    json_object* jobj = json_object_new_string(input);
    const char* escaped = json_object_get_string(jobj);

    // 创建副本
    char* result = strdup(escaped);
    json_object_put(jobj);

    return result;
}
