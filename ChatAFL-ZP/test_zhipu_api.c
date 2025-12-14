#define _GNU_SOURCE // asprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chat-llm.h"

// 测试智谱API的简单脚本
int main(int argc, char **argv)
{
    // 设置默认参数
    char *model = "zhipu-glm-4"; // 默认使用GLM-4模型
    int tries = 3;               // 默认尝试3次
    float temperature = 0.7;      // 默认温度值

    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "m:t:T:h")) != -1) {
        switch (opt) {
            case 'm':
                model = optarg;
                break;
            case 't':
                tries = atoi(optarg);
                break;
            case 'T':
                temperature = atof(optarg);
                break;
            case 'h':
                printf("用法: %s [选项]\n", argv[0]);
                printf("选项:\n");
                printf("  -m <模型>   指定使用的模型 (zhipu-glm-4, zhipu-glm-3-turbo)，默认: zhipu-glm-4\n");
                printf("  -t <次数>   API调用失败时的重试次数，默认: 3\n");
                printf("  -T <温度>   生成文本的随机性 (0.0-1.0)，默认: 0.7\n");
                printf("  -h          显示此帮助信息\n");
                return 0;
            default:
                fprintf(stderr, "未知选项: %c\n", opt);
                return 1;
        }
    }

    // 构造测试提示
    char *test_prompt = NULL;
    asprintf(&test_prompt, "[{"role": "system", "content": "你是一个有帮助的助手。"}, {"role": "user", "content": "请简单介绍一下人工智能。"}]");

    printf("正在测试智谱API调用...\n");
    printf("模型: %s\n", model);
    printf("重试次数: %d\n", tries);
    printf("温度: %.1f\n", temperature);
    printf("提示: %s\n\n", test_prompt);

    // 调用API
    char *response = chat_with_zhipu(test_prompt, model, tries, temperature);

    if (response) {
        printf("API调用成功!\n");
        printf("响应:\n%s\n", response);
        free(response);
        return 0;
    } else {
        printf("API调用失败!\n");
        free(test_prompt);
        return 1;
    }
}
