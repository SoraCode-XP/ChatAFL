# 智谱API模糊测试指南

## 概述

本指南介绍如何使用ChatAFL-ZP和智谱AI API进行模糊测试。智谱API可以帮助生成更有针对性的测试用例，提高模糊测试的效率和效果。

## 准备工作

### 1. 安装依赖

```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev libjson-c-dev libpcre2-dev graphviz-dev build-essential
```

注意：AFL模糊测试器需要graphviz库来生成控制流图，如果缺少此库，编译将会失败。

### 2. 设置API密钥

编辑 `chat-llm.h` 文件，将第16行的 `ZHIPU_TOKEN` 替换为您的实际API密钥：

```c
#define ZHIPU_TOKEN "your_actual_api_key_here"
```

## 快速开始

### 使用自动化脚本

最简单的方法是使用提供的自动化脚本：

```bash
cd ChatAFL-ZP
chmod +x run_zhipu_fuzz.sh
./run_zhipu_fuzz.sh
```

这个脚本会自动：
1. 检查依赖库
2. 编译测试程序和AFL模糊测试器
3. 设置测试用例
4. 运行模糊测试

### 手动步骤

如果需要更多控制，可以手动执行以下步骤：

#### 1. 编译测试目标

```bash
gcc -o test_target test_target.c
```

#### 2. 编译AFL模糊测试器

```bash
make clean && make
```

#### 3. 准备测试用例

创建测试用例目录并添加初始输入：

```bash
mkdir -p testcases
mkdir -p output

# 创建示例JSON文件
echo '{"name": "test", "value": 123, "active": true}' > testcases/sample1.json
echo '{"users": [{"id": 1, "name": "Alice"}, {"id": 2, "name": "Bob"}]' > testcases/sample2.json
```

#### 4. 运行模糊测试

```bash
# 使用GLM-4模型
./afl-fuzz -m zhipu -i testcases -o output -- ./test_target @@

# 使用GLM-3-Turbo模型
./afl-fuzz -m zhipu-glm-3-turbo -i testcases -o output -- ./test_target @@
```

## 测试目标

本指南提供了一个简单的测试目标程序 `test_target.c`，它模拟一个解析JSON格式输入的服务。这个程序会检查输入是否为有效的JSON格式，并在发现格式错误时报告错误。

## 模型选择

ChatAFL-ZP支持以下智谱模型：

- `zhipu` 或 `zhipu-glm-4`：使用GLM-4模型（默认）
- `zhipu-glm-3-turbo`：使用GLM-3-Turbo模型

GLM-4模型通常提供更高质量的回答，而GLM-3-Turbo模型响应更快。根据您的需求选择合适的模型。

## 参数调整

您可以通过以下参数调整模糊测试的行为：

```bash
# 设置最大重试次数
./afl-fuzz -m zhipu -t 5 -i testcases -o output -- ./test_target @@

# 设置温度参数（控制生成文本的随机性）
./afl-fuzz -m zhipu -T 0.2 -i testcases -o output -- ./test_target @@

# 组合多个参数
./afl-fuzz -m zhipu-glm-3-turbo -t 5 -T 0.2 -i testcases -o output -- ./test_target @@
```

## 结果分析

模糊测试完成后，可以在 `output` 目录中查看结果：

- `output/crashes`：发现的崩溃输入
- `output/hangs`：发现的挂起输入
- `output/queue`：有趣的测试用例队列

## 高级用法

### 1. 使用自定义测试目标

您可以创建自己的测试目标程序，替换 `test_target.c`。确保程序接受命令行参数作为输入：

```c
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <input>\n", argv[0]);
        return 1;
    }

    char *input = argv[1];
    // 处理输入...
    return 0;
}
```

### 2. 调试模式

启用调试模式查看智谱API的请求和响应详情：

```bash
export DEBUG_ZHIPU_API=1
./afl-fuzz -m zhipu -i testcases -o output -- ./test_target @@
```

### 3. 多核心并行测试

利用多核心加速模糊测试：

```bash
# 主实例
./afl-fuzz -m zhipu -i testcases -o output -M fuzzer01 -- ./test_target @@

# 从实例
./afl-fuzz -m zhipu -i testcases -o output -S fuzzer02 -- ./test_target @@
```

## 故障排除

### 1. 编译错误

确保已安装所有依赖库：

```bash
sudo apt-get install libcurl4-openssl-dev libjson-c-dev libpcre2-dev graphviz-dev build-essential
```

如果遇到以下错误：
```
/usr/bin/ld: 找不到 -lgvc: 没有那个文件或目录
/usr/bin/ld: 找不到 -lcgraph: 没有那个文件或目录
```
请安装graphviz开发库：
```bash
sudo apt-get install graphviz-dev
```

### 2. API调用失败

- 检查API密钥是否正确设置
- 确认网络可以访问智谱AI的API端点
- 检查API配额是否充足

### 3. 模糊测试速度慢

- 尝试使用GLM-3-Turbo模型（响应更快）
- 减少重试次数：`-t 2`
- 调整温度参数：`-T 0.2`

## 注意事项

1. 确保您的网络环境可以访问智谱AI的API端点：https://open.bigmodel.cn/api/paas/v4/chat/completions
2. 智谱API的调用限制和费用请参考智谱AI官方文档
3. 长时间运行模糊测试可能会产生大量API调用，请注意费用控制
4. 如果API调用失败，系统会自动重试，最多重试次数由`-t`参数决定

## 扩展阅读

- [ChatAFL-ZP README-ZHIPU.md](README-ZHIPU.md)：智谱API集成说明
- [ChatAFL-ZP TEST_ZHIPU.md](TEST_ZHIPU.md)：智谱API测试指南
- [AFL README](README-AFL.md)：AFL模糊测试器使用指南
