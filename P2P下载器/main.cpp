#include"util.hpp"

int main(int argc, char *argv[])
{
	std::vector<Adapter> list;
	AdapterUtil::GetAllAdapter(&list);
	system("pause");
	return 0;
}