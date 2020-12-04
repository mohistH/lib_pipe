#pragma once
#include <string>
#include <utility>


// windows
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32)

//----------------------------------------------------------------------
	// 定义 .dll  导出符号
	#ifndef _lib_pipe_api_
		#define _lib_pipe_api_	__declspec(dllexport)
	#else
		#define _lib_pipe_api_	__declspec(dllimport)
	#endif /// !_lib_pipe_api_
//----------------------------------------------------------------------
#elif defined(_unix_) || defined(_linux_)
//----------------------------------------------------------------------
	// 定义 .dll  导出符号
	#ifndef _lib_pipe_api_
		#define _lib_pipe_api_	__attribute__((visibility ("default")))
	#endif /// !_lib_pipe_api_
//----------------------------------------------------------------------
#endif /// !



// _UNICODE用于C运行库。
#ifdef _UNICODE
	#ifndef UNICODE
		#define UNICODE
	#endif ///! UNICODE
#endif /// !_UNICODE

// UNICODE用于WINAPI
#ifdef UNICODE
	#ifndef _UNICODE
		#define _UNICODE
	#endif///!_UNICODE
#endif ///!UNICODE

#if defined(__clang__) || defined(__GNUC__)

#elif defined(_MSC_VER)
	#ifndef compiler_is_vs
		#define compiler_is_vs 1
	#else 
		#define compiler_is_vs 0
	#endif /// !os_is_win
#endif /// 



#ifdef compiler_is_vs
#include <windows.h>
#endif //! compiler_is_vs




namespace lib_pipe
{
#ifdef __cplusplus              
	extern "C" {
#endif ///! __cplusplus


	//
	// @brief: 管道通信初始化需要的参数
	//
	struct pipe_param_base_
	{
		// 名字
		std::string _name;

		// 是否创建pipe, true-创建，false-不创建
		bool		_to_create_pipe;

		void zero()
		{
			_name			= std::string("");
			_to_create_pipe = false;
		}

		pipe_param_base_()
		{
			zero();
		}
	};

	// pipe参数
	typedef pipe_param_base_ pipe_param_base;


	// 类定义 开始 
//-------------------------------------------------------------------------------------------------------------------

	//
	// @brief: 接收数据,需要继承该类并实现【on_recv_data】接口
	//
	class irecv_data
	{
	public:
		//  
		//  @ brief: 接收底层收到的数据
		//  @ const char * pdata - 收到的数据
		//  @ const unsigned int len_recv_data - 收到的数据长度
		//  @ return - void
		virtual void on_recv_data(const char *pdata, const unsigned int len_recv_data) = 0;
	};


//-------------------------------------------------------------------------------------------------------------------
	
	// 
	// @brief: 管道通信接口类，包括初始化，打开发送，关闭接口
	// 
	class ipipe_interface
	{
	public:
		virtual ~ipipe_interface() {}

		//  
		//  @ brief: 初始化管道
		//  @ const pipe_param_base - 初始化参数
		//  @ irecv_data *precv_data - 接收函数对象,若不需要接收代码，则传递nullptr 或者 NULL
		//  @ return - int	
		//			返回值 X:
		//			0 - 初始化成功
		//			X > 0 - 初始化失败，X为调用GetLastError函数或者来自errno的结果
		virtual int init(const pipe_param_base& param, irecv_data *precv_data) = 0;


		// 
		// @ brief: 向管道发送数据
		// @ const char * pdata_send - 发送的数据内容
		// @ const unsigned int len_data_send - 发送的数据长度
		// @ const unsigned int& len_written - 已经发送的数据长度
		// @ return - int
		//			返回值 X：
		//			X = 0 - 管道写入数据成功，且len_written与len_data_send相等
		//			X > 0 - 管道写入数据失败，X为调用GetLastError函数或者来自errno的结果，且len_written值为0
		virtual  int write(const char *pdata_send, const unsigned int len_data_send, unsigned int& len_written) = 0;

		// 
		// @ brief: 关闭
		// @ return - int
		//			0 - 关闭成功
		//			-2 - 关闭失败，管道没有打开。
		virtual int uninit() = 0;

	};

#ifdef __cplusplus              
	}
#endif ///! __cplusplus


	// 创建导出函数 
//-------------------------------------------------------------------------------------------------------------------
		
		
	// 
	// @ brief: 创建管道通信对象, 失败返回NULL(为了兼容低版本编译器，否则，返回nullptr)，需要手动调用函数【release_pipe】释放
	// @ return - pipe_interface *
	//			NULL - 创建失败
	//
	extern "C" _lib_pipe_api_ ipipe_interface *pipe_create();

	// 
	// @ brief: 释放【ipipe_interface*】对象, 将 pobj 设置为 NULL(为了兼容低版本编译器，否则，设置为nullptr)
	// @ ipipe_interface * pobj - 【create_pipe_win】函数创建的对象
	// @ return - extern " _lib_pipe_api_ void
	//
	extern "C" _lib_pipe_api_ void pipe_release(ipipe_interface* pobj);


//-------------------------------------------------------------------------------------------------------------------
	// 
	// @brief: 常用工具类
	// 
	class _lib_pipe_api_ utils
	{
	public:
		// 将std::string 转为 std::wstring
		static std::wstring str2wstr_win(const std::string &str);
		
		// 获取当前工作目录库，
		static std::string get_cwd();

#ifdef compiler_is_vs
		static void tchar2char(const TCHAR *tchar, char * _char);
#endif/// !complier_is_vs

	private:
		utils(){}
		virtual ~utils(){}
		utils &operator = (const utils& instance) { return *this; }
		utils(const utils& instance) {}
	};
}/// !lib_pipe

