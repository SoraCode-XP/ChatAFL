#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 简单的测试目标程序，模拟一个解析JSON格式输入的服务
int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <input>\n", argv[0]);
        return 1;
    }

    char *input = argv[1];
    printf("Processing input: %s\n", input);

    // 简单的JSON格式检查
    int in_string = 0;
    int brace_level = 0;
    int bracket_level = 0;
    int escape = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];

        if (escape) {
            escape = 0;
            continue;
        }

        if (c == '\\') {
            escape = 1;
            continue;
        }

        if (c == '"' && !escape) {
            in_string = !in_string;
            continue;
        }

        if (in_string) {
            continue;
        }

        if (c == '{') {
            brace_level++;
        } else if (c == '}') {
            brace_level--;
            if (brace_level < 0) {
                printf("Error: Unmatched closing brace\n");
                return 1;
            }
        } else if (c == '[') {
            bracket_level++;
        } else if (c == ']') {
            bracket_level--;
            if (bracket_level < 0) {
                printf("Error: Unmatched closing bracket\n");
                return 1;
            }
        }
    }

    if (in_string) {
        printf("Error: Unclosed string\n");
        return 1;
    }

    if (brace_level != 0) {
        printf("Error: Unmatched braces\n");
        return 1;
    }

    if (bracket_level != 0) {
        printf("Error: Unmatched brackets\n");
        return 1;
    }

    printf("Input is valid JSON format\n");
    return 0;
}
