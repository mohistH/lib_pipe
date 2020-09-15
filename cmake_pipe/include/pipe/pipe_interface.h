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
#endif //!



namespace lib_pipe
{
#ifdef __cplusplus              
	extern "C" {
#endif ///! __cplusplus



	/* 数据类型定义 */
//-------------------------------------------------------------------------------------------------------------------


	/*
	* @brief: 函数返回值， 格式：<错误代码，错误信息字符串>
				说明：
						1. 错误代码
							1.1 负数，则为函数数自定义返回的错误代码，且从 -20开始递减， -21， -22， -23，
							1.2 正数，则为调用【GetLastError】的返回值，且【错误信息字符串】为【"failure"】
						2. 错误信息字符串
							2.1 "success" - 成功， 且【错误代码】为0
							2.2 不为 "success"	- 失败，表示错误提示字符串，且【错误代码】不为0
						3. 例子
							成功 <0, "success">
							失败 <-20, "failure, XXXX">
							失败 <GetLastError(), "GetLastError">
	*/
	typedef std::pair<int, std::string> pair_int_str;

	/*
	* @brief: 函数返回值
	*/
	struct ret_type_
	{
		// 保存
		void set(const int id, std::string str)
		{
			_value = std::make_pair(id, str);
		}

		// 返回错误代码
		int id() { return _value.first; }

		// 返回错误信息字符串
		std::string str() { return _value.second; }

		void zero()
		{
			_value = std::make_pair(0, "success");
		}

		ret_type_()
		{
			zero();
		}

	private:
		pair_int_str	_value;
	};

	// 函数返回值
	typedef ret_type_ ret_type;


	/*
	* @brief: 管道需要的参数
	*/
	struct pipe_param_base_
	{
		// 名字
		std::string _name;

		// 是否创建pipe, true-创建，false-不创建
		bool		_to_create_pipe;

		void zero()
		{
			_name = std::string("");
			_to_create_pipe = false;
		}

		pipe_param_base_()
		{
			zero();
		}
	};

	// pipe参数
	typedef pipe_param_base_ pipe_param_base;


		/* 类定义 */
//-------------------------------------------------------------------------------------------------------------------

	/*
	* @brief: 接收数据,需要继承该类并实现【on_recv_data】接口
	*/
	class irecv_data
	{
	public:
		/*
		*  @ brief: 接收底层收到的数据
		*  @ const char * pdata - 收到的数据
		*  @ const unsigned int len_recv_data - 收到的数据长度
		*  @ return - void
		*/
		virtual void on_recv_data(const char *pdata, const unsigned int len_recv_data) = 0;
	};

	/*
	* @brief: 管道接口

	*  @ std::string str_pipe_name - 管道名字 ， 例： "\\\\.\\pipe\\ReadPipe"
	*  @ bool to_create_pipe - true-创建， false - 不创建，直接连
	*/
	class ipipe_interface
	{
	public:
		virtual ~ipipe_interface() {}

		/*
		*  @ brief: 初始化管道
		*  @ const pipe_param_base - 初始化参数
		*  @ irecv_data *precv_data - 接收函数对象
		*  @ return - lib_pipe::ret_type
		*/
		virtual ret_type init(const pipe_param_base& param, irecv_data *precv_data) = 0;

		/*
		*  @ brief: 向管道发送数据
		*  @ const char * pdata_send - 发送的数据内容
		*  @ const unsigned int len_data_send - 发送的数据长度
		*  @ return - lib_pipe::ret_type
		*/
		virtual ret_type write(const char *pdata_send, const unsigned int len_data_send) = 0;

		/*
		*  @ brief: 关闭
		*  @ return - lib_pipe::ret_type
		*/
		virtual ret_type uninit() = 0;

	};

#ifdef __cplusplus              
	}
#endif ///! __cplusplus


		/* 创建导出函数 */
//-------------------------------------------------------------------------------------------------------------------
		
		
	/* 
	*  @ brief: 创建支持Windows的pipe对象, 失败返回 nullptr，需要手动调用函数【release_pipe】释放
	*  @ return - pipe_interface *
			nullptr - 创建失败
	*/
	extern "C" _lib_pipe_api_ ipipe_interface *pipe_create_win();


	/* 
	*  @ brief: 释放【ipipe_interface*】对象, 将 pobj 设置为 nullptr
	*  @ ipipe_interface * pobj - 【create_pipe_win】函数创建的对象
	*  @ return - extern " _lib_pipe_api_ void
	*/
	extern "C" _lib_pipe_api_ void pipe_release(ipipe_interface* pobj);


//-------------------------------------------------------------------------------------------------------------------
	/*
	* @brief: 常用工具类
	*/
	class _lib_pipe_api_ utils
	{
	public:
		// 将std::string 转为 std::wstring
		static std::wstring str2wstr_win(const std::string &str);
		
		// 获取当前工作墓库，
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

