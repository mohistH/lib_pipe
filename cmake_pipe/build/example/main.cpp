// demo_create.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include <iostream>
#include <windows.h>
using namespace std;
#include "pipe/pipe_interface.h"
using namespace lib_pipe;


// 结束线程和主线互斥使用，保护临界区【process_is_end】
CRITICAL_SECTION cs_proc_is_end;

// 临界区,用于结束进程，避免资源没有释放。 
bool process_is_end = false;


// 保存 main传进来的第一个参数，这样就可以在接收数据的函数中使用该项输出了
std::string str_process_name;

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// 因为要接收数据，需要继承 irecv_data 并实现函数 on_recv_data
class pipe_win : public irecv_data
{
public:
	pipe_win()
	{
		_ppipe = pipe_create_win();
	}

	~pipe_win()
	{
		pipe_release(_ppipe);
	}

	//-------------------------------------------------------------------------------------------------------------------

	/*
	*  @ brief: 初始化管道
	*  @ const pipe_param_base - 初始化参数
	*  @ irecv_data *precv_data - 接收函数对象
	*  @ return - lib_pipe::ret_type
	*/
	ret_type init(const pipe_param_base& param)
	{
		return _ppipe->init(param, this);
	}

	/*
	*  @ brief: 向管道发送数据
	*  @ const char * pdata_send - 发送的数据内容
	*  @ const unsigned int len_data_send - 发送的数据长度
	*  @ return - lib_pipe::ret_type
	*/
	ret_type write(const char *pdata_send, const unsigned int len_data_send)
	{
		return _ppipe->write(pdata_send, len_data_send);
	}

	/*
	*  @ brief: 关闭
	*  @ return - lib_pipe::ret_type
	*/
	ret_type uninit()
	{
		return _ppipe->uninit();
	}

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

private:
	ipipe_interface * _ppipe = nullptr;
};


//-------------------------------------------------------------------------------------------------------------------


int main(int argc, char *argv[])
{
	// 初始化互斥对象
	InitializeCriticalSection(&cs_proc_is_end);

	// 保存第一个参数：暂定位进程的代号
	str_process_name = std::string(argv[0]);

	std::cout << "hello world" << std::endl;
	std::cout << "1 = " << argv[0] << endl;

	cout << "子进程 " << argv[0] << " 创建管道中...\n";
	// 开始创建管道
	pipe_win pipe;// [pipe_count_3];

	// 参数
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


	// 避免一直while, 15秒到，还没收到消息，退出while
	int index = 0;
	while (true)
	{
		bool is_end = false;
		// 进入临界区
		EnterCriticalSection(&cs_proc_is_end);
		is_end = process_is_end;
		// 离开临界区，
		LeaveCriticalSection(&cs_proc_is_end);

		if (is_end)
		{
			cout << "结束while， 即将释放资源\n\n";
			break;
		}
		else
		{
			if (15 < index++)
			{
				cout << "子进程" << argv[0] << ", 15s 到了， 还没收，退出while，即将释放资源...\n\n"; 
				break;
			}

			// 睡眠1秒，等待结果
			Sleep(1000 * 1);
			cout << "子进程" << argv[0] << "睡眠" << index + 1 << "秒...\n\n";
		}
	}

	// 释放空间
	pipe.uninit();

	cout << "资源释放完毕\n\n";

	system("pause");
    return 0;
}

