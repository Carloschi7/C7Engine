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
		void tree_insert_check_1(_Node* node);
		void tree_insert_check_2(_Node* node);
		void tree_insert_check_3(_Node* node);
		void tree_insert_check_4(_Node* node);
		void tree_insert_check_5(_Node* node);

		//This function is used to perform element deletion logic, this includes also rearranging the
		//tree after the deletion is performed, hence the perform_deletion flag
		void tree_delete_check(_Node* node, bool perform_deletion);

		_Node* root = nullptr;
	};

	_Node* find_next_node(const _Node* node);
	_Node* find_prev_node(const _Node* node);

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

	static Allocator* g_engine_allocator = nullptr;
	Allocator allocator_create(u32 permanent_storage_bytes, u32 temporary_storage_bytes);
	void      allocator_cleanup(Allocator* allocator);

	//This functions will either use the g_engine_allocator functionalities or mem_allocate memory from the standard heap
	//allocate_temporary and free_temporary behave the same way to mem_allocate and mem_free if g_engine_allocator is not defined
	void* mem_allocate(u32 bytes);
	void  mem_free(void* ptr);
	void* temporary_allocate(u32 bytes);
	void  temporary_free(void* ptr);
	void  temporary_decrease_counter(u32 bytes);
}