/*
 * Copyright 2026 CrucibleC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IRON_C_FULL_H
#define IRON_C_FULL_H

// No internal ic dependencies
#include "ic_inline.h"
#include "ic_static_assert.h"
#include "ic_bounded_loop.h"
// Internally dependent on the above (no circular dependencies)
#include "ic_memory.h"
#include "ic_num_cast.h"
#include "ic_typenum.h"
#include "ic_result.h"
#include "ic_opaque_storage.h"
#include "ic_concurrency.h"
#include "ic_concurrency_signal.h"
#include "ic_co_job.h"

#endif // IRON_C_FULL_H
