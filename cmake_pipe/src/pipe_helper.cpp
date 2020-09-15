
#include "pipe/pipe_helper.h"

namespace lib_pipe
{
	/*
	* @brief: 线程函数
	*/
	DWORD WINAPI thread_recv_data(LPVOID lpParam)
	{
		// 准备参数
		pipe_helper* pipe_obj	= NULL;
		pipe_obj				= reinterpret_cast<pipe_helper*>(lpParam);
		
		// 1. 没有对象
		if (NULL				== pipe_obj)
			return 0;

		

		// 2. 开始接收数据，先准备参数
		pipe_param_win& pipe_param = pipe_obj->get_pipe_param();

		// 接收缓冲
		char arr_recv_data[pipe_buf_size_4096 + 1] = { 0 };

		// 成功读取的数据
		DWORD len_real_read = 0;


		OVERLAPPED over_lapped = { 0, 0, 0, 0, pipe_param._thread._hevent};

		//if (INVALID_HANDLE_VALUE != pipe_param._thread._hevent)
		//	return 0;


		// 线程运行
		while (pipe_param._thread._is_running)	/// 通过标志位 控制线程运行
		{
			// 等待加入管道, 连接 信息写入overlap中
			// ConnectNamedPipe(pipe_param._handle, &over_lapped);
			//等待
			// WaitForSingleObject(pipe_param._thread._hevent, INFINITE);

			//if (!GetOverlappedResult(pipe_param._handle, &over_lapped, &len_real_read, TRUE))
			//{
			//	break;
			//}

			//std::cout << "\n\n进入接收while\n\n";

			// 读取管道数据 ： 阻塞
			int ret_val			= (int)ReadFile(	pipe_param._handle,	// 管道标识符
													arr_recv_data,		// 接收缓冲
													pipe_buf_size_4096, // 接收缓冲区大小
													&len_real_read,		// 真正读入长度
													NULL				// 默认传递NULL
												);
			// 2.1 读取成功
			if (1 == ret_val)
			{
				// 2.1.1 如果超过缓冲区长度，则丢弃, 准备接收下一组数据
				if (pipe_buf_size_4096 < len_real_read || 0 == len_real_read)
				{
				}
				else
				{
					// 2.1.2 长度合适，加上结束符
					arr_recv_data[len_real_read] = '\0';
					
					// 调用接收函数
					pipe_param._precv_data->on_recv_data(arr_recv_data, len_real_read);
				}
			}
			// 2.2 读取失败
			else 
			{
				// 继续读取下一个
			}

			// 避免CPU被一直忙于处理当前线程
			Sleep(2);

			memset(arr_recv_data, 0, pipe_buf_size_4096 + 1);
			// DisconnectNamedPipe(pipe_param._handle);
		}

		return 0;
	}


	/*
	*	@brief: 初始化参数和创建管道
	*/
	lib_pipe::ret_type pipe_helper::init(const pipe_param_base& param, irecv_data *precv_data)
	{
		// 参数引用
		pipe_param_base& param_base = _pipe_param._base;
		int error_id_20				= -20;

		// 定义返回值
		ret_type ret_val;

		// 1. pipe名字为空
		if (0 == param._name.length())
		{
			ret_val.set(error_id_20, std::string("failure, pipe's name is null"));//  = std::make_pair());
			_pipe_param.zero();

			return ret_val;
		}

		// 2.保存参数
		//_pipe_param._base._is_server		= is_server;
		//_pipe_param._base._name			= str_pipe_name;
		param_base = param;
		// 
		if (NULL != precv_data || nullptr != precv_data)
			_pipe_param._precv_data = precv_data;

		// 3. 检查是否有有可用的管道
		if (true == param_base._to_create_pipe)
		// if (FALSE == WaitNamedPipe(LPTSTR(param_base._name.c_str()), NMPWAIT_WAIT_FOREVER) )
		{
			// 若命名管道不存在则创建（将成为服务端）
			ret_val = create_pipe_name();
		}
		else
		{
			// 若命名管道存在则连接（将成为客户端）
			ret_val = open();
		}

		// 检查创建或者打开管道返回值, 这里，创建或者打开管道失败，则返回失败
		if (0 != ret_val.id())
		{
			_pipe_param.zero();
			return ret_val;
		}


		// 4. 创建接收线程
		pipe_param_thread_win& thread_param = _pipe_param._thread;

		// 需要接收数据，再创建线程
		if (NULL != _pipe_param._precv_data)
		{
			// 4.1 标记线程运行标记为true
			thread_param._is_running = true;

			// 创建事件
			//thread_param._hevent = CreateEvent(NULL, FALSE, FALSE, FALSE);

			// 4.2 创建线程
			thread_param._handle = CreateThread(NULL,	// 默认 NULL
												0,		// 栈大小，默认
												thread_recv_data, // 线程函数，声明格式为：DWORD WINAPI ThreadProc (PVOID pParam) ;
												this,	// 传递到线程函数中的参数
												0,		// 通常为0，但当建立的线程不马上执行时为CREATE_SUSPENDED。线程将暂停直到呼叫ResumeThread来恢复线程的执行为止。
												NULL	// 通常传 0
												);

			// 创建接收线程失败，返回错误
			if (INVALID_HANDLE_VALUE == thread_param._handle)
			{
				ret_val.set(error_id_20 - 1, std::string("failure, recv data thread created failure"));// = std::make_pair(error_id_20 - 1, std::string("failure, recv data thread created failure"));

				return ret_val;
			}
			else
			{
				// 创建接收进程成功
			}
		}
		else
		{
			// 不需要创建进程
		}

		return ret_val;
	}


	/*
	*	@brief: 向管道写入数据
	*/
	lib_pipe::ret_type pipe_helper::write(const char *pdata_send, const unsigned int len_data_send)
	{
		// 函数返回值
		ret_type ret_val;

		// 0. 数据为空
		if (NULL == pdata_send || nullptr == pdata_send)
		{
			ret_val.set(-21, std::string("failure, pdata_send is null"));
			return ret_val;
		}

		// 1. 没有连接，则返回失败
		if (!is_connected())
		{
			ret_val.set(-20, std::string("failure, pipe doesnt created"));
			return ret_val;
		}

		// 2. 向管道写入数据

		// 2.1 返回写入多少数据
		DWORD len_has_written = 0;
		int ret = (int)WriteFile(	_pipe_param._handle,	// 管道标识
									pdata_send,				// 待发送的数据
									len_data_send,			// 发送数据长度
									&len_has_written,		// 多少数据成功写入 
									NULL );					// 重叠结构： 默认传递NULL功
		// 写入失败
		if ( 0 == ret || 0 == len_has_written)
		{
			ret_val.set(GetLastError(), std::string("GetLastError"));
		}
		
		return ret_val;
	}

	/*
	*	@brief: 关闭管道
	*/
	lib_pipe::ret_type pipe_helper::uninit()
	{
		ret_type ret_val;

		// 1.若没有打开，则返回 
		if (!is_connected())
		{
			ret_val.set(-20, std::string("failure, pipe doesnt open"));
			return ret_val;
		}

		// 2. 释放资源
		pre_uninit();

		return ret_val;
	}

	/*
	*	@brief: 构造函数，初始化参数
	*/
	pipe_helper::pipe_helper()
	{
		_pipe_param.zero();
	}

	/*
	*	@brief: 检查管道是否已经连接
	*/
	bool pipe_helper::is_connected()
	{
		return (INVALID_HANDLE_VALUE == _pipe_param._handle) ? false : true;
	}

	/*
	*	@brief:创建管道
	*/
	lib_pipe::ret_type pipe_helper::create_pipe_name()
	{
		// 参数引用
		pipe_param_base& param_base = _pipe_param._base;
		pipe_param_win& param		= _pipe_param;

		// 指定返回值
		ret_type ret_val;///  = std::make_pair(0, std::string("success"));

		// 1. 已经创建，则先关闭
		if (INVALID_HANDLE_VALUE != param._handle)
		{
			DisconnectNamedPipe(_pipe_param._handle);
			CloseHandle(param._handle);
			param._handle = INVALID_HANDLE_VALUE;
		}

		// 为了统一使用Windows的Unicode的api
		TCHAR  * tc_exe_path = NULL;
#ifdef UNICODE
		std::wstring wstr = utils::str2wstr_win(_pipe_param._base._name);
		tc_exe_path = const_cast<TCHAR*>(wstr.c_str());
#else
		tc_exe_path = const_cast<TCHAR*>(_pipe_param._base._name.c_str());
#endif

		// 2. 创建命名管道
		param._handle = CreateNamedPipe(		tc_exe_path,//LPTSTR(param_base._name.c_str()),		// 管道名
												PIPE_ACCESS_DUPLEX |
												FILE_FLAG_OVERLAPPED,				// 管道通信：全双工通信
												PIPE_TYPE_MESSAGE |					// 消息：单字形式
												PIPE_READMODE_MESSAGE |				// 消息块：帧
												PIPE_WAIT,							// 同步操作在等待的时候挂起线程
												PIPE_UNLIMITED_INSTANCES,			// 创建的最大实例数量
												pipe_buf_size_4096,					// 输出缓冲区长度
												pipe_buf_size_4096,					// 输入缓冲区长度
												0,									// 管道的默认等待超时， 0表默认
												NULL);								// 使用不允许继承的一个默认描述符
		
		// 2.1 创建失败
		if ( INVALID_HANDLE_VALUE == param._handle)
		{
			ret_val.set(GetLastError(), std::string("GetLastError"));// = std::make_pair(GetLastError(), std::string("failure"));
			return ret_val;
		}

		// 3. 创建成功
		return ret_val;
	}

	/*
	*	@brief: 关闭，释放资源
	*/
	void pipe_helper::pre_uninit()
	{
		// 2. 结束接收线程
		if (NULL != _pipe_param._precv_data)
		{
			_pipe_param._thread._is_running = false;

			// 等待线程返回结果 
			WaitForSingleObject(_pipe_param._thread._handle, 1000 * 3);	/// 第一个参数： 线程的内核句柄，第二个参数：等待时间：3秒

			// 关闭内核线程句柄
			CloseHandle(_pipe_param._thread._handle);

			// 将线程句柄设置为无效
			_pipe_param._thread._handle = INVALID_HANDLE_VALUE;
		}
		else
		{
			// 没有创建线程
		}

		// 3. 关闭
		if (INVALID_HANDLE_VALUE != _pipe_param._handle)
		{
			DisconnectNamedPipe(_pipe_param._handle);
			CloseHandle(_pipe_param._handle);
			_pipe_param._handle = INVALID_HANDLE_VALUE;
		}

		if (INVALID_HANDLE_VALUE != _pipe_param._thread._hevent)
		{
			CloseHandle(_pipe_param._thread._hevent);
			_pipe_param._thread._hevent = INVALID_HANDLE_VALUE;
		}

		
		

		// 4. 重置标志
		_pipe_param.zero();
	}

	/*
	*	@brief:
	*/
	lib_pipe::ret_type pipe_helper::open()
	{
		ret_type ret_val;

		// 1. 如果已经连接，则关闭
		if (INVALID_HANDLE_VALUE != _pipe_param._handle)
		{
			DisconnectNamedPipe(_pipe_param._handle);
			CloseHandle(_pipe_param._handle);
			_pipe_param._handle = INVALID_HANDLE_VALUE;
		}

		TCHAR  * tc_exe_path = NULL;
#ifdef UNICODE
		std::wstring wstr = utils::str2wstr_win(_pipe_param._base._name);
		tc_exe_path = const_cast<TCHAR*>(wstr.c_str());
#else
		tc_exe_path = const_cast<TCHAR*>(_pipe_param._base._name.c_str());
#endif
		// 2. 打开可用的命名管道 , 并与服务器端进程进行通信  
		_pipe_param._handle = CreateFile(	tc_exe_path,
											GENERIC_READ | GENERIC_WRITE,
											FILE_SHARE_READ | FILE_SHARE_WRITE, // 共享属性
											NULL, 
											OPEN_EXISTING, 
											0, 
											NULL);

		// 2.1 创建失败
		if (INVALID_HANDLE_VALUE == _pipe_param._handle)
		{
			ret_val.set(GetLastError(), std::string("GetLastError"));
		}

		return ret_val;
	}

	/*
	*	@brief:析构函数
	*/
	pipe_helper::~pipe_helper()
	{
		// 1.若打开，则关闭
		if (is_connected())
		{
			pre_uninit();
		}
		else
		{
			// 2. 没有打开管道	
		}
	}

}/// !lib_pipe

