#include "HttpStringParser.h"

//解析第一行
//按照以下"严格"规则:
//先判断是否存在空格, 若存在则切分开第一个空格和后面的
//判断第一个单词属于GET, POST或其他
//若是其他直接退出函数, 对应httpMethod = HTTPMETHOD_OTHER, 另外两个为空
//若是GET则继续用空格切分(第一个), 对第一段进行解析, 第二段丢弃
//  如果第一段中没有问号, 说明是无参数GET; 如果此时末尾是/则去除
//     去除/后如果为空则改成index.html.  结果放入dir, contentPairs清空, 返回函数
//  否则说明是有参数GET:用第一个把字符串切开, 第一段放入dir, 第二段进行如下操作
//     全部用&切开, 每一段用=切开(第一个), 前一部分是键, 后一部分是值, 放入contentPairs, 返回函数
//       如果某一段没有=导致无法分开, 那么当作HTTPMETHOD_GET_PARAERROR返回, dir不为空, contentPairs为空
//若是POST则继续用空格切分, 把第一段放入dir, 第二段丢弃, contentPairs清空, 返回函数

//例子 "GET /ddasads HTTP/1.1
//     "GET /dssd/dd?i=1&p=2 HTTP/1.1"

httpMethodStr parserFirstLine(std::string input)
{
	httpMethodStr returnhms;
	size_t loc;
	loc = input.find(' ');
	if (loc != input.npos)
	{
		std::string pre = input.substr(0, loc);
		std::string aft = input.substr(loc + 1);
		std::transform(pre.begin(), pre.end(), pre.begin(), ::toupper);
		if (pre == "GET")
		{
			loc = aft.find(' ');
			pre = aft.substr(0, loc);
			aft = aft.substr(loc + 1);

			loc = pre.find('?');
			if (loc == pre.npos) //无参数GET
			{
				if (pre.back() == '/') pre.pop_back();
				returnhms.dir = pre;
				returnhms.httpMethod = HTTPMETHOD::HTTPMETHOD_GET_COMMON;
				return returnhms;
			}
			else  //有参数GET
			{
				aft = pre.substr(loc + 1);
				pre = pre.substr(0, loc);
				returnhms.dir = pre;
				int aIndex = aft.find('&');
				while (aIndex != aft.npos)
				{
					std::string innerPre = aft.substr(0, aIndex);
					size_t innerSlice = innerPre.find('=');
					if (innerSlice == innerPre.npos)   //解析错误, 返回HTTPMETHOD_GET_PARAERROR
					{
						returnhms.httpMethod = HTTPMETHOD::HTTPMETHOD_GET_PARAERROR;
						returnhms.contentPairs = {};
						return returnhms;
					}
					returnhms.contentPairs[innerPre.substr(0, innerSlice)] = innerPre.substr(innerSlice + 1);
					aft = aft.substr(aIndex + 1);
					aIndex = aft.find('&');
				}
				if (!aft.empty())
				{
					//最后一段
					std::string innerPre = aft.substr(0, aIndex);
					size_t innerSlice = innerPre.find('=');
					if (innerSlice == innerPre.npos)   //解析错误, 返回HTTPMETHOD_GET_PARAERROR
					{
						returnhms.httpMethod = HTTPMETHOD::HTTPMETHOD_GET_PARAERROR;
						returnhms.contentPairs = {};
						return returnhms;
					}
					returnhms.contentPairs[innerPre.substr(0, innerSlice)] = innerPre.substr(innerSlice + 1);
				}

				returnhms.httpMethod = HTTPMETHOD::HTTPMETHOD_GET_COMMON;
				return returnhms;
			}

		}
		else if (pre == "POST")
		{
			loc = aft.find(' ');
			pre = aft.substr(0, loc);
			aft = aft.substr(loc + 1);

			returnhms.dir = pre;
			returnhms.httpMethod = HTTPMETHOD::HTTPMETHOD_POST;
			return returnhms;
		}
		else
		{
			returnhms.httpMethod = HTTPMETHOD::HTTPMETHOD_OTHER;
			return returnhms;
		}
	}
	else
	{
		returnhms.httpMethod = HTTPMETHOD::HTTPMETHOD_OTHER;
		return returnhms;
	}
}

void parserFirstLineTest(void)
{
	std::vector<std::string> inputs = 
	{
		"GET /foo HTTP/1.1",
		"get / HTTP/1.1",
		"GeT /foo/ HTTP/1.1",
		"GET /foo/bar HTTP/1.1",
		"GEt foo/ HTTP/1.1",
		"GEt foo HTTP/1.1",

		"GET /foo ",
		"GET /foo  ",
		"GET /foo  s",

		"GET ? HTTP/1.1",
		"GET foo/?a=2 HTTP/1.1",
		"GET foo/?a=2&b=3 HTTP/1.1",
		"GET foo/?a=2&b=3HTTP/1.1",
		"GET foo/?a2&b=3HTTP/1.1",

		"GET ?a=2 HTTP/1.1",
		"GET ?a=2&b=3 HTTP/1.1",
		"GET ?a=2&b=3HTTP/1.1",
		"GET ?a2&b=3HTTP/1.1",

		"posT /foo/ HTTP/1.1",
		"POST /foo HTTP/1.1",
		"POST /foodsad!DSP%:>a sda w4 aAA HTTP/1.1",
	
		"GS /foo HTTP/1.1",
		""
	};

	std::vector<httpMethodStr> ress =
	{
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "/foo", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "/foo", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "/foo/bar", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "foo", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "foo", {}),

		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "/foo", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "/foo", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "/foo", {}),

		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "foo/", {{"a", "2"}}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "foo/", {{"a", "2"}, {"b", "3"} }),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "foo/", {{"a", "2"}, {"b", "3HTTP/1.1"} }),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_PARAERROR, "foo/", {}),

		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "", {{"a", "2"} }),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "", {{"a", "2"}, {"b", "3"} }),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_COMMON, "", {{"a", "2"}, {"b", "3HTTP/1.1"} }),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_GET_PARAERROR, "", {}),

		httpMethodStr(HTTPMETHOD::HTTPMETHOD_POST, "/foo/", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_POST, "/foo", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_POST, "/foodsad!DSP%:>a", {}),

		httpMethodStr(HTTPMETHOD::HTTPMETHOD_OTHER, "", {}),
		httpMethodStr(HTTPMETHOD::HTTPMETHOD_OTHER, "", {})
	};

	_ASSERT(inputs.size() == ress.size());

	for (size_t index = 0; index < inputs.size(); ++index)
	{
		httpMethodStr tempres = parserFirstLine(inputs[index]);

		_ASSERT(ress[index] == tempres);
	}

	std::cout << "parserFirstLineTest ok" << std::endl;
}
