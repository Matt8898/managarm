
#include <memory>
#include <iostream>

#include <string.h>

#include <helix/ipc.hpp>
#include "hw.pb.h"
#include "protocols/hw/client.hpp"

namespace protocols {
namespace hw {

async::result<PciInfo> Device::getPciInfo() {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::GET_PCI_INFO);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);
	assert(resp.bars_size() == 6);

	PciInfo info;

	for(int i = 0; i < resp.capabilities_size(); i++)
		info.caps.push_back({resp.capabilities(i).type()});

	for(int i = 0; i < 6; i++) {
		if(resp.bars(i).io_type() == managarm::hw::IoType::NO_BAR) {
			info.barInfo[i].ioType = IoType::kIoTypeNone;
		}else if(resp.bars(i).io_type() == managarm::hw::IoType::PORT) {
			info.barInfo[i].ioType = IoType::kIoTypePort;
		}else if(resp.bars(i).io_type() == managarm::hw::IoType::MEMORY) {
			info.barInfo[i].ioType = IoType::kIoTypeMemory;
		}else{
			throw std::runtime_error("Illegal IoType!\n");
		}
		info.barInfo[i].address = resp.bars(i).address();
		info.barInfo[i].length = resp.bars(i).length();
		info.barInfo[i].offset = resp.bars(i).offset();
	}

	co_return info;
}

async::result<helix::UniqueDescriptor> Device::accessBar(int index) {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;
	helix::PullDescriptor pull_bar;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::ACCESS_BAR);
	req.set_index(index);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp, kHelItemChain),
			helix::action(&pull_bar));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());
	HEL_CHECK(pull_bar.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);

	auto bar = pull_bar.descriptor();
	co_return std::move(bar);
}

async::result<helix::UniqueDescriptor> Device::accessIrq() {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;
	helix::PullDescriptor pull_irq;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::ACCESS_IRQ);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp, kHelItemChain),
			helix::action(&pull_irq));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());
	HEL_CHECK(pull_irq.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);

	co_return pull_irq.descriptor();
}

async::result<void> Device::claimDevice() {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::CLAIM_DEVICE);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);
}

async::result<void> Device::enableBusIrq() {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::BUSIRQ_ENABLE);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);
}

async::result<uint32_t> Device::loadPciSpace(size_t offset, unsigned int size) {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::LOAD_PCI_SPACE);
	req.set_offset(offset);
	req.set_size(size);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);
	co_return resp.word();
}

async::result<void> Device::storePciSpace(size_t offset, unsigned int size, uint32_t word) {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::STORE_PCI_SPACE);
	req.set_offset(offset);
	req.set_size(size);
	req.set_word(word);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);
}

async::result<uint32_t> Device::loadPciCapability(unsigned int index, size_t offset, unsigned int size) {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::LOAD_PCI_CAPABILITY);
	req.set_index(index);
	req.set_offset(offset);
	req.set_size(size);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);
	co_return resp.word();
}

async::result<FbInfo> Device::getFbInfo() {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::GET_FB_INFO);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);

	FbInfo info;

	info.pitch = resp.fb_pitch();
	info.width = resp.fb_width();
	info.height = resp.fb_height();
	info.bpp = resp.fb_bpp();
	info.type = resp.fb_type();

	co_return info;
}

async::result<helix::UniqueDescriptor> Device::accessFbMemory() {
	helix::Offer offer;
	helix::SendBuffer send_req;
	helix::RecvInline recv_resp;
	helix::PullDescriptor pull_bar;

	managarm::hw::CntRequest req;
	req.set_req_type(managarm::hw::CntReqType::ACCESS_FB_MEMORY);

	auto ser = req.SerializeAsString();
	auto &&transmit = helix::submitAsync(_lane, helix::Dispatcher::global(),
			helix::action(&offer, kHelItemAncillary),
			helix::action(&send_req, ser.data(), ser.size(), kHelItemChain),
			helix::action(&recv_resp, kHelItemChain),
			helix::action(&pull_bar));
	co_await transmit.async_wait();
	HEL_CHECK(offer.error());
	HEL_CHECK(send_req.error());
	HEL_CHECK(recv_resp.error());
	HEL_CHECK(pull_bar.error());

	managarm::hw::SvrResponse resp;
	resp.ParseFromArray(recv_resp.data(), recv_resp.length());
	assert(resp.error() == managarm::hw::Errors::SUCCESS);

	auto bar = pull_bar.descriptor();
	co_return std::move(bar);
}

} } // namespace protocols::hw

