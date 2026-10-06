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

#ifndef PREMADE_PARALLEL_WORK_H
#define PREMADE_PARALLEL_WORK_H

#ifndef IC_CONCURRENCY_NULLPTR_PANIC
#include <stdio.h>
#define IC_CONCURRENCY_NULLPTR_PANIC(msg) \
    do { fprintf(stderr, "[CONCURRENCY] NULLPTR: %s\n", msg); } while (0)
#endif

#define PW_USE_C11_THREADING 0 // Set to 1 to use C11 threads and atomics, otherwise will use platform-specific backends (Win / pthreads)
#if PW_USE_C11_THREADING
    #define IC_USE_C11_THREADS_AND_ATOMICS
#endif

#include "ironclib/ic_inline.h"
#include "ironclib/ic_concurrency.h"
#include "ironclib/ic_concurrency_signal.h"
#include "ironclib/ic_typenum.h"
#include "global_error.h"
#include <stdint.h>
#include <stdbool.h>

IC_HEADER_FUNC AppError pw_concurrency_result_to_error(const int concurrency_result)
{
    switch (concurrency_result)
    {
        case IC_CONCURRENCY_OK: return AppError_NoError;
        case IC_CONCURRENCY_NULLREF: return AppError_NullRef;
        case IC_CONCURRENCY_FAILURE: return AppError_Runtime;
        case IC_CONCURRENCY_ALREADY_JOINED: return AppError_InvalidState;
        case IC_CONCURRENCY_ALREADY_LOCKED: return AppError_InvalidState;
        default: return AppError_Unknown;
    }
}

// SLEEP API
IC_HEADER_FUNC void pw_thread_sleep_milliseconds(const int32_t milliseconds)    { ic_thread_sleep(milliseconds); }

// ATOMIC API
typedef ic_atomic_i32 PwAtomicI32;
IC_HEADER_FUNC AppError pw_atomic_init(PwAtomicI32* const out_atom, const int32_t value)    { return pw_concurrency_result_to_error(ic_atomic_init(out_atom, value)); }
IC_HEADER_FUNC PwAtomicI32 pw_atomic_make(const int32_t value)                              { return ic_make_atomic(value); }
IC_HEADER_FUNC int32_t pw_atomic_load(const PwAtomicI32* const atom)                        { return ic_atomic_load(atom); }
IC_HEADER_FUNC void pw_atomic_store(PwAtomicI32* const atom, const int32_t value)           { ic_atomic_store(atom, value); }
IC_HEADER_FUNC int32_t pw_atomic_fetch_add(PwAtomicI32* const atom, const int32_t value)    { return ic_atomic_fetch_add(atom, value); }
IC_HEADER_FUNC int32_t pw_atomic_exchange(PwAtomicI32* const atom, const int32_t value)     { return ic_atomic_exchange(atom, value); }

// TASK API
typedef ic_task PwTask;
typedef ic_task_function PwTaskFunction;
IC_HEADER_FUNC AppError pw_task_init(PwTask* const out_task, const PwTaskFunction function, void* const arg)    { return pw_concurrency_result_to_error(ic_task_init(out_task, function, arg)); }
IC_HEADER_FUNC bool pw_task_is_running(const PwTask* const task)                                                { return (bool)ic_task_is_running(task); }
IC_HEADER_FUNC AppError pw_task_get_result(const PwTask* const task, int* const out_result)                     { return pw_concurrency_result_to_error(ic_task_get_result(task, out_result)); }
IC_HEADER_FUNC AppError pw_task_join(PwTask* const task)                                                        { return pw_concurrency_result_to_error(ic_task_join(task)); }

// MUTEX API
typedef ic_mutex PwMutex;
IC_HEADER_FUNC AppError pw_mutex_init(PwMutex* const out_mutex)    { return pw_concurrency_result_to_error(ic_mutex_init(out_mutex)); }
IC_HEADER_FUNC void pw_mutex_lock(PwMutex* const mutex)            { ic_mutex_lock(mutex); }
IC_HEADER_FUNC AppError pw_mutex_trylock(PwMutex* const mutex)     { return pw_concurrency_result_to_error(ic_mutex_trylock(mutex)); }
IC_HEADER_FUNC void pw_mutex_unlock(PwMutex* const mutex)          { ic_mutex_unlock(mutex); }
IC_HEADER_FUNC AppError pw_mutex_destroy(PwMutex* const mutex)     { return pw_concurrency_result_to_error(ic_mutex_destroy(mutex)); }

// CONDITION VARIABLE API
typedef ic_condition_variable PwConditionVariable;
IC_HEADER_FUNC AppError pw_condition_variable_init(PwConditionVariable* const cv)                          { return pw_concurrency_result_to_error(ic_condition_variable_init(cv)); }
IC_HEADER_FUNC AppError pw_condition_variable_notify_one(PwConditionVariable* const cv)                    { return pw_concurrency_result_to_error(ic_condition_variable_notify_one(cv)); }
IC_HEADER_FUNC AppError pw_condition_variable_notify_all(PwConditionVariable* const cv)                    { return pw_concurrency_result_to_error(ic_condition_variable_notify_all(cv)); }
IC_HEADER_FUNC AppError pw_condition_variable_wait(PwConditionVariable* const cv, PwMutex* const mutex)    { return pw_concurrency_result_to_error(ic_condition_variable_wait(cv, mutex)); }
IC_HEADER_FUNC AppError pw_condition_variable_destroy(PwConditionVariable* const cv)                       { return pw_concurrency_result_to_error(ic_condition_variable_destroy(cv)); }

// GATE API
typedef ic_gate PwGate;
IC_HEADER_FUNC AppError pw_gate_init(PwGate* const out_gate)      { return pw_concurrency_result_to_error(ic_gate_init(out_gate)); }
IC_HEADER_FUNC AppError pw_gate_wait(PwGate* const gate)          { return pw_concurrency_result_to_error(ic_gate_wait(gate)); }
IC_HEADER_FUNC AppError pw_gate_signal_one(PwGate* const gate)    { return pw_concurrency_result_to_error(ic_gate_signal_one(gate)); }
IC_HEADER_FUNC AppError pw_gate_destroy(PwGate* const gate)       { return pw_concurrency_result_to_error(ic_gate_destroy(gate)); }

// BROADCAST API
typedef ic_broadcast PwBroadcast;
IC_HEADER_FUNC AppError pw_broadcast_init(PwBroadcast* const out_broadcast)      { return pw_concurrency_result_to_error(ic_broadcast_init(out_broadcast)); }
IC_HEADER_FUNC AppError pw_broadcast_wait(PwBroadcast* const broadcast)          { return pw_concurrency_result_to_error(ic_broadcast_wait(broadcast)); }
IC_HEADER_FUNC AppError pw_broadcast_signal_all(PwBroadcast* const broadcast)    { return pw_concurrency_result_to_error(ic_broadcast_signal_all(broadcast)); }
IC_HEADER_FUNC AppError pw_broadcast_reset(PwBroadcast* const broadcast)         { return pw_concurrency_result_to_error(ic_broadcast_reset(broadcast)); }
IC_HEADER_FUNC AppError pw_broadcast_destroy(PwBroadcast* const broadcast)       { return pw_concurrency_result_to_error(ic_broadcast_destroy(broadcast)); }

// TASK POOL API

// typedef struct PwTaskPool PwTaskPool;
// typedef void (*PwTaskPoolFunction)(void* arg);
// #ifndef PW_TASKPOOL_MAX_THREADS
// #ifndef PW_TASKPOOL_MAX_PENDING_TASKS
// AppError pw_task_pool_init(PwTaskPool* const out_pool, const uint32_t worker_count);
// AppError pw_task_pool_submit(PwTaskPool* const pool, const PwTaskPoolFunction func, void* const arg, PwTaskCompletion* const out_completion);
// AppError pw_task_pool_destroy(PwTaskPool* const pool);

// typedef struct PwTaskCompletion PwTaskCompletion;
// AppError pw_task_completion_wait(const PwTaskCompletion* const completion, const int32_t retry_period_ms, const int64_t timeout_ms);

// ==============================================================================
// TASK POOL IMPLEMENTATION
// ==============================================================================

#ifndef PW_TASKPOOL_MAX_THREADS
#define PW_TASKPOOL_MAX_THREADS 8
#endif

#ifndef PW_TASKPOOL_MAX_PENDING_TASKS
#define PW_TASKPOOL_MAX_PENDING_TASKS 1024
#endif

typedef void (*PwTaskPoolFunction)(void* arg);

typedef enum PwTaskPoolState
{
    PW_TASKPOOL_RUNNING = 0,
    PW_TASKPOOL_CLOSING_DRAIN = 1,
    PW_TASKPOOL_CLOSING_ABORT = 2

} PwTaskPoolState;

typedef struct PwTaskCompletion
{
    PwAtomicI32 PRIVATE_completed;

} PwTaskCompletion;

typedef struct PwTaskPoolJob
{
    PwTaskPoolFunction PRIVATE_func;
    void* PRIVATE_arg;
    PwTaskCompletion* PRIVATE_completion;

} PwTaskPoolJob;

typedef struct PwTaskPool
{
    PwTask PRIVATE_workers[PW_TASKPOOL_MAX_THREADS];
    uint32_t PRIVATE_worker_count;

    PwMutex PRIVATE_mutex;
    PwGate PRIVATE_work_gate;

    PwAtomicI32 PRIVATE_state;
    PwAtomicI32 PRIVATE_active_jobs;

    PwTaskPoolJob PRIVATE_jobs[PW_TASKPOOL_MAX_PENDING_TASKS];

    uint32_t PRIVATE_head;
    uint32_t PRIVATE_tail;
    uint32_t PRIVATE_count;

} PwTaskPool;

// ==============================================================================
// INTERNAL WORKER
// ==============================================================================

static int PRIVATE_pw_task_pool_worker_main(void* arg)
{
    PwTaskPool* p = (PwTaskPool*)arg;

    while (1)
    {
        if (!AppError_eq(pw_gate_wait(&p->PRIVATE_work_gate), AppError_NoError))
        {
            continue;
        }

        pw_mutex_lock(&p->PRIVATE_mutex);

        const int32_t state = pw_atomic_load(&p->PRIVATE_state);

        // Abort mode ignores remaining queued jobs immediately
        if (state == PW_TASKPOOL_CLOSING_ABORT)
        {
            pw_mutex_unlock(&p->PRIVATE_mutex);
            break;
        }

        // No queued jobs available
        if (p->PRIVATE_count == 0)
        {
            pw_mutex_unlock(&p->PRIVATE_mutex);

            // Drain mode exits once queue becomes empty
            if (state == PW_TASKPOOL_CLOSING_DRAIN)
            {
                break;
            }

            continue;
        }

        PwTaskPoolJob job = p->PRIVATE_jobs[p->PRIVATE_head];

        p->PRIVATE_head = (p->PRIVATE_head + 1) % PW_TASKPOOL_MAX_PENDING_TASKS;
        p->PRIVATE_count--;

        pw_atomic_fetch_add(&p->PRIVATE_active_jobs, 1);

        pw_mutex_unlock(&p->PRIVATE_mutex);

        job.PRIVATE_func(job.PRIVATE_arg);

        if (job.PRIVATE_completion)
        {
            pw_atomic_store(&job.PRIVATE_completion->PRIVATE_completed, 1);
        }

        (void)pw_atomic_fetch_add(&p->PRIVATE_active_jobs, -1);

        // Wake sleeping workers during drain shutdown so they can exit
        if (pw_atomic_load(&p->PRIVATE_state) == PW_TASKPOOL_CLOSING_DRAIN)
        {
            pw_gate_signal_one(&p->PRIVATE_work_gate);
        }
    }

    return 0;
}

// ==============================================================================
// COMPLETION WAIT
// ==============================================================================

#ifndef PW_TASK_COMPLETION_WAIT_FOREVER
#define PW_TASK_COMPLETION_WAIT_FOREVER 0
#endif

IC_HEADER_FUNC AppError pw_task_completion_wait(const PwTaskCompletion* const completion, const int32_t retry_period_ms, const int64_t timeout_ms)
{
    if (!completion)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("pw_task_completion_wait: completion is null");
        return AppError_NullRef;
    }

    if (retry_period_ms < 0)
    {
        return AppError_Argument;
    }

    if (timeout_ms < 0)
    {
        return AppError_Argument;
    }

    // -------------------------------------------------------------------------
    // FAST SPIN FIRST
    // -------------------------------------------------------------------------

    for (uint32_t i = 0; i < 128; i++)
    {
        if (pw_atomic_load(&completion->PRIVATE_completed))
        {
            return AppError_NoError;
        }
    }

    // -------------------------------------------------------------------------
    // WAIT WITH BACKOFF
    // -------------------------------------------------------------------------

    uint32_t it = 0;
    int64_t elapsed_ms = 0;
    int32_t current_sleep_ms = retry_period_ms;

    while (!pw_atomic_load(&completion->PRIVATE_completed))
    {
        pw_thread_sleep_milliseconds(current_sleep_ms);

        if (timeout_ms != PW_TASK_COMPLETION_WAIT_FOREVER)
        {
            // Saturating overflow-safe accumulation
            if (elapsed_ms > INT64_MAX - current_sleep_ms)
            {
                elapsed_ms = INT64_MAX;
            }
            else
            {
                elapsed_ms += current_sleep_ms;
            }

            if (elapsed_ms >= timeout_ms)
            {
                return AppError_Timeout;
            }
        }

        it++;

        if ((it % 10) == 0)
        {
            if (current_sleep_ms < 64)
            {
                current_sleep_ms *= 2;

                if (current_sleep_ms > 64)
                {
                    current_sleep_ms = 64;
                }
            }
        }
    }

    return AppError_NoError;
}

// ==============================================================================
// INIT
// ==============================================================================

IC_HEADER_FUNC AppError pw_task_pool_init(PwTaskPool* const out_pool, uint32_t worker_count)
{
    if (!out_pool)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("pw_task_pool_init: out_pool is null");
        return AppError_NullRef;
    }

    if (worker_count == 0 || worker_count > PW_TASKPOOL_MAX_THREADS)
    {
        return AppError_Argument;
    }

    out_pool->PRIVATE_worker_count = worker_count;

    out_pool->PRIVATE_head = 0;
    out_pool->PRIVATE_tail = 0;
    out_pool->PRIVATE_count = 0;

    pw_atomic_store(&out_pool->PRIVATE_state, PW_TASKPOOL_RUNNING);
    pw_atomic_store(&out_pool->PRIVATE_active_jobs, 0);

    AppError err = pw_mutex_init(&out_pool->PRIVATE_mutex);

    if (!AppError_eq(err, AppError_NoError))
    {
        return err;
    }

    err = pw_gate_init(&out_pool->PRIVATE_work_gate);

    if (!AppError_eq(err, AppError_NoError))
    {
        pw_mutex_destroy(&out_pool->PRIVATE_mutex);
        return err;
    }

    for (uint32_t i = 0; i < worker_count; i++)
    {
        err = pw_task_init(&out_pool->PRIVATE_workers[i], PRIVATE_pw_task_pool_worker_main, out_pool);

        if (!AppError_eq(err, AppError_NoError))
        {
            pw_atomic_store(&out_pool->PRIVATE_state, PW_TASKPOOL_CLOSING_ABORT);

            for (uint32_t j = 0; j < worker_count; j++)
            {
                (void)pw_gate_signal_one(&out_pool->PRIVATE_work_gate);
            }

            for (uint32_t j = 0; j < i; j++)
            {
                (void)pw_task_join(&out_pool->PRIVATE_workers[j]);
            }

            (void)pw_gate_destroy(&out_pool->PRIVATE_work_gate);
            (void)pw_mutex_destroy(&out_pool->PRIVATE_mutex);

            return err;
        }
    }

    return AppError_NoError;
}

// ==============================================================================
// CLOSE MODE ENUM
// ==============================================================================

#define PW_TASK_POOL_CLOSE_LIST(X, Type) \
    X(Type, Drain, 0, "Finish queued and active tasks") \
    X(Type, Abort, 1, "Finish only active tasks")

IC_TYPENUM_FULL(PwTaskPoolClose, uint8_t, PW_TASK_POOL_CLOSE_LIST)

// ==============================================================================
// CLOSE
// ==============================================================================

IC_HEADER_FUNC AppError pw_task_pool_close(PwTaskPool* const pool, const PwTaskPoolClose close_mode, const int64_t timeout_ms)
{
    if (!pool)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("pw_task_pool_close: pool is null");
        return AppError_NullRef;
    }

    if (timeout_ms < 0 && timeout_ms != PW_TASK_COMPLETION_WAIT_FOREVER)
    {
        return AppError_Argument;
    }

    int32_t set_state = -1;

    if (PwTaskPoolClose_eq(close_mode, PwTaskPoolClose_Drain))
    {
        if (pw_atomic_load(&pool->PRIVATE_state) == PW_TASKPOOL_CLOSING_ABORT)
        {
            return AppError_InvalidState;
        }

        set_state = PW_TASKPOOL_CLOSING_DRAIN;
    }
    else if (PwTaskPoolClose_eq(close_mode, PwTaskPoolClose_Abort))
    {
        set_state = PW_TASKPOOL_CLOSING_ABORT;
    }
    else
    {
        return AppError_Argument;
    }

    pw_atomic_store(&pool->PRIVATE_state, set_state);

    for (uint32_t i = 0; i < pool->PRIVATE_worker_count; i++)
    {
        const AppError res = pw_gate_signal_one(&pool->PRIVATE_work_gate);
        if (!AppError_eq(res, AppError_NoError))
        {
            return res;
        }
    }

    // ==========================================================================
    // WAIT FOR SHUTDOWN CONDITION
    // ==========================================================================

    uint32_t sleep_ms = 1;
    int64_t elapsed_ms = 0;

    if (set_state == PW_TASKPOOL_CLOSING_DRAIN)
    {
        while (1)
        {
            pw_mutex_lock(&pool->PRIVATE_mutex);

            const uint32_t queued = pool->PRIVATE_count;

            pw_mutex_unlock(&pool->PRIVATE_mutex);

            const int32_t active = pw_atomic_load(&pool->PRIVATE_active_jobs);

            if (queued == 0 && active == 0)
            {
                break;
            }

            pw_thread_sleep_milliseconds(sleep_ms);

            if (timeout_ms != PW_TASK_COMPLETION_WAIT_FOREVER)
            {
                if (elapsed_ms > INT64_MAX - sleep_ms)
                {
                    elapsed_ms = INT64_MAX;
                }
                else
                {
                    elapsed_ms += sleep_ms;
                }

                if (elapsed_ms >= timeout_ms)
                {
                    return AppError_Timeout;
                }
            }

            if (sleep_ms < 16)
            {
                sleep_ms *= 2;
            }
        }
    }
    else
    {
        while (pw_atomic_load(&pool->PRIVATE_active_jobs) != 0)
        {
            pw_thread_sleep_milliseconds(sleep_ms);

            if (timeout_ms != PW_TASK_COMPLETION_WAIT_FOREVER)
            {
                if (elapsed_ms > INT64_MAX - sleep_ms)
                {
                    elapsed_ms = INT64_MAX;
                }
                else
                {
                    elapsed_ms += sleep_ms;
                }

                if (elapsed_ms >= timeout_ms)
                {
                    return AppError_Timeout;
                }
            }

            if (sleep_ms < 16)
            {
                sleep_ms *= 2;
            }
        }
    }

    // ==========================================================================
    // FINAL WAKEUP FOR EXITING WORKERS
    // ==========================================================================

    for (uint32_t i = 0; i < pool->PRIVATE_worker_count; i++)
    {
        const AppError res = pw_gate_signal_one(&pool->PRIVATE_work_gate);
        if (!AppError_eq(res, AppError_NoError))
        {
            return res;
        }
    }

    return AppError_NoError;
}

// ==============================================================================
// SUBMIT
// ==============================================================================

IC_HEADER_FUNC AppError pw_task_pool_submit(PwTaskPool* const pool, const PwTaskPoolFunction func, void* const arg, PwTaskCompletion* const out_completion)
{
    if (!pool || !func)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("pw_task_pool_submit: pool or func is null");
        return AppError_NullRef;
    }

    if (pw_atomic_load(&pool->PRIVATE_state) != PW_TASKPOOL_RUNNING)
    {
        return AppError_InvalidState;
    }

    if (out_completion)
    {
        pw_atomic_store(&out_completion->PRIVATE_completed, 0);
    }

    pw_mutex_lock(&pool->PRIVATE_mutex);

    if (pool->PRIVATE_count >= PW_TASKPOOL_MAX_PENDING_TASKS)
    {
        pw_mutex_unlock(&pool->PRIVATE_mutex);
        return AppError_OutOfBounds;
    }

    pool->PRIVATE_jobs[pool->PRIVATE_tail] = (PwTaskPoolJob)
    {
        .PRIVATE_func = func,
        .PRIVATE_arg = arg,
        .PRIVATE_completion = out_completion
    };

    pool->PRIVATE_tail = (pool->PRIVATE_tail + 1) % PW_TASKPOOL_MAX_PENDING_TASKS;
    pool->PRIVATE_count++;

    pw_mutex_unlock(&pool->PRIVATE_mutex);

    return pw_gate_signal_one(&pool->PRIVATE_work_gate);
}

// ==============================================================================
// DESTROY
// ==============================================================================

IC_HEADER_FUNC AppError pw_task_pool_destroy(PwTaskPool* const pool)
{
    if (!pool)
    {
        IC_CONCURRENCY_NULLPTR_PANIC("pw_task_pool_destroy: pool is null");
        return AppError_NullRef;
    }

    const int64_t thirty_seconds = 30 * 1000;
    AppError err = pw_task_pool_close(pool, PwTaskPoolClose_Abort, thirty_seconds);
    if (!AppError_eq(err, AppError_NoError))
    {
        return err;
    }

    for (uint32_t i = 0; i < pool->PRIVATE_worker_count; i++)
    {
        err = pw_task_join(&pool->PRIVATE_workers[i]);
    }
    if (!AppError_eq(err, AppError_NoError))
    {
        return AppError_Irrecoverable;
    }

    err = pw_gate_destroy(&pool->PRIVATE_work_gate);
    if (!AppError_eq(err, AppError_NoError))
    {
        return AppError_Irrecoverable;
    }

    err = pw_mutex_destroy(&pool->PRIVATE_mutex);
    if (!AppError_eq(err, AppError_NoError))
    {
        return AppError_Irrecoverable;
    }
    return AppError_NoError;
}

#endif // PREMADE_PARALLEL_WORK_H