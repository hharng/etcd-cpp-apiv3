#include <grpc/support/log.h>
#include "etcd/v3/action_constants.hpp"
#include "etcd/v3/Action.hpp"

etcdv3::Action::Action(etcdv3::ActionParameters const &params)
{
  parameters = params;
  this->InitAction();
}

etcdv3::Action::Action(etcdv3::ActionParameters && params)
{
  parameters = std::move(params);
  this->InitAction();
}

void etcdv3::Action::InitAction() {
  if (!parameters.auth_token.empty()) {
    // use `token` as the key, see:
    //
    //  etcd/etcdserver/api/v3rpc/rpctypes/metadatafields.go
    context.AddMetadata("token", parameters.auth_token);
  }
  start_timepoint = std::chrono::high_resolution_clock::now();
}

etcdv3::ActionParameters::ActionParameters()
{
  withPrefix = false;
  revision = 0;
  old_revision = 0;
  lease_id = 0;
  ttl = 0;
  kv_stub = NULL;
  watch_stub = NULL;
  lease_stub = NULL;
}

void etcdv3::ActionParameters::dump(std::ostream &os) const {
  os << "ActionParameters:" << std::endl;
  os << "  withPrefix:    " << withPrefix << std::endl;
  os << "  revision:      " << revision << std::endl;
  os << "  old_revision:  " << old_revision << std::endl;
  os << "  lease_id:      " << lease_id << std::endl;
  os << "  ttl:           " << ttl << std::endl;
  os << "  limit:         " << limit << std::endl;
  os << "  name:          " << name << std::endl;
  os << "  key:           " << key << std::endl;
  os << "  range_end:     " << range_end << std::endl;
  os << "  value:         " << value << std::endl;
  os << "  old_value:     " << old_value << std::endl;
  os << "  auth_token:    " << auth_token << std::endl;
}

void etcdv3::Action::waitForResponse() 
{
  void* got_tag;
  bool ok = false;

  cq_.Next(&got_tag, &ok);
  GPR_ASSERT(got_tag == (void*)this);
}

const std::chrono::high_resolution_clock::time_point etcdv3::Action::startTimepoint() {
  return this->start_timepoint;
}

std::string etcdv3::detail::string_plus_one(std::string const &value) {
  // Referred from the Go implementation in etcd.
  for (int32_t i = value.size() - 1; i >= 0; --i) {
    if (static_cast<unsigned char>(value[i]) < 0xff) {
      std::string s = value.substr(0, i + 1);
      s[i] = s[i] + 1;
      return s;
    }
  }

  return {etcdv3::NUL};
}
