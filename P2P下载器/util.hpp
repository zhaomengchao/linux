#pragma once
#ifdef _WIN32
#include<iostream>
#include<WS2tcpip.h>
#include<Iphlpapi.h>//��ȡ������Ϣ��ͷ�ļ�
#include<vector>
#include<fstream>
#include<boost/filesystem.hpp>
#pragma comment(lib, "Iphlpapi.lib")//��ȡ������Ϣ�Ŀ�
#pragma comment(lib, "ws2_32.lib")//windows�µ�socket��
#else
//linuxͷ�ļ�
#endif

class FileUtil
{
public:
	static bool Write(const std::string &name, const std::string &body, int64_t offset = 0)//offsetΪƫ����
	{
		//ע���ı������Զ����Ʋ����Ĳ�ͬ
		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "wb+");//�Զ�������ʽ���ļ�
		if (fp == NULL)
		{
			std::cerr << "���ļ�ʧ��\n";
		}
		size_t ret = fwrite(body.c_str(), 1, body.size(), fp);
		if (ret != body.size())
		{
			std::cerr << "���ļ�д������ʧ��\n";
			fclose(fp);
			return false;
		}

		fclose(fp);
		return true;
		//std::ofstream ofs(name);
		//if (ofs.is_open() == false)
		//{
		//	std::cerr << "���ļ�ʧ��\n";
		//	return false;
		//}
		//ofs.seekp(offset, std::ios::binary);//�Ӷ�дλ����ת��������ļ���ʼλ�ÿ�ʼƫ��offset��ƫ����
		//ofs.write(&body[0], body.size());
		//if (ofs.good() == false)
		//{
		//	std::cerr << "���ļ�д������ʧ��\n";
		//	ofs.close();
		//	return false;
		//}
		//ofs.close();
		
	}
	// ָ�������ʾ����һ������Ͳ���
	// const& ��ʾ����һ�������Ͳ���
	// & ��ʾ����һ����������Ͳ���
	static bool Read(const std::string& name, std::string* body)
	{
		int64_t filesize = boost::filesystem::file_size(name);
		body->resize(filesize);
		std::cout << "��ȡ�ļ����ݣ�" << name << "size" << filesize << "\n";
		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "rb+");
		if (fp == NULL)
		{
			std::cerr << "���ļ�ʧ��\n";
		}
		size_t ret = fread(&(*body)[0].c_str(), 1, filesize, fp);
		if (ret != filesize)
		{
			std::cerr << "���ļ���ȡ����ʧ��\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		//std::ifstream ifs(name); //ʵ����
		//if (ifs.is_open() == false)
		//{
		//	std::cerr << "���ļ�ʧ��\n";
		//	return false;
		//} 
		//int64_t filesize = boost::filesystem::file_size(name);  // ͨ���ļ�����ȡ�ļ���С
		//body->resize(filesize);  // ��body�����ļ��ռ�

		////std::cout << "Ҫ��ȡ���ļ���С��" << name << ":" << filesize << std::endl;

		//ifs.read(&(*body)[0], filesize);  // ��ȡ�ļ���С
		///*if (ifs.good() == false)
		//{
		//std::cerr << "��ȡ�ļ�����ʧ��\n";
		//std::cout << *body << std::endl;
		//ifs.close();
		//return false;
		//}*/
		//ifs.close();
		return true;
	}

};

class Adapter//����һ��������
{
public:
	_Uint32t _ip_addr;//������IP��ַ
	_Uint32t _mask_addr;//�����ϵ���������
};
class AdapterUtil//����������������
{
public:
#ifdef _WIN32
	static bool GetAllAdapter(std::vector<Adapter> *list)// windows�»�ȡ��������Ϣʵ��
	{
		//IP_ADAPTER_INFO ��һ�������Ϣ�Ľṹ��
		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO();
		//GetAdaptersInfo window�»�ȡ������Ϣ�Ľӿ�--������Ϣ�п����ж������˴���һ��ָ��
		//��ȡ������Ϣ���ܻ�ʧ�ܣ���Ϊ�ռ䲻�㣬�����һ������Ͳ������������û���������������Ϣ�ռ�ռ�ô�С
		int64_t all_adapter_size = sizeof(IP_ADAPTER_INFO);
		// ���ճɹ����ķ��� ֵ
		int ret = GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
		if (ret == ERROR_BUFFER_OVERFLOW)
		{// ��������ʾ�������ռ䲻��
			//���¸�ָ������ռ�
			delete p_adapters;//���ͷ�
			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];
			GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size);
		}
		while (p_adapters) 
		{
			Adapter adapter;
			//��һ���ַ������ʮ����IP��ַ�������ַIP��ַ
			inet_pton(AF_INET, p_adapters->IpAddressList.IpAddress.String, &adapter._ip_addr);
			inet_pton(AF_INET, p_adapters->IpAddressList.IpMask.String, &adapter._mask_addr);
			if (adapter._ip_addr != 0)//��Щ������Ϣû������
			{
				list->push_back(adapter);//��������Ϣ��ӵ�vector�з��ظ��û�
				//if (p_adapters->IpAddressList.IpAddress.String != "0.0.0.0")
				/*std::cout << "�������ƣ�" << p_adapters->AdapterName << std::endl;
				std::cout << "����������" << p_adapters->Description << std::endl;
				std::cout << "IP��ַ��" << p_adapters->IpAddressList.IpAddress.String << std::endl;
				std::cout << "�������룺" << p_adapters->IpAddressList.IpMask.String << std::endl;*/
			}
			//std::cout << std::endl;
			p_adapters = p_adapters->Next;
		}
		delete p_adapters; 
		return true;
	}
#else
	bool GetAllAdapter(std::vector<Adapter> *list);// linux�»�ȡ��������Ϣ 
#endif
};