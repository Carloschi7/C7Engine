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

void file_open(FILE** file, const char* filename, const char* options)
{
#ifdef DEVELOPER
	fopen_s(file, filename, options);
#else
	String filename_bin = filename;
	filename_bin.cut_from_end(filename_bin.size() - filename_bin.find_last_of('.'));
	filename_bin += ".bin";
	fopen_s(file, filename_bin.c_str(), options);
#endif
}

char* file_get_entire_content(FILE* file, u64* out_bytes_read) //@Free memory
{
    fseek(file, 0, SEEK_END);
	u64 file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

	auto file_buffer = gfx::mem_allocate<char>(file_size + 1);
	const u64 bytes_read = fread(file_buffer, 1, file_size, file);
#ifdef DEVELOPER
	if(out_bytes_read)
		*out_bytes_read = bytes_read;

	file_buffer[file_size] = 0;
	return file_buffer;
#else
	extern gfx::EncryptionKey g_engine_key;
	auto decrypted_buffer = gfx::decrypt_buffer(file_buffer, bytes_read, g_engine_key);
	gfx::mem_free(file_buffer);

	if(out_bytes_read)
		*out_bytes_read = gfx::get_decrypted_buffer_size(bytes_read);

	decrypted_buffer[gfx::get_decrypted_buffer_size(bytes_read)] = 0;
	return decrypted_buffer;
#endif
}

u64 file_get_size(FILE* file)
{
	u64 current_position = ftell(file);
	fseek(file, 0, SEEK_END);
	u64 size = ftell(file);
	fseek(file, current_position, SEEK_SET);
	return size;
}

void file_close(FILE* file)
{
	fclose(file);
}

namespace gfx {
	void test_string()
	{
		//Can be used to test the string when some new features get implemented
	}
}
