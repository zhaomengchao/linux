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
	//boost::filesystem::path().filename() ֻ��ȡ�ļ����ƣ�abc/filename.txt  ->  filename.txt
	//boost::filesystem::exists() �ж��ļ��Ƿ����
	const char *ptr = "./";
	boost::filesystem::directory_iterator begin(ptr); //����һ��Ŀ¼����������
	boost::filesystem::directory_iterator end;
	for (; begin != end; ++begin) {
		//begin->status()  Ŀ¼�е�ǰ�ļ���״̬��Ϣ
		//boost::filesystem::is_directory() �жϵ�ǰ�ļ��Ƿ���һ��Ŀ¼
		if (boost::filesystem::is_directory(begin->status())) {
			//begin->path().string() ��ȡ��ǰ�����ļ����ļ���
			std::cout << begin->path().string() << "��һ��Ŀ¼\n";
		}
		else {
			std::cout << begin->path().string() << "��һ����ͨ�ļ�\n";
			//begin->path().filename() ��ȡ·�ļ�·�����е��ļ����ƣ�����Ҫ·��
			std::cout << "�ļ�����" << begin->path().filename().string() << std::endl;
		}
	}
}
void test1()
{
	/*std::vector<Adapter> list;
	AdapterUtil::GetAllAdapter(&list);*/
	httplib::Server srv;

	srv.Get("/", helloworld);
	//�ڷ�������һ�����get���󷽷��Լ�������Դ�Ĵ���ʽ��Ӧ��ϵ
	srv.listen("0.0.0.0", 9000);
	//�һ�������˱������⡤����9000�˿ڵ�tcp������
	//�ȴ��ͻ��˵���������
	//�����¿ͻ��˵����ӵ������򴴽�һ���̴߳�������ͻ�������
	//���߳��н������ݣ����н����������õ������ݷŵ�ʵ����request������
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
	//�����߳������пͻ���ģ���Լ������ģ��
	//����һ���߳����пͻ���ģ�飬���߳����з����ģ��
	std::thread thr_clilent(ClientRun);//��ͻ������е�ʱ�򣬷���˻�û������

	Server srv;
	srv.Start();

	//test2();
	//system("pause");
	return 0;
}
