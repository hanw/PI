/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Antonin Bas (antonin@barefootnetworks.com)
 *
 */

#ifndef PI_FRONTENDS_PROTO_DEVICE_MGR_H_
#define PI_FRONTENDS_PROTO_DEVICE_MGR_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "google/rpc/status.pb.h"
#include "p4/config/p4info.pb.h"
#include "p4/p4runtime.pb.h"
#include "p4/tmp/device.pb.h"

namespace pi {

namespace fe {

namespace proto {

// forward declaration for PIMPL class
class DeviceMgrImp;

// the gRPC server will instantiate one DeviceMgr object per device
class DeviceMgr {
 public:
  using device_id_t = uint64_t;
  using p4_id_t = uint32_t;
  // may change when we introduce specific error namespace
  using Status = ::google::rpc::Status;
  using PacketInCb =
      std::function<void(device_id_t, std::string packet, void *cookie)>;

  explicit DeviceMgr(device_id_t device_id);

  ~DeviceMgr();

  // 3 temporary methods to manage a device, will be replaced by permanent
  // solution ASAP
  Status init(const p4::config::P4Info &p4info,
              const p4::tmp::DeviceAssignRequest_Extras &extras);

  Status update_start(const p4::config::P4Info &p4info,
                      const std::string &device_data);

  Status update_end();

  // should we use ::google::rpc::Status or should we just return an error code
  // that the gRPC server can then wrap in a Status message
  Status table_write(const p4::TableUpdate &table_update);

  Status table_read(p4_id_t table_id,
                    std::vector<p4::TableEntry> *entries) const;

  // this is likely to be temporary, we may not want to hold all the entries in
  // memory at once; could be replaced by an iterator of some sort
  Status table_read_all(std::vector<p4::TableEntry> *entries) const;

  Status action_profile_write(
      const p4::ActionProfileUpdate &action_profile_update);

  Status action_profile_read(
      p4_id_t action_profile_id,
      std::vector<p4::ActionProfileEntry> *entries) const;

  // this is likely to be temporary, we may not want to hold all the entries in
  // memory at once; could be replaced by an iterator of some sort
  Status action_profile_read_all(
      std::vector<p4::ActionProfileEntry> *entries) const;

  // from the perspective of P4, a punted packet is just bytes. Either the
  // controller is responsible for encapsulating the packet in the appropriate
  // header, or the gRPC server is.
  Status packet_out_send(const std::string &packet) const;

  void packet_in_register_cb(PacketInCb cb, void *cookie);

  Status counter_write(const p4::CounterEntry &entry);

  Status counter_read(p4::CounterEntry *entry) const;

  class counter_iterator;
  counter_iterator counter_read_begin() const;
  counter_iterator counter_read_end() const;

  static void init(size_t max_devices);

  static void destroy();

 private:
  // PIMPL design
  std::unique_ptr<DeviceMgrImp> pimp;
};

class DeviceMgr::counter_iterator
    : public std::iterator<std::forward_iterator_tag, p4::CounterEntry> {
 public:
  enum class InitState { BEGIN, END };

  counter_iterator(const DeviceMgrImp *device_mgr, InitState init);

  p4::CounterEntry &operator*() const;

  p4::CounterEntry *operator->() const;

  bool operator==(const counter_iterator &other) const;

  bool operator!=(const counter_iterator &other) const;

  counter_iterator &operator++();

  const counter_iterator operator++(int);

 private:
  void counter_read();

  const DeviceMgrImp *device_mgr;
  p4_id_t counter_id;
  std::shared_ptr<p4::CounterEntry> entry;
};

}  // namespace proto

}  // namespace fe

}  // namespace pi

#endif  // PI_FRONTENDS_PROTO_DEVICE_MGR_H_
