#ifndef THOR_GENERIC_USERMEM_HPP
#define THOR_GENERIC_USERMEM_HPP

#include <frigg/rbtree.hpp>
#include <frigg/vector.hpp>
#include "error.hpp"
#include "types.hpp"
#include "futex.hpp"

namespace thor {

struct Memory;

using GrabIntent = uint32_t;
enum : GrabIntent {
	kGrabQuery = GrabIntent(1) << 0,
	kGrabFetch = GrabIntent(1) << 1,
	kGrabRead = GrabIntent(1) << 2,
	kGrabWrite = GrabIntent(1) << 3,
	kGrabDontRequireBacking = GrabIntent(1) << 4
};

enum class MemoryTag {
	null,
	hardware,
	allocated,
	backing,
	frontal,
	copyOnWrite
};

struct ManageBase {
	virtual void complete(Error error, uintptr_t offset, size_t size) = 0;

	frigg::IntrusiveSharedLinkedItem<ManageBase> processQueueItem;
};

template<typename F>
struct Manage : ManageBase {
	Manage(F functor)
	: _functor(frigg::move(functor)) { }

	void complete(Error error, uintptr_t offset, size_t size) override {
		_functor(error, offset, size);
	}

private:
	F _functor;
};

struct InitiateBase {
	InitiateBase(size_t offset, size_t length)
	: offset(offset), length(length), progress(0) { }

	virtual void complete(Error error) = 0;
	
	size_t offset;
	size_t length;

	// Current progress in bytes.
	size_t progress;
	

	frigg::IntrusiveSharedLinkedItem<InitiateBase> processQueueItem;
};

template<typename F>
struct Initiate : InitiateBase {
	Initiate(size_t offset, size_t length, F functor)
	: InitiateBase(offset, length), _functor(frigg::move(functor)) { }

	void complete(Error error) override {
		_functor(error);
	}

private:
	F _functor;
};

struct Memory {
	static void transfer(frigg::UnsafePtr<Memory> dest_memory, uintptr_t dest_offset,
			frigg::UnsafePtr<Memory> src_memory, uintptr_t src_offset, size_t length);

	Memory(MemoryTag tag)
	: _tag(tag) { }

	Memory(const Memory &) = delete;

	Memory &operator= (const Memory &) = delete;
	
	MemoryTag tag() const {
		return _tag;
	}

	virtual void copyKernelToThisSync(ptrdiff_t offset, void *pointer, size_t length);

	// Prevents eviction of a range of memory.
	// Does NOT ensure that this range is present before the call returns.
	virtual void acquire(uintptr_t offset, size_t length) = 0;
	virtual void release(uintptr_t offset, size_t length) = 0;
	
	// Optimistically returns the physical memory that backs a range of memory.
	// Result stays valid until the range is evicted.
	virtual PhysicalAddr peekRange(uintptr_t offset) = 0;

	// Returns the physical memory that backs a range of memory.
	// Ensures that the range is present before returning.
	// Result stays valid until the range is evicted.
	// TODO: This should be asynchronous.
	virtual PhysicalAddr fetchRange(uintptr_t offset) = 0;

	size_t getLength();

	void submitInitiateLoad(frigg::SharedPtr<InitiateBase> initiate);
	void submitHandleLoad(frigg::SharedPtr<ManageBase> handle);
	void completeLoad(size_t offset, size_t length);

private:
	MemoryTag _tag;
};

struct CopyToBundleNode {

};

struct CopyFromBundleNode {

};

void copyToBundle(Memory *bundle, ptrdiff_t offset, const void *pointer, size_t size,
		CopyToBundleNode *node, void (*complete)(CopyToBundleNode *));

void copyFromBundle(Memory *bundle, ptrdiff_t offset, void *pointer, size_t size,
		CopyFromBundleNode *node, void (*complete)(CopyFromBundleNode *));

struct HardwareMemory : Memory {
	static bool classOf(const Memory &memory) {
		return memory.tag() == MemoryTag::hardware;
	}

	HardwareMemory(PhysicalAddr base, size_t length);
	~HardwareMemory();

	void acquire(uintptr_t offset, size_t length) override;
	void release(uintptr_t offset, size_t length) override;

	PhysicalAddr peekRange(uintptr_t offset) override;
	PhysicalAddr fetchRange(uintptr_t offset) override;

	size_t getLength();

private:
	PhysicalAddr _base;
	size_t _length;
};

struct AllocatedMemory : Memory {
	static bool classOf(const Memory &memory) {
		return memory.tag() == MemoryTag::allocated;
	}

	AllocatedMemory(size_t length, size_t chunk_size = kPageSize,
			size_t chunk_align = kPageSize);
	~AllocatedMemory();

	void copyKernelToThisSync(ptrdiff_t offset, void *pointer, size_t length) override;

	void acquire(uintptr_t offset, size_t length) override;
	void release(uintptr_t offset, size_t length) override;

	PhysicalAddr peekRange(uintptr_t offset) override;
	PhysicalAddr fetchRange(uintptr_t offset) override;
	
	size_t getLength();

private:
	frigg::Vector<PhysicalAddr, KernelAlloc> _physicalChunks;
	size_t _chunkSize, _chunkAlign;
};

struct ManagedSpace {
	enum LoadState {
		kStateMissing,
		kStateLoading,
		kStateLoaded
	};

	ManagedSpace(size_t length);
	~ManagedSpace();
	
	void progressLoads();
	bool isComplete(frigg::UnsafePtr<InitiateBase> initiate);

	frigg::Vector<PhysicalAddr, KernelAlloc> physicalPages;
	frigg::Vector<LoadState, KernelAlloc> loadState;

	frigg::IntrusiveSharedLinkedList<
		InitiateBase,
		&InitiateBase::processQueueItem
	> initiateLoadQueue;

	frigg::IntrusiveSharedLinkedList<
		InitiateBase,
		&InitiateBase::processQueueItem
	> pendingLoadQueue;

	frigg::IntrusiveSharedLinkedList<
		ManageBase,
		&ManageBase::processQueueItem
	> handleLoadQueue;
};

struct BackingMemory : Memory {
public:
	static bool classOf(const Memory &memory) {
		return memory.tag() == MemoryTag::backing;
	}

	BackingMemory(frigg::SharedPtr<ManagedSpace> managed)
	: Memory(MemoryTag::backing), _managed(frigg::move(managed)) { }

	void acquire(uintptr_t offset, size_t length) override;
	void release(uintptr_t offset, size_t length) override;
	
	PhysicalAddr peekRange(uintptr_t offset) override;
	PhysicalAddr fetchRange(uintptr_t offset) override;

	size_t getLength();

	void submitHandleLoad(frigg::SharedPtr<ManageBase> handle);
	void completeLoad(size_t offset, size_t length);

private:
	frigg::SharedPtr<ManagedSpace> _managed;
};

struct FrontalMemory : Memory {
public:
	static bool classOf(const Memory &memory) {
		return memory.tag() == MemoryTag::frontal;
	}

	FrontalMemory(frigg::SharedPtr<ManagedSpace> managed)
	: Memory(MemoryTag::frontal), _managed(frigg::move(managed)) { }

	void acquire(uintptr_t offset, size_t length) override;
	void release(uintptr_t offset, size_t length) override;
	
	PhysicalAddr peekRange(uintptr_t offset) override;
	PhysicalAddr fetchRange(uintptr_t offset) override;

	size_t getLength();

	void submitInitiateLoad(frigg::SharedPtr<InitiateBase> initiate);

private:
	frigg::SharedPtr<ManagedSpace> _managed;
};

struct Hole {
	Hole(VirtualAddr address, size_t length)
	: _address{address}, _length{length}, largestHole{0} { }

	VirtualAddr address() const {
		return _address;
	}

	size_t length() const {
		return _length;
	}

	frigg::rbtree_hook treeNode;

private:
	VirtualAddr _address;
	size_t _length;

public:
	// Largest hole in the subtree of this node.
	size_t largestHole;
};

enum MappingFlags : uint32_t {
	null = 0,

	forkMask = 0x07,
	dropAtFork = 0x01,
	shareAtFork = 0x02,
	copyOnWriteAtFork = 0x04,

	permissionMask = 0x30,
	readOnly = 0x00,
	readWrite = 0x10,
	readExecute = 0x20,

	dontRequireBacking = 0x100
};

struct CowChain {
	CowChain()
	: mask(*kernelAlloc) { }

	frigg::SharedPtr<Memory> memory;
	VirtualAddr offset;
	frigg::SharedPtr<CowChain> super;
	frigg::Vector<bool, KernelAlloc> mask;
};

struct Mapping {
	Mapping(AddressSpace *owner, VirtualAddr address, size_t length,
			MappingFlags flags);

	AddressSpace *owner() {
		return _owner;
	}

	VirtualAddr address() const {
		return _address;
	}

	size_t length() const {
		return _length;
	}

	MappingFlags flags() const {
		return _flags;
	}

	virtual Mapping *shareMapping(AddressSpace *dest_space) = 0;
	virtual Mapping *copyMapping(AddressSpace *dest_space) = 0;
	virtual Mapping *copyOnWrite(AddressSpace *dest_space) = 0;

	virtual void install(bool overwrite) = 0;
	virtual void uninstall(bool clear) = 0;
	
	virtual PhysicalAddr grabPhysical(VirtualAddr disp) = 0;
	virtual bool handleFault(VirtualAddr disp, uint32_t fault_flags) = 0;

	frigg::rbtree_hook treeNode;

private:
	AddressSpace *_owner;
	VirtualAddr _address;
	size_t _length;
	MappingFlags _flags;
};

struct NormalMapping : Mapping {
	NormalMapping(AddressSpace *owner, VirtualAddr address, size_t length,
			MappingFlags flags, frigg::SharedPtr<Memory> memory, uintptr_t offset);
	~NormalMapping();

	Mapping *shareMapping(AddressSpace *dest_space) override;
	Mapping *copyMapping(AddressSpace *dest_space) override;
	Mapping *copyOnWrite(AddressSpace *dest_space) override;

	void install(bool overwrite) override;
	void uninstall(bool clear) override;

	PhysicalAddr grabPhysical(VirtualAddr disp) override;
	bool handleFault(VirtualAddr disp, uint32_t flags) override;

private:
	frigg::SharedPtr<Memory> _memory;
	size_t _offset;
};

struct CowMapping : Mapping {
	CowMapping(AddressSpace *owner, VirtualAddr address, size_t length,
			MappingFlags flags, frigg::SharedPtr<CowChain> chain);
	~CowMapping();

	Mapping *shareMapping(AddressSpace *dest_space) override;
	Mapping *copyMapping(AddressSpace *dest_space) override;
	Mapping *copyOnWrite(AddressSpace *dest_space) override;

	void install(bool overwrite) override;
	void uninstall(bool clear) override;

	PhysicalAddr grabPhysical(VirtualAddr disp) override;
	bool handleFault(VirtualAddr disp, uint32_t flags) override;

private:
	PhysicalAddr _retrievePage(VirtualAddr disp);

	frigg::SharedPtr<Memory> _copy;
	frigg::Vector<bool, KernelAlloc> _mask;
	frigg::SharedPtr<CowChain> _chain;
};

struct HoleLess {
	bool operator() (const Hole &a, const Hole &b) {
		return a.address() < b.address();
	}
};

struct HoleAggregator;

using HoleTree = frigg::rbtree<
	Hole,
	&Hole::treeNode,
	HoleLess,
	HoleAggregator
>;

struct HoleAggregator {
	static bool aggregate(Hole *node);
	static bool check_invariant(HoleTree &tree, Hole *node);
};

struct MappingLess {
	bool operator() (const Mapping &a, const Mapping &b) {
		return a.address() < b.address();
	}
};

using MappingTree = frigg::rbtree<
	Mapping,
	&Mapping::treeNode,
	MappingLess
>;

struct AddressUnmapNode {
	friend struct AddressSpace;

private:
	AddressSpace *_space;
	ShootNode _shootNode;
};

class AddressSpace {
public:
	typedef frigg::TicketLock Lock;
	typedef frigg::LockGuard<Lock> Guard;

	typedef uint32_t MapFlags;
	enum : MapFlags {
		kMapFixed = 0x01,
		kMapPreferBottom = 0x02,
		kMapPreferTop = 0x04,
		kMapReadOnly = 0x08,
		kMapReadWrite = 0x10,
		kMapReadExecute = 0x20,
		kMapDropAtFork = 0x40,
		kMapShareAtFork = 0x80,
		kMapCopyOnWriteAtFork = 0x100,
		kMapPopulate = 0x200,
		kMapDontRequireBacking = 0x400,
	};

	enum FaultFlags : uint32_t {
		kFaultWrite = 0x01
	};

	AddressSpace();

	~AddressSpace();

	void setupDefaultMappings();

	void map(Guard &guard, frigg::UnsafePtr<Memory> memory,
			VirtualAddr address, size_t offset, size_t length,
			uint32_t flags, VirtualAddr *actual_address);
	
	void unmap(Guard &guard, VirtualAddr address, size_t length,
			AddressUnmapNode *node);

	bool handleFault(VirtualAddr address, uint32_t flags);
	
	frigg::SharedPtr<AddressSpace> fork(Guard &guard);
	
	PhysicalAddr grabPhysical(Guard &guard, VirtualAddr address);

	void activate();

	Lock lock;
	
	Futex futexSpace;
	QueueSpace queueSpace;

private:
	
	// Allocates a new mapping of the given length somewhere in the address space.
	VirtualAddr _allocate(size_t length, MapFlags flags);

	VirtualAddr _allocateAt(VirtualAddr address, size_t length);
	
	Mapping *_getMapping(VirtualAddr address);

	// Splits some memory range from a hole mapping.
	void _splitHole(Hole *hole, VirtualAddr offset, VirtualAddr length);
	
	HoleTree _holes;
	MappingTree _mappings;

public: // TODO: Make this private.
	ClientPageSpace _pageSpace;
};

} // namespace thor

#endif // THOR_GENERIC_USERMEM_HPP
