#include"util.hpp"
#include"httplib.h"

void helloworld(const httplib::Request &req, httplib::Response &rsp)
{
	rsp.set_content("<html><h1>HelloWord</h1></html>", "text/html");
	rsp.status = 200;
}

int main(int argc, char *argv[])
{
	/*std::vector<Adapter> list;
	AdapterUtil::GetAllAdapter(&list);*/
	httplib::Server srv;

	srv.Get("/", helloworld);
	srv.listen("0.0.0.0", 9000);
    system("pause");
	return 0;
}