#include <functional> // 匿名函数
#include <list>
#include<stdlib.h>
#include<iostream>
#include<thread>
#include<mutex> //锁
#include<memory>
#include<string>
#include<atomic>//原子
#include"CELLTimestamp.hpp" // 计算运行时间
#include"CELLObjectPool.hpp" // 对象池

using namespace std;

int lambda_func1(int a, int b) {
    printf("lambda_func1: %d\n", a+b);
    return 0;
}

void lambda_funcion() {
    /*
    捕获列表：lambda表达式的捕获列表精细控制了lambda表达式能够访问的外部变量，以及如何访问这些变量。

    1) []不捕获任何变量。

    2) [&]捕获外部作用域中所有变量，并作为引用在函数体中使用（按引用捕获）。

    3) [=]捕获外部作用域中所有变量，并作为副本在函数体中使用(按值捕获)。

    4) [=, &foo]按值捕获外部作用域中所有变量，并按引用捕获foo变量。

    5) [bar]按值捕获bar变量，同时不捕获其他变量。

    6) [this]捕获当前类中的this指针，让lambda表达式拥有和当前类成员函数同样的访问权限。
    如果已经使用了&或者 = ，就默认含有此选项。
    捕获this的目的是可以在lamda中使用当前类的成员函数和成员变量。

    ////////
    1).capture是捕获列表；

    2).params是参数表；(选填)

    3).opt是函数选项；可以填mutable,exception,attribute（选填）

    mutable说明lambda表达式体内的代码可以修改被捕获的变量，并且可以访问被捕获的对象的non-const方法。

    exception说明lambda表达式是否抛出异常以及何种异常。

    attribute用来声明属性。

    4).ret是返回值类型。(选填)

    5).body是函数体。
    */
    // method 1
    std::list< int > a;
    std::function< int(int, int) > call1 = lambda_func1;
    int n = call1(0,2);

    // method 2
    std::function< int(int,int) > call2;

    // int n = 5;
    // 匿名函数
    // [ caputrue ] ( params ) opt -> ret { body; };
    // [ 外部变量捕获列表 ] ( 参数列表 ) 特殊操作符 -> 返回值类型 { 函数体; };
    call2 = [n](int a,int b) -> int {
        //函数体
        printf("lambda_func2: %d\n",n+a+b);
        return 2;
    };

    int f = call2(3,1);
}

void for_char() {
    char* data[12];
    // size_t是一种无符号的整型数，它的取值没有负数，在数组中也用不到负数，而它的取值范围是整型数的双倍。
    for (size_t i = 0; i < 12; i++) {
        data[i] = new char[1+i];
    }
    for (size_t i = 0; i < 12; i++) {
        cout << data[i] << endl;
        delete[] data[i];
    }
}


// Type, 对象池大小
class ClassA : public ObjectPoolBase<ClassA, 100000> {
public:
    ClassA(int n) {
        num = n;
        cout << "ClassA" << endl;
    }

    ~ClassA() {
        cout << "~ClassA" << endl;
    }
public:
    int num = 0;
};

class ClassB : public ObjectPoolBase<ClassB, 10> {
public:
    ClassB(int n, int m) {
        num = n*m;
        cout << "ClassB" << endl;
    }

    ~ClassB() {
        cout << "~ClassB" << endl;
    }
public:
    int num = 0;
};

//原子操作   原子 分子
const int tCount = 16;
// const int mCount = 100000;
const int mCount = 1000000;
const int nCount = mCount/tCount;
void thread_func(int index) {
    ClassA* data[nCount];
    for (size_t i = 0; i < nCount; i++) {
        data[i] = ClassA::createObject(6);

    }
    for (size_t i = 0; i < nCount; i++) {
        ClassA::destroyObject(data[i]);
    }
}//抢占式

mutex m;
// int sum = 0;
atomic_int sum = {0};
void thread_sum(int index) {
    //原子操作 计算机处理命令时最小的操作单位
    for (int n = 0; n < nCount; n++) {
        // 三种方式

        // 直接累加，线程不安全（会漏算）
        sum++;
        // sum定义需改成原子级 atomic_int sum = {0}，多线程累加中最快（线程数越多越快）

        // 加锁，线程安全，但速度比一般for循环更慢（就累加运算而言）
        //临界区域-开始
        // m.lock();
        // sum++;
        // m.unlock();
        //临界区域-结束

        //自解锁，效果基本等同于加锁
        // lock_guard<mutex> lg(m);
        // sum++;
    }
}


ClassA& fun_count(ClassA& pA) {
    //引用计数
    pA.num++;
    return pA;
}

void fun_count(shared_ptr<ClassA>& pA) {
    //引用计数
    pA->num++;
}

void fun_count(ClassA* pA) {
    //引用计数
    pA->num++;
}

void memory_speed() {
    // 申请内存方式的运行速度对比；a1最慢，a2、a3基本一致，比a1快1.5-2倍
    shared_ptr<ClassA> a1 = make_shared<ClassA>(100);
    ClassA* a2 = new ClassA(100);
    ClassA* a3 = ClassA::createObject(100);

    int num = 200;

    a1->num = num;
    CELLTimestamp tTime;
    for (int n = 0; n < 100000000; n++) {
        fun_count(a1);
    }
    cout << tTime.getElapsedTimeInMilliSec() << endl;

    a2->num = num;
    tTime.update();
    for (int n = 0; n < 100000000; n++) {
        fun_count(a2);
    }
    delete a2;
    cout << tTime.getElapsedTimeInMilliSec() << endl;

    a3->num = num;
    tTime.update();
    for (int n = 0; n < 100000000; n++) {
        fun_count(a3);
    }
    cout << tTime.getElapsedTimeInMilliSec() << endl;
    ClassA::destroyObject(a3);

}

void free_memory() {
    // 释放内存的探讨
    // 需要手动释放
    ClassA* a1 = new ClassA(5);
    delete a1;

    // 二者一致，但都是先申请，全部堆积到程序最后释放
    shared_ptr<ClassA> a2 = make_shared<ClassA>(5);
    shared_ptr<ClassA> a3(new ClassA(5));

}

void memory_part() {
    int* a = new int;
    *a = 100;
    printf("a=%d\n", *a);
    delete a;
    printf("a=%d\n", *a);
    //C++标准库智能指针的一种，可智能释放
    shared_ptr<int> b = make_shared<int>();
    *b = 100;
    printf("b=%d\n", *b);

    // for_char();
    // memory_speed();
    // free_memory();
}

void thread_lock() {
    thread t[tCount];
    for (int n = 0; n < tCount; n++) {
        t[n] = thread(thread_sum,n);
    }
    CELLTimestamp tTime;
    for (int n = 0; n < tCount; n++) {
        t[n].join();
        //t[n].detach();
    }
    cout << tTime.getElapsedTimeInMilliSec()<<",sum=" <<sum<< endl;
    sum = 0;
    tTime.update();
    // 普通for循环累加
    for (int n = 0; n < nCount*tCount; n++) {
        sum++;
    }
    cout << tTime.getElapsedTimeInMilliSec() << ",sum=" << sum << endl;
}

void thread_part() {
    thread t[tCount];
    for (int n = 0; n < tCount; n++) {
        t[n] = thread(thread_func, n);
    }
    CELLTimestamp tTime;
    for (int n = 0; n < tCount; n++) {
        t[n].join();
        //t[n].detach();
    }
    cout << tTime.getElapsedTimeInMilliSec() << endl;
    cout << "Hello,main thread." << endl;
    // thread_lock();
}

int main() {
    // Part 1
    // lambda_funcion();
    // Part 2
    // memory_part();
    // Part 3
    thread_part();


    cout << "中文测试" << endl;
    return 0;
}

