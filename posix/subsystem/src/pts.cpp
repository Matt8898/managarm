
#include <asm/ioctls.h>
#include <sys/epoll.h>

#include <async/doorbell.hpp>

#include "file.hpp"
#include "pts.hpp"
#include "fs.pb.h"

namespace pts {

namespace {

struct MasterFile;
struct RootLink;
struct DeviceNode;
struct RootNode;

bool logReadWrite = false;

int nextPtsIndex = 0;

extern std::shared_ptr<RootLink> globalRootLink;

//-----------------------------------------------------------------------------

struct Packet {
	// The actual octet data that the packet consists of.
	std::vector<char> buffer;

	size_t offset = 0;
};

struct Channel {
	Channel(int pts_index)
	: ptsIndex{pts_index}, currentSeq{1}, masterInSeq{0}, slaveInSeq{0} { }

	int ptsIndex;

	// Status management for poll().
	async::doorbell statusBell;
	uint64_t currentSeq;
	uint64_t masterInSeq;
	uint64_t slaveInSeq;

	// The actual queue of this pipe.
	std::deque<Packet> masterQueue;
	std::deque<Packet> slaveQueue;
};

//-----------------------------------------------------------------------------
// Device and file structs.
//-----------------------------------------------------------------------------

struct MasterDevice : UnixDevice {
	MasterDevice()
	: UnixDevice(VfsType::charDevice) {
		assignId({5, 2});
	}
	
	std::string nodePath() override {
		return "ptmx";
	}
	
	async::result<SharedFilePtr>
	open(std::shared_ptr<FsLink> link, SemanticFlags semantic_flags) override;
};

struct SlaveDevice : UnixDevice {
	SlaveDevice(std::shared_ptr<Channel> channel);
	
	std::string nodePath() override {
		return std::string{};
	}
	
	async::result<SharedFilePtr>
	open(std::shared_ptr<FsLink> link, SemanticFlags semantic_flags) override;

private:
	std::shared_ptr<Channel> _channel;
};

struct MasterFile : File {
public:
	static void serve(smarter::shared_ptr<MasterFile> file) {
		assert(!file->_passthrough);

		helix::UniqueLane lane;
		std::tie(lane, file->_passthrough) = helix::createStream();
		protocols::fs::servePassthrough(std::move(lane),
				smarter::shared_ptr<File>{file}, &File::fileOperations);
	}

	MasterFile(std::shared_ptr<FsLink> link);
	
	expected<size_t>
	readSome(Process *, void *data, size_t max_length) override;

	FutureMaybe<void>
	writeAll(Process *, const void *data, size_t length) override;

	expected<PollResult>
	poll(uint64_t sequence) override;

	async::result<void>
	ioctl(Process *process, managarm::fs::CntRequest req, helix::UniqueLane conversation);

	helix::BorrowedDescriptor getPassthroughLane() override {
		return _passthrough;
	}

private:
	helix::UniqueLane _passthrough;

	std::shared_ptr<Channel> _channel;
};

struct SlaveFile : File {
public:
	static void serve(smarter::shared_ptr<SlaveFile> file) {
		assert(!file->_passthrough);

		helix::UniqueLane lane;
		std::tie(lane, file->_passthrough) = helix::createStream();
		protocols::fs::servePassthrough(std::move(lane),
				smarter::shared_ptr<File>{file}, &File::fileOperations);
	}

	SlaveFile(std::shared_ptr<FsLink> link, std::shared_ptr<Channel> channel);
	
	expected<size_t>
	readSome(Process *, void *data, size_t max_length) override;

	FutureMaybe<void>
	writeAll(Process *, const void *data, size_t length) override;

	expected<PollResult>
	poll(uint64_t sequence) override;

	helix::BorrowedDescriptor getPassthroughLane() override {
		return _passthrough;
	}

private:
	helix::UniqueLane _passthrough;

	std::shared_ptr<Channel> _channel;
};

//-----------------------------------------------------------------------------
// File system structs.
//-----------------------------------------------------------------------------

struct Link : FsLink {
public:
	explicit Link(RootNode *root, std::string name, std::shared_ptr<DeviceNode> device)
	: _root{root}, _name{std::move(name)}, _device{std::move(device)} { }
	
	std::shared_ptr<FsNode> getOwner() override;

	std::string getName() override;

	std::shared_ptr<FsNode> getTarget() override;

private:
	RootNode *_root;
	std::string _name;
	std::shared_ptr<DeviceNode> _device;
};

struct RootLink : FsLink {
	RootLink();
	
	RootNode *rootNode() {
		return _root.get();
	}

	std::shared_ptr<FsNode> getOwner() override {
		throw std::logic_error("posix: pts RootLink has no owner");
	}

	std::string getName() override {
		throw std::logic_error("posix: pts RootLink has no name");
	}

	std::shared_ptr<FsNode> getTarget() override;

private:
	std::shared_ptr<RootNode> _root;
};

struct LinkCompare {
	struct is_transparent { };

	bool operator() (const std::shared_ptr<Link> &link, const std::string &name) const {
		return link->getName() < name;
	}
	bool operator() (const std::string &name, const std::shared_ptr<Link> &link) const {
		return name < link->getName();
	}

	bool operator() (const std::shared_ptr<Link> &a, const std::shared_ptr<Link> &b) const {
		return a->getName() < b->getName();
	}
};

struct DeviceNode : FsNode {
public:
	DeviceNode(DeviceId id)
	: _type{VfsType::charDevice}, _id{id} { }

	VfsType getType() override {
		return _type;
	}
	
	COFIBER_ROUTINE(FutureMaybe<FileStats>, getStats() override, ([=] {
		std::cout << "\e[31mposix: Fix pts DeviceNode::getStats()\e[39m" << std::endl;
		COFIBER_RETURN(FileStats{});
	}))

	DeviceId readDevice() override {
		return _id;
	}
	
	FutureMaybe<SharedFilePtr> open(std::shared_ptr<FsLink> link,
			SemanticFlags semantic_flags) override {
		return openDevice(_type, _id, std::move(link), semantic_flags);
	}

private:
	VfsType _type;
	DeviceId _id;
};

struct RootNode : FsNode, std::enable_shared_from_this<RootNode> {
	friend struct Superblock;
	friend struct DirectoryFile;

public:
	VfsType getType() override {
		return VfsType::directory;
	}

	void linkDevice(std::string name, std::shared_ptr<DeviceNode> node) {
		auto link = std::make_shared<Link>(this, name, std::move(node));
		_entries.insert(std::move(link));
	}

	COFIBER_ROUTINE(FutureMaybe<FileStats>, getStats() override, ([=] {
		std::cout << "\e[31mposix: Fix pts RootNode::getStats()\e[39m" << std::endl;
		COFIBER_RETURN(FileStats{});
	}))

/*
	COFIBER_ROUTINE(FutureMaybe<SharedFilePtr>,
	open(std::shared_ptr<FsLink> link, SemanticFlags semantic_flags), ([=] {
		assert(!semantic_flags);

		auto file = smarter::make_shared<DirectoryFile>(std::move(link));
		DirectoryFile::serve(file);
		COFIBER_RETURN(File::constructHandle(std::move(file)));
	}))
*/

	COFIBER_ROUTINE(FutureMaybe<std::shared_ptr<FsLink>>,
			getLink(std::string name) override, ([=] {
		auto it = _entries.find(name);
		if(it != _entries.end())
			COFIBER_RETURN(*it);
		COFIBER_RETURN(nullptr); // TODO: Return an error code.
	}))

private:
	std::set<std::shared_ptr<Link>, LinkCompare> _entries;
};

//-----------------------------------------------------------------------------
// MasterDevice implementation.
//-----------------------------------------------------------------------------

COFIBER_ROUTINE(FutureMaybe<SharedFilePtr>,
MasterDevice::open(std::shared_ptr<FsLink> link, SemanticFlags semantic_flags), ([=] {
	assert(!semantic_flags);
	auto file = smarter::make_shared<MasterFile>(std::move(link));
	MasterFile::serve(file);
	COFIBER_RETURN(File::constructHandle(std::move(file)));
}))

MasterFile::MasterFile(std::shared_ptr<FsLink> link)
: File{StructName::get("pts.master"), std::move(link), File::defaultPipeLikeSeek},
		_channel{std::make_shared<Channel>(nextPtsIndex++)} {
	auto slave_device = std::make_shared<SlaveDevice>(_channel);
	charRegistry.install(std::move(slave_device));

	globalRootLink->rootNode()->linkDevice(std::to_string(_channel->ptsIndex),
			std::make_shared<DeviceNode>(DeviceId{136, _channel->ptsIndex}));
}

COFIBER_ROUTINE(expected<size_t>,
MasterFile::readSome(Process *, void *data, size_t max_length), ([=] {
	if(logReadWrite)
		std::cout << "posix: Read from tty " << structName() << std::endl;

	while(_channel->masterQueue.empty())
		COFIBER_AWAIT _channel->statusBell.async_wait();
	
	auto packet = &_channel->masterQueue.front();
	assert(!packet->offset);
	auto size = packet->buffer.size();
	assert(max_length >= size);
	memcpy(data, packet->buffer.data(), size);
	_channel->masterQueue.pop_front();
	COFIBER_RETURN(size);
}))

COFIBER_ROUTINE(FutureMaybe<void>,
MasterFile::writeAll(Process *, const void *data, size_t length), ([=] {
	if(logReadWrite)
		std::cout << "posix: Write to tty " << structName() << std::endl;

	Packet packet;
	packet.buffer.resize(length);
	memcpy(packet.buffer.data(), data, length);
	packet.offset = 0;

	_channel->slaveQueue.push_back(std::move(packet));
	_channel->slaveInSeq = ++_channel->currentSeq;
	_channel->statusBell.ring();

	COFIBER_RETURN();
}))

COFIBER_ROUTINE(expected<PollResult>, MasterFile::poll(uint64_t past_seq), ([=] {
	assert(past_seq <= _channel->currentSeq);
	while(past_seq == _channel->currentSeq)
		COFIBER_AWAIT _channel->statusBell.async_wait();

	// For now making pts files always writable is sufficient.
	int edges = EPOLLOUT;
	if(_channel->masterInSeq > past_seq)
		edges |= EPOLLIN;

	int events = EPOLLOUT;
	if(!_channel->masterQueue.empty())
		events |= EPOLLIN;

	COFIBER_RETURN(PollResult(_channel->currentSeq, edges, events));
}))

COFIBER_ROUTINE(async::result<void>, MasterFile::ioctl(Process *, managarm::fs::CntRequest req,
		helix::UniqueLane conversation), ([this, req = std::move(req),
			conversation = std::move(conversation)] {
	if(req.command() == TIOCGPTN) {
		helix::SendBuffer send_resp;
		managarm::fs::SvrResponse resp;
	
		resp.set_error(managarm::fs::Errors::SUCCESS);
		resp.set_pts_index(_channel->ptsIndex);
		
		auto ser = resp.SerializeAsString();
		auto &&transmit = helix::submitAsync(conversation, helix::Dispatcher::global(),
			helix::action(&send_resp, ser.data(), ser.size()));
		COFIBER_AWAIT transmit.async_wait();
		HEL_CHECK(send_resp.error());
	}else{
		throw std::runtime_error("posix: Unknown ioctl() with ID " + std::to_string(req.command()));
	}
	COFIBER_RETURN();
}))

//-----------------------------------------------------------------------------
// SlaveDevice implementation.
//-----------------------------------------------------------------------------

SlaveDevice::SlaveDevice(std::shared_ptr<Channel> channel)
: UnixDevice(VfsType::charDevice), _channel{std::move(channel)} {
	assignId({136, _channel->ptsIndex});
}

COFIBER_ROUTINE(FutureMaybe<SharedFilePtr>,
SlaveDevice::open(std::shared_ptr<FsLink> link, SemanticFlags semantic_flags), ([=] {
	assert(!semantic_flags);
	auto file = smarter::make_shared<SlaveFile>(std::move(link), _channel);
	SlaveFile::serve(file);
	COFIBER_RETURN(File::constructHandle(std::move(file)));
}))

SlaveFile::SlaveFile(std::shared_ptr<FsLink> link, std::shared_ptr<Channel> channel)
: File{StructName::get("pts.slave"), std::move(link), File::defaultPipeLikeSeek},
		_channel{std::move(channel)} { }

COFIBER_ROUTINE(expected<size_t>,
SlaveFile::readSome(Process *, void *data, size_t max_length), ([=] {
	if(logReadWrite)
		std::cout << "posix: Read from tty " << structName() << std::endl;

	while(_channel->slaveQueue.empty())
		COFIBER_AWAIT _channel->statusBell.async_wait();
	
	auto packet = &_channel->slaveQueue.front();
	assert(!packet->offset);
	auto size = packet->buffer.size();
	assert(max_length >= size);
	memcpy(data, packet->buffer.data(), size);
	_channel->slaveQueue.pop_front();
	COFIBER_RETURN(size);
}))


COFIBER_ROUTINE(FutureMaybe<void>,
SlaveFile::writeAll(Process *, const void *data, size_t length), ([=] {
	if(logReadWrite)
		std::cout << "posix: Write to tty " << structName() << std::endl;

	Packet packet;
	packet.buffer.resize(length);
	memcpy(packet.buffer.data(), data, length);
	packet.offset = 0;

	_channel->masterQueue.push_back(std::move(packet));
	_channel->masterInSeq = ++_channel->currentSeq;
	_channel->statusBell.ring();

	COFIBER_RETURN();
}))

COFIBER_ROUTINE(expected<PollResult>, SlaveFile::poll(uint64_t past_seq), ([=] {
	assert(past_seq <= _channel->currentSeq);
	while(past_seq == _channel->currentSeq)
		COFIBER_AWAIT _channel->statusBell.async_wait();

	// For now making pts files always writable is sufficient.
	int edges = EPOLLOUT;
	if(_channel->slaveInSeq > past_seq)
		edges |= EPOLLIN;

	int events = EPOLLOUT;
	if(!_channel->slaveQueue.empty())
		events |= EPOLLIN;

	COFIBER_RETURN(PollResult(_channel->currentSeq, edges, events));
}))

//-----------------------------------------------------------------------------
// Link and RootLink implementation.
//-----------------------------------------------------------------------------

std::shared_ptr<FsNode> Link::getOwner() {
	return _root->shared_from_this();
}

std::string Link::getName() {
	return _name;
}

std::shared_ptr<FsNode> Link::getTarget() {
	return _device;
}

RootLink::RootLink()
: _root(std::make_shared<RootNode>()) { }

std::shared_ptr<FsNode> RootLink::getTarget() {
	return _root->shared_from_this();
}

std::shared_ptr<RootLink> globalRootLink = std::make_shared<RootLink>();

} // anonymous namespace

std::shared_ptr<UnixDevice> createMasterDevice() {
	return std::make_shared<MasterDevice>();
}

std::shared_ptr<FsLink> getFsRoot() {
	return globalRootLink;
}

} // namespace pts
