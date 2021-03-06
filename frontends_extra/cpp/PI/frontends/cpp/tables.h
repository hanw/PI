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

#ifndef PI_FRONTENDS_CPP_TABLES_H_
#define PI_FRONTENDS_CPP_TABLES_H_

#include <PI/pi.h>

#include <string>
#include <vector>

#include <cstdint>

namespace pi {

// TODO(antonin): temporary
typedef int error_code_t;

class MatchKeyReader {
 public:
  explicit MatchKeyReader(const pi_match_key_t *match_key);

  error_code_t get_exact(pi_p4_id_t f_id, std::string *key) const;

  error_code_t get_lpm(pi_p4_id_t f_id, std::string *key,
                       int *prefix_length) const;

  error_code_t get_ternary(pi_p4_id_t f_id, std::string *key,
                           std::string *mask) const;

  error_code_t get_valid(pi_p4_id_t f_id, bool *key) const;

  int get_priority() const;

 private:
  error_code_t read_one(pi_p4_id_t f_id, const char *src, std::string *v) const;
  const pi_match_key_t *match_key;
};

class MatchKey {
  friend class MatchTable;
 public:
  MatchKey(const pi_p4info_t *p4info, pi_p4_id_t table_id);
  ~MatchKey();

  void reset();

  void set_priority(int priority);

  int get_priority() const;

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, error_code_t>::type
  set_exact(pi_p4_id_t f_id, T key);
  error_code_t set_exact(pi_p4_id_t f_id, const char *key, size_t s);

  error_code_t get_exact(pi_p4_id_t f_id, std::string *key) const;

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, error_code_t>::type
  set_lpm(pi_p4_id_t f_id, T key, int prefix_length);
  error_code_t
  set_lpm(pi_p4_id_t f_id, const char *key, size_t s, int prefix_length);

  error_code_t get_lpm(pi_p4_id_t f_id, std::string *key,
                       int *prefix_length) const;

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, error_code_t>::type
  set_ternary(pi_p4_id_t f_id, T key, T mask);
  error_code_t
  set_ternary(pi_p4_id_t f_id, const char *key, const char *mask, size_t s);

  error_code_t get_ternary(pi_p4_id_t f_id, std::string *key,
                           std::string *mask) const;

  error_code_t set_valid(pi_p4_id_t f_id, bool key);
  error_code_t get_valid(pi_p4_id_t f_id, bool *key) const;

 private:
  template <typename T>
  error_code_t format(pi_p4_id_t f_id, T v, size_t offset, size_t *written);
  error_code_t format(pi_p4_id_t f_id, const char *ptr, size_t s,
                      size_t offset, size_t *written);

  pi_match_key_t *get() const {
    return match_key;
  }

  const pi_p4info_t *p4info;
  pi_p4_id_t table_id;
  size_t nset{0};
  size_t mk_size;
  std::vector<char> _data;
  pi_match_key_t *match_key;
  MatchKeyReader reader;
};

class ActionDataReader {
 public:
  explicit ActionDataReader(const pi_action_data_t *action_data);

  error_code_t get_arg(pi_p4_id_t ap_id, std::string *arg) const;

  pi_p4_id_t get_action_id() const;

 private:
  const pi_action_data_t *action_data;
};

class ActionData {
  friend class MatchTable;
  friend class ActProf;
 public:
  ActionData(const pi_p4info_t *p4info, pi_p4_id_t action_id);
  ~ActionData();

  void reset();

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value, error_code_t>::type
  set_arg(pi_p4_id_t ap_id, T arg);
  error_code_t
  set_arg(pi_p4_id_t ap_id, const char *arg, size_t s);

  error_code_t get_arg(pi_p4_id_t ap_id, std::string *arg) const;

 private:
  template <typename T>
  error_code_t format(pi_p4_id_t ap_id, T v);
  error_code_t format(pi_p4_id_t ap_id, const char *ptr, size_t s);

  pi_action_data_t *get() const {
    return action_data;
  }

  const pi_p4info_t *p4info;
#ifdef __clang__
  __attribute__((unused))
#endif
  pi_p4_id_t action_id;
  size_t nset{0};
  size_t ad_size;
  std::vector<char> _data;
  pi_action_data_t *action_data;
  ActionDataReader reader;
};

class ActionEntry {
 public:
  friend class MatchTable;

  ActionEntry()
      : tag(Tag::NONE) { }

  ~ActionEntry() {
    switch (tag) {
      case Tag::NONE:
        break;
      case Tag::ACTION_DATA:
        _action_data.~ActionData();
        break;
      case Tag::INDIRECT_HANDLE:
        break;
    }
  }

  ActionEntry(const ActionEntry &) = delete;
  ActionEntry &operator=(const ActionEntry &) = delete;
  ActionEntry(ActionEntry &&) = delete;
  ActionEntry &operator=(ActionEntry &&) = delete;

  void init_action_data(const pi_p4info_t *p4info, pi_p4_id_t action_id) {
    assert(tag == Tag::NONE);
    new(&_action_data) ActionData(p4info, action_id);
    tag = Tag::ACTION_DATA;
  }

  void init_indirect_handle(pi_indirect_handle_t indirect_handle) {
    assert(tag == Tag::NONE);
    _indirect_handle = indirect_handle;
    tag = Tag::INDIRECT_HANDLE;
  }

  const ActionData &action_data() const {
    assert(tag == Tag::ACTION_DATA);
    return _action_data;
  }

  ActionData *mutable_action_data() {
    assert(tag == Tag::ACTION_DATA);
    return &_action_data;
  }

  pi_indirect_handle_t indirect_handle() const {
    assert(tag == Tag::INDIRECT_HANDLE);
    return _indirect_handle;
  }

 private:
  enum class Tag { NONE, ACTION_DATA, INDIRECT_HANDLE } tag;

  Tag type() const { return tag; }

  union {
    ActionData _action_data;
    pi_indirect_handle_t _indirect_handle;
  };
};

// TODO(antonin): handle device id / pipleline mask
class MatchTable {
 public:
  MatchTable(pi_session_handle_t sess, pi_dev_tgt_t dev_tgt,
             const pi_p4info_t *p4info, pi_p4_id_t table_id);

  pi_status_t entry_add(const MatchKey &match_key,
                        const ActionEntry &action_entry, bool overwrite,
                        pi_entry_handle_t *entry_handle);

  pi_status_t entry_delete(pi_entry_handle_t entry_handle);
  pi_status_t entry_delete_wkey(const MatchKey &match_key);

  pi_status_t default_entry_set(const ActionEntry &action_entry);

  // these overloads are mostly for backward-compatibility, try not to use in
  // new code
  pi_status_t entry_add(const MatchKey &match_key,
                        const ActionData &action_data, bool overwrite,
                        pi_entry_handle_t *entry_handle);
  pi_status_t default_entry_set(const ActionData &action_data);

  // many more APIs

 private:
  pi_table_entry_t build_table_entry(const ActionEntry &action_entry) const;

  pi_session_handle_t sess;
  pi_dev_tgt_t dev_tgt;
  // TODO(antonin): is p4info really needed here?
#ifdef __clang__
  __attribute__((unused))
#endif
  const pi_p4info_t *p4info;
  pi_p4_id_t table_id;
};

// TODO(antonin): move to separate file
class ActProf {
 public:
  ActProf(pi_session_handle_t sess, pi_dev_tgt_t dev_tgt,
          const pi_p4info_t *p4info, pi_p4_id_t act_prof_id);

  pi_status_t member_create(const ActionData &action_data,
                            pi_indirect_handle_t *member_handle);

  pi_status_t member_delete(pi_indirect_handle_t member_handle);

  pi_status_t member_modify(pi_indirect_handle_t member_handle,
                            const ActionData &action_data);

  pi_status_t group_create(size_t max_size, pi_indirect_handle_t *group_handle);

  pi_status_t group_delete(pi_indirect_handle_t group_handle);

  pi_status_t group_add_member(pi_indirect_handle_t group_handle,
                               pi_indirect_handle_t member_handle);

  pi_status_t group_remove_member(pi_indirect_handle_t group_handle,
                                  pi_indirect_handle_t member_handle);

 private:
  pi_session_handle_t sess;
  pi_dev_tgt_t dev_tgt;
#ifdef __clang__
  __attribute__((unused))
#endif
  const pi_p4info_t *p4info;
  pi_p4_id_t act_prof_id;
};

}  // namespace pi

#endif  // PI_FRONTENDS_CPP_TABLES_H_
