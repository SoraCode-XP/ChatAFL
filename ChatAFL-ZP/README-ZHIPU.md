# ChatAFL-ZP 智谱API集成说明

## 概述

ChatAFL-ZP 是 ChatAFL 的一个变体，增加了对智谱AI API的支持。通过这个集成，您可以使用智谱AI的大语言模型（如GLM-4）来增强模糊测试的效果。

## 配置

在使用智谱API之前，您需要：

1. 获取智谱AI的API密钥
2. 在 `chat-llm.h` 文件中，将 `ZHIPU_TOKEN` 的值替换为您的API密钥：

```c
#define ZHIPU_TOKEN "your_actual_api_key_here"
```

## 使用方法

### 1. 编译项目

确保您的系统已安装必要的依赖库：
- libcurl
- json-c
- pcre2

然后使用 Makefile 编译项目：

```bash
make
```

### 2. 使用智谱API

在调用 ChatAFL 时，可以通过指定模型参数来使用智谱API：

```bash
./afl-fuzz -m zhipu -i input_dir -o output_dir -- target_binary
```

或者指定具体的智谱模型：

```bash
./afl-fuzz -m zhipu-glm-4 -i input_dir -o output_dir -- target_binary
./afl-fuzz -m zhipu-glm-3-turbo -i input_dir -o output_dir -- target_binary
```

### 3. 模型参数说明

- `zhipu` 或 `zhipu-glm-4`：使用GLM-4模型（默认）
- `zhipu-glm-3-turbo`：使用GLM-3-Turbo模型

## 技术实现

智谱API的集成是通过以下方式实现的：

1. 在 `chat-llm.h` 中添加了智谱API的token定义
2. 在 `chat-llm.c` 中实现了 `chat_with_zhipu` 函数，用于调用智谱API
3. 修改了 `chat_with_llm` 函数，使其能够根据模型参数自动选择调用OpenAI API还是智谱API

## 注意事项

1. 确保您的网络环境可以访问智谱AI的API端点：https://open.bigmodel.cn/api/paas/v4/chat/completions
2. 智谱API的调用限制和费用请参考智谱AI官方文档
3. 如果API调用失败，系统会自动重试，最多重试次数由调用时的tries参数决定

## 故障排除

如果遇到问题，请检查：

1. API密钥是否正确设置
2. 网络连接是否正常
3. API调用是否超限
4. 终端输出的错误信息

## 示例

以下是一个使用智谱API的完整示例：

```bash
# 设置API密钥
export ZHIPU_API_KEY="your_api_key_here"

# 编译项目
make

# 使用智谱API运行模糊测试
./afl-fuzz -m zhipu -i ./testcases -o ./out -- ./test_program @@
```

## 更新日志

- v1.0: 初始版本，支持智谱GLM-4和GLM-3-Turbo模型
