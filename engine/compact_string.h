#pragma once

#include "utils/types.h"
#include <type_traits>
#include <string>
#include "memory.h"

static constexpr u32 compact_string_max_preallocated_buffer_size = 32;
u32  get_c_string_length(const char* c_string);
u32  get_c_string_length_no_null_terminating(const char* c_string);
//INFO buffer needs to be FREED after function call
char* string_merge(const char* str1, const char* str2);

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

template <typename CharType>
class GenericString
{
	//wchar_t coming soon
	static_assert(std::is_same_v<CharType, char>, "String is only used for char types");
	static constexpr u32 stack_buffer_size = 24;
public:
	GenericString() {}
	GenericString(const char* string)
	{
		string_size = get_c_string_length_no_null_terminating(string);
		if(string_size < stack_buffer_size) {
			std::memcpy(stack_buffer, string, string_size);
		} else {
			heap_capacity = (string_size * 3) / 2;
			heap_buffer = gfx::mem_allocate<CharType>(heap_capacity);
			std::memcpy(heap_buffer, string, string_size);
		}
	}

	GenericString(const GenericString& right) noexcept
	{
		string_size = right.string_size;
		if(right.heap_buffer) {
			heap_capacity = right.heap_capacity;
			heap_buffer = gfx::mem_allocate<CharType>(heap_capacity);
			std::memcpy(heap_buffer, right.heap_buffer, string_size);
		} else {
			std::memcpy(stack_buffer, right.stack_buffer, string_size);
		}
	}

	GenericString(GenericString&& right) noexcept
	{
		string_size = right.string_size;
		if(heap_buffer) {
			heap_buffer = right.heap_buffer;
			right.heap_buffer = nullptr;
		} else {
			std::memcpy(stack_buffer, right.stack_buffer, string_size);
		}
	}

	u32 size()
	{
		return string_size;
	}

	void append(const char* string)
	{
		const u32 size = get_c_string_length_no_null_terminating(string);
		if(!heap_buffer) {
			if(string_size + size <= stack_buffer_size) {
				std::memcpy(stack_buffer + string_size, string, size);
			} else {
				heap_capacity = ((string_size + size) * 3) / 2;
				heap_buffer = gfx::mem_allocate<CharType>(heap_capacity);
				std::memcpy(heap_buffer, stack_buffer, string_size);
				std::memcpy(heap_buffer + string_size, string, size);
			}

		} else {
			if(string_size + size >= heap_capacity) {
				heap_capacity += size * 2;
				CharType* new_heap_buffer = gfx::mem_allocate<CharType>(heap_capacity);
				std::memcpy(new_heap_buffer, heap_buffer, string_size);
				gfx::mem_free(heap_buffer);
				heap_buffer = new_heap_buffer;
			}
			std::memcpy(heap_buffer + string_size, string, size);
		}


		string_size += size;
	}

	void operator+=(const char* string)
	{
		append(string);
	}

	const CharType* data()
	{
		return data();
	}

	const CharType* data() const
	{
		CharType* buf = nullptr;
		if(!heap_buffer && string_size == stack_buffer_size) {
			//INFO @C7 the string internally is stored without the \0, so if we want to forward a string
			//as a c-string, we need to add it at the end, so if the string is fully occupying the stack
			//buffer, we need to move the string to the heap so that we are able to add the \0 without reducing
			//or changing the string
			heap_capacity = (string_size * 3) / 2;
			heap_buffer = gfx::mem_allocate<CharType>(heap_capacity);
			std::memcpy(heap_buffer, stack_buffer, string_size);

			buf = heap_buffer;
		} else if(heap_buffer) {
			buf = heap_buffer;
		} else {
			buf = stack_buffer;
		}

		buf[string_size] = 0;
		return buf;
	}

	CharType& operator[](u32 index)
	{
		if(heap_buffer)
			return heap_buffer[index];

		return stack_buffer[index];
	}

	const CharType& operator[](u32 index) const
	{
		if(heap_buffer)
			return heap_buffer[index];

		return stack_buffer[index];
	}

	void cut_from_start(u32 index)
	{
		CharType* buf = heap_buffer ? heap_buffer : stack_buffer;

		for(u32 i = 0; i < string_size - index; i++) {
			buf[i] = buf[index + i];
		}
		for(u32 i = 0; i < index; i++) {
			buf[string_size - i - 1] = 0; //TEST THIS
		}

		string_size -= index;
		if(string_size <= stack_buffer_size) {
			std::memcpy(stack_buffer, heap_buffer, string_size);
			gfx::mem_free(heap_buffer);
			heap_buffer = nullptr;
			heap_capacity = 0;
		}
	}

	void cut_from_end(u32 index)
	{
		if(heap_buffer && string_size > stack_buffer_size && string_size - index <= stack_buffer_size) {
			std::memset(stack_buffer, 0, stack_buffer_size);
			std::memcpy(stack_buffer, heap_buffer, string_size - index);
			gfx::mem_free(heap_buffer);
			heap_buffer = nullptr;
			heap_capacity = 0;
		} else {
			CharType* buf = heap_buffer ? heap_buffer : stack_buffer;
			std::memset(buf + string_size - index, 0, index);
		}

		string_size -= index;
	}

	~GenericString()
	{
		if(heap_buffer)
			gfx::mem_free(heap_buffer);

		string_size = 0;
	}

private:
	CharType stack_buffer[stack_buffer_size];
	CharType* heap_buffer = nullptr;
	u32 heap_capacity;
	u32 string_size;
};

using String = GenericString<char>;