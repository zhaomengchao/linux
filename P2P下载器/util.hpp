//#pragma once
//#ifdef _WIN32
//#include<iostream>
//#include<WS2tcpip.h>
//#include<Iphlpapi.h>//获取网卡信息的头文件
//#include<vector>
//#include<fstream>
//#include<boost/filesystem.hpp>
//#pragma comment(lib, "Iphlpapi.lib")//获取网卡信息的库
//#pragma comment(lib, "ws2_32.lib")//windows下的socket库
//#else
////linux头文件
//#endif
//
//class FileUtil
//{
//public:
//	static bool Write(const std::string &name, const std::string &body, int64_t offset = 0)//offset为偏移量
//	{
//		std::ofstream ofs(name);
//		if (ofs.is_open() == false)
//		{
//			std::cerr << "打开文件失败\n";
//			return false;
//		}
//		ofs.seekp(offset, std::ios::beg);//从读写位置跳转到相对于文件起始位置开始偏移offset的偏移量
//		ofs.write(&body[0], body.size());
//		if (ofs.good() == false)
//		{
//			std::cerr << "向文件写入数据失败\n";
//			ofs.close();
//			return false;
//		}
//		ofs.close();
//		return true;
//	}
//	// 指针参数表示这是一个输出型参数
//	// const& 表示这是一个输入型参数
//	// & 表示这是一个输入输出型参数
//	static bool Read(const std::string& name, std::string* body)
//	{
//		std::ifstream ifs(name); //实例化
//		if (ifs.is_open() == false)
//		{
//			std::cerr << "打开文件失败\n";
//			return false;
//		}
//		int64_t filesize = boost::filesystem::file_size(name);  // 通过文件名获取文件大小
//		body->resize(filesize);  // 给body分配文件空间
//
//		//std::cout << "要读取的文件大小：" << name << ":" << filesize << std::endl;
//
//		ifs.read(&(*body)[0], filesize);  // 读取文件大小
//		/*if (ifs.good() == false)
//		{
//		std::cerr << "读取文件数据失败\n";
//		std::cout << *body << std::endl;
//		ifs.close();
//		return false;
//		}*/
//		ifs.close();
//		return true;
//	}
//
//};
//
//class Adapter//定义一个网卡类
//{
//public:
//	_Uint32t _ip_addr;//网卡上IP地址
//	_Uint32t _mask_addr;//网卡上的子网掩码
//};
//class AdapterUtil//工具类网络适配器
//{
//public:
//#ifdef _WIN32
//	static bool GetAllAdapter(std::vector<Adapter> *list)// windows下获取的网卡信息实现
//	{
//		//IP_ADAPTER_INFO 是一个存放信息的结构体
//		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO();
//		//GetAdaptersInfo window下获取网卡信息的接口--网卡信息有可能有多个，因此传入一个指针
//		//获取网卡信息可能会失败，因为空间不足，因此有一个输出型参数，用于向用户返回所有网卡信息空间占用大小
//		int64_t all_adapter_size = sizeof(IP_ADAPTER_INFO);
//		// 接收成功与否的返回 值
//		int ret = GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
//		if (ret == ERROR_BUFFER_OVERFLOW)
//		{// 这个错误表示缓冲区空间不足
//			//重新给指针申请空间
//			delete p_adapters;//先释放
//			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];
//			GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
//		}
//		while (p_adapters) 
//		{
//			Adapter adapter;
//			//将一个字符串点分十进制IP地址到网络地址IP地址
//			inet_pton(AF_INET, p_adapters->IpAddressList.IpAddress.String, &adapter._ip_addr);
//			inet_pton(AF_INET, p_adapters->IpAddressList.IpMask.String, &adapter._mask_addr);
//			if (adapter._ip_addr != 0)//有些网卡信息没有启用
//			{
//				list->push_back(adapter);//将网卡信息添加到vector中返回给用户
//				//if (p_adapters->IpAddressList.IpAddress.String != "0.0.0.0")
//				std::cout << "网卡名称：" << p_adapters->AdapterName << std::endl;
//				std::cout << "网卡描述：" << p_adapters->Description << std::endl;
//				std::cout << "IP地址：" << p_adapters->IpAddressList.IpAddress.String << std::endl;
//				std::cout << "子网掩码：" << p_adapters->IpAddressList.IpMask.String << std::endl;
//			}
//			std::cout << std::endl;
//			p_adapters = p_adapters->Next;
//		}
//		delete p_adapters; 
//		return true;
//	}
//#else
//	bool GetAllAdapter(std::vector<Adapter> *list);// linux下获取的网卡信息 
//#endif
//};
//
//



#pragma once   // 头文件只包含一次的说明

#include<iostream>
#include<vector>
#include<fstream>  // 文件操作的头文件
#include<boost/filesystem.hpp>

#ifdef _WIN32
// windows头文件
#include<WS2tcpip.h>  // Windows Socket 协议
#include<iphlpapi.h>  // 获取网卡信息接口的头文件：PIP_ADAPTER_INFO结构、GetAdaptersInfo接口...
#pragma comment(lib,"Iphlpapi.lib")  // 获取网卡信息接口的库文件包含
#pragma comment(lib,"ws2_32.lib")  // Windows下的socket库：inet_pton
#else
// linux头文件
#endif

class FileUtil
{
public:
	static bool Write(const std::string& name, const std::string& body, int64_t offset = 0) // 第三个参数是默认参数
	{
		std::ofstream ofs(name);//打开文件，如果文件不存在就会创建它
		if (ofs.is_open() == false)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
		ofs.seekp(offset, std::ios::beg);//跳转读写位置，参数：偏移量，起始位置；读写位置跳转到相对于文件起始位置开始偏移offset的偏移量
		ofs.write(&body[0], body.size());
		if (ofs.good() == false) //判断上一次操作的结果
		{
			std::cerr << "向文件写入数据失败\n";
			ofs.close();
			return false;
		}
		ofs.close(); //关闭文件，没有关闭就是资源泄漏
		return true;
	}

	// 指针参数表示这是一个输出型参数
	// const& 表示这是一个输入型参数
	// & 表示这是一个输入输出型参数
	static bool Read(const std::string& name, std::string* body)
	{
		std::ifstream ifs(name); //实例化
		if (ifs.is_open() == false)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
		int64_t filesize = boost::filesystem::file_size(name);  // 通过文件名获取文件大小
		body->resize(filesize);  // 给body分配文件空间

		//std::cout << "要读取的文件大小：" << name << ":" << filesize << std::endl;

		ifs.read(&(*body)[0], filesize);  // 读取文件大小
		/*if (ifs.good() == false)
		{
		std::cerr << "读取文件数据失败\n";
		std::cout << *body << std::endl;
		ifs.close();
		return false;
		}*/
		ifs.close();
		return true;
	}


};


//*********************************************************************

class Adapter  // 网卡信息结构体
{
public:
	uint32_t _ip_addr; // 网卡上的IP地址
	uint32_t _mask_addr; // 网卡上的子网掩码
};

class AdapterUtil
{
public:
#ifdef _WIN32
	// windows下的获取网卡信息实现，
	static bool GetAllAdapter(std::vector<Adapter> *list) {

		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO();
		// 开辟一块网卡信息结构的空间
		// p_adapters指针指向的是链表

		// GetAdaptersInfo 表示Windows下获取网卡信息的接口——网卡信息有可能有多个，
		// 因此传入一个指针，这个指针会根据网卡信息创建空间填充信息，将网卡信息拷贝进来；
		// 否则，如果定义一个结构体，就只能保存一个信息，不合适。

		// 获取网卡信息有可能会失败，因为空间不足，因此有一个输出型参数，
		// 用于向用户返回所有网卡信息空间占用大小。

		uint64_t all_adapter_size = sizeof(IP_ADAPTER_INFO);
		// all_adapter_size用于获取实际所有网卡信息所占空间大小
		int ret = GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
		// ret获取接口成功与否的返回值

		// 上面先开辟了一块网卡信息的空间，若有多块网卡，那么走进来，先删掉再重新开辟。
		if (ret == ERROR_BUFFER_OVERFLOW) {
			// 表示当前缓冲区空间不足，因此重新给指针申请空间
			delete p_adapters;
			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];
			GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size); // 重新获取网卡信息
		}

		while (p_adapters) { // p_adapters指针指向的是链表，链表中是所有的网卡信息。

			Adapter adapter; // 实例化一个网卡信息结构体的对象

			//adapter._ip_addr = inet_addr(p_adapters->IpAddressList.IpAddress.String); // ()中直接就是网卡IP地址的字符串
			//adapter._mask_addr = inet_addr(p_adapters->IpAddressList.IpMask.String); // ()中直接就是网卡子网掩码的字符串
			// inet_addr只能转换IPV4的网络地址，有了IPV6，就推荐使用inet_pton

			inet_pton(AF_INET, p_adapters->IpAddressList.IpAddress.String, &adapter._ip_addr);
			inet_pton(AF_INET, p_adapters->IpAddressList.IpMask.String, &adapter._mask_addr);

			if (adapter._ip_addr != 0) // 因为有些网卡并没有启用，导致IP地址为0； 
			{
				list->push_back(adapter);  // 将网卡信息添加到vector中返回给用户
				/*std::cout << "网卡名称:" << p_adapters->AdapterName << std::endl;
				std::cout << "网卡描述:" << p_adapters->Description << std::endl;
				std::cout << "IP地址:" << p_adapters->IpAddressList.IpAddress.String << std::endl;
				std::cout << "子网掩码:" << p_adapters->IpAddressList.IpMask.String << std::endl;*/
			}
			//std::cout << std::endl;
			p_adapters = p_adapters->Next;
		}

		delete p_adapters; // 注意要释放，不释放会造成内存泄漏
		return true;
	}
#else
	// linux下的获取网卡信息实现
	bool GetAllAdapter(std::vector<Adapter> *list) {
		return true;
	}
#endif
};