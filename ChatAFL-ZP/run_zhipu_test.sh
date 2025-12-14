#!/bin/bash

# 智谱API测试脚本

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

    print_info "依赖库检查完成"
}

# 编译测试程序
compile_tests() {
    print_info "编译测试程序..."

    if [ -f "Makefile.test" ]; then
        make -f Makefile.test clean
        make -f Makefile.test all
    else
        gcc -o test_zhipu_enhanced test_zhipu_enhanced.c chat-llm.c -lcurl -ljson-c -lpcre2-8
    fi

    if [ $? -eq 0 ]; then
        print_info "编译成功"
    else
        print_error "编译失败"
        exit 1
    fi
}

# 运行测试
run_tests() {
    print_info "开始运行智谱API测试..."

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

    # 运行基本测试
    print_info "运行基本测试..."
    ./test_zhipu_enhanced -c 1

    if [ $? -eq 0 ]; then
        print_info "基本测试通过"
    else
        print_error "基本测试失败"
        exit 1
    fi

    # 询问是否运行更多测试
    read -p "是否运行更多测试用例? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "运行完整测试套件..."
        ./test_zhipu_enhanced -c 5
    fi

    print_info "测试完成"
}

# 主函数
main() {
    echo "=========================================="
    echo "       智谱API测试脚本"
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
        "test")
            run_tests
            ;;
        "all")
            check_dependencies
            compile_tests
            run_tests
            ;;
        *)
            echo "用法: $0 [deps|compile|test|all]"
            echo "  deps    - 检查依赖库"
            echo "  compile - 编译测试程序"
            echo "  test    - 运行测试"
            echo "  all     - 执行所有步骤（默认）"
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"
