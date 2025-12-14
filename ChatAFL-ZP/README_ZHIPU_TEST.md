# 智谱API增强测试脚本

## 概述

`test_zhipu_enhanced.c` 是一个增强版的智谱API测试脚本，提供了更全面的测试功能和更详细的测试结果统计。

## 功能特点

1. **多测试用例**：内置5个不同领域的测试用例，涵盖通用知识、编程、网络安全、模糊测试和协议分析
2. **详细统计**：提供成功率、响应时间等详细统计信息
3. **调试模式**：可选的调试模式，显示API请求和响应的详细信息
4. **灵活配置**：支持多种参数配置，包括模型选择、重试次数、温度等

## 编译

```bash
cd ChatAFL-ZP
gcc -o test_zhipu_enhanced test_zhipu_enhanced.c chat-llm.c -lcurl -ljson-c -lpcre2-8
```

## 使用方法

### 基本使用

```bash
# 使用默认参数运行测试
./test_zhipu_enhanced

# 指定模型
./test_zhipu_enhanced -m zhipu-glm-4

# 使用GLM-3-Turbo模型
./test_zhipu_enhanced -m zhipu-glm-3-turbo
```

### 高级选项

```bash
# 设置重试次数为5次
./test_zhipu_enhanced -t 5

# 设置温度为0.2（更确定的回答）
./test_zhipu_enhanced -T 0.2

# 启用调试模式
./test_zhipu_enhanced -d

# 运行3个测试用例
./test_zhipu_enhanced -c 3

# 组合多个参数
./test_zhipu_enhanced -m zhipu-glm-3-turbo -t 5 -T 0.2 -d -c 3
```

### 参数说明

- `-m <模型>`：指定使用的模型
  - `zhipu` 或 `zhipu-glm-4`：使用GLM-4模型（默认）
  - `zhipu-glm-3-turbo`：使用GLM-3-Turbo模型
- `-t <次数>`：API调用失败时的重试次数（默认：3）
- `-T <温度>`：生成文本的随机性，范围0.0-1.0（默认：0.7）
  - 较低的值（如0.2）会产生更确定、更一致性的回答
  - 较高的值（如0.9）会产生更多样化、更有创造性的回答
- `-d`：启用调试模式，显示API请求和响应的详细信息
- `-c <数量>`：测试用例数量（默认：1，最大：5）
- `-h`：显示帮助信息

## 测试用例

脚本内置了以下5个测试用例：

1. **人工智能介绍**：测试模型对基本概念的理解
2. **C语言Hello World程序**：测试模型的编程能力
3. **SQL注入攻击解释**：测试模型的安全知识
4. **模糊测试原理**：测试模型对模糊测试的理解
5. **HTTP协议结构**：测试模型对网络协议的了解

## 输出示例

```
===== 智谱API增强测试 =====
模型: zhipu
重试次数: 3
温度: 0.7
调试模式: 关闭
测试用例数量: 3

--- 测试用例 1: 人工智能介绍 ---
提示: [{"role": "system", "content": "你是一个有帮助的助手。"}, {"role": "user", "content": "请简单介绍一下人工智能。"}]
API调用成功!
响应:
人工智能（Artificial Intelligence，简称AI）是计算机科学的一个分支...

--- 测试用例 2: C语言Hello World程序 ---
提示: [{"role": "system", "content": "你是一个代码助手。"}, {"role": "user", "content": "请写一个C语言的Hello World程序。"}]
API调用成功!
响应:
以下是一个简单的C语言Hello World程序：
```c
#include <stdio.h>

int main() {
    printf("Hello, World!\n");
    return 0;
}
```

--- 测试用例 3: SQL注入攻击解释 ---
提示: [{"role": "system", "content": "你是一个网络安全专家。"}, {"role": "user", "content": "请解释一下什么是SQL注入攻击。"}]
API调用成功!
响应:
SQL注入是一种代码注入技术，攻击者通过在应用程序的输入字段中...

===== 测试结果 =====
总测试用例: 3
成功调用: 3
失败调用: 0
成功率: 100.0%
总耗时: 5.23 秒
平均响应时间: 1.74 秒
```

## 故障排除

如果测试失败，请检查以下几点：

1. **API密钥未正确设置**：
   - 确认在`chat-llm.h`文件中已正确设置`ZHIPU_TOKEN`
   - 确认API密钥有效且未过期

2. **网络连接问题**：
   - 确认您的网络可以访问智谱AI的API端点
   - 如果在企业网络环境中，可能需要配置代理

3. **依赖库问题**：
   - 确认已安装所有必需的依赖库
   - 使用`ldd test_zhipu_enhanced`检查依赖是否满足

4. **API配额问题**：
   - 确认您的智谱API账户有足够的调用配额
   - 检查智谱AI控制台中的API使用情况

## 注意事项

1. 确保您的网络环境可以访问智谱AI的API端点：https://open.bigmodel.cn/api/paas/v4/chat/completions
2. 智谱API的调用限制和费用请参考智谱AI官方文档
3. 如果API调用失败，系统会自动重试，最多重试次数由`-t`参数决定
