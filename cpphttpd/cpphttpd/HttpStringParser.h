#pragma once
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <iostream>

enum HTTPMETHOD
{
	HTTPMETHOD_GET_COMMON,
	HTTPMETHOD_POST,
	HTTPMETHOD_OTHER,
	HTTPMETHOD_GET_PARAERROR
};

struct httpMethodStr
{
	HTTPMETHOD httpMethod;
	std::string dir;
	std::unordered_map<std::string, std::string> contentPairs;

	httpMethodStr(HTTPMETHOD h = HTTPMETHOD::HTTPMETHOD_OTHER, std::string d = "", 
		std::unordered_map<std::string, std::string> cP = {})
	{
		httpMethod = h;
		dir = d;
		contentPairs = cP;
	}

	//==ÔËËã·ûÖØÔØ
	bool operator==(const httpMethodStr &hms)
	{
		return (this->httpMethod == hms.httpMethod && 
			this->dir == hms.dir &&
			this->contentPairs == hms.contentPairs
			);
	}
};

httpMethodStr parserFirstLine(std::string input);
void parserFirstLineTest(void);
