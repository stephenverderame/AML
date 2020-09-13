#pragma once
#include <initializer_list>
#include <tuple>
#include <array>
#include <utility>
#include <stdexcept>
namespace cth {
	constexpr static long BASE = 256;
	constexpr static long MOD = 217500000;
	constexpr size_t strlen_c(const char* str)
	{
		size_t len = 0;
		for (; str[len] != '\0'; ++len);
		return len;
	}
	template<typename T>
	constexpr T abs_c(T a) {
		static_assert(std::is_integral<T>::value, "Must take the absolute value of a number");
		return a < (T)0 ? -a : a;
	}
	//The actual hash function for the data
	//Should be overloaded for every typename you wish to support
	constexpr unsigned long firstHash(const char* str)
	{
		size_t len = strlen_c(str);
		unsigned long r = 0;
		for (size_t i = 0; i < len; ++i) {
			r = r * (long long)BASE + str[i];
			r %= MOD;
		}
		return r;
	}
	constexpr unsigned long firstHash(const char str)
	{
		return str;
	}
	template<typename T>
	constexpr void memset_c(T* data, T val, const size_t size)
	{
		for (size_t i = 0; i < size; ++i)
			data[i] = val;
	}
	template<typename T, typename = void>
	struct is_hashable : std::false_type {};

	template<typename T>
	struct is_hashable<T, std::void_t<decltype(cth::firstHash(std::declval<T&>()))>> : std::true_type {};
}
//Exception thrown when the perfect hash could not be found
class perfect_hash_exception : public std::exception 
{
private:
	const char* msg;
public:
	perfect_hash_exception(const char* msg) : msg(msg) {}
	const char* what() const override {
		return msg;
	}
};
//Compile time Pair
template<typename K, typename V>
struct Tuple {
	K k;
	V v;

	constexpr Tuple(K k, V v) : k(k), v(v) {};
	constexpr Tuple() : k(K()), v(V()) {};

//	inline K& key() { return k; }
//	inline V& val() { return v; }

	constexpr inline K key() const { return k; }
	constexpr inline V val() const { return v; }
};
//Compile time array
template<typename T, long SIZE>
class Array {
private:
	T data[SIZE];
public:

	constexpr Array() : data() {
		for (long i = 0; i < SIZE; ++i)
			data[i] = T();
	}
	constexpr Array(T val) : data() {
		for (long i = 0; i < SIZE; ++i)
			data[i] = val;
	}

	constexpr inline void set(const long index, T val) {
		data[index] = val;
	}
	constexpr const T operator[](const long index) const {
		return data[index];
	}
	constexpr T& operator[](const long i) {
		return data[i];
	}
	constexpr inline long size() const {
		return SIZE;
	}
	constexpr const T const & get(const long i) const {
		return data[i];
	}
	constexpr inline void fill(T val, long beg = 0, long end = SIZE) {
		std::fill(&data[beg], data + end, val);
	}
	constexpr inline T* begin() {
		return &data[0];
	}
	constexpr inline T* end() {
		return data + SIZE;
	}
	template<long NEW_SIZE>
	constexpr Array<T, NEW_SIZE> resize(T defaultVal = T()) const {
		Array<T, NEW_SIZE> newArray(defaultVal);
		long min = std::min(SIZE, NEW_SIZE);
		for (int i = 0; i < min; ++i)
			newArray[i] = get(i);
		return newArray();
	}
};

//Compile-time perfect hash table. Should be declared constexpr
template<typename K, typename V>
class CompileTimeHash
{
private:
	constexpr const static long CTM_NAN = ~0;
	//Due to step limits during compile time, size cannot be too big
	//can be altered with compiler options with depth, backtrace and steps
	constexpr const static long CTM_MAX_SIZE = 100;
	Array<K, CTM_MAX_SIZE> keys; //must have an array wrapper bc we need a constexpr constructor
	Array<V, CTM_MAX_SIZE> data;
	Array<long, CTM_MAX_SIZE> shifts;
private:

	/**
	* Determines if there are multiple elements in a single column of the table
	* First row of the table is modified to reflect the collapsed table
	* @param row    only searches the table at this row and above (lower)
	* @param shift    the offset of each element in this row from their original position
	* @return  true if there are multiple elements in the same column, false otherwise
	*/
	constexpr bool isCollision(Array<Array<bool, CTM_MAX_SIZE>, CTM_MAX_SIZE> & table, const long row, const long shift = 0) const
	{
		if (row > 0) {
			bool collision = false;
			for (int i = 0; i < CTM_MAX_SIZE; ++i) {
				if (table[row][i] && table[0][(i + shift) % CTM_MAX_SIZE]) {
					collision = true;
					break;
				}
			}
			if (!collision) {
				for (int i = 0; i < CTM_MAX_SIZE; ++i) {
					if (table[row][i])
						table[0][(i + shift) % CTM_MAX_SIZE] = true;
				}
			}
			return collision;
		}
		return false;
	}
	/**
	* Determines the amoung the row should be shifted to avoid a collision
	* @param row    the current row in the table
	* @param table    2D MAX_SIZE * MAX_SIZE array. First row is modified by isCollision()
	* @return   the amount this row should be shifted or MAX_SIZE on failure
	*/
	constexpr long calcShift(Array<Array<bool, CTM_MAX_SIZE>, CTM_MAX_SIZE> & table, const long row) const
	{
		long shift = 0;
		for (; shift < CTM_MAX_SIZE && isCollision(table, row, shift); ++shift);
		return shift;
	}

	constexpr unsigned long hash(K k) const {
		unsigned long h = cth::firstHash(k);
		return (shifts[(h / CTM_MAX_SIZE) % CTM_MAX_SIZE] + (h % CTM_MAX_SIZE)) % CTM_MAX_SIZE;
	}
	constexpr inline void init(const Tuple<K, V> list[], const long listSize) {
		static_assert(cth::is_hashable<K&>::value, "Key value must be a hashable type");
		Array<Array<bool, CTM_MAX_SIZE>, CTM_MAX_SIZE> table(false);
		for (long i = 0; i < listSize; ++i) {
			const unsigned long hash = cth::firstHash(list[i].key());
			table[(hash / CTM_MAX_SIZE) % CTM_MAX_SIZE][hash % CTM_MAX_SIZE] = true;
		}
		for (long i = 0; i < CTM_MAX_SIZE; ++i) {
			shifts[i] = calcShift(table, i);
			shift_check(shifts[i]);
		}
		for (long i = 0; i < listSize; ++i) {
			long h = hash(list[i].key());
			data[h] = list[i].val();
			keys[h] = list[i].key();

		}
	}
public:
	//Checks if a perfect hash was found
	//throwing during constexpr will be a compiler error
	constexpr long shift_check(const long shiftAmount) const {
		return shiftAmount >= CTM_MAX_SIZE ? throw perfect_hash_exception("Perfect hash cannot be created") : shiftAmount;
	}
	constexpr CompileTimeHash(const Tuple<K, V> list[], const long listSize) {
		init(list, listSize);
	}
	constexpr CompileTimeHash(const Tuple<K, V> list[], const long listSize, const V& defaultVal) {
		data.fill(defaultVal);
		init(list, listSize);
	}
	//Gets the value in the bucker the key was hashed to
	//Still has a return value if the key is not in the hash table
	constexpr V operator[](K k) const {
		return data[hash(k)];
	}
	constexpr bool find(K k) const {
		return cth::firstHash(keys[hash(k)]) == cth::firstHash(k);
	}

	/**
	* Gets a value for the given key if the given key is a value in the hash table
	* @param K the key
	* @param v output parameter for the value returned by hashing the key. Only valid if the return is true
	* @return boolean signifying if the key was found (true) or not (false)
	*/
	constexpr bool getifValid(K k, V& v) const {
		long h = hash(k);
		v = data[h];
		return keys[h] != nullptr && cth::firstHash(keys[h]) == cth::firstHash(k);
	}
	
};




