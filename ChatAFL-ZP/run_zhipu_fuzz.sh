#!/bin/bash

# 智谱API模糊测试脚本

# 设置颜色输出
RED='[0;31m'
GREEN='[0;32m'
YELLOW='[1;33m'
NC='[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查依赖
check_dependencies() {
    print_info "检查依赖库..."

    if ! ldconfig -p | grep -q libcurl; then
        print_error "libcurl 未安装，请运行: sudo apt-get install libcurl4-openssl-dev"
        exit 1
    fi

    if ! ldconfig -p | grep -q json-c; then
        print_error "json-c 未安装，请运行: sudo apt-get install libjson-c-dev"
        exit 1
    fi

    if ! ldconfig -p | grep -q pcre2; then
        print_error "pcre2 未安装，请运行: sudo apt-get install libpcre2-dev"
        exit 1
    fi

    if ! ldconfig -p | grep -q graphviz; then
        print_error "graphviz 未安装，请运行: sudo apt-get install graphviz-dev"
        exit 1
    fi

    if ! ldconfig -p | grep -q cap; then
        print_warning "libcap 未安装，可能影响某些功能，建议安装: sudo apt-get install libcap-dev"
    fi

    print_info "依赖库检查完成"
}

# 编译测试程序
compile_tests() {
    print_info "编译测试程序..."

    # 编译测试目标
    gcc -o test_target test_target.c
    if [ $? -ne 0 ]; then
        print_error "测试目标编译失败"
        exit 1
    fi

    # 编译AFL模糊测试器
    make clean && make
    if [ $? -ne 0 ]; then
        print_error "AFL模糊测试器编译失败"
        exit 1
    fi

    print_info "编译成功"
}

# 创建测试用例目录
setup_testcases() {
    print_info "设置测试用例..."

    mkdir -p testcases
    mkdir -p output

    # 检查测试用例是否存在
    if [ ! -f "testcases/sample1.json" ]; then
        print_warning "测试用例不存在，创建默认测试用例..."
        echo '{"name": "test", "value": 123, "active": true}' > testcases/sample1.json
        echo '{"users": [{"id": 1, "name": "Alice"}, {"id": 2, "name": "Bob"}]' > testcases/sample2.json
    fi

    print_info "测试用例设置完成"
}

# 运行模糊测试
run_fuzzing() {
    print_info "开始运行智谱API增强的模糊测试..."

    # 检查API密钥是否设置
    if grep -q "your_zhipu_api_key_here" chat-llm.h; then
        print_warning "警告: 您尚未在chat-llm.h中设置智谱API密钥"
        read -p "是否继续测试? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_info "测试已取消"
            exit 0
        fi
    fi

    # 选择模型
    MODEL="zhipu"
    read -p "选择模型 (1: GLM-4, 2: GLM-3-Turbo, 默认: GLM-4): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[2]$ ]]; then
        MODEL="zhipu-glm-3-turbo"
    fi

    print_info "使用模型: $MODEL"

    # 运行模糊测试
    print_info "启动模糊测试，按Ctrl+C停止..."
    ./afl-fuzz -m $MODEL -i testcases -o output -- ./test_target @@
}

# 主函数
main() {
    echo "=========================================="
    echo "       智谱API模糊测试"
    echo "=========================================="
    echo

    # 检查当前目录
    if [ ! -f "chat-llm.h" ] || [ ! -f "chat-llm.c" ]; then
        print_error "请在ChatAFL-ZP目录下运行此脚本"
        exit 1
    fi

    # 解析命令行参数
    case "${1:-all}" in
        "deps")
            check_dependencies
            ;;
        "compile")
            compile_tests
            ;;
        "setup")
            setup_testcases
            ;;
        "fuzz")
            run_fuzzing
            ;;
        "all")
            check_dependencies
            compile_tests
            setup_testcases
            run_fuzzing
            ;;
        *)
            echo "用法: $0 [deps|compile|setup|fuzz|all]"
            echo "  deps    - 检查依赖库"
            echo "  compile - 编译测试程序"
            echo "  setup   - 设置测试用例"
            echo "  fuzz    - 运行模糊测试"
            echo "  all     - 执行所有步骤（默认）"
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"
