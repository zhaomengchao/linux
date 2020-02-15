#pragma once
#include<thread>  // 线程的头文件：因为C++中没有pthread_create...
#include<boost/filesystem.hpp>
#include"util.hpp"
#include"httplib.h"

#define P2P_PORT 9000   // 端口
#define MAX_IPBUFFER 16  // IP地址空间
#define SHARED_PATH "./Shared/"  // 共享目录
#define DOWNLOAD_PATH "./Download/"//下载路径

class Host
{
public:
	uint32_t _ip_addr; // 要配对的主机IP地址
	bool _pair_ret; // 用于存放配对结果，配对成功true，失败false；
};


class Client
{
public:

	// 在外部调用start接口就可以完成服务端的搭建整合
	bool Start()
	{
		// 客户端程序需要循环运行，因为下载文件不是只下载一次
		// 循环运行每次下载一个文件之后都会重新进行主机配对，这是不合理的
		while (1)
		{
			GetOnlineHost(); // 获取在线主机
			// 调用这个函数，它自己本身会逐层调用后面的函数，一层一层嵌套调用，这里就不需要将其他函数单独写出来	 
		}
		return true;
	}

	// 主机配对的线程入口函数
	void HostPair(Host* host)  // 类的成员函数，有第一个隐含的this指针
	{
		//1、组织http协议格式的请求数据
		//2、搭建一个tcp的客户端，将数据发送
		//3、等待服务器端的回复，并进行解析
		//这个过程使用第三方库httplib实现，需要明确httplib的实现流程。

		host->_pair_ret = false;

		char buf[MAX_IPBUFFER] = { 0 }; //IP地址长度不会超过16字节
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);
		// httplib::Client cli(host->_ip_addr, ); // 参数错误，因为httplib.h中，这个参数是string类型，这里的是网络字节序
		httplib::Client cli(buf, P2P_PORT);  //httplib客户端对象实例化
		auto rsp = cli.Get("/hostpair");  //向服务端发送资源为/hostpair的GET请求，返回的是智能指针，用auto简单。
		//若连接建立失败，则Get会返回NULL；所以下面还需要进行判空，否则会访问空指针。

		if (rsp && rsp->status == 200) // 判断响应结果是否正确
			host->_pair_ret = true; // 重置主机配对结果

		return;
	}

	bool GetOnlineHost() // 获取在线主机
	{
		//是否重新匹配，默认是进行匹配的，若已经匹配过，online主机不为空，则让用户选择
		char c = 'Y';
		if (!_online_host.empty())
		{
			std::cout << "是否重新查看在线主机(Y/N)："; // 因为循环调用客户端程序，这个在线主机列表获取一次就够了
			fflush(stdout);
			std::cin >> c;
		}
		if (c == 'Y')
		{
			std::cout << "开始主机匹配...\n";

			//1、获取网卡信息，进而得到局域网中所有的IP地址列表
			std::vector<Adapter> list;
			AdapterUtil::GetAllAdapter(&list);

			// 获取所有主机IP地址，添加到host_list 
			std::vector<Host> host_list;
			for (int i = 0; i < list.size(); ++i)  // 循环的是某一块网卡，目的是：得到所有的主机IP地址列表
			{
				uint32_t ip = list[i]._ip_addr;
				uint32_t mask = list[i]._mask_addr;
				// 计算网络号
				uint32_t net = (ntohl(ip & mask));
				//计算最大主机号
				int max_host = (~ntohl(mask));  // ntohl转换成小端字节序
				for (int j = 1; j < 1016; ++j)//(int32_t)max_host
				{
					// 主机号为0是网络号，主机号为max_host是udp的广播地址
					uint32_t host_ip = net + j; // 这个主机IP的计算应该使用主机字节序(小端字节序)的网络号和主机号

					Host host;
					host._ip_addr = htonl(host_ip); // 将这个主机字节序的IP地址转换成网络字节序
					host._pair_ret = false;
					host_list.push_back(host); // 这个host_list中将包含了所有主机地址
				}
			}

			//对host_list中的主机创建线程进行配对
			std::vector<std::thread*> thr_list(host_list.size()); // 创建线程列表
			for (int i = 0; i < host_list.size(); ++i)
			{
				thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
			}
			std::cout << "正在主机匹配中，请稍后...\n";

			//3、若配对请求得到响应，则对应主机为在线主机，则将IP添加到_online_host列表中

			// 等待所有线程主机配对完毕，判断配对结果，将所有在线主机添加到online_host中
			for (int i = 0; i < host_list.size(); ++i)//host_list.size()
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
					_online_host.push_back(host_list[i]);
				delete thr_list[i];
			}
		}

		//4、打印在线主机列表，使用户选择
		for (int i = 0; i < _online_host.size(); ++i)
		{
			char buf[MAX_IPBUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
			std::cout << "\t" << buf << std::endl;
		}


		std::cout << "请选择配对主机，获取共享文件列表：";
		fflush(stdout); //刷新标准输出缓冲区

		std::string select_ip;
		std::cin >> select_ip;
		GetShareList(select_ip);  // 用户选择主机之后，调用获取文件列表接口

		return true;
	}

	// 获取文件列表
	bool GetShareList(const std::string &host_ip)
	{
		// 向服务端发送文件列表获取请求
		//1、先发送请求
		//2、得到响应之后，解析正文（文件名称）
		httplib::Client cli(host_ip.c_str(), P2P_PORT); //实例化客户端对象
		auto rsp = cli.Get("/list"); // 获取文件列表
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "获取文件列表响应错误\n";
			return false;
		}
		// 打印正文——打印服务端响应的文件名称列表供用户选择
		// body：filename1\r\nfilename2\r\n...
		std::cout << rsp->body << std::endl;  // body是一个string对象，里面存放的就是所有的正文信息
		std::cout << "\n请选择要下载的文件：";
		fflush(stdout);
		std::string filename;
		std::cin >> filename;  // 获取完毕
		DownloadFile(host_ip, filename); // 下载文件
		return true;
	}


	// 下载文件，你要下载哪个主机的哪个文件
	bool DownloadFile(const std::string& host_ip, const std::string& filename)
	{
		//1、向服务端发送文件下载请求
		//2、得到响应结果，响应结果中的body正文就是文件数据
		//3、创建文件，将文件数据写入文件中，关闭文件
		std::string req_path = "/download/" + filename; // 因为我们上面都是以/加文件名的形式，所以这里也要加上/
		httplib::Client cli(host_ip.c_str(), P2P_PORT);

		auto rsp = cli.Get(req_path.c_str());

		if (rsp == NULL)
		{
			std::cout << "下载文件，获取响应信息失败!" << std::endl;
			return false;
		}
		if (rsp->status != 200)
		{
			std::cout << "下载文件，获取响应信息失败：" << rsp->status << std::endl;
			return false;
		}
		if (!boost::filesystem::exists(DOWNLOAD_PATH))//判断目录是否存在
		{
			boost::filesystem::create_directory(DOWNLOAD_PATH);
		}

		std::string realpath = DOWNLOAD_PATH + filename;//实际下载路径

		if (FileUtil::Write(realpath, rsp->body) == false)
		{
			std::cerr << "文件下载失败\n";
			return false;
		}

		std::cout << "文件下载成功！" << std::endl;
		return true;
	}

private:
	std::vector<Host> _online_host; // 在线主机
};

class Server
{
public:

	// 在外部调用start接口就可以完成服务端的搭建整合
	bool Start()
	{
		// 添加针对客户端请求的处理方式对应关系
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);

		//文件名是不定的，用正则表达式：将特殊字符以指定的格式，表示具有关键特征的数据
		// .表示除了回车和换行的任意字符；*表示字符任意次;
		// .*表示匹配除\n和\r之外的字符任意次。
		_srv.Get("/download/.*", Download);
		//防止与上方的请求冲突，因此在请求中加入download路径。

		_srv.listen("0.0.0.0", P2P_PORT);  // 是一个阻塞函数，程序运行不下去

		return true;
	}

private: // 下面三个函数是回调函数，它只有两个参数；如果是作为累的成员函数，那么就会多出一个隐藏的this指针，所以前面加上static
	// 主机配对的相应动作
	static void HostPair(const httplib::Request& req, httplib::Response& rsp)
	{
		rsp.status = 200;
		return;
	}

	// 获取共享文件列表——在主机上设置一个共享目录，凡是这个目录下的文件都是要共享给别人的
	static void ShareList(const httplib::Request& req, httplib::Response& rsp)
	{
		//1、查看目录是否存在，若目录不存在，则创建这个目录
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);
		}
		boost::filesystem::directory_iterator begin(SHARED_PATH); // 实例化目录迭代器
		boost::filesystem::directory_iterator end; // 实例化目录迭代器的末尾

		//开始迭代目录
		for (; begin != end; ++begin)
		{
			if (boost::filesystem::is_directory(begin->status()))
			{
				// 当前版本我们只获取普通文件名称，不做多层级目录的操作
				continue;
			}
			std::string name = begin->path().filename().string(); // 获取到文件名称，要转换成string类型
			rsp.body += name + "\r\n"; // filename1\r\nfilename2\r\n...
		}
		rsp.status = 200;
		return;
	}
	// 下载文件的相应动作
	static void Download(const httplib::Request& req, httplib::Response& rsp)
	{
		//std::cout << "服务端收到文件下载请求：" << req.path << std::endl;
		// req.path 客户端请求资源的路径  /download/filename
		boost::filesystem::path req_path(req.path); // 实例化一个path对象
		//boost::filesystem::path req_path().filename() 获取文件名称：abc/filename.txt ---> filename.txt

		std::string name = req_path.filename().string(); // 只获取文件名称: filename,txt
		//std::cout << "服务端收到实际的文件下载名称：" << name << " 路径：" << SHARED_PATH << std::endl;
		std::string realpath = SHARED_PATH + name; // 实际文件的路径应该是在共享的目录下

		// boost::filesystem::exists() 判断文件是否存在
		//std::cout << "服务端收到实际的文件下载路径：" << realpath << std::endl;
		if (!boost::filesystem::exists(realpath) || boost::filesystem::is_directory(realpath))
		{
			rsp.status = 404; // 你要请求的资源不存在
			return;
		}
		if (FileUtil::Read(realpath, &rsp.body) == false)
		{
			rsp.status = 500;  // 服务器内部错误
			return;
		}
		rsp.status = 200;  // 表示这次文件下载正确处理了
		return;
	}

private:
	httplib::Server _srv;
};


//auto通过赋值决定类型