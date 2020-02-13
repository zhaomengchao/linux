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
	uint32_t _ip_addr;//Ҫ���������IP��ַ
	bool _pair_ret;//���ڴ����Խ������ȷ����true,���󷵻�false��
};

class Client
{
public:
	//������Ե��߳���ں��������̵߳���ں�����һ����ĳ�Ա����ʱ��
	void HostPair(Host *host)
	{
		//��֯httpЭ���ʽ����������
		//�һ��tcp�ͻ���
		//�ȴ��������˵Ļض��������н���
		host->_pair_ret = false;
		char buf[MAX_IPBUFFER] = { 0 };
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);//�����ֽ����IP��ַת�����ַ������ʮ���Ƶ�IP��ַ
		httplib::Client cli(buf, P2P_PORT);//ʵ����httplib�ͻ��˶���
		auto rsp = cli.Get("/hostpair");//�����˷�����ԴΪHostpair��GET����//rsp Get�����п���Ϊ��
		if (rsp && rsp->status == 200)//�ж���Ӧ�Ƿ���ȷ
		{
			host->_pair_ret = true;//����������Խ��
		}
		return;

	}
	bool GetOlineHost()//��ȡ��������
	{
		//1��ȡ������Ϣ�������õ��������е�����IP��ַ�б�
		std::vector<Adapter> list;
		AdapterUtil::GetAllAdapter(&list);
		//��ȡ��������IP��ַ
		std::vector<Host> host_list; 
		for (int i = 0; i < list.size(); i++)
		{
			uint32_t ip = list[i]._ip_addr;
			uint32_t mask = list[i]._mask_addr;
			//���������
			uint32_t net = (ntohl(ip & mask));//������С�˽����ڵ����д洢�Ǹ�λ�ڵ�λ�洢
			//�������������
			uint32_t max_host = (~ntohl(mask));

			//����������飬Ŀ��ʱ��Գɹ�����Ϊ1��������Ϊ0���Դ����жϣ��߳̽����󵽵��ж���������Գɹ�
			std::vector<bool> ret_list(max_host);
			//�����
			for (int j = 1; j < max_host; j++)//����Ϊ0��ȫ0Ϊ�����ַ
			{//Ŀ�ģ�Ϊ�˵õ����е�����IP�б�
				uint32_t host_ip = net + j;//�������IP�ļ���Ӧ��ʹ�������ֽ��������ź�������

				Host host;
				host._ip_addr = htonl(host_ip);//�����ֽ���ת��Ϊ�����ֽ���
				host._pair_ret = false;
				host_list.push_back(host);
			}
		}
			//���ȡָ������Ϊstd::thread��һ���ֲ�������Ϊ�˷�ֹ��ɺ��ͷţ����Զ���һ������ָ��
			std::vector<std::thread*> thr_list(host_list.size());
			for (int i = 0; i < host_list.size(); i++)
			{
				//��host_list�е����������߳̽������
				thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
			}
			//�ȴ������߳����������ϣ��ж���Խ����������������ӵ�online_host��
			for (int i = 0; i < host_list.size(); i++)
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
				{
					_online_host.push_back(host_list[i]);
				}
				delete thr_list[i];
				
			}
			//����������������IP��ӡ���������û�ѡ��
			for (int i = 0; i < _online_host.size(); i++)
			{
				char buf[MAX_IPBUFFER] = { 0 };
				inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
				std::cout << "\t" << buf << std::endl;

			}
			//3.���������õ���Ӧ�����Ӧ����Ϊ������������IP��ӵ�_online_host�б���
			//4.��ӡ���������б�ʹ�û�ѡ�� 
			std::cout << "��ѡ�������������ȡ�����ļ��б�";
			fflush(stdout);
			std::string select_ip;
			std::cin >> select_ip;
			GetShareList(select_ip);
			
		}
	
	//��ȡ�ļ��б�
	bool GetShareList(const std::string &host_ip)
	{
		//�����˷���һ���ļ��б��ȡ����
		//1.�ȷ�������
		//2.�õ���Ӧ֮��������ģ��� �����ƣ�
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		auto rsp = cli.Get("/list");
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "��ȡ�ļ��б���Ӧ����\n";
			return false;
		}
		//��ӡ����--��ӡ�������Ӧ���ļ������б��û�ѡ��
		//body  filename\r\nfilename2
		std::cout << rsp->body << std::endl;
		std::cout << "\n��ѡ��Ҫ���ص��ļ���";
			fflush(stdout);
		std::string filename;
		std::cin >> filename;
		DownloadFile(host_ip.c_str(), filename);
		return true;
	}
	//�����ļ�
	bool DownloadFile(const std::string &host_ip, const std::string &filename)
	{
		//1.�����˷����ļ���������
		//2.�õ���Ӧ�������Ӧ����е�body���ľ����ļ�����
		//3.�����ļ������ļ�����д�뵽�ļ��У��ر��ļ�
		std::string req_path = "/download/" + filename;
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		auto rsp = cli.Get(req_path.c_str());
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "�����ļ�����ȡ��Ӧ��Ϣʧ��\n";
			return false;
		}
		if (FileUtil::Write(filename, rsp->body) == false)
		{
			std::cout << "�ļ�����ʧ��\n";
			return false;
		}
		return true;
	}
private:
	std::vector<Host> _online_host;//��������
}; 

class Server
{
public:
	bool Start()
	{
		//�����Կͻ�������Ĵ���ʽ��Ӧ��ϵ
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);
		//������ʽ�У�.ƥ���\n��\r֮���������ĸ  *����ʾƥ��ǰ�ߵ��ַ������
		_srv.Get("/download/.*",Download);//������ʽ���������ַ���ָ����ʽ����ʾ���йؼ����Ե�����
		_srv.listen("0.0.0.0", P2P_PORT);
		return true;
	}
private:
	static void HostPair(const httplib::Request &req, httplib::Response &rsp)
	{
		rsp.status = 200;
		return;
	}
	//��ȡ�����ļ��б�---������������һ������Ŀ¼�����������Ŀ¼�µ��ļ�����Ҫ��������˵�
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


//ԭʼ���
//for (int j = 1; j < max_host; j++)//����Ϊ0��ȫ0Ϊ�����ַ
//{ 
//	uint32_t host_ip = net + j;//�������IP�ļ���Ӧ��ʹ�������ֽ��������ź�������
//	//2.�����IP��ַ�б��е����������������

//	//�ڶ��Ͽ���һ��ռ䣬��֤���ռ䲻���Զ����٣���ÿһ���̣߳�ÿһ��������������󣩵ĵ�ַ���뵽���鵱�С�
//	thr_list[i] = new std::thread (&Client::HostPair, this, host_ip, &ret_list[i]);//���̵߳���ں�����һ����ĳ�Ա����ʱ��ע��д����
//	//thr.join();//�ȴ��߳��˳�
//}

//for (int j = 1; j < max_host; j++)//�ж���һ���߳��˳���
//{
//	thr_list[i]->join();//�߳��˳���һ��������Գɹ����п����������ֽ���ʧ��
//	delete thr_list[i];
//}