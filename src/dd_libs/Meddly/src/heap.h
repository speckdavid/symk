
// $Id$

#ifndef HEAP_H
#define HEAP_H

#include <vector>

using namespace std;

template <typename T, typename Comp>
class IndexedHeap
{
private:
	static const int NOT_IN_HEAP=-1;

	struct Item
	{
		int key;
		T value;
	};

	static inline size_t left(size_t i)
	{
		return i*2+1;
	}

	static inline size_t right(size_t i)
	{
		return i*2+2;
	}

	static inline size_t parent(size_t i)
	{
		return (i-1)/2;
	}

	vector<Item> _heap;
	vector<int> _indices;
	Comp _comp;

	inline bool lessthan(const T& x, const T& y)
	{
		return _comp(x, y);
	}

	inline bool has_left(size_t index)
	{
		return left(index) < _heap.size();
	}

	inline bool has_right(size_t index)
	{
		return right(index) < _heap.size();
	}

	void percolate_down(int key)
	{
		MEDDLY_DCASSERT(is_in_heap(key));
		int index = _indices[key];
		Item item = _heap[index];
		while(has_left(index)){
			int min_index = left(index);
			if (has_right(index)){
				if (lessthan(_heap[right(index)].value, _heap[min_index].value)) {
					min_index = right(index);
				}
			}
			if (lessthan(_heap[min_index].value, item.value)) {
				_indices[_heap[min_index].key]=index;
				_heap[index] = _heap[min_index];
				index = min_index;
			}
			else{
				break;
			}
		}
		_indices[key] = index;
		_heap[index] = item;
	}

	void percolate_up(int key)
	{
		MEDDLY_DCASSERT(is_in_heap(key));
		int index = _indices[key];
		Item item = _heap[index];
		int p = parent(index);
		while (index>0 && lessthan(item.value, _heap[p].value)){
			_indices[_heap[p].key] = index;
			_heap[index] = _heap[p];
			index = p;
			p = parent(index);
		}
		_indices[key] = index;
		_heap[index] = item;
	}

public:
	IndexedHeap(int size)
	{
		_indices.assign(size+1, NOT_IN_HEAP);
		_heap.reserve(size);
	}

	void push(int key, T value)
	{
		if(!is_in_heap(key)){
			_indices[key] = _heap.size(); 
			Item i; i.key = key; i.value = value;
			_heap.push_back(i);
			percolate_up(key);
		}
		else {
			// If the index is in the heap
			// Update its value and location
			T old_value = _heap[_indices[key]].value;
			_heap[_indices[key]].value = value;
			if(old_value < value) {
				percolate_down(key);
			}
			else if(old_value>value) {
				percolate_up(key);
			}
		}
	}

	void pop()
	{
		int key = top_key();
		_heap[0] = _heap.back();
		_indices[_heap[0].key] = 0;
		_indices[key] = NOT_IN_HEAP;
		_heap.pop_back();
		if(_heap.size()>1) {
			percolate_down(_heap[0].key);
		}
	}

	int top_key() const
	{
		return _heap[0].key;
	}

	T top() const
	{
		return _heap[0].value;
	}

	bool is_in_heap(int key) const
	{
		return _indices[key]!=NOT_IN_HEAP;
	}

	bool empty() const
	{
		return _heap.empty();
	}
};

template <typename T, typename Comp>
const int IndexedHeap<T, Comp>::NOT_IN_HEAP;

#endif
