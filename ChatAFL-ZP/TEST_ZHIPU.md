# 智谱API调用效果测试指南

## 概述

本指南将帮助您测试ChatAFL-ZP中集成的智谱API调用功能，确保API密钥正确配置且网络连接正常。

## 前提条件

1. 已正确设置智谱API密钥（在chat-llm.h文件中）
2. 已安装必要的依赖库（libcurl、json-c、pcre2）
3. 已编译ChatAFL-ZP项目

## 编译测试程序

使用以下命令编译测试程序：

```bash
cd ChatAFL-ZP
gcc -o test_zhipu_api test_zhipu_api.c chat-llm.c -lcurl -ljson-c -lpcre2-8
```

## 执行测试

### 基本测试

使用默认参数运行测试：

```bash
./test_zhipu_api
```

这将使用GLM-4模型，温度为0.7，重试3次。

### 自定义参数测试

您可以通过命令行参数自定义测试：

```bash
# 使用GLM-3-Turbo模型
./test_zhipu_api -m zhipu-glm-3-turbo

# 设置重试次数为5次
./test_zhipu_api -t 5

# 设置温度为0.2（更确定的回答）
./test_zhipu_api -T 0.2

# 组合多个参数
./test_zhipu_api -m zhipu-glm-3-turbo -t 5 -T 0.2
```

### 参数说明

- `-m <模型>`: 指定使用的模型
  - `zhipu-glm-4`: GLM-4模型（默认）
  - `zhipu-glm-3-turbo`: GLM-3-Turbo模型
- `-t <次数>`: API调用失败时的重试次数（默认：3）
- `-T <温度>`: 生成文本的随机性，范围0.0-1.0（默认：0.7）
  - 较低的值（如0.2）会产生更确定、更一致性的回答
  - 较高的值（如0.9）会产生更多样化、更有创造性的回答
- `-h`: 显示帮助信息

## 预期结果

如果测试成功，您将看到类似以下的输出：

```
正在测试智谱API调用...
模型: zhipu-glm-4
重试次数: 3
温度: 0.7
提示: [{"role": "system", "content": "你是一个有帮助的助手。"}, {"role": "user", "content": "请简单介绍一下人工智能。"}]

API调用成功!
响应:
人工智能（Artificial Intelligence，简称AI）是计算机科学的一个分支，它致力于创建能够执行通常需要人类智能的任务的系统...
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
   - 使用`ldd test_zhipu_api`检查依赖是否满足

4. **API配额问题**：
   - 确认您的智谱API账户有足够的调用配额
   - 检查智谱AI控制台中的API使用情况

## 常见错误及解决方案

1. **认证失败**：
   ```
   Zhipu API Error: invalid api key
   ```
   解决方案：检查并更新API密钥

2. **网络超时**：
   ```
   Error: Operation timed out
   ```
   解决方案：增加重试次数或检查网络连接

3. **模型不存在**：
   ```
   Zhipu API Error: model not found
   ```
   解决方案：使用正确的模型名称（zhipu-glm-4或zhipu-glm-3-turbo）

## 高级测试

您可以通过修改`test_zhipu_api.c`中的测试提示来测试更复杂的场景：

1. 修改测试提示内容
2. 添加更多测试用例
3. 测试不同温度值对输出结果的影响

完成测试后，如果一切正常，您就可以在ChatAFL-ZP中使用智谱API了。
