#define _GNU_SOURCE // asprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "chat-llm.h"

// 增强版智谱API测试脚本
int main(int argc, char **argv)
{
    // 设置默认参数
    char *model = "zhipu"; // 默认使用智谱API
    int tries = 3;          // 默认尝试3次
    float temperature = 0.7; // 默认温度值
    int debug = 0;          // 默认关闭调试模式
    int test_cases = 1;     // 默认测试用例数量

    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "m:t:T:dhc:")) != -1) {
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
            case 'd':
                debug = 1;
                setenv("DEBUG_ZHIPU_API", "1", 1);
                break;
            case 'c':
                test_cases = atoi(optarg);
                break;
            case 'h':
                printf("用法: %s [选项]\n", argv[0]);
                printf("选项:\n");
                printf("  -m <模型>   指定使用的模型 (zhipu, zhipu-glm-4, zhipu-glm-3-turbo)，默认: zhipu\n");
                printf("  -t <次数>   API调用失败时的重试次数，默认: 3\n");
                printf("  -T <温度>   生成文本的随机性 (0.0-1.0)，默认: 0.7\n");
                printf("  -d          启用调试模式，显示API请求和响应详情\n");
                printf("  -c <数量>   测试用例数量，默认: 1\n");
                printf("  -h          显示此帮助信息\n");
                return 0;
            default:
                fprintf(stderr, "未知选项: %c\n", opt);
                return 1;
        }
    }

    printf("===== 智谱API增强测试 =====\n");
    printf("模型: %s\n", model);
    printf("重试次数: %d\n", tries);
    printf("温度: %.1f\n", temperature);
    printf("调试模式: %s\n", debug ? "开启" : "关闭");
    printf("测试用例数量: %d\n\n", test_cases);

    // 测试用例
    const char *test_prompts[] = {
        "[{\"role\": \"system\", \"content\": \"你是一个有帮助的助手。\"}, {\"role\": \"user\", \"content\": \"请简单介绍一下人工智能。\"}]",
        "[{\"role\": \"system\", \"content\": \"你是一个代码助手。\"}, {\"role\": \"user\", \"content\": \"请写一个C语言的Hello World程序。\"}]",
        "[{\"role\": \"system\", \"content\": \"你是一个网络安全专家。\"}, {\"role\": \"user\", \"content\": \"请解释一下什么是SQL注入攻击。\"}]",
        "[{\"role\": \"system\", \"content\": \"你是一个模糊测试专家。\"}, {\"role\": \"user\", \"content\": \"请解释一下模糊测试的基本原理。\"}]",
        "[{\"role\": \"system\", \"content\": \"你是一个协议分析专家。\"}, {\"role\": \"user\", \"content\": \"请解释一下HTTP协议的基本结构。\"}]"
    };

    const char *test_descriptions[] = {
        "人工智能介绍",
        "C语言Hello World程序",
        "SQL注入攻击解释",
        "模糊测试原理",
        "HTTP协议结构"
    };

    int total_tests = sizeof(test_prompts) / sizeof(test_prompts[0]);
    if (test_cases > total_tests) {
        test_cases = total_tests;
        printf("警告: 请求的测试用例数量超过可用数量，将使用最大数量 %d\n\n", total_tests);
    }

    // 执行测试
    int success_count = 0;
    clock_t start_time = clock();

    for (int i = 0; i < test_cases; i++) {
        printf("--- 测试用例 %d: %s ---\n", i+1, test_descriptions[i]);
        printf("提示: %s\n", test_prompts[i]);

        // 调用API
        char *response = chat_with_zhipu((char*)test_prompts[i], model, tries, temperature);

        if (response) {
            printf("API调用成功!\n");
            printf("响应:\n%s\n", response);
            free(response);
            success_count++;
        } else {
            printf("API调用失败!\n");
        }

        printf("\n");
    }

    // 统计结果
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("===== 测试结果 =====\n");
    printf("总测试用例: %d\n", test_cases);
    printf("成功调用: %d\n", success_count);
    printf("失败调用: %d\n", test_cases - success_count);
    printf("成功率: %.1f%%\n", (float)success_count / test_cases * 100);
    printf("总耗时: %.2f 秒\n", elapsed_time);
    printf("平均响应时间: %.2f 秒\n", elapsed_time / test_cases);

    return (success_count == test_cases) ? 0 : 1;
}
