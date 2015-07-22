
namespace thor {
namespace util {

template<typename Key, typename Value, typename Hasher, typename Allocator>
class Hashmap {
public:
	typedef int SizeType;

	Hashmap(const Hasher &hasher, Allocator *allocator);

	void insert(const Key &key, const Value &value);
	void insert(const Key &key, Value &&value);

	Value &get(const Key &key);
	
	void rehash(SizeType new_capacity);

private:
	struct Item {
		Key key;
		Value value;
		Item *chain;

		Item(const Key &new_key, const Value &new_value)
				: key(new_key), value(new_value), chain(nullptr) { }
		Item(const Key &new_key, Value &&new_value)
				: key(new_key), value(util::move(new_value)), chain(nullptr) { }
	};
	
	Hasher p_hasher;
	Allocator *p_allocator;
	Item **p_table;
	SizeType p_capacity;
	SizeType p_size;
};

template<typename Key, typename Value, typename Hasher, typename Allocator>
Hashmap<Key, Value, Hasher, Allocator>::Hashmap(const Hasher &hasher, Allocator *allocator)
		: p_hasher(hasher), p_allocator(allocator), p_capacity(10), p_size(0) {
	p_table = (Item **)allocator->allocate(sizeof(Item *) * p_capacity);
	for(SizeType i = 0; i < p_capacity; i++)
		p_table[i] = nullptr;
}

template<typename Key, typename Value, typename Hasher, typename Allocator>
void Hashmap<Key, Value, Hasher, Allocator>::insert(const Key &key, const Value &value) {
	if(p_size > p_capacity)
		rehash(2 * p_size);

	unsigned int bucket = ((unsigned int)p_hasher(key)) % p_capacity;
	
	auto item = new (p_allocator) Item(key, value);
	item->chain = p_table[bucket];
	p_table[bucket] = item;
	p_size++;
}
template<typename Key, typename Value, typename Hasher, typename Allocator>
void Hashmap<Key, Value, Hasher, Allocator>::insert(const Key &key, Value &&value) {
	if(p_size > p_capacity)
		rehash(2 * p_size);

	unsigned int bucket = ((unsigned int)p_hasher(key)) % p_capacity;
	
	auto item = new (p_allocator) Item(key, util::move(value));
	item->chain = p_table[bucket];
	p_table[bucket] = item;
	p_size++;
}

template<typename Key, typename Value, typename Hasher, typename Allocator>
Value &Hashmap<Key, Value, Hasher, Allocator>::get(const Key &key) {
	unsigned int bucket = ((unsigned int)p_hasher(key)) % p_capacity;

	for(Item *item = p_table[bucket]; item != nullptr; item = item->chain) {
		if(item->key == key)
			return item->value;
	}

	debug::criticalLogger->log("Hashmap::get(): Element not found");
	debug::panic();
}

template<typename Key, typename Value, typename Hasher, typename Allocator>
void Hashmap<Key, Value, Hasher, Allocator>::rehash(SizeType new_capacity) {
	debug::criticalLogger->log("FIXME: Implement rehash");
}

template<typename T>
class DefaultHasher;

template<>
class DefaultHasher<uint64_t> {
public:
	unsigned int operator() (uint64_t v) {
		static_assert(sizeof(unsigned int) == 4, "Expected sizeof(int) == 4");
		return (unsigned int)(v ^ (v >> 32));
	}
};

}} // namespace thor::util
