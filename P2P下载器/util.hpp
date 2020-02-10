#pragma once
#ifdef _WIN32
#include<iostream>
#include<WS2tcpip.h>
#include<Iphlpapi.h>
#include<vector>
#pragma comment(lib, "Iphlpapi.lib")
#else
//linux头文件
#endif

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
		int all_adapter_size = sizeof(IP_ADAPTER_INFO);
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
			std::cout << "网卡名称：" << p_adapters->AdapterName << std::endl;
			std::cout << "网卡描述：" << p_adapters->Description << std::endl;
			std::cout << "IP地址：" << p_adapters->IpAddressList.IpAddress.String << std::endl;
			std::cout << "子网掩码：" << p_adapters->IpAddressList.IpMask.String << std::endl;
			p_adapters = p_adapters->Next;
		}
		delete p_adapters; 
		return true;
	}
#else
	bool GetAllAdapter(std::vector<Adapter> *list);// linux下获取的网卡信息 
#endif
};