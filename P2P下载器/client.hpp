#pragma once
#include<thread>
#include"util.hpp" 

class Host
{
public:
	uint32_t _ip_addr;//要配对主机的IP地址
	bool _pair_ret;//用于存放配对结果，正确返回true,错误返回false。
};

class Client
{
public:
	//主机配对的线程入口函数（当线程得入口函数是一个类的成员函数时）
	void HostPair(Host *host)
	{

	}
	bool GetOlineHost()//获取在线主机
	{
		//1获取网卡信息，进而得到局域网中的所有IP地址列表
		std::vector<Adapter> list;
		AdapterUtil::GetAllAdapter(&list);
		//获取所有主机IP地址
		std::vector<Host> host_list; 
		for (int i = 0; i < list.size(); i++)
		{
			uint32_t ip = list[i]._ip_addr;
			uint32_t mask = list[i]._mask_addr;
			//计算网络号
			uint32_t net = (ntohl(ip & mask));//网络字小端节序，在电脑中存储是高位在低位存储
			//计算最大主机号
			uint32_t max_host = (~ntohl(mask));

			

			//定义这个数组，目的时配对成功，置为1，否则置为0，以此来判断，线程结束后到底有多少主机配对成功
			std::vector<bool> ret_list(max_host);
			//简化设计
			for (int j = 1; j < max_host; j++)//不能为0，全0为网络地址
			{//目的，为了得到所有的主机IP列表
				uint32_t host_ip = net + j;//这个主机IP的计算应该使用主机字节序的网络号和主机号
				
				Host host;
				host._ip_addr = host_ip;
				host._pair_ret = false;
				host_list.push_back(host);
			}
			//这块取指针是因为std::thread是一个局部变量，为了防止完成后被释放，所以定义一个数组指针
			std::vector<std::thread*> thr_list(host_list.size());
			for (int i = 0; i < host_list.size(); i++)
			{
				//对host_list中的主机创建线程进行配对
				thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
			}
			//等待多有线程主机配对完毕，判断配对结果，将在线主机添加到online_host中
			for (int i = 0; i < host_list.size(); i++)
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
				{
					_online_host.push_back(host_list[i]);
				}
				delete thr_list[i];
				
			}

			 


			//原始设计
			//for (int j = 1; j < max_host; j++)//不能为0，全0为网络地址
			//{
			//	uint32_t host_ip = net + j;//这个主机IP的计算应该使用主机字节序的网络号和主机号
			//	//2.逐个对IP地址列表中的主机发送配对请求

			//	//在堆上开辟一块空间，保证这块空间不能自动销毁，将每一个线程（每一个主机的配对请求）的地址存入到数组当中。
			//	thr_list[i] = new std::thread (&Client::HostPair, this, host_ip, &ret_list[i]);//当线程得入口函数是一个类的成员函数时（注意写法）
			//	//thr.join();//等待线程退出
			//}

			//for (int j = 1; j < max_host; j++)//判断那一个线程退出，
			//{
			//	thr_list[i]->join();//线程退出不一定代表配对成功，有可能三次握手建立失败
			//	delete thr_list[i];
			//}
		}
		//3.如配对请求得到响应，则对应主机为在线主机，则将IP添加到_online_host列表中
		//4.打印在线主机列表，使用户选择 
	}
private:
	std::vector<Host> _online_host;//在线主机
};