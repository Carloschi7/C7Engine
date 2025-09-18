#include "compact_string.h"
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

namespace gfx
{
	CompactString compact_string_create(const char* c_string)
	{
		CompactString str = {};
		std::memset(&str, 0, sizeof(CompactString));

		u32 string_length = get_c_string_length(c_string);
		if(string_length < compact_string_max_preallocated_buffer_size) {
			std::memcpy(str.stack_buffer, c_string, string_length);
			str.stored_in_heap_buffer = false;
		} else {
			str.heap_buffer = mem_allocate<char>(string_length);
			std::memcpy(str.heap_buffer, c_string, string_length);
			str.stored_in_heap_buffer = true;
		}

		str.count = string_length;
		return str;
	}

	CompactString compact_string_create(const std::string& cpp_string)
	{
		return compact_string_create(cpp_string.c_str());
	}

	void compact_string_append(CompactString& compact_string, const char* c_string)
	{
		const u32 size_of_string_to_append = get_c_string_length(c_string);
		if(compact_string.stored_in_heap_buffer) {
			auto current_ptr = compact_string.heap_buffer;
			defer { mem_free(current_ptr); };
			//TODO @C7 maybe add a capacity marker in the future? so we dont always allocate the exact size
			compact_string.heap_buffer = mem_allocate<char>(compact_string.count + size_of_string_to_append);
			std::memcpy(compact_string.heap_buffer, current_ptr, compact_string.count);
		} else {
			if(compact_string.count + size_of_string_to_append < compact_string_max_preallocated_buffer_size) {
				std::memcpy(compact_string.heap_buffer + compact_string.count, c_string, size_of_string_to_append);
			} else {
				compact_string.heap_buffer = mem_allocate<char>(compact_string.count + size_of_string_to_append);
				//Copy the old string to the heap allocation and then append
				std::memcpy(compact_string.heap_buffer, compact_string.stack_buffer, compact_string.count);
				std::memcpy(compact_string.heap_buffer + compact_string.count, c_string, size_of_string_to_append);
				//We dont delete or reset the contents that are left in the old stack buffer, the flag below will make
				//the string automatically load the heap buffer when doing string operations
				compact_string.stored_in_heap_buffer = true;
			}
		}

		compact_string.count += size_of_string_to_append;
	}

	bool compact_string_match(const CompactString& first, const char* second)
	{
		const u32 second_string_count = get_c_string_length(second);
		if(first.count != second_string_count)
			return false;

		auto first_pointer  = first.stored_in_heap_buffer ? first.heap_buffer : first.stack_buffer;

		return (std::memcmp(first_pointer, second, second_string_count) == 0);
	}

	bool compact_string_match(const CompactString& first, const CompactString& second)
	{
		if(first.count != second.count)
			return false;

		auto first_pointer  = first.stored_in_heap_buffer ? first.heap_buffer : first.stack_buffer;
		auto second_pointer = second.stored_in_heap_buffer ? second.heap_buffer : second.stack_buffer;

		const u32 count = first.count;
		return (std::memcmp(first_pointer, second_pointer, count) == 0);
	}

	char* compact_string_c_str(const CompactString& compact_string) {
		//The +1 is for the \0
		auto c_string_pointer = temporary_allocate<char>(compact_string.count + 1);
		auto original_buffer  = compact_string.stored_in_heap_buffer ? compact_string.heap_buffer : compact_string.stack_buffer;
		assert(original_buffer, "no heap allocation was detected, despite what the flag said, fix this");
		std::memcpy(c_string_pointer, original_buffer, compact_string.count);
		c_string_pointer[compact_string.count] = '\0';
		return c_string_pointer;
	}

	void compact_string_cleanup(CompactString* compact_string)
	{
		assert(compact_string, "cannot delete a null object\n");
		if(!compact_string->stored_in_heap_buffer) return;

		mem_free(compact_string->heap_buffer);
	}

	namespace test
	{
		void compact_string_run_tests()
		{
			Allocator allocator = allocator_create(1024, 1024);
			g_engine_allocator = &allocator;
			defer {
				allocator_cleanup(&allocator);
				g_engine_allocator = nullptr;
			};

			{
				CompactString current_string = compact_string_create("Hello world");
				defer { compact_string_cleanup(&current_string); };
				char* ptr = compact_string_c_str(current_string);
				defer { temporary_free_c_str(ptr); };
			}

			{
				CompactString current_string = compact_string_create("World hello");
				defer { compact_string_cleanup(&current_string); };
				char* ptr = compact_string_c_str(current_string);
				defer { temporary_free_c_str(ptr); };
			}


			{
				std::string to_copy = "Ok now lets try to write a string that will 100% be allocated on the heap, lets see how it goes";
				CompactString current_string = to_copy;
				defer { compact_string_cleanup(&current_string); };
				char* ptr = compact_string_c_str(current_string);
				defer { temporary_free_c_str(ptr); };
			}

			{
				//30 character string
				CompactString current_string = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
				compact_string_append(current_string, "aaa");
				char* ptr = compact_string_c_str(current_string);
				defer { temporary_free_c_str(ptr); };
			}
		}
	}
}