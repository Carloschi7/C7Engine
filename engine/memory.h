#pragma once

//TOBECONTINUED @C7, insert the file in the cmake build
namespace gfx
{
	//Jai inspired this and its very fast
	struct TemporaryStorage
	{
		void* buffer;
		u32 size;
		u32 used;
	};

	struct PermanentStorage
	{
		void* buffer;
		u32 size;
		u32 used;

		struct Segment
		{
			u32 start;
			u32 size;
		}

		Segment* memory_segments;
	}

	static Allocator* g_engine_allocator = nullptr;

	Allocator allocator_create(u32 permanent_storage_bytes, u32 temporary_storage_bytes);

	//This functions will either use the g_engine_allocator functionalities or allocate memory from the standard heap
	//allocate_temporary and free_temporary behave the same way to allocate and free if g_engine_allocator is not defined
	void* allocate(u32 bytes);
	void  free(void* ptr);
	void* allocate_temporary(u32 bytes);
	void  free_temporary(void* ptr);
}