#include<boost/filesystem.hpp>
#include"util.hpp"
#include"client.hpp"


void helloworld(const httplib::Request &req, httplib::Response &rsp)
{
	printf("httplib server recv a req:%s\n", req.path.c_str());
	rsp.set_content("<html><h1>HelloWorld</h1></html>", "text/html");
	rsp.status = 200;
}

void Scandir()
{
	//boost::filesystem::path().filename() 只获取文件名称；abc/filename.txt  ->  filename.txt
	//boost::filesystem::exists() 判断文件是否存在
	const char *ptr = "./";
	boost::filesystem::directory_iterator begin(ptr); //定义一个目录迭代器对象
	boost::filesystem::directory_iterator end;
	for (; begin != end; ++begin) {
		//begin->status()  目录中当前文件的状态信息
		//boost::filesystem::is_directory() 判断当前文件是否是一个目录
		if (boost::filesystem::is_directory(begin->status())) {
			//begin->path().string() 获取当前迭代文件的文件名
			std::cout << begin->path().string() << "是一个目录\n";
		}
		else {
			std::cout << begin->path().string() << "是一个普通文件\n";
			//begin->path().filename() 获取路文件路径名中的文件名称，而不要路径
			std::cout << "文件名：" << begin->path().filename().string() << std::endl;
		}
	}
}
void test1()
{
	/*std::vector<Adapter> list;
	AdapterUtil::GetAllAdapter(&list);*/
	httplib::Server srv;

	srv.Get("/", helloworld);
	//在服务端添加一个针对get请求方法以及请求资源的处理方式对应关系
	srv.listen("0.0.0.0", 9000);
	//搭建一个监听了本机任意·网卡9000端口的tcp服务器
	//等待客户端的连接请求
	//若有新客户端的连接到来，则创建一格线程处理这个客户端请求。
	//在线程中接收数据，进行解析，解析得到的数据放到实例化request对象中
}
void test2()
{
	Scandir();
	Sleep(1111111);
	/*
	httplib::Server srv;

	srv.Get("/", helloworld);

	srv.listen("0.0.0.0", 9000);
	*/
}

void ClientRun()
{ 
	Sleep(1);
	Client cli;
	cli.Start();
}
int main(int argc, char *argv[])
{
	//在主线程中运行客户端模块以及服务端模块
	//创建一个线程运行客户端模块，主线程运行服务端模块
	std::thread thr_clilent(ClientRun);//如客户端运行的时候，服务端还没有启动

	Server srv;
	srv.Start();

	//test2();
	//system("pause");
	return 0;
}
