// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>


#include "../lib_pipe/pipe_interface.h"
using namespace std;
using namespace lib_pipe;



void new_pro(const int index)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	std::cout << "main_thread" << std::endl;

	std::string exe = "C:\\Users\\cube\\Desktop\\demo_create\\Debug\\demo_create.exe";

	//WCHAR ssssss(exe.c_str());
	std::string exe_path = "C:\\Users\\cube\\Desktop\\demo_create\\Debug";
	std::string exe_name = "demo_create.exe";
	std::string param_str2;

	std::wstring wexe_path(L"C:\\Users\\cube\\Desktop\\demo_create\\Debug");
	std::wstring wexe_name(L"demo_create.exe");

	std::wstring param_wstr;
	std::wstring param_wstr_2;

	switch (index)
	{
	case 0:
		{
		param_wstr = wexe_path + L"\\1\\" + wexe_name;
		param_wstr_2 = L"A";
		}
		break;

	case 1:
	{
		param_wstr = wexe_path + L"\\2\\" + wexe_name;
		param_wstr_2 = L"B";
	}
	break;

	case 2:
	{
		param_wstr = wexe_path + L"\\3\\" + wexe_name;
		param_wstr_2 = L"C";
	}
	break;

	}


	LPCWSTR lp_p1	= param_wstr.c_str();
	LPWSTR lp_p2	= LPWSTR(param_wstr_2.c_str());


	//创建一个新进程
	//if (CreateProcess(TEXT("C:\\Users\\cube\\Desktop\\demo_create\\Debug\\demo_create.exe"),
	if (CreateProcess(lp_p1,
		lp_p2,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE, // CREATE_NEW_CONSOLE
		NULL,
		NULL,
		&si,
		&pi)

		)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

	}
	else
	{
		cout << "\n\n创建失败\n\n";
		cout << "id = " << GetLastError() << "\n\n\n";
	}
}
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
		cout << "\n\n收到数据了: " << len_recv_data << "\n";
	}

private:
	ipipe_interface * _ppipe = nullptr;
};
//-------------------------------------------------------------------------------------------------------------------


// 入口函数
int main()
{
	// 管道数量
	const int pipe_count_3 = 1;

	// 1. 创建管道
	pipe_win pipe_arr[pipe_count_3];

	// 参数
	pipe_param_base base_param;
	base_param._to_create_pipe = true;
	base_param._name = std::string("\\\\.\\pipe\\ReadPipe");

	for (int i = 0; i < pipe_count_3; i++)
	{
		ret_type ret_val = pipe_arr[i].init(base_param);

		if (0 != ret_val.id())
		{
			std::cout << "error id = " << ret_val.id() << ", str = " << ret_val.str().c_str() << "\n\n";
		}
	}

	// 
	cout << "主线程创建管道结束 \n\n";

	// 2. 创建进程
	for (int i = 0; i < pipe_count_3; i++)
		new_pro(i);
	
	cout << "主线程创建进程结束， 即将进入等待3秒：将子进程启动 \n\n";
	Sleep(1000 * 3);

	// 3. 尝试发送数据给子线程
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


	Sleep(2);

	// 4. 释放资源
	for (int i = 0; i < pipe_count_3; i++)
		pipe_arr[i].uninit();

	cout << "主线程释放自己创建的管道资源结束 \n\n";
	cout << "\n\n\n\n";
	system("pause");
	return 0;
}
