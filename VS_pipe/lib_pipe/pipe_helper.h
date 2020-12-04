#pragma once
#include "pipe_interface.h"

#ifdef compiler_is_vs
#include <Windows.h>
#include <iostream>


// 用作调试用，
#define is_debug

#ifdef is_debug
#include <iostream>
// 使用va_start需要的头文件
#include <stdarg.h>

#endif //!is_debug

namespace lib_pipe
{
	// 数据定义
//-------------------------------------------------------------------------------------------------------------------
	

	//
	// @brief: 一些枚举参数
	//
	enum 
	{
		// 管道缓冲区大小
		pipe_buf_size_4096 = 4096,
	};


	
	//
	// @brief: 线程参数
	//
	struct pipe_param_thread_win_
	{
		// 线程句柄
		HANDLE		_handle;

		// 是否运行
		bool		_is_running;

		HANDLE		_hevent;

		void zero()
		{
			_handle			= INVALID_HANDLE_VALUE;
			_is_running		= false;
			_hevent			= INVALID_HANDLE_VALUE;
		}

		pipe_param_thread_win_()
		{
			zero();
		}
	};

	//
	// @brief: 线程参数
	//
	typedef pipe_param_thread_win_ pipe_param_thread_win;

	

	//
	// @brief: pipe类需要的参数
	//
	struct  pipe_param_win_
	{
		// pipe参数
		pipe_param_base			_base;

		// 线程参数
		pipe_param_thread_win	_thread;

		// 线程收到数据将数据发给谁
		irecv_data				*_precv_data;

		// 管道标识符
		HANDLE					_handle;

		void zero()
		{
			_base.zero();
			_thread.zero();
			_precv_data			= NULL;
			_handle				= INVALID_HANDLE_VALUE;
		}

		pipe_param_win_()
		{
			zero();
		}
	};
	
	//
	// @brief: 将类需要的参数封装到结构体
	//
	typedef pipe_param_win_	pipe_param_win;


	// 类定义
//-------------------------------------------------------------------------------------------------------------------

	


	//
	// @brief: 管道类
	//
	class pipe_helper : public ipipe_interface
	{
	public:
		//
		// @brief: 构造函数
		//
		pipe_helper();

		//
		// @brief: 析构函数
		//
		virtual ~pipe_helper();


		//  
		//  @ brief: 初始化管道
		//  @ const pipe_param_base - 初始化参数
		//  @ irecv_data *precv_data - 接收函数对象,若不需要接收代码，则传递nullptr 或者 NULL
		//  @ return - int	
		//			返回值 X:
		//			0 - 初始化成功
		//			X > 0 - 初始化失败，X为调用GetLastError()函数的结果，
		int init(const pipe_param_base& param, irecv_data *precv_data);


		// 
		// @ brief: 向管道发送数据
		// @ const char * pdata_send - 发送的数据内容
		// @ const unsigned int len_data_send - 发送的数据长度
		// @ const unsigned int& len_written - 已经发送的数据长度
		// @ return - int
		//			返回值 X：
		//			X = 0 - 管道写入数据成功，且len_written与len_data_send相等
		//			X > 0 - 管道写入数据失败，X为调用GetLastError函数返回结果，且len_written值为0
		int write(const char *pdata_send, const unsigned int len_data_send, unsigned int& len_written);

		// 
		// @ brief: 关闭
		// @ return - int
		//			0 - 关闭成功
		//			-2 - 关闭失败，管道没有打开。
		int uninit();

		// 
		// @ brief: 返回初始化参数，线程调用使用
		// @ return - lib_pipe::pipe_param_win& 	
		//
		pipe_param_win& get_pipe_param() { return _pipe_param; }


		//
		// @brief:  接收管道数据线程函数
		//
		static DWORD WINAPI thread_recv_data(LPVOID lpParam);


		//
		// @brief: 读写接收数据线程标志
		//
		bool get_thread_recv_is_running();
		void set_thread_recv_is_running(const bool val);

		//
		// @brief: 输出调试信息
		//
		void log(const char *pdata, ...);
	private:

		//
		// @brief: 屏蔽拷贝构造函数
		//
		pipe_helper(const pipe_helper& instance) {}

		//
		// @brief: 屏蔽运算符=
		//
		pipe_helper & operator = (const pipe_helper& instance) { return *this; }

		//
		// @brief:  管道连接状态
		// @return-bool 
		//			true-打开
		//			false-关闭
		//
		bool is_connected();


	private:

		//  
		//  @ brief: 创建管道
		//  @ return - int
		//			0 - 创建成功
		//			X - 创建失败，X为调用GetLastError()函数的返回值
		int create_pipe_name();

		// 
		// @ brief: 关闭. 释放资源
		// @ return - void
		//	
		void pre_uninit();

		
		// 
		// @ brief: 打开管道
		// @ return - int
		//			0 - 创建成功
		//			X - 创建失败，X为调用GetLastError()函数的返回值
		int open();

	private:
		// 管道参数
		pipe_param_win	_pipe_param;
	};
}



#endif // !compiler_is_vs