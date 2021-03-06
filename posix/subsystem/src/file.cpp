
#include <string.h>
#include <future>

#include <helix/ipc.hpp>
#include "file.hpp"
#include "process.hpp"
#include "fs.pb.h"

namespace {

constexpr bool logDestruction = false;

} // anonymous namespace

// --------------------------------------------------------
// File implementation.
// --------------------------------------------------------

async::result<protocols::fs::SeekResult>
File::ptSeekAbs(void *object, int64_t offset) {
	auto self = static_cast<File *>(object);
	auto result = co_await self->seek(offset, VfsSeek::absolute);
	auto error = std::get_if<Error>(&result);
	if(error && *error == Error::seekOnPipe) {
		co_return protocols::fs::Error::seekOnPipe;
	}else{
		assert(!error);
		co_return std::get<off_t>(result);
	}
}

async::result<protocols::fs::SeekResult>
File::ptSeekRel(void *object, int64_t offset) {
	auto self = static_cast<File *>(object);
	auto result = co_await self->seek(offset, VfsSeek::relative);
	auto error = std::get_if<Error>(&result);
	if(error && *error == Error::seekOnPipe) {
		co_return protocols::fs::Error::seekOnPipe;
	}else{
		assert(!error);
		co_return std::get<off_t>(result);
	}
}

async::result<protocols::fs::SeekResult>
File::ptSeekEof(void *object, int64_t offset) {
	auto self = static_cast<File *>(object);
	auto result = co_await self->seek(offset, VfsSeek::eof);
	auto error = std::get_if<Error>(&result);
	if(error && *error == Error::seekOnPipe) {
		co_return protocols::fs::Error::seekOnPipe;
	}else{
		assert(!error);
		co_return std::get<off_t>(result);
	}
}

async::result<protocols::fs::ReadResult>
File::ptRead(void *object, const char *credentials,
		void *buffer, size_t length) {
	auto self = static_cast<File *>(object);
	auto process = findProcessWithCredentials(credentials);
	auto result = co_await self->readSome(process.get(), buffer, length);
	auto error = std::get_if<Error>(&result);
	if(error && *error == Error::illegalOperationTarget) {
		co_return protocols::fs::Error::illegalArguments;
	}else if(error && *error == Error::wouldBlock) {
		co_return protocols::fs::Error::wouldBlock;
	}else{
		assert(!error);
		co_return std::get<size_t>(result);
	}
}

async::result<void> File::ptWrite(void *object, const char *credentials,
		const void *buffer, size_t length) {
	auto self = static_cast<File *>(object);
	auto process = findProcessWithCredentials(credentials);
	return self->writeAll(process.get(), buffer, length);
}

async::result<ReadEntriesResult> File::ptReadEntries(void *object) {
	auto self = static_cast<File *>(object);
	return self->readEntries();
}

async::result<void> File::ptTruncate(void *object, size_t size) {
	auto self = static_cast<File *>(object);
	return self->truncate(size);
}

async::result<void> File::ptAllocate(void *object,
		int64_t offset, size_t size) {
	auto self = static_cast<File *>(object);
	return self->allocate(offset, size);
}

async::result<int> File::ptGetOption(void *object, int option) {
	auto self = static_cast<File *>(object);
	return self->getOption(option);
}

async::result<void> File::ptSetOption(void *object, int option, int value) {
	auto self = static_cast<File *>(object);
	return self->setOption(option, value);
}

async::result<void> File::ptBind(void *object, const char *credentials,
		const void *addr_ptr, size_t addr_length) {
	auto self = static_cast<File *>(object);
	auto process = findProcessWithCredentials(credentials);
	return self->bind(process.get(), addr_ptr, addr_length);
}

async::result<void> File::ptConnect(void *object, const char *credentials,
		const void *addr_ptr, size_t addr_length) {
	auto self = static_cast<File *>(object);
	auto process = findProcessWithCredentials(credentials);
	return self->connect(process.get(), addr_ptr, addr_length);
}

async::result<size_t> File::ptSockname(void *object, void *addr_ptr, size_t max_addr_length) {
	auto self = static_cast<File *>(object);
	return self->sockname(addr_ptr, max_addr_length);
}

async::result<void> File::ptIoctl(void *object, managarm::fs::CntRequest req,
		helix::UniqueLane conversation) {
	auto self = static_cast<File *>(object);
	return self->ioctl(nullptr, std::move(req), std::move(conversation));
}

async::result<int> File::ptGetFileFlags(void *object) {
	auto self = static_cast<File *>(object);
	return self->getFileFlags();
}

async::result<void> File::ptSetFileFlags(void *object, int flags) {
	auto self = static_cast<File *>(object);
	return self->setFileFlags(flags);
}

File::~File() {
	// Nothing to do here.
	if(logDestruction)
		std::cout << "\e[37mposix \e[1;34m" << structName()
				<< "\e[0m\e[37m: File was destructed\e[39m" << std::endl;
}

bool File::isTerminal() {
	return _defaultOps & defaultIsTerminal;
}

FutureMaybe<void> File::readExactly(Process *process,
		void *data, size_t length) {
	size_t offset = 0;
	while(offset < length) {
		auto result = co_await readSome(process, (char *)data + offset, length - offset);
		assert(std::get<size_t>(result) > 0);
		offset += std::get<size_t>(result);
	}
}

expected<size_t> File::readSome(Process *, void *, size_t) {
	std::cout << "\e[35mposix \e[1;34m" << structName()
			<< "\e[0m\e[35m: File does not support read()\e[39m" << std::endl;
	co_return Error::illegalOperationTarget;
}

void File::handleClose() {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement handleClose()" << std::endl;
}

FutureMaybe<void> File::writeAll(Process *, const void *, size_t) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement writeAll()" << std::endl;
	throw std::runtime_error("posix: Object has no File::writeAll()");
}

async::result<ReadEntriesResult> File::readEntries() {
	throw std::runtime_error("posix: Object has no File::readEntries()");
}

expected<RecvResult> File::recvMsg(Process *, MsgFlags, void *, size_t,
		void *, size_t, size_t) {
	throw std::runtime_error("posix: Object has no File::recvMsg()");
}

expected<size_t> File::sendMsg(Process *, MsgFlags, const void *, size_t,
		const void *, size_t,
		std::vector<smarter::shared_ptr<File, FileHandle>>) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement sendMsg()" << std::endl;
	throw std::runtime_error("posix: Object has no File::sendMsg()");
}

async::result<void> File::truncate(size_t) {
	throw std::runtime_error("posix: Object has no File::truncate()");
}

async::result<void> File::allocate(int64_t, size_t) {
	throw std::runtime_error("posix: Object has no File::allocate()");
}

expected<off_t> File::seek(off_t, VfsSeek) {
	if(_defaultOps & defaultPipeLikeSeek) {
		async::promise<std::variant<Error, off_t>> promise;
		promise.set_value(Error::seekOnPipe);
		return promise.async_get();
	}else{
		std::cout << "posix \e[1;34m" << structName()
				<< "\e[0m: Object does not implement seek()" << std::endl;
		throw std::runtime_error("posix: Object has no File::seek()");
	}
}

expected<PollResult> File::poll(Process *, uint64_t, async::cancellation_token) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement poll()" << std::endl;
	throw std::runtime_error("posix: Object has no File::poll()");
}

expected<PollResult> File::checkStatus(Process *process) {
	return poll(process, 0, async::cancellation_token{});
}

async::result<int> File::getOption(int) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement getOption()" << std::endl;
	throw std::runtime_error("posix: Object has no File::getOption()");
}

async::result<void> File::setOption(int, int) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement setOption()" << std::endl;
	throw std::runtime_error("posix: Object has no File::setOption()");
}

async::result<AcceptResult> File::accept(Process *) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement accept()" << std::endl;
	throw std::runtime_error("posix: Object has no File::accept()");
}

async::result<void> File::bind(Process *, const void *, size_t) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement bind()" << std::endl;
	throw std::runtime_error("posix: Object has no File::bind()");
}

async::result<void> File::connect(Process *, const void *, size_t) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement connect()" << std::endl;
	throw std::runtime_error("posix: Object has no File::connect()");
}

async::result<size_t> File::sockname(void *addr_ptr, size_t max_addr_length) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement sockname()" << std::endl;
	throw std::runtime_error("posix: Object has no File::sockname()");
}

FutureMaybe<helix::UniqueDescriptor> File::accessMemory(off_t) {
	// TODO: Return an error.
	throw std::runtime_error("posix: Object has no File::accessMemory()");
}

async::result<void> File::ioctl(Process *, managarm::fs::CntRequest,
		helix::UniqueLane) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement ioctl()" << std::endl;
	throw std::runtime_error("posix: Object has no File::ioctl()");
}

async::result<int> File::getFileFlags() {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement getFileFlags()" << std::endl;
	co_return 0;
}

async::result<void> File::setFileFlags(int flags) {
	std::cout << "posix \e[1;34m" << structName()
			<< "\e[0m: Object does not implement setFileFlags()" << std::endl;
	co_return;
}

