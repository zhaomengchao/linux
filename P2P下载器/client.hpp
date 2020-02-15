#pragma once
#include<thread>  // �̵߳�ͷ�ļ�����ΪC++��û��pthread_create...
#include<boost/filesystem.hpp>
#include"util.hpp"
#include"httplib.h"

#define P2P_PORT 9000   // �˿�
#define MAX_IPBUFFER 16  // IP��ַ�ռ�
#define SHARED_PATH "./Shared/"  // ����Ŀ¼
#define DOWNLOAD_PATH "./Download/"//����·��

class Host
{
public:
	uint32_t _ip_addr; // Ҫ��Ե�����IP��ַ
	bool _pair_ret; // ���ڴ����Խ������Գɹ�true��ʧ��false��
};


class Client
{
public:

	// ���ⲿ����start�ӿھͿ�����ɷ���˵Ĵ����
	bool Start()
	{
		// �ͻ��˳�����Ҫѭ�����У���Ϊ�����ļ�����ֻ����һ��
		// ѭ������ÿ������һ���ļ�֮�󶼻����½���������ԣ����ǲ������
		while (1)
		{
			GetOnlineHost(); // ��ȡ��������
			// ����������������Լ�����������ú���ĺ�����һ��һ��Ƕ�׵��ã�����Ͳ���Ҫ��������������д����	 
		}
		return true;
	}

	// ������Ե��߳���ں���
	void HostPair(Host* host)  // ��ĳ�Ա�������е�һ��������thisָ��
	{
		//1����֯httpЭ���ʽ����������
		//2���һ��tcp�Ŀͻ��ˣ������ݷ���
		//3���ȴ��������˵Ļظ��������н���
		//�������ʹ�õ�������httplibʵ�֣���Ҫ��ȷhttplib��ʵ�����̡�

		host->_pair_ret = false;

		char buf[MAX_IPBUFFER] = { 0 }; //IP��ַ���Ȳ��ᳬ��16�ֽ�
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);
		// httplib::Client cli(host->_ip_addr, ); // ����������Ϊhttplib.h�У����������string���ͣ�������������ֽ���
		httplib::Client cli(buf, P2P_PORT);  //httplib�ͻ��˶���ʵ����
		auto rsp = cli.Get("/hostpair");  //�����˷�����ԴΪ/hostpair��GET���󣬷��ص�������ָ�룬��auto�򵥡�
		//�����ӽ���ʧ�ܣ���Get�᷵��NULL���������滹��Ҫ�����пգ��������ʿ�ָ�롣

		if (rsp && rsp->status == 200) // �ж���Ӧ����Ƿ���ȷ
			host->_pair_ret = true; // ����������Խ��

		return;
	}

	bool GetOnlineHost() // ��ȡ��������
	{
		//�Ƿ�����ƥ�䣬Ĭ���ǽ���ƥ��ģ����Ѿ�ƥ�����online������Ϊ�գ������û�ѡ��
		char c = 'Y';
		if (!_online_host.empty())
		{
			std::cout << "�Ƿ����²鿴��������(Y/N)��"; // ��Ϊѭ�����ÿͻ��˳���������������б��ȡһ�ξ͹���
			fflush(stdout);
			std::cin >> c;
		}
		if (c == 'Y')
		{
			std::cout << "��ʼ����ƥ��...\n";

			//1����ȡ������Ϣ�������õ������������е�IP��ַ�б�
			std::vector<Adapter> list;
			AdapterUtil::GetAllAdapter(&list);

			// ��ȡ��������IP��ַ����ӵ�host_list 
			std::vector<Host> host_list;
			for (int i = 0; i < list.size(); ++i)  // ѭ������ĳһ��������Ŀ���ǣ��õ����е�����IP��ַ�б�
			{
				uint32_t ip = list[i]._ip_addr;
				uint32_t mask = list[i]._mask_addr;
				// ���������
				uint32_t net = (ntohl(ip & mask));
				//�������������
				int max_host = (~ntohl(mask));  // ntohlת����С���ֽ���
				for (int j = 1; j < 1016; ++j)//(int32_t)max_host
				{
					// ������Ϊ0������ţ�������Ϊmax_host��udp�Ĺ㲥��ַ
					uint32_t host_ip = net + j; // �������IP�ļ���Ӧ��ʹ�������ֽ���(С���ֽ���)������ź�������

					Host host;
					host._ip_addr = htonl(host_ip); // ����������ֽ����IP��ַת���������ֽ���
					host._pair_ret = false;
					host_list.push_back(host); // ���host_list�н�����������������ַ
				}
			}

			//��host_list�е����������߳̽������
			std::vector<std::thread*> thr_list(host_list.size()); // �����߳��б�
			for (int i = 0; i < host_list.size(); ++i)
			{
				thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
			}
			std::cout << "��������ƥ���У����Ժ�...\n";

			//3�����������õ���Ӧ�����Ӧ����Ϊ������������IP��ӵ�_online_host�б���

			// �ȴ������߳����������ϣ��ж���Խ��������������������ӵ�online_host��
			for (int i = 0; i < host_list.size(); ++i)//host_list.size()
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
					_online_host.push_back(host_list[i]);
				delete thr_list[i];
			}
		}

		//4����ӡ���������б�ʹ�û�ѡ��
		for (int i = 0; i < _online_host.size(); ++i)
		{
			char buf[MAX_IPBUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
			std::cout << "\t" << buf << std::endl;
		}


		std::cout << "��ѡ�������������ȡ�����ļ��б�";
		fflush(stdout); //ˢ�±�׼���������

		std::string select_ip;
		std::cin >> select_ip;
		GetShareList(select_ip);  // �û�ѡ������֮�󣬵��û�ȡ�ļ��б�ӿ�

		return true;
	}

	// ��ȡ�ļ��б�
	bool GetShareList(const std::string &host_ip)
	{
		// �����˷����ļ��б��ȡ����
		//1���ȷ�������
		//2���õ���Ӧ֮�󣬽������ģ��ļ����ƣ�
		httplib::Client cli(host_ip.c_str(), P2P_PORT); //ʵ�����ͻ��˶���
		auto rsp = cli.Get("/list"); // ��ȡ�ļ��б�
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "��ȡ�ļ��б���Ӧ����\n";
			return false;
		}
		// ��ӡ���ġ�����ӡ�������Ӧ���ļ������б��û�ѡ��
		// body��filename1\r\nfilename2\r\n...
		std::cout << rsp->body << std::endl;  // body��һ��string���������ŵľ������е�������Ϣ
		std::cout << "\n��ѡ��Ҫ���ص��ļ���";
		fflush(stdout);
		std::string filename;
		std::cin >> filename;  // ��ȡ���
		DownloadFile(host_ip, filename); // �����ļ�
		return true;
	}


	// �����ļ�����Ҫ�����ĸ��������ĸ��ļ�
	bool DownloadFile(const std::string& host_ip, const std::string& filename)
	{
		//1�������˷����ļ���������
		//2���õ���Ӧ�������Ӧ����е�body���ľ����ļ�����
		//3�������ļ������ļ�����д���ļ��У��ر��ļ�
		std::string req_path = "/download/" + filename; // ��Ϊ�������涼����/���ļ�������ʽ����������ҲҪ����/
		httplib::Client cli(host_ip.c_str(), P2P_PORT);

		auto rsp = cli.Get(req_path.c_str());

		if (rsp == NULL)
		{
			std::cout << "�����ļ�����ȡ��Ӧ��Ϣʧ��!" << std::endl;
			return false;
		}
		if (rsp->status != 200)
		{
			std::cout << "�����ļ�����ȡ��Ӧ��Ϣʧ�ܣ�" << rsp->status << std::endl;
			return false;
		}
		if (!boost::filesystem::exists(DOWNLOAD_PATH))//�ж�Ŀ¼�Ƿ����
		{
			boost::filesystem::create_directory(DOWNLOAD_PATH);
		}

		std::string realpath = DOWNLOAD_PATH + filename;//ʵ������·��

		if (FileUtil::Write(realpath, rsp->body) == false)
		{
			std::cerr << "�ļ�����ʧ��\n";
			return false;
		}

		std::cout << "�ļ����سɹ���" << std::endl;
		return true;
	}

private:
	std::vector<Host> _online_host; // ��������
};

class Server
{
public:

	// ���ⲿ����start�ӿھͿ�����ɷ���˵Ĵ����
	bool Start()
	{
		// �����Կͻ�������Ĵ���ʽ��Ӧ��ϵ
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);

		//�ļ����ǲ����ģ���������ʽ���������ַ���ָ���ĸ�ʽ����ʾ���йؼ�����������
		// .��ʾ���˻س��ͻ��е������ַ���*��ʾ�ַ������;
		// .*��ʾƥ���\n��\r֮����ַ�����Ρ�
		_srv.Get("/download/.*", Download);
		//��ֹ���Ϸ��������ͻ������������м���download·����

		_srv.listen("0.0.0.0", P2P_PORT);  // ��һ�������������������в���ȥ

		return true;
	}

private: // �������������ǻص���������ֻ�������������������Ϊ�۵ĳ�Ա��������ô�ͻ���һ�����ص�thisָ�룬����ǰ�����static
	// ������Ե���Ӧ����
	static void HostPair(const httplib::Request& req, httplib::Response& rsp)
	{
		rsp.status = 200;
		return;
	}

	// ��ȡ�����ļ��б���������������һ������Ŀ¼���������Ŀ¼�µ��ļ�����Ҫ��������˵�
	static void ShareList(const httplib::Request& req, httplib::Response& rsp)
	{
		//1���鿴Ŀ¼�Ƿ���ڣ���Ŀ¼�����ڣ��򴴽����Ŀ¼
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);
		}
		boost::filesystem::directory_iterator begin(SHARED_PATH); // ʵ����Ŀ¼������
		boost::filesystem::directory_iterator end; // ʵ����Ŀ¼��������ĩβ

		//��ʼ����Ŀ¼
		for (; begin != end; ++begin)
		{
			if (boost::filesystem::is_directory(begin->status()))
			{
				// ��ǰ�汾����ֻ��ȡ��ͨ�ļ����ƣ�������㼶Ŀ¼�Ĳ���
				continue;
			}
			std::string name = begin->path().filename().string(); // ��ȡ���ļ����ƣ�Ҫת����string����
			rsp.body += name + "\r\n"; // filename1\r\nfilename2\r\n...
		}
		rsp.status = 200;
		return;
	}
	// �����ļ�����Ӧ����
	static void Download(const httplib::Request& req, httplib::Response& rsp)
	{
		//std::cout << "������յ��ļ���������" << req.path << std::endl;
		// req.path �ͻ���������Դ��·��  /download/filename
		boost::filesystem::path req_path(req.path); // ʵ����һ��path����
		//boost::filesystem::path req_path().filename() ��ȡ�ļ����ƣ�abc/filename.txt ---> filename.txt

		std::string name = req_path.filename().string(); // ֻ��ȡ�ļ�����: filename,txt
		//std::cout << "������յ�ʵ�ʵ��ļ��������ƣ�" << name << " ·����" << SHARED_PATH << std::endl;
		std::string realpath = SHARED_PATH + name; // ʵ���ļ���·��Ӧ�����ڹ����Ŀ¼��

		// boost::filesystem::exists() �ж��ļ��Ƿ����
		//std::cout << "������յ�ʵ�ʵ��ļ�����·����" << realpath << std::endl;
		if (!boost::filesystem::exists(realpath) || boost::filesystem::is_directory(realpath))
		{
			rsp.status = 404; // ��Ҫ�������Դ������
			return;
		}
		if (FileUtil::Read(realpath, &rsp.body) == false)
		{
			rsp.status = 500;  // �������ڲ�����
			return;
		}
		rsp.status = 200;  // ��ʾ����ļ�������ȷ������
		return;
	}

private:
	httplib::Server _srv;
};


//autoͨ����ֵ��������