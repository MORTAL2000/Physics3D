#pragma once

#include "iteratorFactory.h"
#include "iteratorEnd.h"

#include "../math/position.h"
#include "../math/fix.h"
#include "../math/bounds.h"

#include <utility>
#include <new>
#include <assert.h>


#define MAX_BRANCHES 4
#define MAX_HEIGHT 64
#define LEAF_NODE_SIGNIFIER 0x7FFFFFFF

struct TreeNode {
	Bounds bounds;
	union {
		TreeNode* subTrees;
		void* object;
	};
	int nodeCount;
	/* means that the nodes within this node belong to a specific group, if true, the tree will not separate the elements below this one. 
	New elements will not be added to this group unless specifically specified
	If false, then no subnodes are allowed to be exchanged with the rest of the tree. This node must be viewed as a black box. 
	*/
	bool isGroupHead = false;

	inline bool isLeafNode() const { return nodeCount == LEAF_NODE_SIGNIFIER; }

	inline TreeNode() : nodeCount(0), object(nullptr) {}
	TreeNode(TreeNode* subTrees, int nodeCount);
	inline TreeNode(void* object, const Bounds& bounds) : nodeCount(LEAF_NODE_SIGNIFIER), object(object), bounds(bounds) {}
	inline TreeNode(void* object, const Bounds& bounds, bool isGroupHead) : nodeCount(LEAF_NODE_SIGNIFIER), object(object), bounds(bounds), isGroupHead(isGroupHead) {}
	inline TreeNode(const Bounds& bounds, TreeNode* subTrees, int nodeCount) : bounds(bounds), subTrees(subTrees), nodeCount(nodeCount) {}

	explicit TreeNode(const TreeNode& original);
	TreeNode& operator=(const TreeNode& original);

	inline TreeNode(TreeNode&& other) noexcept : nodeCount(other.nodeCount), subTrees(other.subTrees), bounds(other.bounds), isGroupHead(other.isGroupHead) {
		other.subTrees = nullptr;
		other.nodeCount = LEAF_NODE_SIGNIFIER;
	}
	inline TreeNode& operator=(TreeNode&& other) noexcept {
		std::swap(this->nodeCount, other.nodeCount);
		std::swap(this->subTrees, other.subTrees);
		std::swap(this->bounds, other.bounds);
		std::swap(this->isGroupHead, other.isGroupHead);
		return *this;
	}
	
	inline TreeNode* begin() const { return subTrees; }
	inline TreeNode* end() const { return subTrees+nodeCount; }
	inline TreeNode& operator[](int index) { return subTrees[index]; }
	inline const TreeNode& operator[](int index) const { return subTrees[index]; }

	~TreeNode();


	void addOutside(TreeNode&& newNode);
	void addInside(TreeNode&& newNode);
	TreeNode remove(int index);
	

	void recalculateBounds();
	void recalculateBoundsFromSubBounds();
	void recalculateBoundsRecursive();

	void improveStructure();

	size_t getNumberOfObjectsInNode() const;
	size_t getLengthOfLongestBranch() const;
};

long long computeCost(const Bounds& bounds);

Bounds computeBoundsOfList(const TreeNode* const* list, size_t count);

Bounds computeBoundsOfList(const TreeNode* list, size_t count);

struct TreeStackElement {
	TreeNode* node;
	int index;
};

struct NodeStack {
	TreeStackElement* top;
	TreeStackElement stack[MAX_HEIGHT];

	NodeStack() = default;
	NodeStack(TreeNode& rootNode);
	// a find function, returning the stack of all nodes leading up to the requested object
	NodeStack(TreeNode& rootNode, const void* objToFind, const Bounds& objBounds);

	NodeStack(const NodeStack& other);
	NodeStack(NodeStack&& other) noexcept;
	NodeStack& operator=(const NodeStack& other);
	NodeStack& operator=(NodeStack&& other) noexcept;

	inline bool operator!=(IteratorEnd) const {
		return top + 1 != stack;
	}
	inline TreeNode* operator*() const {
		return top->node;
	}
	void riseUntilAvailableWhile();
	void riseUntilAvailableDoWhile();
	void riseUntilGroupHeadDoWhile();
	void riseUntilGroupHeadWhile();
	void updateBoundsAllTheWayToTop();
	void expandBoundsAllTheWayToTop();

	// removes the object currently pointed to
	TreeNode remove();
};

struct ConstTreeIterator : public NodeStack {
	ConstTreeIterator() = default;
	ConstTreeIterator(TreeNode& rootNode) : NodeStack(rootNode) {
		// the very first element is a dummy, in order to detect when the tree is done
		
		if (rootNode.nodeCount == 0) return;
		
		while(!top->node->isLeafNode())	{
			TreeNode* nextNode = &top->node->subTrees[top->index];
			top++;
			top->node = nextNode;
			top->index = 0;
		} 
	}
	inline void delveDown() {
		while(!top->node->isLeafNode()) {
			TreeNode* nextNode = &top->node->subTrees[top->index];
			top++;
			top->node = nextNode;
			top->index = 0;
		}
	}
	inline void operator++() {
		// go back up until a new available node is found
		do {
			top--;
			if (top < stack) return;
			top->index++;
		} while (top->index == top->node->nodeCount);
		// delve down until a feasible leaf node is found
		delveDown();
	}
	inline TreeNode remove() {
		TreeNode result = NodeStack::remove();
		if(top >= stack) {
			delveDown();
		}
		return result;
	}
};

struct TreeIterator : public ConstTreeIterator {
	TreeIterator() = default;
	TreeIterator(TreeNode& rootNode) : ConstTreeIterator(rootNode) {
		// the very first element is a dummy, in order to detect when the tree is done

		if (rootNode.nodeCount == 0) return;
	}
};

/*
	Iterates through the tree, applying Filter at every level to cull branches that should not be searched

	Filter must define an operator(Bounds) returning true if the filter passes for this bound, and false if it does not.

	For correct operation, it must abide by the following:
	- If the filter returns true for some bound, then it must also return true for any bound fully encompassing the first bound. 
	- If the filter returns false for some bound, then it must return false for all bounds it encompasses. 

	Correct filter:
	- BoundIntersectsRay
	    If the bound intersects a ray, then this ray must also intersect it's parent
		If the bound does not intersect the ray, then it's children cannot
	
	Incorrect filter:
	- BoundsContainedIn
		If a given bound is contained in another bound2, that does not mean that it's parent must also be contained in this bound2
		However, BoundsNotContainedIn IS a correct filter
*/
template<typename Filter>
struct FilteredTreeIterator : public NodeStack {
	Filter filter;
	FilteredTreeIterator(TreeNode& rootNode, const Filter& filter) : NodeStack(rootNode), filter(filter) {
		// the very first element is a dummy, in order to detect when the tree is done
		if (rootNode.nodeCount == 0) return;

		if(!rootNode.isLeafNode())
			delveDownFiltered();
	}

	void delveDownFiltered() {
		while (true) {
			// go down
			TreeNode* nextNode = &top->node->subTrees[top->index];
			top++;
			top->node = nextNode;

			if (filter(*nextNode)) {
				if (nextNode->isLeafNode()) {
					return;
				} else {
					top->index = 0;
				}
			} else {
				top--;
				top->index++;
				while (top->index == top->node->nodeCount) {
					top--;
					if (top < stack) return;
					top->index++;
				}
			}
		}
	}

	inline void operator++() {
		// go back up until a new available node is found
		do {
			top--;
			if (top < stack) return;
			top->index++;
		} while (top->index == top->node->nodeCount);

		delveDownFiltered();
	}
	inline TreeNode remove() {
		TreeNode result = NodeStack::remove();
		if(top >= stack) {
			delveDownFiltered();
		}
		return result;
	}
};

template<typename Iter, typename Boundable>
struct BoundsTreeIter : public Iter {
	BoundsTreeIter() = default;
	BoundsTreeIter(const Iter& iter) : Iter(iter) {}

	Boundable& operator*() const {
		return *static_cast<Boundable*>(Iter::operator*()->object);
	}
};

template<typename Filter, typename Boundable>
using FilteredBoundsTreeIter = BoundsTreeIter<FilteredTreeIterator<Filter>, Boundable>;

template<typename Boundable>
struct DoNothingFilter {
	constexpr bool operator()(const TreeNode& node) const { return true; }
	constexpr bool operator()(const Boundable& b) const { return true; }
};
struct FinderFilter {
	Bounds filterBounds;

	FinderFilter() = default;
	FinderFilter(const Bounds& filterBounds) : filterBounds(filterBounds) {}

	bool operator()(const TreeNode& b) const { return b.bounds.contains(filterBounds); }
};

template<typename Boundable, typename Filter>
struct TreeIterFactory;

template<typename Boundable>
struct BoundsTree {
	TreeNode rootNode;

	BoundsTree() : rootNode() {

	}

	inline bool isEmpty() const {
		return this->rootNode.nodeCount == 0;
	}

	void add(TreeNode&& node) {
		if(isEmpty()) {
			this->rootNode = std::move(node);
		} else {
			this->rootNode.addOutside(std::move(node));
			if(this->rootNode.nodeCount == 1) {
				this->rootNode.bounds = node.bounds; // bit of a band-aid fix, as an empty treeNode's bounds can't really be defined
			}
		}
	}

	void add(Boundable* obj, const Bounds& bounds) {
		this->add(TreeNode(obj, bounds, true));
	}
	
	void addToExistingGroup(Boundable* obj, const Bounds& bounds, TreeNode& groupNode) {
		groupNode.addInside(TreeNode(obj, bounds, false));
	}

	NodeStack find(const Boundable* obj, const Bounds& objBounds) {
		return NodeStack(rootNode, obj, objBounds);
	}

	NodeStack findGroupFor(const Boundable* obj, const Bounds& objBounds) {
		NodeStack iter(rootNode, obj, objBounds);
		iter.riseUntilGroupHeadWhile();
		return iter;
	}

	void addToExistingGroup(Boundable* obj, const Bounds& bounds, const Boundable* objInGroup, const Bounds& objInGroupBounds) {
		NodeStack iter(rootNode, objInGroup, objInGroupBounds);
		iter.riseUntilGroupHeadWhile();
		addToExistingGroup(obj, bounds, **iter);
		iter.expandBoundsAllTheWayToTop();
	}

	void remove(const Boundable* obj, const Bounds& strictBounds) {
		if (rootNode.isLeafNode()) {
			if (rootNode.object == obj) {
				rootNode.nodeCount = 0;
				rootNode.bounds = Bounds();
				rootNode.object = nullptr;
			} else {
				throw "Attempting to remove nonexistent object!";
			}
		} else {
			NodeStack stack(rootNode, obj, strictBounds);
			stack.remove();
		}
	}
	void remove(const Boundable* obj) {
		this->remove(obj, obj->getStrictBounds());
	}

	// removes and returns the node for the given object
	inline TreeNode grab(const Boundable* obj, const Bounds& objBounds) {
		if(this->rootNode.isLeafNode()) {
			if(this->rootNode.object == obj) {
				TreeNode result(std::move(this->rootNode));
				this->rootNode.object = nullptr;
				this->rootNode.nodeCount = 0;
				return result;
			} else {
				throw "Attempting to remove nonexistent object!";
			}
		}
		NodeStack iter(rootNode, obj, objBounds);
		return iter.remove();
	}

	// removes and returns the group node for the given object
	inline TreeNode grabGroupFor(const Boundable* obj, const Bounds& objBounds) {
		if(this->rootNode.isLeafNode()) {
			if(this->rootNode.object == obj) {
				TreeNode result(std::move(this->rootNode));
				this->rootNode.object = nullptr;
				this->rootNode.nodeCount = 0;
				return result;
			} else {
				throw "Attempting to remove nonexistent object!";
			}
		}
		NodeStack iter(rootNode, obj, objBounds);
		iter.riseUntilGroupHeadWhile();
		return iter.remove();
	}

	inline void recalculateBounds() {
		if(isEmpty()) return;
		for (TreeNode* currentNode : *this) {
			Boundable* obj = static_cast<Boundable*>(currentNode->object);
			currentNode->bounds = obj->getStrictBounds();
		}

		rootNode.recalculateBoundsRecursive();
	}
	
	void updateObjectBounds(const Boundable* obj, const Bounds& oldBounds) {
		assert(!isEmpty());
		NodeStack stack(rootNode, obj, oldBounds);
		stack.top->node->bounds = obj->getStrictBounds();
		stack.top--;
		stack.updateBoundsAllTheWayToTop();
	}
	void updateObjectGroupBounds(const Boundable* objInGroup, const Bounds& objOldBounds) {
		assert(!isEmpty());
		NodeStack stack(rootNode, objInGroup, objOldBounds);
		stack.riseUntilGroupHeadWhile(); // find group obj belongs to

		for (TreeIterator iter(*stack.top->node); iter != IteratorEnd(); ++iter) {
			TreeNode* node = *iter;
			node->bounds = static_cast<Boundable*>(node->object)->getStrictBounds();
		}
		stack.top->node->recalculateBoundsRecursive(); // refresh group bounds
		stack.top--;
		stack.updateBoundsAllTheWayToTop(); // refresh rest of tree to accommodate
	}

	inline void improveStructure() { if(!isEmpty()) rootNode.improveStructure(); }
	
	inline size_t getNumberOfObjects() const {
		if(isEmpty()) {
			return 0;
		} else {
			return getNumberOfObjectsInNode(this->rootNode);
		}
	}

	inline TreeIterator begin() { return TreeIterator(rootNode); };
	inline ConstTreeIterator begin() const { return ConstTreeIterator(const_cast<TreeNode&>(rootNode)); };
	inline IteratorEnd end() const { return IteratorEnd(); };

	template<typename Filter>
	inline TreeIterFactory<Boundable, Filter> iterFiltered(const Filter& filter);

	template<typename Filter>
	inline TreeIterFactory<const Boundable, Filter> iterFiltered(const Filter& filter) const;
};

template<typename Boundable, typename Filter>
struct TreeIterFactory {
	TreeNode* rootNode;
	Filter filter;
	TreeIterFactory() {}
	TreeIterFactory(TreeNode* rootNode, const Filter& filter) : rootNode(rootNode), filter(filter) {}
	FilteredBoundsTreeIter<Filter, Boundable> begin() const {
		return FilteredBoundsTreeIter<Filter, Boundable>(FilteredTreeIterator<Filter>(*rootNode, filter));
	}
	constexpr IteratorEnd end() const { return IteratorEnd(); }
};

template<typename Boundable>
template<typename Filter>
inline TreeIterFactory<Boundable, Filter> BoundsTree<Boundable>::iterFiltered(const Filter& filter) {
	return TreeIterFactory<Boundable, Filter>(&this->rootNode, filter);
}

template<typename Boundable>
template<typename Filter>
inline TreeIterFactory<const Boundable, Filter> BoundsTree<Boundable>::iterFiltered(const Filter& filter) const {
	return TreeIterFactory<const Boundable, Filter>(&this->rootNode, filter);
}
