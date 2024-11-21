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
		void add_element(u32 start, u32 size);
		_Node** find_element(u32 start, _Node*& parent);
		void remove_element(u32 start);
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

		SegmentTree memory_segments;
	};

	struct Allocator
	{
		TemporaryStorage temporary_storage;
		PermanentStorage permanent_storage;
	};

	static Allocator* g_engine_allocator = nullptr;

	Allocator allocator_create(u32 permanent_storage_bytes, u32 temporary_storage_bytes);

	//This functions will either use the g_engine_allocator functionalities or allocate memory from the standard heap
	//allocate_temporary and free_temporary behave the same way to allocate and free if g_engine_allocator is not defined
	void* allocate(u32 bytes);
	void  free(void* ptr);
	void* allocate_temporary(u32 bytes);
	void  free_temporary(void* ptr);

	void assert_red_black_tree_validity(const SegmentTree& segment_tree);

	namespace test
	{
		void memory_run_tests();
	}
}