#define ZLIB_API
#define ZLIB_API_VAL
#include "xmalloc_new_delete.h"
#include "archive/pch.h"
#include "test_refl.h"
using namespace api;
using namespace std;
using namespace refl;
int main() {
	{
		using T = Guid;
		T a(1,1.0);
		auto ppp = new int(120);
		T* c = new T();
		a.view = "hello";
		auto text = TextSerialize(a);
		auto b = TextDeserialize<T>(text);
		if (b) {
			auto c = b.value();
		}
	}
	{
		using T = Guid;
		T a(1, 1.0);
		a.view = "hello";
		auto text = JsonSerialize(a);
		auto b = JsonDeserialize<T>(text);
		if (b) {
			auto c = b.value();
		}
	}
	{
		using T = std::basic_string<char>;
		T a = "123456";
		auto text = TextSerialize(a);
		auto b = TextDeserialize<T>(text);
		if (b) {
			auto c = b.value();
		}
	}
	std::cout << "hello world" << std::endl;
}