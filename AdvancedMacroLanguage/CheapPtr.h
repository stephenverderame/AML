#pragma once
#include <stdexcept>
class unallocated_memory_exception : public std::exception {
private:
	const char* msg;
public:
	unallocated_memory_exception(const char* msg) : msg(msg) {};
	const char* what() const override {
		return msg;
	}
};
//Non atomic shared_ptr
template<typename T>
class CheapPtr
{
	//Invariants: refCount cannot be 0, data cannot be nullptr (assuming block is not nullptr)
private:
	struct mem {
		T* data = nullptr;
		unsigned int refCount = 0;
	};
	mem* block; //can be null
private:
	//releases control of this instance of the shared memory by decrementing the reference count
	//becomes a null CheapPtr
	inline void decCount() {
		if (block != nullptr) {
			if (--block->refCount == 0) {
				delete block->data;
				delete block;
			}
			block = nullptr;
		}
	}
	//takes ownership of t
	//requires that t be an unmanged ptr resource
	CheapPtr(T* t) {
		block = new mem();
		block->data = t;
		block->refCount++;
	}
public:
	constexpr CheapPtr() : block(nullptr) {}

	CheapPtr(const T&& t) {
		block = new mem();
		block->data = new T();
		*block->data = t;
		block->refCount++;
	}
	CheapPtr(const CheapPtr& other) {
		block = other.block;
		if (block != nullptr) ++block->refCount;
	}
	CheapPtr(CheapPtr&& other) {
		block = other.block;
		other.block = nullptr;
	}
	~CheapPtr() {
		decCount();
	}
	CheapPtr& operator=(const CheapPtr& other) {
		decCount();
		block = other.block;
		if (block != nullptr) ++block->refCount;
		return *this;
	}
	CheapPtr& operator=(CheapPtr&& other) {
		decCount();
		block = other.block;
		other.block = nullptr;
		return *this;
	}
	//Requires to be instantiated (no default constructor)
	inline T& operator*() {
		if (block == nullptr) throw unallocated_memory_exception("Tried to dereference unallocated memory!");
		return *block->data;
	}
	//Requires to be instantiated (no default constructor)
	inline T* operator->() {
		if (block == nullptr) throw unallocated_memory_exception("Tried to dereference unallocated memory!");
		return block->data;
	}
	//Requires to be instantiated (no default constructor)
	inline T* get() {
		if (block == nullptr) throw unallocated_memory_exception("Tried to dereference unallocated memory!");
		return block->data;
	}
	//Requires that T be allocated with an array
	inline T& operator[](unsigned int u) {
		if (block == nullptr) throw unallocated_memory_exception("Tried to dereference unallocated memory!");
		return block->data[u];
	}
	inline bool isNull() const {
		return block == nullptr;
	}
	inline void toNull() {
		decCount();
	}
	inline bool operator==(const CheapPtr<T>& other) const {
		return (other.block == nullptr && block == nullptr) || (other.block != nullptr && block != nullptr && block->data == other.block->data);
	}
	inline bool operator==(T* p) const {
		return (block == nullptr && p == nullptr) || (block != nullptr && block->data == p);
	}
public:
	//Function to make it clear that we want to create a new managed pointer from an unmanaged ptr
	//requires t not be nullptr
	static CheapPtr<T> make_cheap_ptr(T* t) {
		return CheapPtr<T>(t);
	}
	template<typename... Args>
	static CheapPtr<T> make_cheap_ptr(Args&&... a) {
		return CheapPtr<T>(new T(std::forward<Args>(a)...));
	}
};
