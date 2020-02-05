# CPP11笔记

```bash
# A2格式化
astyle -A2 main.cpp

# 直接编译
g++.exe -std=c++11 .\main.cpp -o output

# 使用Makefile编译
make

# 使用ccache加速编译
ccache g++.exe -std=c++11 .\main.cpp -o output

# 运行
.\output

# PowerShell测试运行时间，替换command。无输出日志
Measure-Command -Expression {command}
```



