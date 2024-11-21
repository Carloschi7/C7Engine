#include "memory.h"
#include "MainIncl.h"



namespace gfx
{

	static _Node* grandparent(_Node* node)
	{
		assert(node->parent, "no grandparent can be found without a parent");
		return node->parent->parent;
	}

	static _Node* uncle(_Node* node)
	{
		if(node->parent == grandparent(node)->left) {
			return grandparent(node)->right;
		}

		return grandparent(node)->left;
	}

	static _Node* sibling(_Node* node)
	{
		assert(node->parent, "no sibling can be found without a defined parent");
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

		auto rotated = node->left;
		rotated->parent = node->parent;
		_Node* old_right = rotated->right;

		rotated->right = node;
		rotated->right->parent = rotated;
		rotated->right->left = old_right;
		if(old_right) old_right->parent = node;

		return rotated;
	}

	static _Node* rotate_left(_Node* node)
	{
		if(node->parent) {
			if(node == node->parent->left) {
				node->parent->left = node->right;
			} else {
				node->parent->right = node->right;
			}
		}

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
	void SegmentTree::tree_insert_check_1(_Node* node)
	{
		if(!node->parent) {
			node->color = NODE_COLOR_BLACK;
			return;
		}

		tree_insert_check_2(node);
	}

	void SegmentTree::tree_insert_check_2(_Node* node)
	{
		if(node->parent->color & NODE_COLOR_BLACK) {
			//Leave the new leaf color as NODE_COLOR_RED which is already set
			return;
		}

		tree_insert_check_3(node);
	}

	void SegmentTree::tree_insert_check_3(_Node* node)
	{
		if(uncle(node) && uncle(node)->color & NODE_COLOR_RED) {
			node->parent->color      = NODE_COLOR_BLACK;
			uncle(node)->color       = NODE_COLOR_BLACK;
			grandparent(node)->color = NODE_COLOR_RED;

			tree_insert_check_1(grandparent(node));
			return;
		}

		tree_insert_check_4(node);
	}

	void SegmentTree::tree_insert_check_4(_Node* node)
	{
		if(node->parent->right == node && node->parent == grandparent(node)->left) {
			rotate_left(node->parent);
			node = node->left;
		}


		if(node->parent->left == node && node->parent == grandparent(node)->right) {
			rotate_right(node->parent);
			node = node->right;
		}

		tree_insert_check_5(node);
	}

	void SegmentTree::tree_insert_check_5(_Node* node)
	{
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

	void SegmentTree::add_element(u32 start, u32 size)
	{
		_Node* parent = nullptr;
		_Node** iterator = find_element(start, parent);

		*iterator = new _Node;
		std::memset(*iterator, 0, sizeof(_Node));
		_Node* current_node = *iterator;

		//The color gets defaulted as red, then eventually that will change in the insert check chain
		current_node->parent        = parent;
		current_node->color         = NODE_COLOR_RED;
		current_node->segment.start = start;
		current_node->segment.size  = size;

		tree_insert_check_1(*iterator);
	}

	_Node** SegmentTree::find_element(u32 start, _Node*& parent)
	{
		_Node** iterator = &root;
		while(*iterator) {
			u32 segment_start = (*iterator)->segment.start;
			//assert(start != segment_start, "cannot allocate a new segment of memory where another one is already defined, check the memory declaration");
			if(segment_start > start) {
				parent = *iterator;
				iterator = &(*iterator)->left;
			} else {
				parent = *iterator;
				iterator = &(*iterator)->right;
			}
		}

		return iterator;
	}

	void SegmentTree::remove_element(u32 start)
	{
		_Node* parent = nullptr;
		_Node** iterator = find_element(start, parent);

		auto current_node = *iterator;

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
		tree_delete_check(node_to_delete, true);
	}



	static void red_black_tree_delete(_Node* node)
	{
		if(!node) return;

		red_black_tree_delete(node->left);
		red_black_tree_delete(node->right);
		delete node;
	}

	void SegmentTree::cleanup()
	{
		red_black_tree_delete(root);
		root = nullptr;
	}

	void SegmentTree::tree_delete_check(_Node* node_to_delete, bool perform_deletion)
	{
		auto node_sibling = sibling(node_to_delete);
		auto delete_node_util = [&](_Node* node_to_delete)
		{
			if(!perform_deletion) {
				if(node_to_delete->color & NODE_COLOR_DOUBLE_BLACK) {
					node_to_delete->color = NODE_COLOR_BLACK;
				}

				return;
			}

			if(node_to_delete->parent) {
				if(node_to_delete->parent->left == node_to_delete)
					node_to_delete->parent->left = nullptr;
				else
					node_to_delete->parent->right = nullptr;
			}

			delete node_to_delete;
		};

		if(!node_to_delete->left && !node_to_delete->right && get_node_color(node_to_delete) & NODE_COLOR_RED) {
			delete_node_util(node_to_delete);
			return;
		}

		if(!node_to_delete->parent) {
			//This was the last node, reset tree
			root = nullptr;
			assert(perform_deletion, "there should not be no way this flag is false when we have only one node");
			delete_node_util(node_to_delete);
			return;
		}

		if(node_sibling) {
			if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_BLACK) {
				auto parent_color   = node_sibling->parent->color;
				node_sibling->color = NODE_COLOR_RED;

				if(parent_color & NODE_COLOR_RED) {
					node_sibling->parent->color = NODE_COLOR_BLACK;
					delete_node_util(node_to_delete);
				} else {
					//Black or double black
					node_sibling->parent->color = NODE_COLOR_DOUBLE_BLACK;
					delete_node_util(node_to_delete);
					tree_delete_check(node_sibling->parent, false);
				}

				return;
			}

			//TOBECONTINUED @C7 TEST THIS::::::::::::::::::::
			if(get_node_color(node_sibling) & NODE_COLOR_RED) {
				auto temp_color             = node_sibling->parent->color;
				node_sibling->parent->color = node_sibling->color;
				node_sibling->color         = temp_color;

				if(node_sibling == node_sibling->parent->left) {
					auto new_node = rotate_left(node_sibling->parent);
					tree_delete_check(new_node, true);
				}
				if(node_sibling == node_sibling->parent->right) {
					auto new_node = rotate_right(node_sibling->parent);
					tree_delete_check(new_node, true);
				}

				delete_node_util(node_to_delete);
				return;
			}

			if(node_sibling == node_sibling->parent->left) {
				//Case 5
				if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_RED) {
					node_sibling->right->color = node_sibling->color;
					node_sibling->color        = NODE_COLOR_RED;

					rotate_right(node_sibling->parent);
				}
				//Case 6
				if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_RED) {
					auto temp_color             = node_sibling->parent->color;
					node_sibling->parent->color = node_sibling->color;
					node_sibling->color         = temp_color;

					node_sibling->left->color   = NODE_COLOR_BLACK;

					rotate_right(node_sibling->parent);
				}

			} else {
			//Case 5
				if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_BLACK && get_node_color(node_sibling->left) & NODE_COLOR_RED) {
					node_sibling->left->color = node_sibling->color;
					node_sibling->color       = NODE_COLOR_RED;

					rotate_left(node_sibling->parent);
				}

				//Case 6
				if(get_node_color(node_sibling) & NODE_COLOR_BLACK && get_node_color(node_sibling->right) & NODE_COLOR_RED) {
					auto temp_color             = node_sibling->parent->color;
					node_sibling->parent->color = node_sibling->color;
					node_sibling->color         = temp_color;

					node_sibling->right->color  = NODE_COLOR_BLACK;

					rotate_left(node_sibling->parent);
				}
			}

			if (!node_sibling->parent)
				root = node_sibling;

			delete_node_util(node_to_delete);
		}
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
		assert(root->color & NODE_COLOR_BLACK, "the start of a red black tree requires a black node");

		//If all the black paths need to have the same black lenght, just pick a random path first, measure the black
		//lenght, and then check all the other lenghts against it
		u32 left_path_black_nodes = 0;
		for(auto iter = root; iter; iter = iter->left) {
			if(iter->color & NODE_COLOR_BLACK)
				left_path_black_nodes++;
		}

		check_all_paths_have_the_same_amount_of_black_nodes(root, left_path_black_nodes);
		check_all_red_nodes_have_black_children(root);
	}

	namespace test
	{
		static void tree_test_code_1()
		{
			gfx::SegmentTree this_tree;
			defer { this_tree.cleanup(); };

			this_tree.add_element(1, 0);
			this_tree.add_element(2, 0);
			this_tree.add_element(3, 0);
			this_tree.add_element(4, 0);
			this_tree.add_element(5, 0);
			this_tree.add_element(6, 0);
			this_tree.add_element(7, 0);
			this_tree.add_element(8, 0);
			this_tree.add_element(9, 0);
			this_tree.add_element(10, 0);
			this_tree.add_element(20, 0);
			this_tree.add_element(30, 0);

			this_tree.remove_element(2);
			this_tree.remove_element(4);
			this_tree.remove_element(1);
			this_tree.remove_element(8);

			assert_red_black_tree_validity(this_tree);
		}

		static void tree_test_code_2()
		{
			gfx::SegmentTree this_tree;
			defer { this_tree.cleanup(); };

			this_tree.add_element(2, 0);
			this_tree.add_element(1, 0);
			this_tree.add_element(3, 0);

			this_tree.remove_element(1);
			this_tree.remove_element(3);

			assert_red_black_tree_validity(this_tree);
		}

		static void tree_test_code_3()
		{
			gfx::SegmentTree this_tree;
			defer { this_tree.cleanup(); };

			this_tree.add_element(10, 0);
			this_tree.add_element(9, 0);
			this_tree.add_element(8, 0);
			this_tree.add_element(7, 0);
			this_tree.add_element(6, 0);
			this_tree.add_element(5, 0);
			this_tree.add_element(4, 0);
			this_tree.add_element(3, 0);
			this_tree.add_element(2, 0);

			this_tree.remove_element(9);
			this_tree.remove_element(8);

			assert_red_black_tree_validity(this_tree);
		}

		void memory_run_tests()
		{
			tree_test_code_1();
			tree_test_code_2();
			tree_test_code_3();
			log_message("(memory_run_tests) tests: OK");
		}


	}
}