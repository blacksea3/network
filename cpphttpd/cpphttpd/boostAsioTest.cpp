//boostÐèÒª
#ifdef _MSC_VER
#define _WIN32_WINNT 0x0601   
#endif
#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING

//¿â
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void print(const boost::system::error_code& /*e*/,
	boost::asio::deadline_timer* t, int* count)
{
	if (*count < 5)
	{
		std::cout << *count << std::endl;
		++(*count);

		t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
		t->async_wait(boost::bind(print,
			boost::asio::placeholders::error, t, count));
	}
}

int main()
{
	boost::asio::io_context io;

	int count = 0;
	boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
	t.async_wait(boost::bind(print,
		boost::asio::placeholders::error, &t, &count));

	io.run();

	std::cout << "Final count is " << count << std::endl;

	return 0;
}