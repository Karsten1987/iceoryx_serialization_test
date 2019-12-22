#include <iostream>
#include <chrono>

#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

#include "rclcpp/rclcpp.hpp"

#include "test_msgs/message_fixtures.hpp"

#include "rmw_iceoryx_cpp/iceoryx_serialize.hpp"

template<class MSG_T>
void serialize(void * msg)
{
  auto ts = rosidl_typesupport_cpp::get_message_type_support_handle<MSG_T>();
  auto introspection_ts = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
    get_message_typesupport_handle(
      ts, rosidl_typesupport_introspection_cpp::typesupport_identifier)->data);

  std::vector<char> payload_vector{};
  rmw_iceoryx_cpp::serialize(msg, introspection_ts, payload_vector);
  payload_vector.clear();
}

template<class MSG_T, size_t reps = 100, class UNIT = std::chrono::microseconds>
void measure_serialize(void * msg, const std::string & prefix = "", bool verbose = false)
{
  int64_t d_min = 999999999;
  int64_t d_max = 0;
  float d_std = 0.0;

  for (auto i = 0u; i < reps; ++i) {
    auto t1 = std::chrono::steady_clock::now();
    serialize<MSG_T>(msg);
    auto t2 = std::chrono::steady_clock::now();
    auto d_nano = std::chrono::duration_cast<UNIT>(t2 - t1).count();
    if (verbose) {
      std::cout << "rep: " << i << " = " << d_nano << std::endl;
    }

    d_min = std::min(d_nano, d_min);
    d_max = std::max(d_nano, d_max);
    d_std = (i * d_std + d_nano) / (i + 1);
  }
  std::cout << "[" << prefix << "]"
    << " std: " << d_std
    << " min: " << d_min
    << " max: " << d_max
    << std::endl;
}

int main(int argc, char ** argv)
{
  (void) argc;
  (void) argv;
  auto bt_msgs = get_messages_basic_types();
  measure_serialize<test_msgs::msg::BasicTypes, 1>(bt_msgs[0].get(), "basic types");
  measure_serialize<test_msgs::msg::BasicTypes>(bt_msgs[0].get(), "basic types");

  auto mn_msgs = get_messages_multi_nested();
  measure_serialize<test_msgs::msg::BasicTypes>(mn_msgs[0].get(), "multi nested");
  return 0;
}
