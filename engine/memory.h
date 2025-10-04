#pragma once
#include <type_traits>
#include "utils/types.h"

namespace gfx
{
	struct MemorySegment
	{
		u32 start, size;
	};

	enum _NodeColor : u8
	{
		NODE_COLOR_UNDEFINED    = 0x00,
		NODE_COLOR_BLACK        = 0x01,
		NODE_COLOR_RED          = 0x02,
		//Marks imperfections when performing deletions that need to be solved
		NODE_COLOR_DOUBLE_BLACK = 0x04
	};

	struct _Node
	{
		//segment.start is treated as the ordering id
		MemorySegment segment;
		_NodeColor color;

		_Node* parent;
		_Node *left, *right;
	};

	class SegmentTree
	{
	public:
		SegmentTree() {}
		~SegmentTree() {
			if(root) { cleanup(); }
		}
		void add_node(u32 start, u32 size);
		_Node** find_place_to_insert_node(u32 start, _Node** parent);
		_Node* find_node(u32 start);
		const _Node* find_first_node();
		const _Node* find_last_node();
		void remove_node(u32 start);
		void cleanup();

		inline const _Node* get_root() const { return root; }
	private:
		//INFO @C7: In addition to performing the insertion/deletion operations, the functions also can make sure
		//with recursive calls that the tree is still valid according to the RB tree rules, the flag in the
		//tree_delete_check function explicitly says if the recursive calls perform a deletion or just
		//rearranges the newly created tree structure
		void tree_insert_check(_Node* node);
		void tree_delete_check(_Node* node, bool perform_deletion);

		_Node* root = nullptr;
	};

	struct MemoryStorage
	{
		void* buffer;
		u32 size;
		u32 used;
	};

	struct Allocator
	{
		MemoryStorage temporary_storage;
		MemoryStorage permanent_storage;
		//Maps allocations in the permanent_storage
		SegmentTree segment_tree;
		//This basically marks the number of allocation done in O(n*log(n)) time complexity
		//to cover holes generated from deallocations. If the value is zero the allocation
		//takes O(log(n))
		u32 dense_allocations_left = 0;
	};

	void assert_red_black_tree_validity(const SegmentTree& segment_tree);

	namespace test
	{
		void memory_run_tests();
	}

	//static Allocator* g_engine_allocator = nullptr;
	Allocator allocator_create(u32 permanent_storage_bytes, u32 temporary_storage_bytes);
	void      allocator_cleanup(Allocator* allocator);

	//This functions will either use the g_engine_allocator functionalities or mem_allocate memory from the standard heap
	//allocate_temporary and free_temporary behave the same way to mem_allocate and mem_free if g_engine_allocator is not defined
	void* mem_allocate(u32 bytes);
	void* mem_allocate_zeroed(u32 bytes);
	void  mem_free(void* ptr);
	void* temporary_allocate(u32 bytes);
	void  temporary_free(void* ptr);
	//Frees a c_string allocated in the temporary storage
	void  temporary_free_c_str(char* c_string);
	void  temporary_decrease_counter(u32 bytes);

	template<typename T>
	T* mem_allocate(u32 count = 1)
	{
		//TODO @C7 should also check for triviality in the future
		static_assert(std::is_standard_layout_v<T>, "T in an invalid type");
		return reinterpret_cast<T*>(mem_allocate(count * sizeof(T)));
	}

	template<typename T>
	T* mem_allocate_zeroed(u32 count = 1)
	{
		//TODO @C7 should also check for triviality in the future
		static_assert(std::is_standard_layout_v<T>, "T in an invalid type");
		return reinterpret_cast<T*>(mem_allocate_zeroed(count * sizeof(T)));
	}

	//only for structs that have members with constructors (needs to have default constructor)
	template<typename T>
	T* mem_allocate_and_construct(u32 count = 1)
	{
		//A type can also be standard layout while requiring explicit constructor code to be executed
		//so std::is_standard_layout is wrong here
		static_assert(std::is_default_constructible_v<T>, "T in an invalid type");
		void* ptr = mem_allocate(count * sizeof(T));
		new (ptr) T;
		return reinterpret_cast<T*>(ptr);
	}

	template<typename T>
	void mem_free_and_destroy(T* ptr)
	{
		ptr->~T();
		mem_free(ptr);
	}


	template<typename T>
	T* temporary_allocate(u32 count)
	{
		static_assert(std::is_standard_layout_v<T>, "T in an invalid type");
		return reinterpret_cast<T*>(temporary_allocate(count * sizeof(T)));
	}

}