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

#ifndef PI_SRC_P4INFO_FIELDS_INT_H_
#define PI_SRC_P4INFO_FIELDS_INT_H_

#include "PI/p4info/fields.h"

#ifdef __cplusplus
extern "C" {
#endif

void pi_p4info_field_init(pi_p4info_t *p4info, size_t num_fields);

void pi_p4info_field_add(pi_p4info_t *p4info, pi_p4_id_t field_id,
                         const char *name, size_t bitwidth);

typedef struct cJSON cJSON;
void pi_p4info_field_serialize(cJSON *root, const pi_p4info_t *p4info);

#ifdef __cplusplus
}
#endif

#endif  // PI_SRC_P4INFO_FIELDS_INT_H_
