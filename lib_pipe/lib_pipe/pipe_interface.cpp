
#include "stdafx.h"
#include "pipe_interface.h"

#ifdef compiler_is_vs
#include "pipe_helper.h"
#endif //!os_is_win

namespace lib_pipe
{
	/*
	*	@brief: 创建windows pipe接口
	*/
	ipipe_interface *pipe_create_win()
	{
		ipipe_interface* pobj = nullptr;
		pobj = new(std::nothrow) pipe_helper;

		return pobj;
	}

	/* 
	*	@brief:
	*/
	void pipe_release(ipipe_interface* pobj)
	{
		if (NULL != pobj || nullptr != pobj)
		{
			delete pobj;
			pobj = nullptr;
		}
	}

	/*
	*	@brief: 常用工具类-std::string转std::wstring
	*/
	std::wstring utils::str2wstr_win(const std::string &str)
	{
		if (str.empty())
		{
			return std::wstring();
		}

		int size = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring ret = std::wstring(size, 0);
		MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &ret[0], size);

		return ret;
	}

	/*
	*	@brief:
	*/
	std::string utils::get_cwd()
	{
		const int arr_size_256 = 256;
		char path[arr_size_256] = { 0 };

#ifdef compiler_is_vs
	#ifdef UNICODE
			TCHAR tpath[arr_size_256] = { 0 };
			GetModuleFileName(NULL, tpath, arr_size_256);
			tchar2char(tpath, path);
	#else
			GetModuleFileName(NULL, path, arr_size_256);
	#endif ///	
#endif
		// win:path = XX\xx\xx\x\F.exe
		std::string str_ret_val(path);
		int last_pos = str_ret_val.find_last_of('\\');

		// 找不到\\，直接返回
		if (-1 == last_pos)
		{
		}
		else
		{
			// 找到了，去除最后的F.exe
			str_ret_val = str_ret_val.substr(0, last_pos);
		}

		return str_ret_val;
	}

#ifdef compiler_is_vs
	/*
	*	@brief:
	*/
	void utils::tchar2char(const TCHAR *ptchar_arr, char * pchar_arr)
	{
		if (NULL == ptchar_arr || nullptr == ptchar_arr)
			return;

		if (NULL == pchar_arr || nullptr == pchar_arr)
			return;

		int iLength = 0;
		//获取字节长度   
		iLength = WideCharToMultiByte(CP_ACP, 0, ptchar_arr, -1, NULL, 0, NULL, NULL);

		//将tchar值赋给_char    
		WideCharToMultiByte(CP_ACP, 0, ptchar_arr, -1, pchar_arr, iLength, NULL, NULL);
	}

#endif /// !compiler_is_vs

}