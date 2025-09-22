#include "containers.h"
#include "MainIncl.h"

extern gfx::Allocator* g_engine_allocator;

u32 get_c_string_length(const char* c_string)
{
	u32 size = 0;
	for(; c_string[size] != 0; size++) {}
	//Returns the size of the string + the null terminating character
	return size + 1;
}

u32 get_c_string_length_no_null_terminating(const char* c_string)
{
	u32 size = 0;
	for(; c_string[size] != 0; size++) {}
	return size;
}

char* string_merge(const char* str1, const char* str2)
{
	const u32 first_buffer_size  = get_c_string_length_no_null_terminating(str1);
	const u32 second_buffer_size = get_c_string_length(str2);
	char* buffer = gfx::temporary_allocate<char>(first_buffer_size + second_buffer_size);

	std::memcpy(buffer, str1, first_buffer_size);
	std::memcpy(buffer + first_buffer_size, str2, second_buffer_size);
	return buffer;
}


namespace gfx {
	void test_string()
	{
		//Can be used to test the string when some new features get implemented
	}
}
