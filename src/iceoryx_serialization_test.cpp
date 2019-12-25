#include <iostream>
#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

#include "rmw_iceoryx_cpp/iceoryx_serialize.hpp"
#include "rmw_iceoryx_cpp/iceoryx_type_info_introspection.hpp"

#include "test_msgs/message_fixtures.hpp"

#define measure(fnc, prefix, reps) \
  { \
    using UNIT = std::chrono::microseconds; \
    bool verbose = false; \
    int64_t d_min = 999999999; \
    int64_t d_max = 0; \
    float d_std = 0.0; \
  \
    for (auto i = 0u; i < reps; ++i) { \
      auto t1 = std::chrono::steady_clock::now(); \
      fnc; \
      auto t2 = std::chrono::steady_clock::now(); \
      auto d_nano = std::chrono::duration_cast<UNIT>(t2 - t1).count(); \
      if (verbose) { \
        std::cout << "rep: " << i << " = " << d_nano << std::endl; \
      } \
  \
      d_min = std::min(d_nano, d_min); \
      d_max = std::max(d_nano, d_max); \
      d_std = (i * d_std + d_nano) / (i + 1); \
    } \
    std::cout << "[" << prefix << "]" \
      << " std: " << d_std \
      << " min: " << d_min \
      << " max: " << d_max \
      << std::endl; \
  } \

template<class MSG_T>
void serialize(void * msg)
{
  auto ts = rosidl_typesupport_cpp::get_message_type_support_handle<MSG_T>();
  auto introspection_ts = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
    get_message_typesupport_handle(
      ts, rosidl_typesupport_introspection_cpp::typesupport_identifier)->data);

  std::vector<char> payload_vector{};
  payload_vector.reserve(1000);
  rmw_iceoryx_cpp::serialize(msg, introspection_ts, payload_vector);
  payload_vector.clear();
}

template<class MSG_T>
void is_fixed_size()
{
  auto ts = rosidl_typesupport_cpp::get_message_type_support_handle<MSG_T>();
  rmw_iceoryx_cpp::iceoryx_is_fixed_size(ts);
}

int main(int argc, char ** argv)
{
  (void) argc;
  (void) argv;

  measure(std::this_thread::sleep_for(std::chrono::microseconds(1000)), "sleep1", 10);

  auto bt_msgs = get_messages_basic_types();
  measure(serialize<test_msgs::msg::BasicTypes>(bt_msgs[0].get()), "basic_types1", 1);
  measure(serialize<test_msgs::msg::BasicTypes>(bt_msgs[0].get()), "basic_types100", 100);

  auto ubs_msgs = get_messages_unbounded_sequences();
  measure(serialize<test_msgs::msg::UnboundedSequences>(ubs_msgs[0].get()), "unbounded_sequences1", 1);
  measure(serialize<test_msgs::msg::UnboundedSequences>(ubs_msgs[0].get()), "unbounded_sequences100", 100);

  auto bs_msgs = get_messages_bounded_sequences();
  measure(serialize<test_msgs::msg::BoundedSequences>(bs_msgs[0].get()), "bounded_sequences1", 1);
  measure(serialize<test_msgs::msg::BoundedSequences>(bs_msgs[0].get()), "bounded_sequences100", 100);

// SEQFAULT!
//  auto mn_msgs = get_messages_multi_nested();
//  measure_serialize<test_msgs::msg::MultiNested>(mn_msgs[0].get(), "multi nested", true);

  // measure_type_introspection<test_msgs::msg::MultiNested>();
  return 0;
}
