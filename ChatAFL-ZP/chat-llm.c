
#define _GNU_SOURCE // asprintf
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>



#include "chat-llm.h"
#include "alloc-inl.h"
#include "hash.h"

// -lcurl -ljson-c -lpcre2-8
// apt install libcurl4-openssl-dev libjson-c-dev libpcre2-dev libpcre2-8-0

#define MAX_TOKENS 2048
#define CONFIDENT_TIMES 3

struct MemoryStruct
{
    char *memory;
    size_t size;
};

static size_t chat_with_llm_helper(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char *chat_with_llm(char *prompt, char *model, int tries, float temperature)
{
    CURL *curl;
    CURLcode res = CURLE_OK;
    char *answer = NULL;
    char *url = NULL;

    // 添加日志：记录大模型调用开始
    ACTF("Calling LLM model: %s with temperature: %.1f", model, temperature);
    ACTF("Prompt length: %d characters", (int)strlen(prompt));

    // 统一使用智谱AI的API
    url = "https://open.bigmodel.cn/api/paas/v4/chat/completions";
    char *auth_header;
    // 使用头文件中定义的ZHIPU_TOKEN宏
    const char *zhipu_token = ZHIPU_TOKEN;
    if (strlen(zhipu_token) == 0) {
        printf("Error: ZHIPU_TOKEN is not set in chat-llm.h\n");
        return NULL;
    }
    asprintf(&auth_header, "Authorization: Bearer %s", zhipu_token);
    char *content_header = "Content-Type: application/json";
    char *accept_header = "Accept: application/json";
    char *user_header = "User-Agent: ChatAFL/1.0";
    char *charset_header = "charset: utf-8";

    // 使用优化的JSON构建函数
    char *data = build_zhipu_request_string("glm-4.5-flash", prompt, MAX_TOKENS, temperature);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    do
    {
        struct MemoryStruct chunk;

        chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
        chunk.size = 0;           /* no data at this point */

        curl = curl_easy_init();
        if (curl)
        {
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, auth_header);
            headers = curl_slist_append(headers, content_header);
            headers = curl_slist_append(headers, accept_header);
            headers = curl_slist_append(headers, user_header);
            headers = curl_slist_append(headers, charset_header);

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, chat_with_llm_helper);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                // 检查响应是否为空
                if (chunk.size == 0 || !chunk.memory || chunk.memory[0] == '\0') {
                    printf("警告: 智谱AI返回了空响应\n");
                    answer = strdup("错误: 智谱AI返回了空响应");
                    continue;
                }

                // 尝试解析JSON
                json_object *jobj = json_tokener_parse(chunk.memory);
                if (!jobj) {
                    // 如果直接解析失败，尝试提取JSON部分
                    char *json_start = strchr(chunk.memory, '{');
                    if (json_start) {
                        jobj = json_tokener_parse(json_start);
                    }

                    if (!jobj) {
                        printf("错误: 无法解析智谱AI响应为JSON: %s\n", chunk.memory);
                        continue;
                    }
                }

                // 打印原始响应以便调试
                printf("智谱AI原始响应: %s\n", chunk.memory);

                // 检查智谱AI API的错误响应
                json_object *error_obj = NULL;
                if (json_object_object_get_ex(jobj, "error", &error_obj))
                {
                    json_object *message_obj = json_object_object_get(error_obj, "message");
                    const char *error_msg = json_object_get_string(message_obj);
                    printf("智谱AI API错误: %s\n", error_msg ? error_msg : "未知错误");
                    sleep(2); // 等待一段时间以便服务恢复
                }
                // 检查"choices"键是否存在
                json_object *choices = NULL;
                if (json_object_object_get_ex(jobj, "choices", &choices))
                {
                    // 确保choices是数组类型
                    if (json_object_get_type(choices) != json_type_array) {
                        printf("智谱AI API响应格式错误: choices不是数组类型，当前类型: %s\n",
                               json_type_to_name(json_object_get_type(choices)));

                        // 尝试将其他类型转换为数组
                        if (json_object_get_type(choices) == json_type_string) {
                            const char *str = json_object_get_string(choices);
                            printf("尝试将字符串转换为数组: %s\n", str);
                            // 这里可以添加字符串解析逻辑
                        }

                        // 设置默认响应并继续
                        answer = strdup("错误: 智谱AI API返回格式不正确");
                    } else if (json_object_array_length(choices) > 0)
                    {
                        json_object *first_choice = json_object_array_get_idx(choices, 0);
                        const char *data;

                        // 使用智谱AI的响应格式
                        json_object *jobj4 = json_object_object_get(first_choice, "message");
                        if (jobj4 && json_object_object_get_ex(jobj4, "content", NULL))
                        {
                            json_object *jobj5 = json_object_object_get(jobj4, "content");
                            data = json_object_get_string(jobj5);
                            if (data && data[0] == '\n')
                                data++;
                            if (data && strlen(data) > 0) {
                                answer = strdup(data);
                            } else {
                                printf("警告: 智谱AI返回了空响应\n");
                                // 设置一个默认错误响应
                                answer = strdup("错误: 智谱AI返回了空响应");
                            }

                            // 添加日志：记录大模型调用结果
                            if (strlen(answer) > 0) {
                                ACTF("LLM call successful. Response length: %d characters", (int)strlen(answer));
                            } else {
                                ACTF("LLM call returned empty response");
                            }
                        }
                        else
                        {
                            printf("智谱AI API响应格式错误: 缺少content字段\n");
                        }
                    }
                    else
                    {
                        printf("智谱AI API响应格式错误: choices数组为空\n");
                    }
                }
                else
                {
                    printf("智谱AI API未知响应格式: %s\n", chunk.memory);
                    // 添加调试信息，打印响应类型
                    json_object *obj_type;
                    if (json_object_object_get_ex(jobj, "choices", &obj_type)) {
                        printf("choices类型: %s\n", json_type_to_name(json_object_get_type(obj_type)));
                    }
                    sleep(2); // 等待一段时间以便服务恢复
                }
                json_object_put(jobj);
            }
            else
            {
                printf("Error: %s\n", curl_easy_strerror(res));
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        free(chunk.memory);
    } while ((res != CURLE_OK || answer == NULL) && (--tries > 0));

    if (data != NULL)
    {
        free(data);
    }

    curl_global_cleanup();
    return answer;
}

char *construct_prompt_stall(char *protocol_name, char *examples, char *history)
{
    char *template = "In the %s protocol, the communication history between the %s client and the %s server is as follows."
                     "The next proper client request that can affect the server's state are:\n\n"
                     "Desired format of real client requests:\n%sCommunication History:\n\"\"\"\n%s\"\"\"";

    char *prompt = NULL;
    asprintf(&prompt, template, protocol_name, protocol_name, protocol_name, examples, history);

    // 使用优化的JSON构建函数
    json_object *messages_array = build_message_array("You are a helpful assistant.", prompt);
    const char *messages_json = json_object_to_json_string(messages_array);
    char *final_prompt = strdup(messages_json);

    free(prompt);
    json_object_put(messages_array);

    return final_prompt;
}

char *construct_prompt_for_templates(char *protocol_name, char **final_msg)
{
    // 使用安全的字符串转义
    char *prompt_rtsp_example = safe_escape_for_json(
        "For the RTSP protocol, the DESCRIBE client request template is:\n"
        "DESCRIBE: [\"DESCRIBE <<VALUE>>\\r\\n\","
        "\"CSeq: <<VALUE>>\\r\\n\","
        "\"User-Agent: <<VALUE>>\\r\\n\","
        "\"Accept: <<VALUE>>\\r\\n\","
        "\"\\r\\n\"]"
    );

    char *prompt_http_example = safe_escape_for_json(
        "For the HTTP protocol, the GET client request template is:\n"
        "GET: [\"GET <<VALUE>>\\r\\n\"]"
    );

    char *msg = NULL;
    asprintf(&msg, "%s\n%s\nFor the %s protocol, all of client request templates are :", 
             prompt_rtsp_example, prompt_http_example, protocol_name);
    *final_msg = msg;

    // 使用优化的JSON构建函数
    json_object *messages_array = build_message_array("You are a helpful assistant.", msg);
    const char *messages_json = json_object_to_json_string(messages_array);
    char *prompt_grammars = strdup(messages_json);

    free(prompt_rtsp_example);
    free(prompt_http_example);
    json_object_put(messages_array);

    return prompt_grammars;
}

char *construct_prompt_for_remaining_templates(char *protocol_name, char *first_question, char *first_answer)
{
    char *second_question = NULL;
    asprintf(&second_question, "For the %s protocol, other templates of client requests are:", protocol_name);

    // 使用优化的JSON构建函数
    json_object *messages_array = build_conversation_array(
        "You are a helpful assistant.", 
        first_question, 
        first_answer, 
        second_question
    );
    const char *messages_json = json_object_to_json_string(messages_array);
    char *prompt = strdup(messages_json);

    free(second_question);
    json_object_put(messages_array);

    return prompt;
}

char *extract_stalled_message(char *message, size_t message_len)
{

    int errornumber;
    size_t erroroffset;
    // After a lot of iterations, the model consistently responds with an empty line and then a line of text
    pcre2_code *extracter = pcre2_compile("\r?\n?.*?\r?\n", PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL);
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(extracter, NULL);
    int rc = pcre2_match(extracter, message, message_len, 0, 0, match_data, NULL);
    char *res = NULL;
    if (rc >= 0)
    {
        size_t *ovector = pcre2_get_ovector_pointer(match_data);
        res = strdup(message + ovector[1]);
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(extracter);

    return res;
}

char *format_request_message(char *message)
{

    int message_len = strlen(message);
    int max_len = message_len;
    int res_len = 0;
    char *res = ck_alloc(message_len * sizeof(char));
    for (int i = 0; i < message_len; i++)
    {
        // If an \n is not padded with an \r before, we add it
        if (message[i] == '\n' && (i == 0 || (message[i - 1] != '\r')))
        {
            if (res_len == max_len)
            {
                res = ck_realloc(res, max_len + 10);
                max_len += 10;
            }
            res[res_len++] = '\r';
        }

        if (res_len == max_len)
        {
            res = ck_realloc(res, max_len + 10);
            max_len += 10;
        }
        res[res_len++] = message[i];
    }

    // Add \r\n\r\n to ensure that the packet is accepted
    for (int i = 0; i < 2; i++)
    {
        if (res_len == max_len)
        {
            res = ck_realloc(res, max_len + 10);
            max_len += 10;
        }
        res[res_len++] = '\r';
        if (res_len == max_len)
        {
            res = ck_realloc(res, max_len + 10);
            max_len += 10;
        }
        res[res_len++] = '\n';
    }

    if (res_len == max_len)
    {
        res = ck_realloc(res, max_len + 1);
        max_len++;
    }
    res[res_len++] = '\0';
    free(message);
    return res;
}

char *construct_prompt_for_protocol_message_types(char *protocol_name)
{
    /***
     * Prompt to ask the protocol states as follow:
     * ```
     * In the RTSP protocol, the protocol states are:
     *
     * Desired format:
     * <comma_separated_list_of_states_in_uppercase>
     * ```
     * ***/
    char *prompt = NULL;

    // transfer the prompt into string
    asprintf(&prompt, "In the %s protocol, the message types are: \n\nDesired format:\n<comma_separated_list_of_states_in_uppercase_and_without_whitespaces>", protocol_name);

    return prompt;
}

char *construct_prompt_for_requests_to_states(const char *protocol_name,
                                              const char *protocol_state,
                                              const char *example_requests)
{
    /***
     Prompt to ask the sequence of client requests to reach a protocol state as follows:
        ```
        In the RTSP protocol, if the server just starts, to reach the PLAYING state, the sequence of client requests can be:
        DESCRIBE rtsp://127.0.0.1:8554/aacAudioTest RTSP/1.0
        CSeq: 2
        User-Agent: ./testRTSPClient (LIVE555 Streaming Media v2018.08.28)
        Accept: application/sdp

        SETUP rtsp://127.0.0.1:8554/aacAudioTest/track1 RTSP/1.0
        CSeq: 3
        User-Agent: ./testRTSPClient (LIVE555 Streaming Media v2018.08.28)
        Transport: RTP/AVP;unicast;client_port=38784-38785

        PLAY rtsp://127.0.0.1:8554/aacAudioTest/ RTSP/1.0
        CSeq: 4
        User-Agent: ./testRTSPClient (LIVE555 Streaming Media v2018.08.28)
        Session: 000022B8
        Range: npt=0.000-

        Similarly, in the RTSP protocol, if the server just starts, to reach the RECORD state, the sequence of client requests can be:
     ***/

    // Transfer formats of example_requests
    json_object *example_requests_json = json_object_new_string(example_requests);
    const char *example_requests_json_str = json_object_to_json_string(example_requests_json);

    json_object *protocol_state_json = json_object_new_string(protocol_state);
    const char *protocol_state_json_str = json_object_to_json_string(protocol_state_json);

    char *prompt = NULL;

    int example_request_len = strlen(example_requests_json_str) - 2;
    if (example_request_len > EXAMPLE_SEQUENCE_PROMPT_LENGTH)
    {
        example_request_len = EXAMPLE_SEQUENCE_PROMPT_LENGTH;
    }

    asprintf(&prompt,
             "In the %s protocol, if the server just starts, to reach the INIT state, the sequence of client requests can be:\n"
             "%.*s\nSimilarly, in the %s protocol, if the server just starts, to reach the %.*s state, the sequence of client requests can be:\n",
             protocol_name,
             example_request_len,
             example_requests_json_str + 1,
             protocol_name,
             (int)strlen(protocol_state_json_str) - 2,
             protocol_state_json_str + 1);

    json_object_put(protocol_state_json);
    json_object_put(example_requests_json);

    return prompt;
}

void extract_message_grammars(char *answers, klist_t(gram) * grammar_list)
{

    char *ptr = answers;
    int len = strlen(answers);

    while (ptr < answers + len)
    {
        char *start = strchr(ptr, '[');
        if (start == NULL)
            break;
        char *end = strchr(start, ']');
        if (end == NULL)
            break;
        int count = end - start + 1;
        char *temp = (char *)ck_alloc(count + 1);
        strncpy(temp, start, count);
        temp[count] = '\0';
        ptr = end + 1;

        // conver temp to json object and save it to the list
        json_object *temp_obj = json_tokener_parse(temp);
        if (temp_obj)
        {
            // printf("Extracted grammar: %s\n", json_object_to_json_string(temp_obj));
            *kl_pushp(gram, grammar_list) = temp_obj;
        }
        else
        {
            printf("Failed to parse grammar: %s\n", temp);
        }
        ck_free(temp);
    }
}

char *extract_message_pattern(const char *header_str,
                               khash_t(field_table) * field_table,
                               pcre2_code **patterns,
                               FILE *debug_file,
                               const char *debug_file_name)
{
    char *pattern = NULL;

    // printf("Extracting pattern for header: %s\n", header_str);
    // printf("Field table size: %d\n", kh_size(field_table));

    // Extract the header name
    char *header_name = strdup(header_str);
    char *colon_pos = strchr(header_name, ':');
    if (colon_pos)
        *colon_pos = '\0';

    // Create the header pattern
    asprintf(&pattern, "^(?:%s (.*)\r\n)", header_name);

    // Create the fields pattern
    char *fields_pattern = NULL;
    int fields_pattern_len = 0;
    int fields_pattern_capacity = 100;
    fields_pattern = ck_alloc(fields_pattern_capacity);

    // Iterate over the field table
    khiter_t k;
    for (k = kh_begin(field_table); k != kh_end(field_table); ++k)
    {
        if (!kh_exist(field_table, k))
            continue;

        const char *field_name = kh_key(field_table, k);
        int needed_len = strlen(field_name) + 50; // Add some extra space for the pattern

        if (fields_pattern_len + needed_len > fields_pattern_capacity)
        {
            fields_pattern_capacity += 2 * needed_len;
            fields_pattern = ck_realloc(fields_pattern, fields_pattern_capacity);
        }

        // Add the field pattern
        if (fields_pattern_len > 0)
        {
            memcpy(fields_pattern + fields_pattern_len, "|", 1);
            fields_pattern_len += 1;
        }

        int field_pattern_len = sprintf(fields_pattern + fields_pattern_len, "(?:%s: (.*)\r\n)", field_name);
        fields_pattern_len += field_pattern_len;
    }

    // Add the end of message pattern
    if (fields_pattern_len > 0)
    {
        if (fields_pattern_len + 2 > fields_pattern_capacity)
        {
            fields_pattern_capacity += 2;
            fields_pattern = ck_realloc(fields_pattern, fields_pattern_capacity);
        }

        memcpy(fields_pattern + fields_pattern_len, "|", 1);
        fields_pattern_len += 1;
        memcpy(fields_pattern + fields_pattern_len, "(?:\r\n)", 6);
        fields_pattern_len += 6;
    }
    else
    {
        // If there are no fields, just match the end of message
        if (fields_pattern_len + 6 > fields_pattern_capacity)
        {
            fields_pattern_capacity += 6;
            fields_pattern = ck_realloc(fields_pattern, fields_pattern_capacity);
        }

        memcpy(fields_pattern + fields_pattern_len, "(?:\r\n)", 6);
        fields_pattern_len += 6;
    }

    fields_pattern[fields_pattern_len] = '\0';

    // Compile the patterns
    int errornumber;
    size_t erroroffset;
    patterns[0] = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL);
    patterns[1] = pcre2_compile(fields_pattern, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL);

    // Debug output
    if (debug_file)
    {
        fprintf(debug_file, "Header pattern: %s\n", pattern);
        fprintf(debug_file, "Fields pattern: %s\n", fields_pattern);
        fprintf(debug_file, "Field table: \n");
        for (k = kh_begin(field_table); k != kh_end(field_table); ++k)
        {
            if (!kh_exist(field_table, k))
                continue;

            const char *field_name = kh_key(field_table, k);
            fprintf(debug_file, "  %s\n", field_name);
        }
        fprintf(debug_file, "\n");
    }

    free(header_name);
    free(pattern);
    return fields_pattern;
}

range_list starts_with(char *line, int length, pcre2_code *pattern)
{
    range_list result;
    kv_init(result);

    int errornumber;
    size_t erroroffset;
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(pattern, NULL);
    int rc = pcre2_match(pattern, line, length, 0, 0, match_data, NULL);

    if (rc >= 0)
    {
        size_t *ovector = pcre2_get_ovector_pointer(match_data);
        range r;
        r.start = 0;
        r.len = ovector[1] - ovector[0];
        r.mutable = 0;
        kv_push(range, result, r);
    }

    pcre2_match_data_free(match_data);
    return result;
}

range_list get_mutable_ranges(char *line, int length, int offset, pcre2_code *pattern)
{
    range_list result;
    kv_init(result);

    int errornumber;
    size_t erroroffset;
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(pattern, NULL);
    int rc = pcre2_match(pattern, line, length, 0, 0, match_data, NULL);

    if (rc >= 0)
    {
        size_t *ovector = pcre2_get_ovector_pointer(match_data);
        for (int i = 1; i < rc; i++)
        {
            if (ovector[2 * i] != -1)
            {
                range r;
                r.start = offset + ovector[2 * i];
                r.len = ovector[2 * i + 1] - ovector[2 * i];
                r.mutable = 1;
                kv_push(range, result, r);
            }
        }
    }

    pcre2_match_data_free(match_data);
    return result;
}

void get_protocol_message_types(char *state_prompt, khash_t(strSet) * message_types)
{
    char *response = chat_with_llm(state_prompt, "glm-4.5-flash", MESSAGE_TYPE_RETRIES, 0.5);

    if (response)
    {
        // printf("Protocol message types response: %s\n", response);

        // Split the response by commas
        char *token = strtok(response, ",");
        while (token != NULL)
        {
            // Trim whitespace
            while (isspace(*token))
                token++;

            char *end = token + strlen(token) - 1;
            while (end > token && isspace(*end))
                end--;

            // Create a null-terminated string
            char *message_type = ck_alloc(end - token + 2);
            memcpy(message_type, token, end - token + 1);
            message_type[end - token + 1] = '\0';

            // Convert to uppercase
            for (int i = 0; message_type[i]; i++)
                message_type[i] = toupper(message_type[i]);

            // Add to the set
            int ret;
            khiter_t k = kh_put(strSet, message_types, message_type, &ret);
            if (ret == 0)
            {
                // Already exists
                ck_free(message_type);
            }

            token = strtok(NULL, ",");
        }

        free(response);
    }
}

char *enrich_sequence(char *sequence, khash_t(strSet) * missing_message_types)
{
    // 添加日志：记录种子增强开始
    ACTF("Enriching sequence with %d missing message types", kh_size(missing_message_types));

    const char *prompt_template =
        "The following is one sequence of client requests:\n"
        "%.*s\n"
        "Please add the %.*s client requests in the proper locations, and the modified sequence of client requests is:";

    int missing_fields_len = 0;
    int missing_fields_capacity = 100;
    char *missing_fields_seq = ck_alloc(missing_fields_capacity);

    khiter_t k;
    int i = 0;
    for (k = kh_begin(missing_message_types);
    k != kh_end(missing_message_types) && i < MIN(MAX_ENRICHMENT_MESSAGE_TYPES, kh_size(missing_message_types));
    ++k)
    {
        if (!kh_exist(missing_message_types, k))
            continue;
        ++i; // Increment only after seeing a message type
        const char *message_type = kh_key(missing_message_types, k);
        int needed_len = strlen(message_type) + 2; // add for the ', '

        if (missing_fields_len + needed_len > missing_fields_capacity)
        {
            missing_fields_capacity += 2 * needed_len;
            missing_fields_seq = ck_realloc(missing_fields_seq, missing_fields_capacity);
        }

        memcpy(missing_fields_seq + missing_fields_len, message_type, strlen(message_type));
        memcpy(missing_fields_seq + missing_fields_len + needed_len - 2, ", ", 2);

        missing_fields_len += needed_len;
    }
    missing_fields_len -= 2; // ignore the last ', '

    // 使用安全的字符串转义
    char *safe_sequence = safe_escape_for_json(sequence);
    int sequence_len = strlen(safe_sequence);

    int allowed_tokens = (MAX_TOKENS - strlen(prompt_template) - missing_fields_len);
    if (sequence_len > allowed_tokens)
    {
        sequence_len = allowed_tokens;
        // 截断安全的字符串
        safe_sequence[sequence_len] = '\0';
    }

    // 构建用户消息
    char *user_msg = NULL;
    asprintf(&user_msg, prompt_template, sequence_len, safe_sequence, missing_fields_len, missing_fields_seq);

    // 使用优化的JSON构建函数创建消息数组
    json_object *messages_array = build_message_array("You are a helpful assistant.", user_msg);
    const char *messages_json = json_object_to_json_string(messages_array);
    char *prompt = strdup(messages_json);

    // 清理内存
    ck_free(missing_fields_seq);
    free(safe_sequence);
    free(user_msg);
    json_object_put(messages_array);

    // 调用LLM
    char *response = chat_with_llm(prompt, "glm-4.5-flash", ENRICHMENT_RETRIES, 0.5);
    free(prompt);

    return response;
}

khash_t(strSet)* duplicate_hash(khash_t(strSet)* set)
{
    khash_t(strSet)* new_set = kh_init(strSet);

    khiter_t k;
    for (k = kh_begin(set); k != kh_end(set); ++k)
    {
        if (!kh_exist(set, k))
            continue;

        const char *key = kh_key(set, k);
        char *new_key = strdup(key);

        int ret;
        kh_put(strSet, new_set, new_key, &ret);
    }

    return new_set;
}

void write_new_seeds(char *enriched_file, char *contents)
{
    FILE *fp = fopen(enriched_file, "w");
    if (fp)
    {
        fprintf(fp, "%s", contents);
        fclose(fp);
    }
}

char *unescape_string(const char *input)
{
    if (!input)
        return NULL;

    size_t len = strlen(input);
    char *result = ck_alloc(len + 1);
    size_t j = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (input[i] == '\\' && i + 1 < len)
        {
            switch (input[i + 1])
            {
                case 'n':
                    result[j++] = '\n';
                    i++;
                    break;
                case 'r':
                    result[j++] = '\r';
                    i++;
                    break;
                case 't':
                    result[j++] = '\t';
                    i++;
                    break;
                case '\\':
                    result[j++] = '\\';
                    i++;
                    break;
                case '"':
                    result[j++] = '"';
                    i++;
                    break;
                default:
                    result[j++] = input[i];
                    break;
            }
        }
        else
        {
            result[j++] = input[i];
        }
    }

    result[j] = '\0';
    return result;
}

char *format_string(char *state_string)
{
    if (!state_string)
        return NULL;

    size_t len = strlen(state_string);
    char *result = ck_alloc(len + 1);
    size_t j = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (state_string[i] == '_')
            result[j++] = ' ';
        else
            result[j++] = state_string[i];
    }

    result[j] = '\0';
    return result;
}

message_set_list message_combinations(khash_t(strSet)* sequence, int size)
{
    message_set_list result;
    kv_init(result);

    if (size <= 0 || kh_size(sequence) == 0)
        return result;

    // Create an array of the keys
    const char **keys = ck_alloc(kh_size(sequence) * sizeof(const char *));
    int count = 0;

    khiter_t k;
    for (k = kh_begin(sequence); k != kh_end(sequence); ++k)
    {
        if (!kh_exist(sequence, k))
            continue;

        keys[count++] = kh_key(sequence, k);
    }

    // Generate all combinations of size 'size'
    if (size == 1)
    {
        for (int i = 0; i < count; i++)
        {
            khash_t(strSet)* new_set = kh_init(strSet);
            char *key = strdup(keys[i]);
            int ret;
            kh_put(strSet, new_set, key, &ret);
            kv_push(khash_t(strSet)*, result, new_set);
        }
    }
    else if (size > 1)
    {
        // This is a more complex case, for now we just return the original sequence
        kv_push(khash_t(strSet)*, result, duplicate_hash(sequence));
    }

    ck_free(keys);
    return result;
}


/**
 * 创建一个安全的JSON字符串，确保所有特殊字符都被正确转义
 */
char* create_safe_json_string(const char* input) {
    if (!input) {
        json_object* empty_obj = json_object_new_string("");
        const char* empty_str = json_object_get_string(empty_obj);
        char* result = strdup(empty_str);
        json_object_put(empty_obj);
        return result;
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


