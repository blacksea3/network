#include "Log.h"

Mlog::Mlog(const std::string &name)
{
	_name = name;
	//std::cout << "This class Mlog, Mlog: object name:" << _name.c_str() << std::endl;
}

Mlog::~Mlog()
{
	//std::cout << "This class Mlog, ~Mlog: object name:" << _name.c_str() << std::endl;
}

//我不能找到Instance的函数定义,猜测为
//无输入
//返回Mlog的引用
Mlog* Mlog::Instance()
{
	if (pMlog == NULL)
	{
		pMlog = new Mlog(std::string("111"));
	}
	return pMlog;
}

/*
 * 打印信息:
 * s是自定义字符串
 * 目标打印一行信息, 含s和当前时间
 */
void Mlog::print(std::string filename, std::string content)
{
	//如果content有换行符, 就删掉它
	if (!content.empty() && content.back() == '\n') content.pop_back();

	std::ofstream out(filename.c_str(), std::ios::app);
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	char timestr[26];
	ctime_s(timestr, sizeof timestr, &end_time);
	out << content << " time: "<< timestr;  //注:timestr自带换行符
	out.close();
}

//销毁
void Mlog::destroy()
{
	if (pMlog != NULL)
	{
		delete pMlog;
		pMlog = NULL;
	}
}

Mlog *Mlog::pMlog = NULL;
