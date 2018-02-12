#ifndef POSIX_SUBSYSTEM_SYSFS_HPP
#define POSIX_SUBSYSTEM_SYSFS_HPP

#include <protocols/fs/server.hpp>

#include "vfs.hpp"

namespace sysfs {

struct LinkCompare;
struct Link;
struct DirectoryNode;

struct Attribute;
struct Object;
struct Hierarchy;

// ----------------------------------------------------------------------------
// FS data structures.
// This API is only intended for private use.
// ----------------------------------------------------------------------------

struct LinkCompare {
	struct is_transparent { };

	bool operator() (const std::shared_ptr<Link> &a, const std::shared_ptr<Link> &b) const;
	bool operator() (const std::shared_ptr<Link> &link, const std::string &name) const;
	bool operator() (const std::string &name, const std::shared_ptr<Link> &link) const;
};

struct AttributeFile : ProperFile {
private:
	static async::result<size_t> ptRead(std::shared_ptr<void> object,
			void *buffer, size_t length);

	static constexpr auto fileOperations = protocols::fs::FileOperations{}
			.withRead(&ptRead);

public:
	static void serve(std::shared_ptr<AttributeFile> file);

	explicit AttributeFile(std::shared_ptr<FsLink> link);

	FutureMaybe<size_t> readSome(void *data, size_t max_length) override;
	helix::BorrowedDescriptor getPassthroughLane() override;

private:
	helix::UniqueLane _passthrough;

	bool _cached;
	std::string _buffer;
	size_t _offset;
};

struct DirectoryFile : ProperFile {
private:
	static async::result<protocols::fs::ReadEntriesResult>
	ptReadEntries(std::shared_ptr<void> object);

	static constexpr auto fileOperations = protocols::fs::FileOperations{}
		.withReadEntries(&ptReadEntries);

public:
	static void serve(std::shared_ptr<DirectoryFile> file);

	explicit DirectoryFile(std::shared_ptr<FsLink> link);

	FutureMaybe<size_t> readSome(void *data, size_t max_length) override;
	helix::BorrowedDescriptor getPassthroughLane() override;

private:
	// TODO: Remove this and extract it from the associatedLink().
	DirectoryNode *_node;

	helix::UniqueLane _passthrough;
	std::set<std::shared_ptr<Link>, LinkCompare>::iterator _iter;
};

struct Link : FsLink {
	explicit Link(std::shared_ptr<FsNode> owner,
			std::string name, std::shared_ptr<FsNode> target);
	
	std::shared_ptr<FsNode> getOwner() override;
	std::string getName() override;
	std::shared_ptr<FsNode> getTarget() override;

private:
	std::shared_ptr<FsNode> owner;
	std::string name;
	std::shared_ptr<FsNode> target;
};

struct AttributeNode : FsNode, std::enable_shared_from_this<AttributeNode> {
	friend struct AttributeFile;

	AttributeNode(Object *object, Attribute *attr);

	VfsType getType() override;
	FutureMaybe<FileStats> getStats() override;
	FutureMaybe<std::shared_ptr<ProperFile>> open(std::shared_ptr<FsLink> link) override;

private:
	Object *_object;
	Attribute *_attr;
};

struct SymlinkNode : FsNode, std::enable_shared_from_this<SymlinkNode> {
	SymlinkNode() = default;

	VfsType getType() override;
	FutureMaybe<FileStats> getStats() override;
	expected<std::string> readSymlink() override;

private:
};

struct DirectoryNode : FsNode, std::enable_shared_from_this<DirectoryNode> {
	friend struct DirectoryFile;

	DirectoryNode() = default;

	std::shared_ptr<Link> directMkattr(Object *object, Attribute *attr);
	std::shared_ptr<Link> directMklink(std::string name);
	std::shared_ptr<Link> directMkdir(std::string name);

	VfsType getType() override;
	FutureMaybe<FileStats> getStats() override;
	FutureMaybe<std::shared_ptr<ProperFile>> open(std::shared_ptr<FsLink> link) override;
	FutureMaybe<std::shared_ptr<FsLink>> getLink(std::string name) override;

private:
	std::set<std::shared_ptr<Link>, LinkCompare> _entries;
};

// ----------------------------------------------------------------------------
// Object abstraction.
// Subsystems should use this API to manage sysfs.
// ----------------------------------------------------------------------------

struct Attribute {
	Attribute(std::string name);

	const std::string &name() {
		return _name;
	}

	virtual std::string show(Object *object) = 0;

private:
	const std::string _name;
};

// Object corresponds to Linux kobjects.
struct Object {
	Object(std::shared_ptr<Object> parent, std::string name);

	void createAttribute(Attribute *attr);
	void createSymlink(std::string name, std::shared_ptr<Object> target);

	void addObject();

private:
	std::shared_ptr<Object> _parent;
	std::string _name;

	std::shared_ptr<Link> _dirLink;
};

// Hierarchy corresponds to Linux ksets.
struct Hierarchy {

};

} // namespace sysfs

std::shared_ptr<FsLink> getSysfs();

#endif // POSIX_SUBSYSTEM_SYSFS_HPP
