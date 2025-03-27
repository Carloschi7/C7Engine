#pragma once

#include "utils/types.h"
#include <type_traits>
#include <string>

static constexpr u32 compact_string_max_preallocated_buffer_size = 32;
u32 get_c_string_length(const char* c_string);
u32 get_c_string_length_no_null_terminating(const char* c_string);

namespace gfx {

	//Alternative to the standard std::string, this class is a zeroable version of std::string that is better
	//suited for memory arena allocation and was created to allow low level structs that want to have a sort of string
	//in its members to be completely memset to 0 without running into weird implementation bugs
	template<typename CharType>
	struct GenericCompactString
	{
		static_assert(std::is_same_v<CharType, char> || std::is_same_v<CharType, wchar_t>, "only supported for char and wchar_t at the moment");

		GenericCompactString() {}

		//Allows the user to write CompactString string = "hello world" instead of always having to type
		//CompactString string = compact_string_create(...);
		template<typename = std::enable_if_t<std::is_same_v<CharType, char>>>
		GenericCompactString(const char* c_string)
		{
			GenericCompactString<char> compact_string_create(const char* c_string);
			*this = compact_string_create(c_string);
		}

		template<typename = std::enable_if_t<std::is_same_v<CharType, char>>>
		GenericCompactString(const std::string& cpp_string)
		{
			GenericCompactString<char> compact_string_create(const std::string& cpp_string);
			*this = compact_string_create(cpp_string);
		}

		CharType stack_buffer[compact_string_max_preallocated_buffer_size];
		//Used if the string exceeds the local buffer size -> heap allocation
		CharType* heap_buffer;
		u32 count;
		bool stored_in_heap_buffer;
	};

	using CompactString = GenericCompactString<char>;

	CompactString compact_string_create(const char* c_string);
	CompactString compact_string_create(const std::string& cpp_string);
	void          compact_string_append(CompactString& compact_string, const char* c_string);

	bool          compact_string_match(const CompactString& first, const char* second);
	bool          compact_string_match(const CompactString& first, const CompactString& second);
	//This function allocates a c string using the temporary_allocate function, remember to use defer after calling
	char*         compact_string_c_str(const CompactString& compact_string);
	void          compact_string_cleanup(CompactString* compact_string);

	//TODO @C7 a wchar_t implementation could be handy in the future

	namespace test
	{
		void compact_string_run_tests();
	}
}