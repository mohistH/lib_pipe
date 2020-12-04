

#include "pipe/pipe_helper.h"
#ifdef compiler_is_vs

namespace lib_pipe
{

	// 
	// @brief:接收管道数据线程函数
	// 
	DWORD WINAPI pipe_helper::thread_recv_data(LPVOID lpParam)
	{
		// 准备参数
		pipe_helper* pipe_obj = NULL;
		pipe_obj = reinterpret_cast<pipe_helper*>(lpParam);

		// 1. 没有对象
		if (NULL == pipe_obj)
			return 0;



		// 2. 开始接收数据，先准备参数
		pipe_param_win& pipe_param = pipe_obj->get_pipe_param();

		// 接收缓冲
		char arr_recv_data[pipe_buf_size_4096 + 1] = { 0 };

		// 成功读取的数据
		DWORD len_real_read = 0;


		OVERLAPPED over_lapped = { 0, 0, 0, 0, pipe_param._thread._hevent };

		if (INVALID_HANDLE_VALUE != pipe_param._thread._hevent)
			return 0;

		pipe_obj->log("\n接收线程，1111即将进入while\n\n");
		// 线程运行
		while (pipe_obj->get_thread_recv_is_running())	// 通过标志位 控制线程运行
		{
			pipe_obj->log("\n接收线程，2222等待加入管道, 连接 信息写入overlap中\n\n");

			 // 等待加入管道, 连接 信息写入overlap中
			 ConnectNamedPipe(pipe_param._handle, &over_lapped);

			 pipe_obj->log("\n接收线程，等待, 3333INFINITE:一直等待\n\n");
			 // 等待, INFINITE:一直等待
			 WaitForSingleObject(pipe_param._thread._hevent, INFINITE);

			 pipe_obj->log("\n接收线程，等待, 4444I检测io是否完成,完成的话就停止\n\n");
			 // 检测io是否完成,完成的话就停止
			if (!GetOverlappedResult(pipe_param._handle, &over_lapped, &len_real_read, TRUE))
			{
				break;
			}


			pipe_obj->log("\n接收线程，等待, 5555读取管道数据 ： 阻塞\n\n");
			// 读取管道数据 ： 阻塞
			int ret_val = (int)ReadFile(pipe_param._handle,	// 管道标识符
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
					pipe_obj->log("\n接收线程，等待, 6666如果超过缓冲区长度，则丢弃, 准备接收下一组数据\n\n");
				}
				else
				{

					pipe_obj->log("\n接收线程，等待, 7777 长度合适，加上结束符,调用接收函数\n\n");

					// 2.1.2 长度合适，加上结束符
					arr_recv_data[len_real_read] = '\0';

					// 调用接收函数
					pipe_param._precv_data->on_recv_data(arr_recv_data, len_real_read);
				}

				memset(arr_recv_data, 0, pipe_buf_size_4096 + 1);
			}
			// 2.2 读取失败
			else
			{
				// 继续读取下一个
				pipe_obj->log("\n接收线程，等待, 8888 读取失败\n\n");
			}

			// 避免CPU被一直忙于处理当前线程，考虑改为信号量
			Sleep(2);

			pipe_obj->log("\n接收线程，等待, 9999 断开管道\n\n");
			// 断开管道
			DisconnectNamedPipe(pipe_param._handle);
		}

		return 0;
	}




	// 
	// @brief:获取接收线程运行状态
	// 
	bool pipe_helper::get_thread_recv_is_running()
	{
		return _pipe_param._thread._is_running;
	}

	// 
	// @brief:设置接收线程运行状态
	// 
	void pipe_helper::set_thread_recv_is_running(const bool val)
	{
		_pipe_param._thread._is_running = val;
	}

	//
	//	@brief: 初始化参数和创建管道
	//
	int pipe_helper::init(const pipe_param_base& param, irecv_data *precv_data)
	{
		// 参数引用
		pipe_param_base& param_base = _pipe_param._base;

		// 定义返回值
		int ret_val = 0;

		// 1. 管道名字为空
		if (0 == param._name.length())
		{
			ret_val = -2;
			_pipe_param.zero();

			return ret_val;
		}

		// 2.保存参数
		//_pipe_param._base._is_server		= is_server;
		//_pipe_param._base._name			= str_pipe_name;
		param_base = param;

		// 需要接收
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
		if (0 != ret_val)
		{
			_pipe_param.zero();
			return ret_val;
		}


		// 4. 创建接收线程
		pipe_param_thread_win& thread_param = _pipe_param._thread;

		// 需要接收数据，再创建线程
		if (NULL != _pipe_param._precv_data)
		{
			// 创建事件
			//
			// 参数1 lpEventAttributes    权限,一般NULL就是默认权限
			// 参数2 bManualReset        TRUE代表手动重置,FALSE自动重置
			// 参数3 bInitialState       TRUE代表可触发, FALSE非触发(阻塞)
			// 参数4 lpName              一个对象的名称,跨进程寻址,一般NULL
			//
			thread_param._hevent = CreateEvent(NULL, FALSE, FALSE, FALSE);

			if (INVALID_HANDLE_VALUE == thread_param._hevent)
			{
				ret_val = GetLastError();
				_pipe_param.zero();
				return ret_val;
			}


			// 4.1 标记线程运行标记为true
				set_thread_recv_is_running(true);


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

				//ret_val.set(error_id_20 - 1, std::string("failure, recv data thread created failure"));// = std::make_pair(error_id_20 - 1, std::string("failure, recv data thread created failure"));
				_pipe_param.zero();
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


	//
	//	@brief: 向管道写入数据
	//
	int pipe_helper::write(const char *pdata_send, const unsigned int len_data_send, unsigned int& len_written)
	{
		// 函数返回值
		int ret_val = 0;

		// 0. 数据为空
		if (NULL == pdata_send || nullptr == pdata_send)
		{
			ret_val = -2;
			len_written = 0;
			// ret_val.set(-21, std::string("failure, pdata_send is null"));
			return ret_val;
		}

		// 1. 没有连接，则返回失败
		if (!is_connected())
		{
			ret_val			= -3;
			len_written = 0;
			// ret_val.set(-20, std::string("failure, pipe doesnt created"));
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
			//ret_val.set(GetLastError(), std::string("GetLastError"));
			ret_val = GetLastError();
			len_written = 0;
		}
		// 写入成功
		else
		{
			ret_val = 0;
			len_written = len_has_written;
		}
		
		return ret_val;
	}

	//
	//	@brief: 关闭管道
	//
	int pipe_helper::uninit()
	{
		int ret_val = 0;

		// 1.若没有打开，则返回 
		if (!is_connected())
		{
			ret_val = -2;
			// ret_val.set(-20, std::string("failure, pipe doesnt open"));
			return ret_val;
		}

		// 2. 释放资源
		pre_uninit();

		return ret_val;
	}

	// 
	// @brief: 构造函数，初始化参数
	// 
	pipe_helper::pipe_helper()
	{
		_pipe_param.zero();
	}

	//
	//	@brief: 检查管道是否已经连接
	//
	bool pipe_helper::is_connected()
	{
		return (INVALID_HANDLE_VALUE == _pipe_param._handle) ? false : true;
	}


	// 
	// @brief: 调试日输出
	// 
	void pipe_helper::log(const char *pdata, ...)
	{
#ifdef is_debug
		va_list ap;
		va_start(ap, pdata);

		// 1、计算得到长度
		//---------------------------------------------------
		// 返回 成功写入的字符个数
		int count_write = snprintf(NULL, 0, pdata, ap);
		va_end(ap);

		// 长度为空
		if (0 >= count_write)
			return;

		count_write++;

		// 2、构造字符串再输出
		//---------------------------------------------------
		va_start(ap, pdata);

		char *pbuf_out = NULL;
		pbuf_out = (char *)malloc(count_write);
		if (NULL == pbuf_out)
		{
			va_end(ap);
			return;
		}

		// 构造输出
		vsnprintf(pbuf_out, count_write, pdata, ap);
		// 释放空间
		va_end(ap);

		// 输出结果
		std::cout << pbuf_out;

		// 释放内存空间
		free(pbuf_out);
		pbuf_out = NULL;
#endif // 
	}

	// 
	// @brief:创建管道
	// 
	int pipe_helper::create_pipe_name()
	{
		// 参数引用
		pipe_param_base& param_base = _pipe_param._base;
		pipe_param_win& param		= _pipe_param;

		// 指定返回值
		int ret_val = 0;///  = std::make_pair(0, std::string("success"));

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
			ret_val = GetLastError(); // ret_val.set(GetLastError(), std::string("GetLastError"));// = std::make_pair(GetLastError(), std::string("failure"));
			return ret_val;
		}

		// 3. 创建成功，直接返回
		return ret_val;
	}

	//
	//	@brief: 关闭，释放资源
	//
	void pipe_helper::pre_uninit()
	{
		try
		{
			// 2. 结束接收线程
			if (NULL != _pipe_param._precv_data)
			{
				set_thread_recv_is_running(false);

				// 创建接收线程句柄不为空，再关闭线程
				if (INVALID_HANDLE_VALUE != _pipe_param._thread._handle)
				{
					// 等待线程返回结果 
					WaitForSingleObject(_pipe_param._thread._handle, 1000 * 3);	/// 第一个参数： 线程的内核句柄，第二个参数：等待时间：3秒

					// 关闭内核线程句柄
					CloseHandle(_pipe_param._thread._handle);

					// 将线程句柄设置为无效
					_pipe_param._thread._handle = INVALID_HANDLE_VALUE;
				}
				// 接收线程句柄为空，创建接收线程失败，无需关闭
				else
				{
					;
				}
			}
			else
			{
				// 不需要接收数据，即没有创建线程
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

			// 4. 重置管道参数
			_pipe_param.zero();
		}
		catch (...)
		{
			// 异常出现了 ,吞下异常。
		}
	}

	//
	//	@brief:打开管道
	//
	int pipe_helper::open()
	{
		int ret_val = 0;

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
			//ret_val.set(GetLastError(), std::string("GetLastError"));
			ret_val = GetLastError();
		}

		return ret_val;
	}

	//
	//	@brief:析构函数
	//
	pipe_helper::~pipe_helper()
	{
		// 1.若打开，则关闭
		if (is_connected())
		{
			// 如果出现异常，该函数将吞下异常，不会继续传播异常
			pre_uninit();
		}
		else
		{
			// 2. 没有打开管道	
		}
	}

}/// !lib_pipe



#endif // !compiler_is_vs