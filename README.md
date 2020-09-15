# lib_pipe
--------------------------------
## 1. 关于  

  lib_pipe是一个用c++编写的管道通信动态库，截至目前（15/9/2020），完成了Windows上的收发。创建这个项目的初衷： 
  * 现在有一个main程序，需要创建1个或者多个子进程，每个子进程都有自己的活儿要干，且，创建子进程时，还需要读取每个进程的配置文件
  * main程序需要支持控制子进程的退出  
  之前没有做过这样的需求，都是多线程玩的嗨。弥补多进程知识....  
  封装的很简陋，持续完善

## 2. 目录说明  
  * 2.1 根目录说明 
```
.
├───Debug		# exe的输出路径，目前,exe编译环境：win10 1909 + VS2015up3
│   ├───1		# 子进程1所需文件
│   ├───2		# 子进程2所需文件
│   └───3		# 子进程3所需文件
├───demo_create # client项目，创建一个子进程的程序
├───lib_pipe	# 管道通信项目，动态库
└───main		# main项目，负责调用client子进程  
```

  * 2.2 cmake_pipe目录说明  
  增加lib_pipe的cmake版（以后主要维护这个）  
 ```
 .
│   CMakeLists.txt	#cmake配置文件
│   
├───build			# 习惯，将cmake输出到这个文件夹下
│       light_file.bat	# 用作删除Visual studio 编译器产生的中间项，递归删除子目录
│       
├───example			# demo目录
│       main.cpp	# 一个demo演示使用lib_pipe
│       
├───include						# 引用头文件目录
│   └───pipe
│           pipe_helper.h		# pipe的实现头文件定义
│           pipe_interface.h	# pipe的接口文件，定义了pipe的操作接口， lib使用这个头文件
│           
└───src							# 源文件目录
        pipe_helper.cpp			# pipe的实现
        pipe_interface.cpp		# pipe接口类的实现
 ``` 

## 3. 项目说明  

* 3.1 main项目将创建子进程，新创建的子进程独立运行

* 3.2 创建的子进程所需文件在Debug目录下的1、2和3目录

* 3.3 main进程将与子进程采用管道通信
* 3.4 main将发送Q通知子进程结束，子进程收到Q后，释放自己创建时申请的一些资源，再退出
* 3.5 lib_pipe, 目前（15/9/2020）仅支持Windows，后期将持续完善，简单封装了常用操作，欢迎指正，一起完善
* 3.6 解决方案是用VS2015 up3创建的，若尝试用低于这个版本的VS打开项目，请创建一个空的解决方案，再添加项目即可
* 3.7 代码中使用 nullptr 关键字，请选择支持对应所需的编译器
* 3.8 main项目和  demo_create 都使用lib_pipe, 动态库名字和目录，配置到了VS的项目属性中。因为lib输出到Debug目录下，所以将lib目录配置为：${TargetDir}

## 4.lib_pipe使用  

  按照习惯，封装了以下操作：
  * init-初始化管道信息
  * wrie-向管道写入数据
  * uninit-释放初始化申请的资源  
  * on_recv_data-接收数据（单独创建了一个线程接收数据  

  文件，包括，lib库文件，dll动态库，和头文件，头文件名： pipe_interface.h

## 5. 返回值说明  
  c++11引入了tuple，但是当初考虑到需要兼容不支持c++11的环境，故换作了std::pair作为函数的返回值，以便能获取更多有效的信息。  
  之前以int为函数的返回值，通过定义各种数值对应其结果，比如0-成功，1，字符串为空，2-文件不存在之类的。  
  lib_pipe使用的返回值声明如下：
```
struct ret_type_
{
/* 省略一些函数的声明和定义*/
private:
	pair_int_str	_value;
};
```
  pair_int_str的定义及说明如下：
```
typedef std::pair<int, std::string> pair_int_str;
```
  请查看文件**pipe_interface.h**中的定义。


## 6. 接收  
  * 接收需要继承类【irecv_data】,并实现函数【on_recv_data】
  * 初始化函数 init的第二个参数需要传递为继承【ipipe_interface】类的对象
  * 不需要接收，传递NULL即可

## 7.一个例子（非完整）  
  lib_pipe的用法可以在项目 main 和 demo_create中找到，包括收发。

  * 初始化
```
pipe_param_base base_param;
base_param._to_create_pipe = false;
base_param._name = std::string("\\\\.\\pipe\\ReadPipe");
ret_type ret_val = pipe.init(base_param);
if (0 != ret_val.id())
{
	std::cout << "error id = " << ret_val.id() << ", str = " << ret_val.str().c_str() << "\n\n";
}
else
{
	cout << "子进程 " << argv[0] << " 创建管道成功...\n";
}
```
  * 写入数据（发送） 
  这里演示的是一个管道数组发送数据
```
const char arr_send[] = "Q";	/// 子进程约定收到 Q 就结束进程
for (int i = 0; i < pipe_count_3; i++)
{
	cout << "\n\n正在发送:";
	ret_type ret_val = pipe_arr[i].write(arr_send, sizeof(arr_send));
	if (0 != ret_val.id())
		cout << "i = " << i << ", 发送失败，id = " << ret_val.id() << "\n\n";
	else
		cout << "i = " << i << ", 发送成功，id = " << ret_val.id() << "\n\n";
}
```

  * 释放
```
pipe.uninit();
```

  * 接收
    接收需要重写函数 on_recv_data 
```
void on_recv_data(const char *pdata, const unsigned int len_recv_data)
{
	cout << "\n\n进程： " << str_process_name.c_str() << " 收到数据了: " << len_recv_data << "\n";
	cout << "进程： " << str_process_name.c_str() << "收到的数据是：";
	for (unsigned int i = 0; i < len_recv_data; i++)
	{
		cout << pdata[i];
	}
	// 约定 第一个字符为Q，退出
	if ('Q' == pdata[0])
	{
		EnterCriticalSection(&cs_proc_is_end);
		process_is_end = true;
		LeaveCriticalSection(&cs_proc_is_end);
	}
}
```
## 8. License 
  > [BSD licenses](https://opensource.org/licenses/BSD-3-Clause)

