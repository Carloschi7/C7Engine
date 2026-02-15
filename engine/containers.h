#pragma once

#include "utils/types.h"
#include <type_traits>
#include <string>
#include "memory.h"

#define local_assert(x, msg) if(!(x)) { *(int*)0 = 0; }

u32  get_c_string_length(const char* c_string);
u32  get_c_string_length_no_null_terminating(const char* c_string);
//INFO buffer needs to be FREED after function call
char* string_merge(const char* str1, const char* str2);

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
		operator=(string);
	}

	GenericString(const GenericString& string)
	{
		operator=(string);
	}

	GenericString(GenericString&& right)
	{
		operator=(static_cast<GenericString&&>(right));
	}


	GenericString(const char* string, u32 size)
	{
		u32 actual_size = get_c_string_length_no_null_terminating(string);
		local_assert(size < actual_size, "specified size must be smaller than the actual string size");

		CharType* buf = gfx::temporary_allocate<CharType>(size + 1);

		std::memcpy(buf, string, size);
		buf[size] = 0;
		*this = buf;
		gfx::temporary_free(buf);
	}

	GenericString& operator=(const char* string)
	{
		string_size = get_c_string_length_no_null_terminating(string);
		if(string_size < stack_buffer_size) {
			std::memset(stack_buffer, 0, stack_buffer_size);
			std::memcpy(stack_buffer, string, string_size);
			stack_buffer[string_size] = 0;

			if (heap_buffer)
				_free_heap();

		} else {
			if(heap_buffer) {
				if(string_size < heap_capacity) {
					std::memset(heap_buffer, 0, heap_capacity);
					std::memcpy(heap_buffer, string, string_size);
					return *this;
				} else {
					_free_heap();
				}
			}

			heap_capacity = (string_size * 3) / 2;
			heap_buffer = gfx::mem_allocate_zeroed<CharType>(heap_capacity);
			std::memcpy(heap_buffer, string, string_size);

			std::memset(stack_buffer, 0, stack_buffer_size);
		}

		return *this;
	}

	GenericString& operator=(const GenericString& string) noexcept
	{
		string_size = string.string_size;
		if(string.heap_buffer) {
			heap_capacity = string.heap_capacity;
			heap_buffer = gfx::mem_allocate_zeroed<CharType>(heap_capacity);
			std::memcpy(heap_buffer, string.heap_buffer, string_size);
		} else {
			std::memcpy(stack_buffer, string.stack_buffer, string_size);
			stack_buffer[string_size] = 0;
		}

		return *this;
	}

	GenericString& operator=(GenericString&& right) noexcept
	{
		string_size = right.string_size;
		if(right.heap_buffer) {
			heap_buffer   = right.heap_buffer;
			heap_capacity = right.heap_capacity;
			right.heap_buffer = nullptr;
		} else {
			std::memcpy(stack_buffer, right.stack_buffer, string_size);
			stack_buffer[string_size] = 0;
		}

		return *this;
	}

	u32 size()
	{
		return string_size;
	}

	void append(const CharType ch)
	{
		CharType buf[2] = {};
		buf[0] = ch;
		append(buf);
	}

	void append(const CharType* string)
	{
		const u32 size = get_c_string_length_no_null_terminating(string);
		if(!heap_buffer) {
			if(string_size + size < stack_buffer_size) {
				std::memcpy(stack_buffer + string_size, string, size);
			} else {
				heap_capacity = ((string_size + size) * 3) / 2;
				heap_buffer = gfx::mem_allocate_zeroed<CharType>(heap_capacity);
				std::memcpy(heap_buffer, stack_buffer, string_size);
				std::memcpy(heap_buffer + string_size, string, size);

				std::memset(stack_buffer, 0, stack_buffer_size);
			}

		} else {
			if(string_size + size >= heap_capacity) {
				//Still allocate a bit of quantity so that even if we are appending a single char we are not
				//reallocating for just 2 slots
				heap_capacity += (size > 3 ? size : 3) * 2;
				CharType* new_heap_buffer = gfx::mem_allocate_zeroed<CharType>(heap_capacity);
				std::memcpy(new_heap_buffer, heap_buffer, string_size);
				gfx::mem_free(heap_buffer);
				heap_buffer = new_heap_buffer;
			}
			std::memcpy(heap_buffer + string_size, string, size);
		}


		string_size += size;
	}

	void append_s32(const s32 value)
	{
		CharType buf[16] = {};
		_itoa_s(value, buf, sizeof(buf), 10);
		append(buf);
	}

	void append_u32(const u32 value)
	{
		CharType buf[16] = {};
		_ultoa_s(value, buf, sizeof(buf), 10);
		append(buf);
	}

	void operator+=(const CharType ch)
	{
		append(ch);
	}
	void operator+=(const CharType* string)
	{
		append(string);
	}
	void operator+=(const GenericString& string)
	{
		append(string.c_str());
	}

	const CharType* data() const
	{
		return heap_buffer ? heap_buffer : stack_buffer;
	}

	const CharType* c_str() const
	{
		return data();
	}

	CharType& operator[](u32 index)
	{
		local_assert(index < string_size, "index out of bounds");
		if(heap_buffer)
			return heap_buffer[index];

		return stack_buffer[index];
	}

	const CharType& operator[](u32 index) const
	{
		local_assert(index < string_size, "index out of bounds");
		if(heap_buffer)
			return heap_buffer[index];

		return stack_buffer[index];
	}

	void cut_from_start(u32 index)
	{
		if(index > string_size)
			return;

		CharType* buf = data();

		for(u32 i = 0; i < string_size - index; i++) {
			buf[i] = buf[index + i];
		}
		for(u32 i = 0; i < index; i++) {
			buf[string_size - i - 1] = 0;
		}

		string_size -= index;
		if(heap_buffer && string_size < stack_buffer_size) {
			std::memcpy(stack_buffer, heap_buffer, string_size);
			_free_heap();
		}
	}

	void cut_from_end(u32 index)
	{
		if(index > string_size)
			return;

		if(heap_buffer && string_size >= stack_buffer_size && string_size - index < stack_buffer_size) {
			std::memset(stack_buffer, 0, stack_buffer_size);
			std::memcpy(stack_buffer, heap_buffer, string_size - index);
			_free_heap();
		} else {
			CharType* buf = data();
			std::memset(buf + string_size - index, 0, index);
		}

		string_size -= index;
	}

	bool operator==(const GenericString& string) const
	{
		if(string_size != string.string_size)
			return false;

		const CharType* first_buf  = data();
		const CharType* second_buf = string.data();

		for(u32 i = 0; i < string_size; i++) {
			if(first_buf[i] != second_buf[i])
				return false;
		}

		return true;
	}

	bool operator==(const char* string) const
	{
		if(string_size != get_c_string_length_no_null_terminating(string))
			return false;

		const CharType* first_buf  = data();
		const CharType* second_buf = string;

		for(u32 i = 0; i < string_size; i++) {
			if(first_buf[i] != second_buf[i])
				return false;
		}

		return true;
	}

	bool operator!=(const GenericString& string) const
	{
		return !operator==(string);
	}

	bool operator!=(const char* string) const
	{
		return !operator==(string);
	}

	s32 find_first_of(CharType c) const
	{
		const CharType* buf = data();
		for(s32 i = 0; i < (s32)string_size; i++) {
			if(buf[i] == c)
				return i;
		}

		return -1;
	}

	s32 find_last_of(CharType c) const
	{
		const CharType* buf = data();
		for(s32 i = 0; i < (s32)string_size; i++) {
			if(buf[(string_size - 1) - i] == c)
				return (string_size - 1) - i;
		}

		return -1;
	}

	GenericString substr(u32 begin, u32 end) const
	{
		if(begin >= string_size || end >= string_size)
			return {};

		return GenericString(data() + begin, end - begin);
	}

	//Actual resize/reserve not implemented atm
	void allocate_buffer(u32 size)
	{
		clear();

		if(size >= stack_buffer_size) {
			gfx::mem_allocate_zeroed<CharType>(size);
			heap_capacity = size;
		}

		//Otherwise just mark that the string is tot characters long
		string_size = size;
	}

	void clear()
	{
		if(heap_buffer)
			_free_heap();
		else
			std::memset(stack_buffer, 0, stack_buffer_size);

		string_size = 0;
	}

	bool empty() const
	{
		return string_size == 0;
	}

	~GenericString()
	{
		if(heap_buffer)
			_free_heap();

		string_size = 0;
	}

	CharType* data()
	{
		return heap_buffer ? heap_buffer : stack_buffer;
	}

private:

	void _free_heap()
	{
		gfx::mem_free(heap_buffer);
		heap_capacity = 0;
		heap_buffer = nullptr;
	}

	CharType stack_buffer[stack_buffer_size] = {};
	CharType* heap_buffer = nullptr;
	u32 heap_capacity = 0;
	u32 string_size = 0;
};

template<typename T>
GenericString<T> operator+(const GenericString<T>& a, const GenericString<T>& b)
{
	static_assert(std::is_same_v<T, char>, "Only char is supported at the moment");
	GenericString<T> result = a;
	result += b;
	return result;
}

template<typename T>
GenericString<T> operator+(const GenericString<T>& a, const T* b)
{
	static_assert(std::is_same_v<T, char>, "Only char is supported at the moment");
	GenericString<T> result = a;
	result += b;
	return result;
}

using String = GenericString<char>;

namespace gfx {
	void test_string();
}

#undef local_assert