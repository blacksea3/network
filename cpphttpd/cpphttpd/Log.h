#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <ctime>

//µ¥ÀýMlog
class Mlog
{
public:
	static Mlog* Instance();

	void print(std::string filename, std::string content);
	void destroy();

private:
	std::string _name;
	Mlog(std::string const &name);
	~Mlog();
	static Mlog *pMlog;
};
