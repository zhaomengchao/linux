#pragma once
#ifdef _WIN32
#include<iostream>
#include<WS2tcpip.h>
#include<Iphlpapi.h>
#include<vector>
#pragma comment(lib, "Iphlpapi.lib")
#else
//linuxͷ�ļ�
#endif

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
		int all_adapter_size = sizeof(IP_ADAPTER_INFO);
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
			std::cout << "�������ƣ�" << p_adapters->AdapterName << std::endl;
			std::cout << "����������" << p_adapters->Description << std::endl;
			std::cout << "IP��ַ��" << p_adapters->IpAddressList.IpAddress.String << std::endl;
			std::cout << "�������룺" << p_adapters->IpAddressList.IpMask.String << std::endl;
			p_adapters = p_adapters->Next;
		}
		delete p_adapters; 
		return true;
	}
#else
	bool GetAllAdapter(std::vector<Adapter> *list);// linux�»�ȡ��������Ϣ 
#endif
};