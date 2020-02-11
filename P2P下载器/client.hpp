#pragma once
#include<thread>
#include"util.hpp" 

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
				host._ip_addr = host_ip;
				host._pair_ret = false;
				host_list.push_back(host);
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
		}
		//3.���������õ���Ӧ�����Ӧ����Ϊ������������IP��ӵ�_online_host�б���
		//4.��ӡ���������б�ʹ�û�ѡ�� 
	}
private:
	std::vector<Host> _online_host;//��������
};