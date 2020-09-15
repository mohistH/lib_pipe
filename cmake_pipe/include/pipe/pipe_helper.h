#pragma once
#include <Windows.h>
#include <iostream>
#include "pipe_interface.h"

namespace lib_pipe
{
	// 数据定义
//-------------------------------------------------------------------------------------------------------------------
	

	/*
	* @brief: 一些枚举参数
	*/
	enum 
	{
		// 管道缓冲区大小
		pipe_buf_size_4096 = 4096,
	};

	
	/*
	* @brief: 线程参数
	*/
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
	// 线程参数
	typedef pipe_param_thread_win_ pipe_param_thread_win;

	

	/*
	* @brief:  pipe 需要的参数
	*/
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
	// 将类需要的参数封装到结构体,
	typedef pipe_param_win_	pipe_param_win;


	// 类定义
//-------------------------------------------------------------------------------------------------------------------

	


	// 管道类
	class pipe_helper : public ipipe_interface
	{
	public:
		/*
		* @brief: 构造函数
		*/
		pipe_helper();

		/*
		* @brief: 析构函数
		*/
		virtual ~pipe_helper();


		/* 
		*  @ brief: 初始化管道
		*  @ const pipe_param_base - 初始化参数
		*  @ irecv_data *precv_data - 接收函数对象
		*  @ return - lib_pipe::ret_type
		*/
		ret_type init(const pipe_param_base& param, irecv_data *precv_data);

		/* 
		*  @ brief: 向管道发送数据
		*  @ const char * pdata_send - 发送的数据内容
		*  @ const unsigned int len_data_send - 发送的数据长度
		*  @ return - lib_pipe::ret_type
		*/
		ret_type write(const char *pdata_send, const unsigned int len_data_send);

		/* 
		*  @ brief: 关闭
		*  @ return - lib_pipe::ret_type
		*/
		ret_type uninit();

		/* 
		*  @ brief: 返回初始化参数，线程调用使用
		*  @ return - lib_pipe::pipe_param_win& 
				
		*/
		pipe_param_win& get_pipe_param() { return _pipe_param; }

	private:

		/*
		* @brief: 屏蔽拷贝构造函数
		*/ 
		pipe_helper(const pipe_helper& instance) {}

		/*
		* @brief: 屏蔽运算符=
		*/
		pipe_helper & operator = (const pipe_helper& instance) { return *this; }

		/* 
		*  @ brief: 管道是连接
		*  @ return - bool
				
		*/
		bool is_connected();

	private:
		/*
		* @brief: 创建管道
		*/
		ret_type create_pipe_name();

		/* 
		*  @ brief: 关闭. 释放资源
		*  @ return - void
				
		*/
		void pre_uninit();

		
		/* 
		*  @ brief: 打开管道
		*  @ return - lib_pipe::ret_type
				
		*/
		ret_type open();

	private:
		// 管道参数
		pipe_param_win	_pipe_param;

	};
}

