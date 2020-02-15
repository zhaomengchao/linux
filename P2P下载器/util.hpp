#pragma once
#ifdef _WIN32
#include<iostream>
#include<WS2tcpip.h>
#include<Iphlpapi.h>//获取网卡信息的头文件
#include<vector>
#include<fstream>
#include<boost/filesystem.hpp>
#pragma comment(lib, "Iphlpapi.lib")//获取网卡信息的库
#pragma comment(lib, "ws2_32.lib")//windows下的socket库
#else
//linux头文件
#endif

class FileUtil
{
public:
	static bool Write(const std::string &name, const std::string &body, int64_t offset = 0)//offset为偏移量
	{
		//注意文本操作以二进制操作的不同
		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "wb+");//以二进制形式打开文件
		if (fp == NULL)
		{
			std::cerr << "打开文件失败\n";
		}
		size_t ret = fwrite(body.c_str(), 1, body.size(), fp);
		if (ret != body.size())
		{
			std::cerr << "向文件写入数据失败\n";
			fclose(fp);
			return false;
		}

		fclose(fp);
		return true;
		//std::ofstream ofs(name);
		//if (ofs.is_open() == false)
		//{
		//	std::cerr << "打开文件失败\n";
		//	return false;
		//}
		//ofs.seekp(offset, std::ios::binary);//从读写位置跳转到相对于文件起始位置开始偏移offset的偏移量
		//ofs.write(&body[0], body.size());
		//if (ofs.good() == false)
		//{
		//	std::cerr << "向文件写入数据失败\n";
		//	ofs.close();
		//	return false;
		//}
		//ofs.close();
		
	}
	// 指针参数表示这是一个输出型参数
	// const& 表示这是一个输入型参数
	// & 表示这是一个输入输出型参数
	static bool Read(const std::string& name, std::string* body)
	{
		int64_t filesize = boost::filesystem::file_size(name);
		body->resize(filesize);
		std::cout << "读取文件数据：" << name << "size" << filesize << "\n";
		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "rb+");
		if (fp == NULL)
		{
			std::cerr << "打开文件失败\n";
		}
		size_t ret = fread(&(*body)[0].c_str(), 1, filesize, fp);
		if (ret != filesize)
		{
			std::cerr << "从文件读取数据失败\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		//std::ifstream ifs(name); //实例化
		//if (ifs.is_open() == false)
		//{
		//	std::cerr << "打开文件失败\n";
		//	return false;
		//} 
		//int64_t filesize = boost::filesystem::file_size(name);  // 通过文件名获取文件大小
		//body->resize(filesize);  // 给body分配文件空间

		////std::cout << "要读取的文件大小：" << name << ":" << filesize << std::endl;

		//ifs.read(&(*body)[0], filesize);  // 读取文件大小
		///*if (ifs.good() == false)
		//{
		//std::cerr << "读取文件数据失败\n";
		//std::cout << *body << std::endl;
		//ifs.close();
		//return false;
		//}*/
		//ifs.close();
		return true;
	}

};

class Adapter//定义一个网卡类
{
public:
	_Uint32t _ip_addr;//网卡上IP地址
	_Uint32t _mask_addr;//网卡上的子网掩码
};
class AdapterUtil//工具类网络适配器
{
public:
#ifdef _WIN32
	static bool GetAllAdapter(std::vector<Adapter> *list)// windows下获取的网卡信息实现
	{
		//IP_ADAPTER_INFO 是一个存放信息的结构体
		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO();
		//GetAdaptersInfo window下获取网卡信息的接口--网卡信息有可能有多个，因此传入一个指针
		//获取网卡信息可能会失败，因为空间不足，因此有一个输出型参数，用于向用户返回所有网卡信息空间占用大小
		int64_t all_adapter_size = sizeof(IP_ADAPTER_INFO);
		// 接收成功与否的返回 值
		int ret = GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
		if (ret == ERROR_BUFFER_OVERFLOW)
		{// 这个错误表示缓冲区空间不足
			//重新给指针申请空间
			delete p_adapters;//先释放
			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];
			GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
		}
		while (p_adapters) 
		{
			Adapter adapter;
			//将一个字符串点分十进制IP地址到网络地址IP地址
			inet_pton(AF_INET, p_adapters->IpAddressList.IpAddress.String, &adapter._ip_addr);
			inet_pton(AF_INET, p_adapters->IpAddressList.IpMask.String, &adapter._mask_addr);
			if (adapter._ip_addr != 0)//有些网卡信息没有启用
			{
				list->push_back(adapter);//将网卡信息添加到vector中返回给用户
				//if (p_adapters->IpAddressList.IpAddress.String != "0.0.0.0")
				/*std::cout << "网卡名称：" << p_adapters->AdapterName << std::endl;
				std::cout << "网卡描述：" << p_adapters->Description << std::endl;
				std::cout << "IP地址：" << p_adapters->IpAddressList.IpAddress.String << std::endl;
				std::cout << "子网掩码：" << p_adapters->IpAddressList.IpMask.String << std::endl;*/
			}
			//std::cout << std::endl;
			p_adapters = p_adapters->Next;
		}
		delete p_adapters; 
		return true;
	}
#else
	bool GetAllAdapter(std::vector<Adapter> *list);// linux下获取的网卡信息 
#endif
};