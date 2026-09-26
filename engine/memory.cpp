#include "memory.h"
#include "MainIncl.h"
#include <mutex>
#ifdef _WIN32
#include "Windows.h"
#endif

gfx::Allocator* g_engine_allocator = nullptr;
std::mutex      g_engine_allocator_mutex;

//overloaded operators to track allocation when needed
/*
void* operator new(u64 size)
{
	return malloc(size);
}

void operator delete(void* ptr)
{
	free(ptr);
}
*/

namespace gfx
{

	static _Node* grandparent(const _Node* node)
	{
		assert(node->parent, "no grandparent can be found without a parent");
		return node->parent->parent;
	}

	static _Node* uncle(const _Node* node)
	{
		if(node->parent == grandparent(node)->left) {
			return grandparent(node)->right;
		}

		return grandparent(node)->left;
	}

	static _Node* sibling(const _Node* node)
	{
		//assert(node->parent, "no sibling can be found without a defined parent");
		if(!node->parent)
			return nullptr;

		if(node->parent->left == node) {
			return node->parent->right;
		}

		return node->parent->left;
	}

	static _NodeColor get_node_color(_Node* node)
	{
		if(!node) return NODE_COLOR_BLACK;
		return node->color;
	}

	static _Node* rotate_right(_Node* node)
	{
		if(node->parent) {
			if(node == node->parent->left) {
				node->parent->left = node->left;
			} else {
				node->parent->right = node->left;
			}
		}

		//Changing pointers to replace node with node->left
		//and then making node node->left's right son
		auto rotated = node->left;
		rotated->parent = node->parent;
		_Node* old_right = rotated->right;

		rotated->right = node;
		rotated->right->parent = rotated;
		rotated->right->left = old_right;
		if(old_right) old_right->parent = node;

		return rotated;
	}

/*
	more descriptive rotation method, left for clarity but commented because there
	is a faster function above

	static _Node* rotate_right(_Node* node)
	{
		if (node->parent) {
			if (node == node->parent->left) {
				node->parent->left = node->left;
			}
			else {
				node->parent->right = node->left;
			}
		}

		auto old_parent = node->parent;
		auto old_node   = node;
		auto old_node_left       = node->left;
		auto old_node_left_right = node->left->right;
		auto old_node_left_left  = node->left->left;
		auto old_node_right      = node->right;

		old_node_left->parent = old_parent;
		old_node_left->left   = old_node_left_left;
		old_node_left->right  = old_node;

		old_node->parent = old_node_left;
		old_node->left   = old_node_left_right;

		if(old_node_left_right) old_node_left_right->parent = old_node;
		if(old_node_left_left)  old_node_left_left->parent  = old_node_left;


		return old_node_left;
	}
	*/

	static _Node* rotate_left(_Node* node)
	{
		if(node->parent) {
			if(node == node->parent->left) {
				node->parent->left = node->right;
			} else {
				node->parent->right = node->right;
			}
		}

		//Changing pointers to replace node with node->right
		//and then making node node->right's left son
		auto rotated = node->right;
		rotated->parent = node->parent;
		_Node* old_left = rotated->left;

		rotated->left = node;
		rotated->left->parent = rotated;
		rotated->left->right = old_left;
		if(old_left) old_left->parent = node;

		return rotated;
	}


	//Definitions written according to the red black tree principles
	void SegmentTree::rearrange_tree_for_insertion(_Node* node)
	{
		if(!node->parent) {
			node->color = NODE_COLOR_BLACK;
			return;
		}

		if(node->parent->color & NODE_COLOR_BLACK) {
			//Leave the new leaf color as NODE_COLOR_RED which is already set
			return;
		}

		if(uncle(node) && uncle(node)->color & NODE_COLOR_RED) {
			node->parent->color      = NODE_COLOR_BLACK;
			uncle(node)->color       = NODE_COLOR_BLACK;
			grandparent(node)->color = NODE_COLOR_RED;

			rearrange_tree_for_insertion(grandparent(node));
			return;
		}

		if(node->parent->right == node && node->parent == grandparent(node)->left) {
			rotate_left(node->parent);
			node = node->left;
		}


		if(node->parent->left == node && node->parent == grandparent(node)->right) {
			rotate_right(node->parent);
			node = node->right;
		}

		node->parent->color      = NODE_COLOR_BLACK;
		grandparent(node)->color = NODE_COLOR_RED;

		if(node == node->parent->left && node->parent == grandparent(node)->left) {
			auto new_node = rotate_right(grandparent(node));
			if(!new_node->parent) root = new_node;
		} else {
			auto new_node = rotate_left(grandparent(node));
			if(!new_node->parent) root = new_node;
		}
	}

	void SegmentTree::add_node(u32 start, u32 size)
	{
		_Node* parent = nullptr;
		_Node** iterator = find_place_to_insert_node(start, &parent);

#ifdef ARENA_USE_NODE_POOL

		if(pool.array_capacity == 0) {
			pool.array_capacity = 300;

			pool.node_array = (_Node*)::operator new(pool.array_capacity * sizeof(_Node));
			memset(pool.node_array, 0, pool.array_capacity * sizeof(_Node));
			pool.array_size = 0;
		}

		if(pool.array_size >= pool.array_capacity) {
			assert(false, "Too many allocations have been made, expand the capacity at compile time");
		}

		pool.array_size += 1;
		_Node null_node = {};
		//Grab a node from the array only if it is null, otherwise it is still being used
		for(u32 i = 0; i < pool.array_capacity; i++) {
			if(memcmp(&pool.node_array[i], &null_node, sizeof(_Node)) == 0) {
				*iterator = &pool.node_array[i];
				break;
			}
		}
#else
		*iterator = new _Node;
		memset(*iterator, 0, sizeof(_Node));
#endif
		_Node* current_node = *iterator;

		//The color gets defaulted as red, then eventually that will change in the insert check chain
		current_node->parent        = parent;
		current_node->color         = NODE_COLOR_RED;
		current_node->segment.start = start;
		current_node->segment.size  = size;

		rearrange_tree_for_insertion(*iterator);
	}

	_Node** SegmentTree::find_place_to_insert_node(u32 start, _Node** parent)
	{
		_Node** iterator = &root;
		while(*iterator) {
			u32 segment_start = (*iterator)->segment.start;
			assert(start != segment_start, "cannot allocate a new segment of memory where another one is already defined, check the memory declaration");
			if(segment_start > start) {
				if(parent) *parent = *iterator;
				iterator = &(*iterator)->left;
			} else {
				if(parent) *parent = *iterator;
				iterator = &(*iterator)->right;
			}
		}

		return iterator;
	}

	_Node* SegmentTree::find_node(u32 start)
	{
		_Node** iterator = &root;
		while(*iterator) {
			u32 segment_start = (*iterator)->segment.start;

			if((*iterator)->segment.start == start)
				return *iterator;

			if(segment_start > start) {
				iterator = &(*iterator)->left;
			} else {
				iterator = &(*iterator)->right;
			}
		}

		return nullptr;
	}

	const _Node* SegmentTree::find_first_node()
	{
		if(!root) return nullptr;

		auto iter = root;
		while(iter->left)
			iter = iter->left;

		return iter;
	}

	const _Node* SegmentTree::find_last_node()
	{
		if(!root) return nullptr;

		auto iter = root;
		while(iter->right)
			iter = iter->right;

		return iter;
	}

	void SegmentTree::remove_node(u32 start)
	{
		_Node* parent = nullptr;
		_Node* iterator = find_node(start);

		auto current_node = iterator;

		//The node to delete does not exist, just exit
		if(!current_node) return;
		_Node* node_to_delete = nullptr;

		if(!current_node->left || !current_node->right) {
			//In the case in which both of the nodes are null, null gets yielded, which is fine
			auto replaced_node = current_node->left ? current_node->left : current_node->right;

			//The null nodes are considered black by default
			if(replaced_node) {
				node_to_delete = replaced_node;
				current_node->segment = replaced_node->segment;
			} else {
				node_to_delete = current_node;
			}
		} else {
			auto lowest_in_right_subtree = current_node->right;
			bool has_the_right_son_one_single_child = true;

			while(lowest_in_right_subtree->left) {
				lowest_in_right_subtree = lowest_in_right_subtree->left;
				has_the_right_son_one_single_child = false;
			}

			node_to_delete = lowest_in_right_subtree;
			current_node->segment = lowest_in_right_subtree->segment;
		}

		assert(node_to_delete, "there was an implementation problem if the node here is null");
		bool already_deleted = preliminary_deletion(node_to_delete);
		if(!already_deleted)
			rearrange_tree_for_deletion(node_to_delete);
	}


	static void red_black_tree_delete(_Node* node)
	{
		if(!node) return;

		red_black_tree_delete(node->left);
		red_black_tree_delete(node->right);
		delete node;
	}

	static u32 get_max_depth(_Node* node)
	{
		if(!node)
			return 0;

		u32 left_depth  = get_max_depth(node->left)  + 1;
		u32 right_depth = get_max_depth(node->right) + 1;

		return left_depth > right_depth ? left_depth : right_depth;
	}

	static void print_tree_level(_Node* root, u32 current_depth, u32 max_depth, u32* total_nodes)
	{
		if(current_depth >= max_depth)
			return;

#ifdef _WIN32
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
		assert(max_depth < 32, "Tree too big to print");

		const u32 tabs       = glm::pow(2, max_depth - current_depth);
		const u32 iterations = glm::pow(2, current_depth);
		const u32 mask = current_depth > 0 ? glm::pow(2, current_depth - 1) : 0;

		auto current_node = root;
		for (u32 i = 0; i < iterations; i++) {
			current_node = root;
			for(u32 j = 0; j < current_depth && current_node; j++) {
				if (i & (mask >> j))
					current_node = current_node->right;
				else
					current_node = current_node->left;
			}

			u32 current_tabs = tabs;
			if(i == 0)
				current_tabs /= 2;

			for(u32 j = 0; j < current_tabs; j++) {
				if(j == current_tabs - 1) {
					log_message(" ");
					continue;
				}

				log_message("    ");
			}


			if (current_node && current_node->color & NODE_COLOR_BLACK) {
#ifdef _WIN32
			//Representing black as green to avoid console darkness
				SetConsoleTextAttribute(console, FOREGROUND_GREEN);
#endif
				log_message("BLA{}", current_depth == 0 ? "(root)" : "");
				if(total_nodes) *total_nodes += 1;
			} else if(current_node && current_node->color & NODE_COLOR_RED) {
#ifdef _WIN32
				SetConsoleTextAttribute(console, FOREGROUND_RED);
#endif
				log_message("RED{}", current_depth == 0 ? "(root)" : "");
				if(total_nodes) *total_nodes += 1;
			} else if(current_node && current_node->color & NODE_COLOR_DOUBLE_BLACK) {
				//Left as legacy, double black is not currently used in the implementation
#ifdef _WIN32
				SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN);
#endif
				log_message("DBL{}", current_depth == 0 ? "(root)" : "");
				if(total_nodes) *total_nodes += 1;
			} else {
#ifdef _WIN32
				SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
				log_message("NUL");
			}
		}

		log_message("\n");
		print_tree_level(root, current_depth + 1, max_depth, total_nodes);
	}

	void SegmentTree::print()
	{
		if(!root) {
			log_message("(empty)");
			return;
		}

		log_message("\n\n\n\n");
		u32 max_depth = get_max_depth(root);
		u32 total_nodes = 0;
		print_tree_level(root, 0, max_depth, &total_nodes);

#ifdef _WIN32
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
		log_message("total_nodes:{}\n", total_nodes);
	}

	void SegmentTree::cleanup()
	{
#ifndef ARENA_USE_NODE_POOL
		red_black_tree_delete(root);
		root = nullptr;
#else
		if(pool.node_array) {
			::operator delete(pool.node_array);
			pool.node_array = nullptr;
		}
		root = nullptr;
#endif
	}

	void SegmentTree::delete_node(_Node* node)
	{
		if(node->parent->left == node)
			node->parent->left  = nullptr;
		else
			node->parent->right = nullptr;

		//Decrease the counter in the node pool by one, signaling that the node is free for next use
		//The node will be zero initialized again when requested
#ifdef ARENA_USE_NODE_POOL
		memset(node, 0, sizeof(_Node));
		pool.array_size -= 1;
#else
		delete node;
#endif
	}

	bool SegmentTree::preliminary_deletion(_Node* node_to_delete)
	{
		if((!node_to_delete->left && node_to_delete->right) || (node_to_delete->left && !node_to_delete->right)) {
			auto only_son = node_to_delete->left ? &node_to_delete->left : &node_to_delete->right;
			if(get_node_color(node_to_delete) & NODE_COLOR_RED) {
				node_to_delete->color = NODE_COLOR_BLACK;
			}
			node_to_delete->segment = (*only_son)->segment;
			delete_node(*only_son);
			*only_son = nullptr;
			return true;
		}

		if(node_to_delete == root && !node_to_delete->left && !node_to_delete->right) {
#ifndef ARENA_USE_NODE_POOL
			delete root;
#endif
			root = nullptr;
			return true;
		}

		if(get_node_color(node_to_delete) & NODE_COLOR_RED && !node_to_delete->left && !node_to_delete->right) {
			delete_node(node_to_delete);
			return true;
		}

		return false;
	}

	void SegmentTree::rearrange_tree_for_deletion(_Node* node_to_delete, bool perform_deletion, bool just_456, bool just_6)
	{
		//removal of black non root
		//Name references (parent: p, sibling: s, close_nephew: c, distant_nephew: d)
		auto p = node_to_delete->parent;
		auto s = sibling(node_to_delete);
		_Node* c = nullptr, *d = nullptr;
		if(s) {
			auto node = node_to_delete;
			c = node->parent->left == node ? sibling(node)->left  : sibling(node)->right;
			d = node->parent->left == node ? sibling(node)->right : sibling(node)->left;
		}

		//Very rare instance where goto is useful
		if(just_456)
			goto label_456;
		if(just_6)
			goto label_6;

		//Deletion case 2
		if(get_node_color(p) & NODE_COLOR_BLACK && get_node_color(c) & NODE_COLOR_BLACK && get_node_color(s) & NODE_COLOR_BLACK && get_node_color(d) & NODE_COLOR_BLACK) {
			if (!s)
				return;

			s->color = NODE_COLOR_RED;
			rearrange_tree_for_deletion(node_to_delete->parent, false);
			if(perform_deletion)
				delete_node(node_to_delete);
			return;
		}

		//Deletion case 3
		if(get_node_color(s) & NODE_COLOR_RED) {
			if(p->left == s)
				rotate_right(p);
			else
				rotate_left(p);

			if(!s->parent)
				root = s;

			auto saved_color = s->color;
			s->color = p->color;
			p->color = saved_color;

			rearrange_tree_for_deletion(node_to_delete, false, true, false);
			if(perform_deletion)
				delete_node(node_to_delete);
			return;
		}

		//Deletion case 4
label_456:
		if(get_node_color(p) & NODE_COLOR_RED && get_node_color(c) & NODE_COLOR_BLACK && get_node_color(s) & NODE_COLOR_BLACK && get_node_color(d) & NODE_COLOR_BLACK) {
			auto saved_color = s->color;
			s->color = p->color;
			p->color = saved_color;
		}

		//Deletion case 5
		if(get_node_color(c) & NODE_COLOR_RED && get_node_color(s) & NODE_COLOR_BLACK && get_node_color(d) & NODE_COLOR_BLACK) {
			if(s->left == c)
				rotate_right(s);
			else
				rotate_left(s);

			if(!c->parent)
				root = c;

			auto saved_color = s->color;
			s->color = c->color;
			c->color = saved_color;

			rearrange_tree_for_deletion(node_to_delete, false, false, true);
			if(perform_deletion)
				delete_node(node_to_delete);
			return;
		}

		//Deletion case 6
label_6:
		if(get_node_color(s) & NODE_COLOR_BLACK && get_node_color(d) & NODE_COLOR_RED) {
			if(p->left == s)
				rotate_right(p);
			else
				rotate_left(p);

			if(!s->parent)
				root = s;

			auto saved_color = p->color;
			p->color = s->color;
			s->color = saved_color;
			d->color = NODE_COLOR_BLACK;
		}

		if(perform_deletion) {
			delete_node(node_to_delete);
		}
	}

/*
	old version of the removal method, left just in case, was not working properly
	void SegmentTree::tree_delete_check(_Node* node_to_delete, bool perform_deletion)
	{
		auto node_sibling = sibling(node_to_delete);
		if(!node_to_delete->left && !node_to_delete->right && get_node_color(node_to_delete) & NODE_COLOR_RED) {
			node_removal_utility(node_to_delete, perform_deletion);
			return;
		}

		if(!node_to_delete->parent && perform_deletion) {
			//This was the last node, reset tree
			root = nullptr;
			assert(perform_deletion, "there should not be no way this flag is false when we have only one node");
			node_removal_utility(node_to_delete, perform_deletion);
			return;
		}

		if(!node_sibling) {
			if(node_to_delete->color == NODE_COLOR_DOUBLE_BLACK)
				node_to_delete->color = NODE_COLOR_BLACK;

			return;
		}

		if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_BLACK) {
			auto parent_color   = node_sibling->parent->color;
			node_sibling->color = NODE_COLOR_RED;

			if(parent_color & NODE_COLOR_RED) {
				node_sibling->parent->color = NODE_COLOR_BLACK;
				node_removal_utility(node_to_delete, perform_deletion);
			} else {
				//Black or double black
				node_sibling->parent->color = NODE_COLOR_DOUBLE_BLACK;
				node_removal_utility(node_to_delete, perform_deletion);
				tree_delete_check(node_sibling->parent, false);
			}

			return;
		}

		if(get_node_color(node_sibling) & NODE_COLOR_RED) {
			auto temp_color             = node_sibling->parent->color;
			node_sibling->parent->color = node_sibling->color;
			node_sibling->color         = temp_color;

			if(node_sibling == node_sibling->parent->left) {
				auto new_node = rotate_right(node_sibling->parent);
				if(!new_node->parent)
					root = new_node;
				tree_delete_check(node_to_delete, true);
			}
			//INFO @C7 node sibling could be the new root node here, so make sure it still has a parent
			else if(node_sibling->parent && node_sibling == node_sibling->parent->right) {
				auto new_node = rotate_left(node_sibling->parent);
				if(!new_node->parent)
					root = new_node;
				tree_delete_check(node_to_delete, true);
			}

			return;
		}

		_Node* rotated = nullptr;
		if(node_sibling == node_sibling->parent->left) {
			//Case 5
			if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_RED) {
				auto temp_color            = node_sibling->right->color;
				node_sibling->right->color = node_sibling->color;
				if (node_sibling->parent->color & NODE_COLOR_RED)
					node_sibling->color = temp_color;
				//node_sibling->color        = temp_color;
				//node_to_delete->color      = NODE_COLOR_RED;

				rotate_left(node_sibling);
				rotated = rotate_right(grandparent(node_sibling));
			}
			//Case 6
			if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_RED) {
				auto temp_color             = node_sibling->parent->color;
				node_sibling->parent->color = node_sibling->color;
				node_sibling->color         = temp_color;

				node_sibling->left->color   = NODE_COLOR_BLACK;

				rotated = rotate_right(node_sibling->parent);
			}

		} else {
			//Case 5
			if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_RED) {
				auto temp_color           = node_sibling->left->color;
				node_sibling->left->color = node_sibling->color;
				if (node_sibling->parent->color & NODE_COLOR_RED)
					node_sibling->color = temp_color;
				//node_to_delete->color     = NODE_COLOR_RED;

				rotate_right(node_sibling);
				rotated = rotate_left(grandparent(node_sibling));
			}

			//Case 6
			if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_RED) {
				auto temp_color             = node_sibling->parent->color;
				node_sibling->parent->color = node_sibling->color;
				node_sibling->color         = temp_color;

				node_sibling->right->color  = NODE_COLOR_BLACK;

				rotated = rotate_left(node_sibling->parent);
			}
		}

		if (rotated && !rotated->parent)
			root = rotated;

		node_removal_utility(node_to_delete, perform_deletion);
	}
	*/

	static _Node* find_next_node(const _Node* node)
	{
		if(!node->right && !node->parent)
			return nullptr;
		if(node->right) {
			auto iter = node->right;
			while(iter->left)
				iter = iter->left;

			return iter;
		}

		if(node->parent->left == node)
			return node->parent;

		if(node->parent->right == node) {
			auto iter = node->parent->right;
			while(iter->parent && iter->parent->right == iter)
				iter = iter->parent;

			//if iter was the left son of the node, then the parent is returned, otherwise null gets returned
			//because there would not be successors anyway
			return iter->parent;
		}

		return nullptr;
	}

	static _Node* find_prev_node(const _Node* node)
	{
		assert(false, "function still not implemented");
		return nullptr;
	}

	static void check_all_paths_have_the_same_amount_of_black_nodes(const _Node* node, u32 check_val, u32 current_val = 0)
	{
		if(!node) {
			assert(check_val == current_val, "not all paths have the same amount of black nodes");
			return;
		}

		u32 next_val = (node->color & NODE_COLOR_BLACK) ? current_val + 1 : current_val;
		check_all_paths_have_the_same_amount_of_black_nodes(node->left, check_val, next_val);
		check_all_paths_have_the_same_amount_of_black_nodes(node->right, check_val, next_val);
	}

	static void check_quantities_are_correct(const _Node* node)
	{
		if(!node)
			return;

		if(node->left)
			assert(node->segment.start > node->left->segment.start, "Incongruent");
		if(node->right)
			assert(node->segment.start < node->right->segment.start, "Incongruent");

		check_quantities_are_correct(node->left);
		check_quantities_are_correct(node->right);
	}

	static void check_all_red_nodes_have_black_children(const _Node* node)
	{
		if(!node) return;

		check_all_red_nodes_have_black_children(node->left);
		check_all_red_nodes_have_black_children(node->right);

		if(node->color & NODE_COLOR_RED) {
			assert(get_node_color(node->left) & NODE_COLOR_BLACK && get_node_color(node->right) & NODE_COLOR_BLACK, "there is a red node with at least a red child, fix the tree");
		}
	}

	void assert_red_black_tree_validity(const SegmentTree& segment_tree)
	{
		const auto root = segment_tree.get_root();
		if (!root)
			return;

		assert(root->color & NODE_COLOR_BLACK, "the start of a red black tree requires a black node");

		//If all the black paths need to have the same black lenght, just pick a random path first, measure the black
		//lenght, and then check all the other lenghts against it
		u32 left_path_black_nodes = 0;
		for(auto iter = root; iter; iter = iter->left) {
			if(iter->color & NODE_COLOR_BLACK)
				left_path_black_nodes++;
		}

		check_quantities_are_correct(root);
		check_all_paths_have_the_same_amount_of_black_nodes(root, left_path_black_nodes);
		check_all_red_nodes_have_black_children(root);
	}

	namespace test
	{
		static void tree_test_code_1()
		{
			gfx::SegmentTree this_tree;
			defer { this_tree.cleanup(); };

			this_tree.add_node(1, 0);
			this_tree.add_node(2, 0);
			this_tree.add_node(3, 0);
			this_tree.add_node(4, 0);
			this_tree.add_node(5, 0);
			this_tree.add_node(6, 0);
			this_tree.add_node(7, 0);
			this_tree.add_node(8, 0);
			this_tree.add_node(9, 0);
			this_tree.add_node(10, 0);
			this_tree.add_node(20, 0);
			this_tree.add_node(30, 0);

			this_tree.remove_node(2);
			this_tree.remove_node(4);
			this_tree.remove_node(1);
			this_tree.remove_node(8);

			assert_red_black_tree_validity(this_tree);
		}

		static void tree_test_code_2()
		{
			gfx::SegmentTree this_tree;
			defer { this_tree.cleanup(); };

			this_tree.add_node(2, 0);
			this_tree.add_node(1, 0);
			this_tree.add_node(3, 0);

			this_tree.remove_node(1);
			this_tree.remove_node(3);



			assert_red_black_tree_validity(this_tree);
		}

		static void tree_test_code_3()
		{
			gfx::SegmentTree this_tree;
			defer { this_tree.cleanup(); };

			this_tree.add_node(10, 0);
			this_tree.add_node(9, 0);
			this_tree.add_node(8, 0);
			this_tree.add_node(7, 0);
			this_tree.add_node(6, 0);
			this_tree.add_node(5, 0);
			this_tree.add_node(4, 0);
			this_tree.add_node(3, 0);
			this_tree.add_node(2, 0);

			auto node_to_find = this_tree.find_node(6);
			auto next_node = find_next_node(node_to_find);
			assert(next_node->segment.start == 7, "implementation error");

			this_tree.remove_node(9);
			this_tree.remove_node(8);

			assert_red_black_tree_validity(this_tree);
		}

		static void tree_test_code_4()
		{
			gfx::SegmentTree this_tree;
			defer { this_tree.cleanup(); };

			this_tree.add_node(1, 0);
			this_tree.add_node(2, 0);
			this_tree.add_node(3, 0);
			this_tree.add_node(4, 0);
			this_tree.add_node(5, 0);

			auto node_to_check = this_tree.find_first_node();
			for(u32 i = 0; i < 5; i++) {
				assert(node_to_check->segment.start == i + 1, "find_next_node algorithm not working\n");
				node_to_check = find_next_node(node_to_check);
			}
		}

		static void allocator_test_code_1()
		{
			auto allocator = allocator_create(1024, 1024);
			auto current_allocator_save = g_engine_allocator;
			defer {
				g_engine_allocator = current_allocator_save;
				allocator_cleanup(&allocator);
			};

			g_engine_allocator = &allocator;
			void* ptr1 = mem_allocate(0x10);
			void* ptr2 = mem_allocate(0x10);
			void* ptr3 = mem_allocate(0x10);
			void* ptr4 = mem_allocate(0x10);
			void* ptr5 = mem_allocate(0x10);
			void* ptr6 = mem_allocate(0x10);

			mem_free(ptr1);
			mem_free(ptr2);
			mem_free(ptr3);
			mem_free(ptr5);

			void* new_ptr = mem_allocate(0x10);
		}

		static void allocator_test_code_2()
		{
			auto allocator = allocator_create(1024, 1024);
			auto current_allocator_save = g_engine_allocator;
			defer {
				g_engine_allocator = current_allocator_save;
				allocator_cleanup(&allocator);
			};
			g_engine_allocator = &allocator;


			const u32 pointer_count = 8;
			void* ptrs[pointer_count];

			for(u32 i = 0; i < pointer_count; i++) {
				ptrs[i] = mem_allocate(10);
			}


			for(u32 i = 0; i < pointer_count; i++) {
				mem_free(ptrs[i]);
			}

		}

		void allocator_test_code_3()
		{
			g_engine_allocator = new Allocator;
			*g_engine_allocator = allocator_create(1024, 1024);

			defer {
				allocator_cleanup(g_engine_allocator);
				delete g_engine_allocator;
			};

			auto& segment_tree = g_engine_allocator->segment_tree;
			segment_tree.add_node(4, 1);
			segment_tree.add_node(2, 1);
			segment_tree.add_node(1, 1);
			segment_tree.add_node(6, 1);
			segment_tree.add_node(5, 1);
			segment_tree.add_node(7, 1);
			segment_tree.add_node(8, 1);
			segment_tree.add_node(9, 1);

			segment_tree.remove_node(9);
			segment_tree.remove_node(4);
			segment_tree.remove_node(5);

			const _Node* root = segment_tree.get_root();
			assert(root->segment.start == 6               && root->color               & NODE_COLOR_BLACK, "");
			assert(root->left->segment.start == 2         && root->left->color         & NODE_COLOR_BLACK, "");
			assert(root->left->left->segment.start == 1   && root->left->left->color   & NODE_COLOR_RED, "");
			assert(root->right->segment.start == 7        && root->right->color        & NODE_COLOR_BLACK, "");
			assert(root->right->right->segment.start == 8 && root->right->right->color & NODE_COLOR_RED, "");
		}

		void memory_run_tests()
		{
			tree_test_code_1();
			tree_test_code_2();
			tree_test_code_3();
			tree_test_code_4();
			allocator_test_code_1();
			allocator_test_code_2();
			allocator_test_code_3();
			log_message("(memory_run_tests) tests: OK\n");
		}
	}

	Allocator allocator_create(u32 permanent_storage_bytes, u32 temporary_storage_bytes)
	{
		Allocator allocator = {};
		allocator.permanent_storage.buffer = ::operator new(permanent_storage_bytes);
		allocator.temporary_storage.buffer = ::operator new(temporary_storage_bytes);

		std::memset(allocator.permanent_storage.buffer, 0, permanent_storage_bytes);
		std::memset(allocator.temporary_storage.buffer, 0, temporary_storage_bytes);

		allocator.permanent_storage.size = permanent_storage_bytes;
		allocator.temporary_storage.size = temporary_storage_bytes;

		return allocator;
	}

	void allocator_cleanup(Allocator* allocator)
	{
		if(!allocator) return;

		::operator delete(allocator->permanent_storage.buffer);
		::operator delete(allocator->temporary_storage.buffer);
	}

	void* mem_allocate(u32 bytes)
	{

		//INFO @C7 for some reason ::operator new called with 0 bytes does not return nullptr...
		if(bytes == 0)
			return nullptr;

		if(!g_engine_allocator) {
			return ::operator new(bytes);
		}

		std::scoped_lock lock(g_engine_allocator_mutex);

		//the allocation methodology now consist of pushing the allocation start and size at the
		//end of the tree. The tree having a red-black implementation will balance itself
		auto& segment_tree = g_engine_allocator->segment_tree;
		auto last_allocation_node = segment_tree.find_last_node();
		u8* storage_u8 = reinterpret_cast<u8*>(g_engine_allocator->permanent_storage.buffer);

		u32 new_allocation_start = 0;
		if(last_allocation_node) {
			if(last_allocation_node->segment.start > g_engine_allocator->permanent_storage.used * 2 || g_engine_allocator->dense_allocations_left != 0) {
				u32& dense_allocations_left = g_engine_allocator->dense_allocations_left;
				if(dense_allocations_left == 0) {
					dense_allocations_left = 10;
				} else {
					dense_allocations_left--;
				}

				for(auto node = segment_tree.find_first_node();;) {
					auto next_node = find_next_node(node);
					if(!next_node) break;

					u32 node_start      = node->segment.start;
					u32 node_size       = node->segment.size;
					u32 next_node_start = next_node->segment.start;

					if(next_node_start - (node_start + node_size) >= bytes) {
						u32 new_allocation_start = node_start + node_size;
						segment_tree.add_node(new_allocation_start, bytes);
						g_engine_allocator->permanent_storage.used += bytes;
						return storage_u8 + new_allocation_start;
					}

					node = next_node;
				}
				//If we get here no allocation with the right amount of space was found between the holes,
				//just mem_allocate at the end
			}

			new_allocation_start = last_allocation_node->segment.start + last_allocation_node->segment.size;
		}

		g_engine_allocator->permanent_storage.used += bytes;
		segment_tree.add_node(new_allocation_start, bytes);

		//Debug calls
		//log_message(":::+{}\n", g_engine_allocator->permanent_storage.used);
		//segment_tree.print();
		//assert_red_black_tree_validity(segment_tree);

		return storage_u8 + new_allocation_start;
	}

	void* mem_allocate_zeroed(u32 bytes)
	{
		void* ptr = mem_allocate(bytes);
		std::memset(ptr, 0, bytes);
		return ptr;
	}

	void mem_free(void* ptr)
	{
		if (!ptr)
			return;

		if(!g_engine_allocator) {
			::operator delete(ptr);
			return;
		}

		std::scoped_lock lock(g_engine_allocator_mutex);

		u32 node_start = (u8*)ptr - (u8*)g_engine_allocator->permanent_storage.buffer;
		auto& segment_tree = g_engine_allocator->segment_tree;
		auto node = segment_tree.find_node(node_start);

		if(!node) {
			log_message("user tried to free a nullptr node");
			return;
		}

		g_engine_allocator->permanent_storage.used -= node->segment.size;
		segment_tree.remove_node(node_start);

		//Debug calls
		//log_message(":::-{}\n", g_engine_allocator->permanent_storage.used);
		//segment_tree.print();
		//assert_red_black_tree_validity(segment_tree);
	}

	void* temporary_allocate(u32 bytes)
	{
		if(bytes == 0)
			return nullptr;

		if(!g_engine_allocator)
			return ::operator new(bytes);

		//Add 4 to the total amount of bytes used in the system to make space for the object size.
		//That makes it easier to understand by how much the counter needs to be decremented
		const u32 size_of_padding_at_beginning = sizeof(u32);
		bytes  += size_of_padding_at_beginning;

		auto& temporary_storage = g_engine_allocator->temporary_storage;
		u8* storage_u8 = static_cast<u8*>(temporary_storage.buffer);
		assert(temporary_storage.used + bytes < temporary_storage.size, "there is now enough space in the temporary allocator for the requested space\n");
		u8* return_address = storage_u8 + temporary_storage.used;
		temporary_storage.used += bytes;

		//Write the allocation size at the end
		*reinterpret_cast<u32*>(return_address) = bytes;

		return return_address + size_of_padding_at_beginning;
	}

	void temporary_free(void* ptr)
	{
		if(!g_engine_allocator) {
			::operator delete(ptr);
			return;
		}

		//Load the temporary allocation size
		const u32 size_of_padding_at_beginning = sizeof(u32);
		u32* size_ptr = reinterpret_cast<u32*>((u8*)ptr - size_of_padding_at_beginning);
		assert(size_ptr, "There is a weird issue going on here, this should never be 0");
		temporary_decrease_counter(*size_ptr);

		//INFO @C7 there is really nothing to mem_free if a temporary buffer is used, you can mark the memory on top
		//of the temporary stack being no longer used by decreasing the counter with the related function
		//declared in the memory header
	}

	void temporary_free_c_str(char* c_string)
	{
		if(!g_engine_allocator) {
			::operator delete(c_string);
			return;
		}

		temporary_free(c_string);
	}

	void temporary_decrease_counter(u32 bytes)
	{
#ifdef _DEBUG
		//Keep this for debugging
		assert(g_engine_allocator, "the function was called without a custom allocator defined, this should not happen in a correct implementation\n");
#else
		if(!g_engine_allocator)
			return;
#endif

		g_engine_allocator->temporary_storage.used -= bytes;
	}
}