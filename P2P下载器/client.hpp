#pragma once
#include<thread>
#include"util.hpp" 
#include"httplib.h"

#define P2P_PORT 9000
#define MAX_IPBUFFER 16
#define SHARE_PATH "./Shared"

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
		//组织http协议格式的请求数据
		//搭建一个tcp客户端
		//等待服务器端的回都，并进行解析
		host->_pair_ret = false;
		char buf[MAX_IPBUFFER] = { 0 };
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);//网络字节序的IP地址转换成字符串点分十进制的IP地址
		httplib::Client cli(buf, P2P_PORT);//实例化httplib客户端对象
		auto rsp = cli.Get("/hostpair");//向服务端发送资源为Hostpair的GET请求//rsp Get请求有可能为空
		if (rsp && rsp->status == 200)//判断相应是否正确
		{
			host->_pair_ret = true;//重置主机配对结果
		}
		return;

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
				host._ip_addr = htonl(host_ip);//主机字节序转化为网络字节序
				host._pair_ret = false;
				host_list.push_back(host);
			}
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
			//将所有在线主机的IP打印出来，供用户选择
			for (int i = 0; i < _online_host.size(); i++)
			{
				char buf[MAX_IPBUFFER] = { 0 };
				inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
				std::cout << "\t" << buf << std::endl;

			}
			//3.如配对请求得到响应，则对应主机为在线主机，则将IP添加到_online_host列表中
			//4.打印在线主机列表，使用户选择 
			std::cout << "请选则配对主机，获取共享文件列表：";
			fflush(stdout);
			std::string select_ip;
			std::cin >> select_ip;
			GetShareList(select_ip);
			
		}
	
	//获取文件列表
	bool GetShareList(const std::string &host_ip)
	{
		//向服务端发送一个文件列表获取请求
		//1.先发送请求
		//2.得到相应之后解析正文（文 件名称）
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		auto rsp = cli.Get("/list");
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "获取文件列表响应错误\n";
			return false;
		}
		//打印正文--打印服务端响应的文件名称列表供用户选择
		//body  filename\r\nfilename2
		std::cout << rsp->body << std::endl;
		std::cout << "\n请选则要下载的文件：";
			fflush(stdout);
		std::string filename;
		std::cin >> filename;
		DownloadFile(host_ip.c_str(), filename);
		return true;
	}
	//下载文件
	bool DownloadFile(const std::string &host_ip, const std::string &filename)
	{
		//1.向服务端发送文件下载请求
		//2.得到响应结果，响应结果中的body正文就是文件数据
		//3.创建文件，将文件数据写入到文件中，关闭文件
		std::string req_path = "/download/" + filename;
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		auto rsp = cli.Get(req_path.c_str());
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "下载文件，获取相应信息失败\n";
			return false;
		}
		if (FileUtil::Write(filename, rsp->body) == false)
		{
			std::cout << "文件下载失败\n";
			return false;
		}
		return true;
	}
private:
	std::vector<Host> _online_host;//在线主机
}; 

class Server
{
public:
	bool Start()
	{
		//添加针对客户端请求的处理方式对应关系
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);
		//正则表达式中：.匹配除\n或\r之外的任意字母  *：表示匹配前边的字符任意次
		_srv.Get("/download/.*",Download);//正则表达式，将特殊字符以指定格式，表示具有关键特性的数据
		_srv.listen("0.0.0.0", P2P_PORT);
		return true;
	}
private:
	static void HostPair(const httplib::Request &req, httplib::Response &rsp)
	{
		rsp.status = 200;
		return;
	}
	//获取共享文件列表---在主机上设置一个共享目录，凡是在这个目录下的文件都是要共享给别人的
	static void ShareList(const httplib::Request &req, httplib::Response &rsp)
	{
		return;
	}
	static void Download(const httplib::Request &req, httplib::Response &rsp)
	{
		return;
	}
private:
	httplib::Server _srv;
};


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