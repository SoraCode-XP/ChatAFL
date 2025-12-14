# ChatAFL 中文文档

![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.10115151.svg)

<img align="right" src="https://github.com/ChatAFLndss/ChatAFL/assets/7456946/266f7d4f-c0af-4846-9e13-064e79c812b0">

## 项目简介

ChatAFL 是一个由大型语言模型（LLMs）引导的协议模糊测试工具。它基于 [AFLNet](https://github.com/aflnet/aflnet) 构建，但集成了三个核心组件：

1. 使用 LLM 提取协议的机器可读语法，用于结构感知的变异
2. 使用 LLM 增加初始种子中消息序列的多样性
3. 使用 LLM 打破覆盖停滞，提示 LLM 生成消息以到达新状态

ChatAFL 工具在 [ProfuzzBench](https://github.com/profuzzbench/profuzzbench) 框架内配置，这是一个广泛使用的有状态网络协议模糊测试基准。

## 项目结构

```
ChatAFL-Artifact
├── aflnet: 修改版的 AFLNet，输出状态和状态转换
├── analyse.sh: 分析脚本
├── benchmark: 修改版的 ProfuzzBench，仅包含基于文本的协议，并添加了 Lighttpd 1.4
├── clean.sh: 清理脚本
├── ChatAFL: ChatAFL 的源代码，包含论文中提出的所有策略
├── ChatAFL-CL1: 仅使用结构感知变异的 ChatAFL（参见消融研究）
├── ChatAFL-CL2: 使用结构感知和初始种子增强的 ChatAFL（参见消融研究）
├── deps.sh: 安装依赖项的脚本，执行时需要密码
├── README.md: 英文版说明文件
├── README_CN.md: 中文版说明文件（本文件）
├── run.sh: 在目标上运行模糊测试并收集数据的执行脚本
└── setup.sh: 准备 Docker 镜像的脚本
```

## 运行环境

### 系统要求

- Linux 或 macOS（Windows 下可通过 WSL 或 Docker 使用）
- 至少 4GB RAM
- 至少 10GB 可用磁盘空间

### 依赖项

- Docker
- Bash
- Python3（需要安装 pandas 和 matplotlib 库）

## 安装与配置

### 1. 安装依赖项

我们提供了一个辅助脚本 `deps.sh` 来确保安装所有必要的依赖项：

```bash
./deps.sh
```

### 2. 准备 Docker 镜像

运行以下命令设置所有 Docker 镜像，包括所有模糊测试器及其目标：

```bash
KEY=<你的OPENAI_API_KEY> ./setup.sh
```

这个过程大约需要 40 分钟。OPENAI_API_KEY 是您的 OpenAI 密钥，请参考 [OpenAI 官方网站](https://openai.com/) 了解如何获取密钥。

### 3. 配置文件说明

#### config.h

`config.h` 文件包含 ChatAFL 的基本配置参数，以下是与 ChatAFL 相关的关键参数：

- `EPSILON_CHOICE`: 选择基于语法进行变异的阈值（默认值：0.5）
- `UNINTERESTING_THRESHOLD`: 判定为无趣状态的阈值（默认值：512）
- `CHATTING_THRESHOLD`: 最大与 LLM 交互次数（默认值：64）

#### chat-llm.h

`chat-llm.h` 文件包含与 LLM 交互相关的配置参数：

- `STALL_RETRIES`: 状态停滞时的最大重试次数（默认值：2）
- `GRAMMAR_RETRIES`: 获取语法的最大尝试次数（默认值：5）
- `MESSAGE_TYPE_RETRIES`: 获取消息类型的最大尝试次数（默认值：5）
- `ENRICHMENT_RETRIES`: 种子增强的最大尝试次数（默认值：5）
- `MAX_ENRICHMENT_MESSAGE_TYPES`: 最大添加的消息类型数量（默认值：2）
- `MAX_ENRICHMENT_CORPUS_SIZE`: 检查添加的最大语料库大小（默认值：10）

#### Dockerfile

Dockerfile 定义了 ChatAFL 的运行环境，基于 Ubuntu 18.04，并安装了以下关键依赖：

- build-essential
- clang
- libcurl-openssl1.0-dev
- libjson-c-dev
- libpcre2-dev
- graphviz-dev

## 使用方法

### 运行实验

使用 `run.sh` 脚本运行实验，命令格式如下：

```bash
./run.sh <容器数量> <模糊测试时间> <目标> <模糊测试器>
```

其中：
- `容器数量`：指定为每个模糊测试器和目标创建的容器数量
- `模糊测试时间`：模糊测试时间（分钟）
- `目标`：测试目标列表
- `模糊测试器`：使用的模糊测试器列表

例如，命令 `run.sh 1 5 pure-ftpd chatafl` 将创建 1 个容器，让 ChatAFL 模糊测试器对 pure-ftpd 目标进行 5 分钟的测试。

可以使用 `all` 代替目标和模糊测试器列表，以运行所有模糊测试器和目标。

脚本完成后，在 `benchmark` 目录中将创建一个 `result-<目标名称>` 文件夹，包含每次运行的模糊测试结果。

### 分析结果

使用 `analyze.sh` 脚本分析数据并构建图表，显示模糊测试器在每个目标上的平均代码和状态覆盖率随时间的变化。命令格式如下：

```bash
./analyze.sh <目标> <模糊测试时间>
```

脚本接受 2 个参数：
- `目标`：测试目标列表
- `模糊测试时间`：要分析的运行持续时间（可选，默认为 1440 分钟，即 1 天）

例如，命令 `analyze.sh exim 240` 将分析 exim 目标前 4 小时的执行结果。

执行完成后，脚本将处理归档文件，构建包含随时间变化的覆盖分支数、状态数和状态转换数的 CSV 文件。这些 CSV 文件将被处理成 PNG 文件，显示模糊测试器在每个目标上的平均代码和状态覆盖率随时间的变化。所有这些信息将移动到根目录中带有时间戳的 `res_<目标名称>` 文件夹。

### 清理

当评估完成后，运行 `clean.sh` 脚本可以清理所有临时文件：

```bash
./clean.sh
```

## 功能分析

### 检查 LLM 生成的语法

语法生成的源代码位于 `afl-fuzz.c` 中的 `setup_llm_grammars` 函数，辅助函数在 `chat-llm.c` 中。

LLM 对语法生成的响应可以在运行结果归档的 `protocol-grammars` 目录中找到。

### 检查增强的种子

种子增强的源代码位于 `afl-fuzz.c` 中的 `get_seeds_with_messsage_types` 函数，辅助函数在 `chat-llm.c` 中。

增强的种子可以在运行结果归档的种子 `queue` 目录中找到。这些文件名为 `id:...,orig:enriched_`。

### 检查状态停滞响应

状态停滞处理的源代码位于 `fuzz_one` 函数中，从第 6846 行开始（`if (uninteresting_times >= UNINTERESTING_THRESHOLD && chat_times < CHATTING_THRESHOLD){`）。

状态停滞提示及其相应响应可以在运行结果归档的 `stall-interactions` 目录中找到。文件格式为 `request-<id>` 和 `response-<id>`，包含我们构建的请求和 LLM 的响应。

## 自定义配置

### 增强或实验 ChatAFL

如果对任何模糊测试器进行了修改，重新执行 `setup.sh` 将使用修改后的版本重建所有镜像。所有提供的 ChatAFL 版本都包含一个 Dockerfile，允许在与目标相同的环境中检查构建失败，并拥有一个干净的镜像，可以在其中设置不同的目标。

### 调整模糊测试器参数

实验中使用的所有参数位于 `config.h` 和 `chat-llm.h` 中。ChatAFL 特定的参数已在前面列出。

### 添加新目标

要添加额外的目标，请参考 [ProfuzzBench 提供的说明](https://github.com/profuzzbench/profuzzbench#1-how-do-i-extend-profuzzbench)。作为示例，我们已将 Lighttpd 1.4 添加为新目标。

### 故障排除

如果模糊测试器因错误终止，将显示过早的 "I am done" 消息。要检查此问题，运行 `docker logs <容器ID>` 将显示失败容器的日志。

### 使用 GPT-4

我们发布了使用 GPT-4 的新版 ChatAFL：[gpt4-version](https://github.com/ChatAFLndss/ChatAFL/tree/gpt4-version)。然而，它尚未经过广泛测试。如果在使用过程中遇到任何问题，请随时联系我们。

## 限制

当前的工具与 OpenAI 的大型语言模型（`gpt-3.5-turbo-instruct` 和 `gpt-3.5-turbo`）交互。这对并行化程度施加了第三方限制。此工具中使用的模型每分钟有 150,000 个令牌的硬限制。

## 引用 ChatAFL

ChatAFL 已被第 31 届网络与分布式系统安全研讨会（NDSS）2024 接受发表。论文也可在[此处](https://mengrj.github.io/pdfs/chatafl.pdf)获取。如果在您的科学工作中使用此代码，请按如下方式引用论文：

```
@inproceedings{chatafl,
author={Ruijie Meng and Martin Mirchev and Marcel B"{o}hme and Abhik Roychoudhury},
title={Large Language Model guided Protocol Fuzzing},
booktitle={Proceedings of the 31st Annual Network and Distributed System Security Symposium (NDSS)},
year={2024},}
```

## 特别感谢

我们要感谢 [AFLNet](https://github.com/aflnet/aflnet) 和 [ProFuzzBench](https://github.com/profuzzbench/profuzzbench) 的创建者提供的工具和基础设施。
