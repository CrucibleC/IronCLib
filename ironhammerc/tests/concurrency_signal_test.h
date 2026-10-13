#ifndef IRON_HAMMER_C_TESTS_CONCURRENCY_SIGNAL_TEST_H
#define IRON_HAMMER_C_TESTS_CONCURRENCY_SIGNAL_TEST_H

#ifndef RUN_STRESS_TESTS
#define RUN_STRESS_TESTS 0
#endif

#include "ic_hammer.h"
#include "ironclib/ic_concurrency.h"
#include "ironclib/ic_concurrency_signal.h"
#include "ironclib/ic_static_assert.h"
#include "docs/premade/parallel_work.h"

// NOTE: IHC_CHECK / IHC_ASSERT are not thread-safe (see ic_hammer.h), so worker threads never call them.
// Threads record failures in their test data instead, and the test checks them on the main thread
// after joining. Values a thread writes are only read under the same mutex, or after joining it.

// ==============================================================================
// CONDITION VARIABLE TESTS
// ==============================================================================

typedef struct condition_variable_test_data
{
    ic_condition_variable cv;
    ic_mutex mutex;

    int ready;
    int wake_count;
    int thread_failures; // guarded by mutex
} condition_variable_test_data;

static int condition_variable_waiter_thread(void* arg)
{
    condition_variable_test_data* data = (condition_variable_test_data*)arg;

    ic_mutex_lock(&data->mutex);

    while (!data->ready)
    {
        // wait returns with the mutex held again, so the failure count stays guarded
        if (ic_condition_variable_wait(&data->cv, &data->mutex) != IC_CONCURRENCY_OK)
        {
            data->thread_failures++;
        }
    }

    data->wake_count++;

    ic_mutex_unlock(&data->mutex);

    return 0;
}

IHC_TEST(verify_condition_variable_can_signal_waiting_thread)
{
    condition_variable_test_data data = {0};

    IHC_ASSERT(ic_condition_variable_init(&data.cv) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.ready = 0;
    data.wake_count = 0;

    ic_task waiter;

    IHC_ASSERT(ic_task_init(&waiter, condition_variable_waiter_thread, &data) == IC_CONCURRENCY_OK);

    ic_thread_sleep(100);

    ic_mutex_lock(&data.mutex);
    data.ready = 1;
    ic_condition_variable_notify_one(&data.cv);
    ic_mutex_unlock(&data.mutex);

    ic_task_join(&waiter);

    IHC_CHECK(data.wake_count == 1);
    IHC_CHECK(data.thread_failures == 0);

    IHC_CHECK(ic_condition_variable_destroy(&data.cv) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&data.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(verify_condition_variable_notify_all_wakes_all_waiters)
{
    condition_variable_test_data data = {0};

    IHC_ASSERT(ic_condition_variable_init(&data.cv) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    ic_task t1, t2, t3, t4;

    ic_task_init(&t1, condition_variable_waiter_thread, &data);
    ic_task_init(&t2, condition_variable_waiter_thread, &data);
    ic_task_init(&t3, condition_variable_waiter_thread, &data);
    ic_task_init(&t4, condition_variable_waiter_thread, &data);

    ic_thread_sleep(100);

    ic_mutex_lock(&data.mutex);

    data.ready = 1;

    IHC_CHECK(ic_condition_variable_notify_all(&data.cv) == IC_CONCURRENCY_OK);

    ic_mutex_unlock(&data.mutex);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);
    ic_task_join(&t4);

    IHC_CHECK(data.wake_count == 4);
    IHC_CHECK(data.thread_failures == 0);

    ic_condition_variable_destroy(&data.cv);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_condition_variable_does_not_crash_when_given_nullptr)
{   
    IHC_CHECK(ic_condition_variable_init(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_condition_variable_notify_one(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_condition_variable_notify_all(NULL) == IC_CONCURRENCY_NULLREF);
    ic_mutex mutex;
    IHC_CHECK(ic_condition_variable_wait(NULL, &mutex) == IC_CONCURRENCY_NULLREF);
    
    ic_condition_variable cv;
    IHC_CHECK(ic_condition_variable_wait(&cv, NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_condition_variable_destroy(NULL) == IC_CONCURRENCY_NULLREF);
}

IHC_TEST(verify_condition_variable_wait_rechecks_predicate)
{
    condition_variable_test_data data = {0};

    IHC_ASSERT(ic_condition_variable_init(&data.cv) == IC_CONCURRENCY_OK);

    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.ready = 0;
    data.wake_count = 0;

    ic_task waiter;

    IHC_ASSERT(ic_task_init(&waiter, condition_variable_waiter_thread, &data) == IC_CONCURRENCY_OK);

    ic_thread_sleep(100);

    // Notify without changing predicate
    IHC_CHECK(ic_condition_variable_notify_one(&data.cv) == IC_CONCURRENCY_OK);

    ic_thread_sleep(200);

    // Thread should still be blocked (read under the mutex, as the waiter writes it under the mutex)
    ic_mutex_lock(&data.mutex);
    const int wake_count_while_blocked = data.wake_count;
    ic_mutex_unlock(&data.mutex);
    IHC_CHECK(wake_count_while_blocked == 0);

    ic_mutex_lock(&data.mutex);

    data.ready = 1;

    IHC_CHECK(ic_condition_variable_notify_one(&data.cv) == IC_CONCURRENCY_OK);

    ic_mutex_unlock(&data.mutex);

    ic_task_join(&waiter);

    IHC_CHECK(data.wake_count == 1);
    IHC_CHECK(data.thread_failures == 0);

    ic_condition_variable_destroy(&data.cv);
    ic_mutex_destroy(&data.mutex);
}

// ==============================================================================
// GATE TESTS
// ==============================================================================

IHC_TEST(verify_gate_nullptr_safety)
{
    IHC_CHECK(ic_gate_init(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_gate_wait(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_gate_signal_one(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_gate_destroy(NULL) == IC_CONCURRENCY_NULLREF);
}

typedef struct gate_test_data
{
    ic_gate gate;
    ic_mutex mutex;
    int wake_count;      // guarded by mutex
    int thread_failures; // guarded by mutex

} gate_test_data;

static int gate_waiter_thread(void* arg)
{
    gate_test_data* data = (gate_test_data*)arg;
    const int wait_result = ic_gate_wait(&data->gate);

    ic_mutex_lock(&data->mutex);
    if (wait_result != IC_CONCURRENCY_OK)
    {
        data->thread_failures++;
    }
    data->wake_count++;
    ic_mutex_unlock(&data->mutex);

    return 0;
}

// Reads the wake count under the mutex, as waiter threads may still be writing it
static int gate_test_wake_count(gate_test_data* const data)
{
    ic_mutex_lock(&data->mutex);
    const int wake_count = data->wake_count;
    ic_mutex_unlock(&data->mutex);
    return wake_count;
}

IHC_TEST(verify_gate_releases_exactly_one_thread_per_signal)
{
    gate_test_data data = {0};

    IHC_ASSERT(ic_gate_init(&data.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    ic_task t1, t2, t3;
    ic_task_init(&t1, gate_waiter_thread, &data);
    ic_task_init(&t2, gate_waiter_thread, &data);
    ic_task_init(&t3, gate_waiter_thread, &data);

    ic_thread_sleep(100);

    ic_gate_signal_one(&data.gate);
    ic_thread_sleep(50);
    IHC_CHECK(gate_test_wake_count(&data) == 1);

    ic_gate_signal_one(&data.gate);
    ic_thread_sleep(50);
    IHC_CHECK(gate_test_wake_count(&data) == 2);

    ic_gate_signal_one(&data.gate);
    ic_thread_sleep(50);
    IHC_CHECK(gate_test_wake_count(&data) == 3);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);

    IHC_CHECK(data.thread_failures == 0);

    ic_gate_destroy(&data.gate);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_gate_is_lossless)
{
    gate_test_data data = {0};

    IHC_ASSERT(ic_gate_init(&data.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    ic_gate_signal_one(&data.gate);

    ic_task t;
    ic_task_init(&t, gate_waiter_thread, &data);

    ic_task_join(&t);

    IHC_CHECK(data.wake_count == 1);
    IHC_CHECK(data.thread_failures == 0);

    ic_gate_destroy(&data.gate);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_gate_rejects_signals_when_permits_are_full)
{
    ic_gate gate;
    IHC_ASSERT(ic_gate_init(&gate) == IC_CONCURRENCY_OK);

    // Reaching the limit with real signals would take billions of calls, so start just below it
    gate.UNSAFE_PRIVATE_ACCESS_PERMITS = UINT32_MAX - 1;

    IHC_CHECK(ic_gate_signal_one(&gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(gate.UNSAFE_PRIVATE_ACCESS_PERMITS == UINT32_MAX);
    IHC_CHECK(ic_gate_signal_one(&gate) == IC_CONCURRENCY_FAILURE);
    IHC_CHECK(gate.UNSAFE_PRIVATE_ACCESS_PERMITS == UINT32_MAX); // not wrapped to 0

    // Consuming a permit makes room for exactly one more signal
    IHC_CHECK(ic_gate_wait(&gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_gate_signal_one(&gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_gate_signal_one(&gate) == IC_CONCURRENCY_FAILURE);

    IHC_CHECK(ic_gate_destroy(&gate) == IC_CONCURRENCY_OK);
}

// ==============================================================================
// BROADCAST TESTS
// ==============================================================================

IHC_TEST(verify_broadcast_nullptr_safety)
{
    IHC_CHECK(ic_broadcast_init(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_wait(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_signal_all(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_reset(NULL) == IC_CONCURRENCY_NULLREF);
    IHC_CHECK(ic_broadcast_destroy(NULL) == IC_CONCURRENCY_NULLREF);
}

typedef struct broadcast_test_data
{
    ic_broadcast broadcast;
    ic_mutex mutex;
    int wake_count;      // guarded by mutex
    int thread_failures; // guarded by mutex
} broadcast_test_data;

static int broadcast_waiter_thread(void* arg)
{
    broadcast_test_data* data = (broadcast_test_data*)arg;

    const int wait_result = ic_broadcast_wait(&data->broadcast);

    ic_mutex_lock(&data->mutex);
    if (wait_result != IC_CONCURRENCY_OK)
    {
        data->thread_failures++;
    }
    data->wake_count++;
    ic_mutex_unlock(&data->mutex);

    return 0;
}

// Reads the wake count under the mutex, as waiter threads may still be writing it
static int broadcast_test_wake_count(broadcast_test_data* const data)
{
    ic_mutex_lock(&data->mutex);
    const int wake_count = data->wake_count;
    ic_mutex_unlock(&data->mutex);
    return wake_count;
}

IHC_TEST(verify_broadcast_wakes_all_waiters)
{
    broadcast_test_data data = {0};

    IHC_ASSERT(ic_broadcast_init(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    ic_task t1, t2, t3, t4;
    ic_task_init(&t1, broadcast_waiter_thread, &data);
    ic_task_init(&t2, broadcast_waiter_thread, &data);
    ic_task_init(&t3, broadcast_waiter_thread, &data);
    ic_task_init(&t4, broadcast_waiter_thread, &data);

    ic_thread_sleep(100);

    ic_broadcast_signal_all(&data.broadcast);

    ic_task_join(&t1);
    ic_task_join(&t2);
    ic_task_join(&t3);
    ic_task_join(&t4);

    IHC_CHECK(data.wake_count == 4);
    IHC_CHECK(data.thread_failures == 0);

    ic_broadcast_destroy(&data.broadcast);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_broadcast_is_sticky_until_reset)
{
    broadcast_test_data data = {0};

    IHC_ASSERT(ic_broadcast_init(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    ic_broadcast_signal_all(&data.broadcast);

    ic_task t1, t2;

    ic_task_init(&t1, broadcast_waiter_thread, &data);
    ic_task_init(&t2, broadcast_waiter_thread, &data);

    ic_task_join(&t1);
    ic_task_join(&t2);

    IHC_CHECK(data.wake_count == 2);

    ic_broadcast_reset(&data.broadcast);

    data.wake_count = 0;

    ic_task_init(&t1, broadcast_waiter_thread, &data);
    ic_thread_sleep(50);

    IHC_CHECK(broadcast_test_wake_count(&data) == 0);

    ic_broadcast_signal_all(&data.broadcast);

    ic_task_join(&t1);

    IHC_CHECK(data.wake_count == 1);
    IHC_CHECK(data.thread_failures == 0);

    ic_broadcast_destroy(&data.broadcast);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_broadcast_reset_blocks_new_waiters)
{
    broadcast_test_data data = {0};

    IHC_ASSERT(ic_broadcast_init(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    data.wake_count = 0;

    // Step 1: signal broadcast (sticky state ON)
    IHC_CHECK(ic_broadcast_signal_all(&data.broadcast) == IC_CONCURRENCY_OK);

    // Step 2: reset it immediately (sticky state OFF again)
    IHC_CHECK(ic_broadcast_reset(&data.broadcast) == IC_CONCURRENCY_OK);

    // Step 3: start waiter AFTER reset
    ic_task t;
    IHC_ASSERT(ic_task_init(&t, broadcast_waiter_thread, &data) == IC_CONCURRENCY_OK);

    // Give it time to potentially pass incorrectly if broken
    ic_thread_sleep(100);

    // If reset works correctly, this MUST still be 0
    IHC_CHECK(broadcast_test_wake_count(&data) == 0);

    // Step 4: now signal again
    IHC_CHECK(ic_broadcast_signal_all(&data.broadcast) == IC_CONCURRENCY_OK);

    ic_task_join(&t);

    // Now it should pass
    IHC_CHECK(data.wake_count == 1);
    IHC_CHECK(data.thread_failures == 0);

    ic_broadcast_destroy(&data.broadcast);
    ic_mutex_destroy(&data.mutex);
}

IHC_TEST(verify_broadcast_signal_then_reset_wakes_current_waiters)
{
    broadcast_test_data data = {0};

    IHC_ASSERT(ic_broadcast_init(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&data.mutex) == IC_CONCURRENCY_OK);

    ic_task t1, t2;
    IHC_ASSERT(ic_task_init(&t1, broadcast_waiter_thread, &data) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_task_init(&t2, broadcast_waiter_thread, &data) == IC_CONCURRENCY_OK);

    ic_thread_sleep(100); // let both waiters block

    // Pulse: waiters that were blocked during the signal must wake even though the reset follows at once
    IHC_CHECK(ic_broadcast_signal_all(&data.broadcast) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_broadcast_reset(&data.broadcast) == IC_CONCURRENCY_OK);

    int woken = 0;
    for (int i = 0; i < 2000 && woken < 2; i++)
    {
        ic_mutex_lock(&data.mutex);
        woken = data.wake_count;
        ic_mutex_unlock(&data.mutex);
        ic_thread_sleep(1);
    }
    IHC_CHECK(woken == 2);

    // Release any waiter a regression left blocked, so the test fails instead of hanging
    ic_broadcast_signal_all(&data.broadcast);
    ic_task_join(&t1);
    ic_task_join(&t2);
    IHC_CHECK(data.thread_failures == 0);

    ic_broadcast_destroy(&data.broadcast);
    ic_mutex_destroy(&data.mutex);
}

#define JOIN_RACE_THREADS 4

typedef struct join_race_ctx
{
    ic_task* target;
    ic_broadcast start;
    ic_atomic_i32 release_target;
    ic_atomic_i32 ok_count;
    ic_atomic_i32 already_joined_count;
} join_race_ctx;

static int join_race_target_thread(void* arg)
{
    join_race_ctx* const ctx = (join_race_ctx*)arg;
    while (ic_atomic_load(&ctx->release_target) == 0)
    {
        ic_thread_sleep(1);
    }
    return 0;
}

static int join_race_joiner_thread(void* arg)
{
    join_race_ctx* const ctx = (join_race_ctx*)arg;
    (void)ic_broadcast_wait(&ctx->start);

    const int result = ic_task_join(ctx->target);
    if (result == IC_CONCURRENCY_OK)
    {
        (void)ic_atomic_fetch_add(&ctx->ok_count, 1);
    }
    else if (result == IC_CONCURRENCY_ALREADY_JOINED)
    {
        (void)ic_atomic_fetch_add(&ctx->already_joined_count, 1);
    }
    return 0;
}

IHC_TEST(verify_task_can_only_be_joined_once_under_contention)
{
    join_race_ctx ctx;
    ic_task target;
    ctx.target = &target;
    ctx.release_target = ic_make_atomic(0);
    ctx.ok_count = ic_make_atomic(0);
    ctx.already_joined_count = ic_make_atomic(0);
    IHC_ASSERT(ic_broadcast_init(&ctx.start) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_task_init(&target, join_race_target_thread, &ctx) == IC_CONCURRENCY_OK);

    ic_task joiners[JOIN_RACE_THREADS];
    for (int i = 0; i < JOIN_RACE_THREADS; i++)
    {
        IHC_ASSERT(ic_task_init(&joiners[i], join_race_joiner_thread, &ctx) == IC_CONCURRENCY_OK);
    }

    ic_thread_sleep(50); // let every joiner block on the start signal
    IHC_ASSERT(ic_broadcast_signal_all(&ctx.start) == IC_CONCURRENCY_OK);

    // The losers return at once, while the winner stays blocked in the join until the target exits
    int waited_ms = 0;
    while (ic_atomic_load(&ctx.already_joined_count) < JOIN_RACE_THREADS - 1 && waited_ms < 10000)
    {
        ic_thread_sleep(1);
        waited_ms++;
    }
    ic_atomic_store(&ctx.release_target, 1);

    for (int i = 0; i < JOIN_RACE_THREADS; i++)
    {
        IHC_CHECK(ic_task_join(&joiners[i]) == IC_CONCURRENCY_OK);
    }

    IHC_CHECK(ic_atomic_load(&ctx.ok_count) == 1);
    IHC_CHECK(ic_atomic_load(&ctx.already_joined_count) == JOIN_RACE_THREADS - 1);

    IHC_CHECK(ic_broadcast_destroy(&ctx.start) == IC_CONCURRENCY_OK);
}

// ==============================================================================
// STRESS TESTS
// ==============================================================================

#if RUN_STRESS_TESTS

#define GATE_STRESS_ITERS 200
#define GATE_THREADS 8

typedef struct gate_stress_ctx
{
    ic_gate gate;
    ic_mutex mutex;
    int wakes;
} gate_stress_ctx;

static int gate_worker(void* arg)
{
    gate_stress_ctx* c = arg;

    ic_gate_wait(&c->gate);

    ic_mutex_lock(&c->mutex);
    c->wakes++;
    ic_mutex_unlock(&c->mutex);

    return 0;
}

// Reads the wake count under the mutex, as worker threads may still be writing it
static int gate_stress_wakes(gate_stress_ctx* const c)
{
    ic_mutex_lock(&c->mutex);
    const int wakes = c->wakes;
    ic_mutex_unlock(&c->mutex);
    return wakes;
}

IHC_TEST(stress_gate_interleave_deterministic)
{
    gate_stress_ctx c = {0};

    IHC_ASSERT(ic_gate_init(&c.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < GATE_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        ic_task threads[GATE_THREADS];

        const int pre_signals = GATE_THREADS / 2;

        for (int i = 0; i < pre_signals; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        const int post_signals = GATE_THREADS - pre_signals;

        for (int i = 0; i < post_signals; i++)
        {
            ic_thread_sleep(1); // deterministic "interleaving pressure"
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes <= GATE_THREADS);
    }

    IHC_CHECK(ic_gate_destroy(&c.gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_gate_signal_before_wait_deterministic)
{
    gate_stress_ctx c = {0};

    IHC_ASSERT(ic_gate_init(&c.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < GATE_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        const int pre_signals = GATE_THREADS / 2;

        for (int i = 0; i < pre_signals; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        ic_task threads[GATE_THREADS];

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        const int late_signals = GATE_THREADS - pre_signals;

        for (int i = 0; i < late_signals; i++)
        {
            ic_thread_sleep(1); // forces interleaving pressure
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes <= GATE_THREADS);
    }

    IHC_CHECK(ic_gate_destroy(&c.gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_gate_consumes_permits_correctly)
{
    gate_stress_ctx c = {0};

    IHC_ASSERT(ic_gate_init(&c.gate) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < GATE_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        ic_task threads[GATE_THREADS];

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == GATE_THREADS);

        c.wakes = 0;

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], gate_worker, &c) == IC_CONCURRENCY_OK);
        }

        ic_thread_sleep(2);

        IHC_CHECK(gate_stress_wakes(&c) == 0);

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_gate_signal_one(&c.gate) == IC_CONCURRENCY_OK);
        }

        for (int i = 0; i < GATE_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == GATE_THREADS);
    }

    IHC_CHECK(ic_gate_destroy(&c.gate) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

#define BC_STRESS_ITERS 200
#define BC_THREADS 8

typedef struct bc_ctx
{
    ic_broadcast bc;
    ic_mutex mutex;
    int wakes;           // guarded by mutex
    int thread_failures; // guarded by mutex
} bc_ctx;

static int bc_worker(void* arg)
{
    bc_ctx* c = arg;

    const int wait_result = ic_broadcast_wait(&c->bc);

    ic_mutex_lock(&c->mutex);
    if (wait_result != IC_CONCURRENCY_OK)
    {
        c->thread_failures++;
    }
    c->wakes++;
    ic_mutex_unlock(&c->mutex);

    return 0;
}

// Reads the wake count under the mutex, as worker threads may still be writing it
static int bc_wakes(bc_ctx* const c)
{
    ic_mutex_lock(&c->mutex);
    const int wakes = c->wakes;
    ic_mutex_unlock(&c->mutex);
    return wakes;
}

IHC_TEST(stress_broadcast_deterministic_signal_reset_interleave)
{
    bc_ctx c = {0};

    IHC_ASSERT(ic_broadcast_init(&c.bc) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < BC_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        ic_task threads[BC_THREADS];

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);

            if ((i % 2) == 0)
            {
                ic_thread_sleep(1);
            }
        }

        for (int i = 0; i < BC_THREADS; i++)
        {
            if ((i % 3) == 1)
            {
                IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);
            }
            else
            {
                IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);
            }

            ic_thread_sleep(1);
        }

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);
    }

    IHC_CHECK(c.thread_failures == 0);
    IHC_CHECK(ic_broadcast_destroy(&c.bc) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_broadcast_signal_before_wait_deterministic)
{
    bc_ctx c = {0};

    IHC_ASSERT(ic_broadcast_init(&c.bc) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < BC_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        if ((iter % 2) == 0)
        {
            IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);
        }

        ic_task threads[BC_THREADS];

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);

            if ((i % 3) == 0)
            {
                ic_thread_sleep(1);
            }
        }

        for (int i = 0; i < BC_THREADS; i++)
        {
            if ((i % 2) == 0)
            {
                IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);
            }
            else
            {
                IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);
            }

            ic_thread_sleep(1);
        }

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);
    }

    IHC_CHECK(c.thread_failures == 0);
    IHC_CHECK(ic_broadcast_destroy(&c.bc) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

IHC_TEST(stress_broadcast_reset_reblocks_waiters)
{
    bc_ctx c = {0};

    IHC_ASSERT(ic_broadcast_init(&c.bc) == IC_CONCURRENCY_OK);
    IHC_ASSERT(ic_mutex_init(&c.mutex) == IC_CONCURRENCY_OK);

    for (int iter = 0; iter < BC_STRESS_ITERS; iter++)
    {
        c.wakes = 0;

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        ic_task threads[BC_THREADS];

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);
        }

        ic_thread_sleep(2);

        IHC_CHECK(bc_wakes(&c) == 0);

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);

        IHC_ASSERT(ic_broadcast_reset(&c.bc) == IC_CONCURRENCY_OK);

        c.wakes = 0;

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_init(&threads[i], bc_worker, &c) == IC_CONCURRENCY_OK);
        }

        ic_thread_sleep(2);

        IHC_CHECK(bc_wakes(&c) == 0);

        IHC_ASSERT(ic_broadcast_signal_all(&c.bc) == IC_CONCURRENCY_OK);

        for (int i = 0; i < BC_THREADS; i++)
        {
            IHC_ASSERT(ic_task_join(&threads[i]) == IC_CONCURRENCY_OK);
        }

        IHC_CHECK(c.wakes == BC_THREADS);
    }

    IHC_CHECK(c.thread_failures == 0);
    IHC_CHECK(ic_broadcast_destroy(&c.bc) == IC_CONCURRENCY_OK);
    IHC_CHECK(ic_mutex_destroy(&c.mutex) == IC_CONCURRENCY_OK);
}

#endif // RUN_STRESS_TESTS

// ==============================================================================
// TASK POOL TESTS
// ==============================================================================

#define TASKPOOL_THREADS 4
IC_STATIC_ASSERT((TASKPOOL_THREADS > 0) && (TASKPOOL_THREADS <= PW_TASKPOOL_MAX_THREADS), "TASKPOOL_THREADS must be in legal range");
#define TASKPOOL_TASKS 64
IC_STATIC_ASSERT((TASKPOOL_TASKS > 0) && (TASKPOOL_TASKS <= PW_TASKPOOL_MAX_PENDING_TASKS + PW_TASKPOOL_MAX_THREADS), "TASKPOOL_TASKS must be in legal range");
#define TASK_TEST_WAIT_TIMEOUT (100 * 1000)

static void taskpool_increment(void* arg)
{
    ic_atomic_i32* counter = (ic_atomic_i32*)arg;
    (void)ic_atomic_fetch_add(counter, 1);
}

IHC_TEST(verify_task_pool_executes_all_tasks_with_completion)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));

    ic_atomic_i32 counter = ic_make_atomic(0);
    PwTaskCompletion completions[TASKPOOL_TASKS];
    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        AppError err = pw_task_pool_submit(&pool, taskpool_increment, &counter, &completions[i]);
        IHC_CHECK(AppError_eq(err, AppError_NoError));
    }

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        AppError err = pw_task_completion_wait(&completions[i], 1, TASK_TEST_WAIT_TIMEOUT);
        IHC_CHECK(AppError_eq(err, AppError_NoError));
    }

    IHC_CHECK(ic_atomic_load(&counter) == TASKPOOL_TASKS);

    IHC_ASSERT(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));
    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

IHC_TEST(verify_task_pool_fire_and_forget_executes_tasks)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));
    
    ic_atomic_i32 counter = ic_make_atomic(0);
    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        AppError err = pw_task_pool_submit(&pool, taskpool_increment, &counter, NULL);
        IHC_CHECK(AppError_eq(err, AppError_NoError));
    }

    ic_thread_sleep(10);

    IHC_ASSERT(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));
    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

typedef struct taskpool_order_ctx
{
    ic_mutex mutex;
    int last;
    int ok;
} taskpool_order_ctx;

static void taskpool_order_worker(void* arg)
{
    taskpool_order_ctx* ctx = (taskpool_order_ctx*)arg;

    ic_mutex_lock(&ctx->mutex);

    if (ctx->last > 0)
    {
        ctx->ok = 0;
    }

    ctx->last++;

    ic_mutex_unlock(&ctx->mutex);
}

IHC_TEST(verify_task_pool_processes_tasks_consistently)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));

    taskpool_order_ctx ctx = {0};

    IHC_ASSERT(ic_mutex_init(&ctx.mutex) == IC_CONCURRENCY_OK);

    ctx.last = 0;
    ctx.ok = 1;

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        AppError err = pw_task_pool_submit(&pool, taskpool_order_worker, &ctx, NULL);
        IHC_CHECK(AppError_eq(err, AppError_NoError));
    }

    ic_thread_sleep(10);

    IHC_ASSERT(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));
    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));

    ic_mutex_destroy(&ctx.mutex);
}

static void do_nothing(void* arg)
{
    (void)arg;
}

IHC_TEST(verify_task_pool_rejects_null_arguments)
{
    IHC_ASSERT(AppError_eq(pw_task_pool_init(NULL, TASKPOOL_THREADS), AppError_NullRef));
    IHC_ASSERT(AppError_eq(pw_task_pool_close(NULL, PwTaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT), AppError_NullRef));
    IHC_ASSERT(AppError_eq(pw_task_pool_close(NULL, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), AppError_NullRef));
    IHC_ASSERT(AppError_eq(pw_task_pool_destroy(NULL), AppError_NullRef));
    IHC_ASSERT(AppError_eq(pw_task_pool_submit(NULL, taskpool_increment, NULL, NULL), AppError_NullRef));
    
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));
    IHC_CHECK(AppError_eq(pw_task_pool_submit(&pool, NULL, NULL, NULL), AppError_NullRef));
    PwTaskCompletion completion;
    IHC_CHECK(AppError_eq(pw_task_pool_submit(&pool, do_nothing, NULL, &completion), AppError_NoError));
    IHC_ASSERT(AppError_eq(pw_task_completion_wait(NULL, 1, TASK_TEST_WAIT_TIMEOUT), AppError_NullRef));
    IHC_ASSERT(AppError_eq(pw_task_completion_wait(&completion, -1, TASK_TEST_WAIT_TIMEOUT), AppError_Argument));
    IHC_ASSERT(AppError_eq(pw_task_completion_wait(&completion, 1, -2), AppError_Argument)); // -1 is PW_TASK_COMPLETION_WAIT_FOREVER

    IHC_ASSERT(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));
    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

IHC_TEST(verify_task_completion_wait_times_out_and_resolves)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));
    ic_atomic_i32 counter = ic_make_atomic(0);

    PwTaskCompletion completion;
    AppError err = pw_task_pool_submit(&pool, taskpool_increment, &counter, &completion);
    IHC_CHECK(AppError_eq(err, AppError_NoError));

    err = pw_task_completion_wait(&completion, 1, TASK_TEST_WAIT_TIMEOUT);
    IHC_CHECK(AppError_eq(err, AppError_NoError));

    IHC_ASSERT(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));
    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

typedef struct taskpool_lifecycle_ctx
{
    ic_atomic_i32 counter;
} taskpool_lifecycle_ctx;

static void taskpool_lifecycle_worker(void* arg)
{
    taskpool_lifecycle_ctx* ctx = (taskpool_lifecycle_ctx*)arg;
    (void)ic_atomic_fetch_add(&ctx->counter, 1);
}

IHC_TEST(verify_task_pool_close_state_transitions)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));

    taskpool_lifecycle_ctx ctx;
    ctx.counter = ic_make_atomic(0);

    PwTaskCompletion completions[TASKPOOL_TASKS];

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        AppError err = pw_task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, &completions[i]);
        IHC_CHECK(AppError_eq(err, AppError_NoError));
    }

    // Wait for all tasks to finish execution (completion correctness test)
    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        IHC_CHECK(AppError_eq(pw_task_completion_wait(&completions[i], 1, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));
    }

    // Drain queue AND finish active tasks, but don't allow new submissions
    IHC_CHECK(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));

    // Submissions must now fail
    AppError err = pw_task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, NULL);
    IHC_CHECK(AppError_eq(err, AppError_InvalidState));

    IHC_CHECK(ic_atomic_load(&ctx.counter) == TASKPOOL_TASKS);

    err = pw_task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, NULL);
    IHC_CHECK(AppError_eq(err, AppError_InvalidState));

    // Finish active tasks, but don't execute any pending tasks, and don't allow new submissions
    IHC_CHECK(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));

    err = pw_task_pool_close(&pool, PwTaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT);
    IHC_ASSERT(AppError_eq(err, AppError_InvalidState));

    err = pw_task_pool_submit(&pool, taskpool_lifecycle_worker, &ctx, NULL);
    IHC_CHECK(AppError_eq(err, AppError_InvalidState));

    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

#define TASKPOOL_CLOSE_TEST_TASKS 32

typedef struct taskpool_close_test_ctx
{
    ic_atomic_i32 started;
    ic_atomic_i32 completed;
} taskpool_close_test_ctx;

static void taskpool_slow_worker(void* arg)
{
    taskpool_close_test_ctx* ctx = (taskpool_close_test_ctx*)arg;

    ic_atomic_fetch_add(&ctx->started, 1);

    pw_thread_sleep_milliseconds(50);

    ic_atomic_fetch_add(&ctx->completed, 1);
}

IHC_TEST(verify_task_pool_close_drain_completes_all_tasks)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));

    taskpool_close_test_ctx ctx = {0};
    ctx.started = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);

    PwTaskCompletion completions[TASKPOOL_CLOSE_TEST_TASKS];

    for (int i = 0; i < TASKPOOL_CLOSE_TEST_TASKS; i++)
    {
        IHC_ASSERT(AppError_eq( 
            pw_task_pool_submit(&pool, taskpool_slow_worker, &ctx, &completions[i]),
            AppError_NoError
        ));
    }

    while (ic_atomic_load(&ctx.started) < 4)
    {
        ic_thread_sleep(1);
    }

    IHC_ASSERT(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT),
        AppError_NoError
    ));

    IHC_CHECK(ic_atomic_load(&ctx.started) == TASKPOOL_CLOSE_TEST_TASKS);
    IHC_CHECK(ic_atomic_load(&ctx.completed) == TASKPOOL_CLOSE_TEST_TASKS);

    IHC_ASSERT(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

IHC_TEST(verify_task_pool_close_abort_drops_queued_tasks)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));

    taskpool_close_test_ctx ctx = {0};
    ctx.started = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);

    for (int i = 0; i < TASKPOOL_CLOSE_TEST_TASKS; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_pool_submit(&pool, taskpool_slow_worker, &ctx, NULL),
            AppError_NoError
        ));
    }

    while (ic_atomic_load(&ctx.started) < 4)
    {
        ic_thread_sleep(1);
    }

    IHC_ASSERT(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT),
        AppError_NoError
    ));

    int started = ic_atomic_load(&ctx.started);
    int completed = ic_atomic_load(&ctx.completed);

    // abort must NOT guarantee queue completion
    IHC_CHECK(completed <= TASKPOOL_CLOSE_TEST_TASKS);

    // but anything that started must finish
    IHC_CHECK(completed == started);

    IHC_ASSERT(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

static void worker_stuck_until_signaled(void* arg)
{
    ic_broadcast* broadcast = (ic_broadcast*)arg;
    ic_broadcast_wait(broadcast);
}

IHC_TEST(verify_task_pool_reaches_timeout_for_blocking_tasks)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));
    ic_broadcast broadcast;
    IHC_ASSERT(ic_broadcast_init(&broadcast) == IC_CONCURRENCY_OK);

    PwTaskCompletion first_completion;
    IHC_ASSERT(AppError_eq(
        pw_task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, &first_completion),
        AppError_NoError
    ));
    for (int i = 1; i < TASKPOOL_TASKS; i++) // Note that it starts at 1 since we already submitted one above
    {
        IHC_ASSERT(AppError_eq(
            pw_task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL),
            AppError_NoError
        ));
    }

    // If the close call doesn't time out correctly, this test will hang instead of fail
    IHC_CHECK(AppError_eq(
        pw_task_completion_wait(&first_completion, 1, 100),
        AppError_Timeout
    ));
    // A zero retry period must still time out instead of spinning forever
    IHC_CHECK(AppError_eq(
        pw_task_completion_wait(&first_completion, 0, 50),
        AppError_Timeout
    ));
    // A zero timeout only checks once
    IHC_CHECK(AppError_eq(
        pw_task_completion_wait(&first_completion, 1, 0),
        AppError_Timeout
    ));
    IHC_CHECK(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Drain, 100),
        AppError_Timeout
    ));
    IHC_CHECK(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Abort, 100),
        AppError_Timeout
    ));


    // Now signal the tasks so they can finish and not leak
    IHC_ASSERT(ic_broadcast_signal_all(&broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT),
        AppError_NoError
    ));

    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
    IHC_CHECK(ic_broadcast_destroy(&broadcast) == IC_CONCURRENCY_OK);
}

IHC_TEST(verify_task_pool_abort_marks_queued_completions_as_interrupted)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));
    ic_broadcast broadcast;
    IHC_ASSERT(ic_broadcast_init(&broadcast) == IC_CONCURRENCY_OK);

    // Occupy every worker, so the last task can only ever be queued, never running
    for (int i = 0; i < TASKPOOL_THREADS; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL),
            AppError_NoError
        ));
    }
    PwTaskCompletion queued_completion;
    IHC_ASSERT(AppError_eq(
        pw_task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, &queued_completion),
        AppError_NoError
    ));

    // The close itself times out on the stuck active tasks, but must drop the queued one at once
    (void)pw_task_pool_close(&pool, PwTaskPoolClose_Abort, 0);
    IHC_CHECK(AppError_eq(
        pw_task_completion_wait(&queued_completion, 1, 0),
        AppError_Interrupted
    ));

    IHC_ASSERT(ic_broadcast_signal_all(&broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Abort, PW_TASK_COMPLETION_WAIT_FOREVER),
        AppError_NoError
    ));
    IHC_CHECK(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
    IHC_CHECK(ic_broadcast_destroy(&broadcast) == IC_CONCURRENCY_OK);
}

IHC_TEST(verify_task_pool_can_safely_be_destroyed_without_manually_closing)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, TASKPOOL_THREADS), AppError_NoError));

    taskpool_close_test_ctx ctx = {0};
    ctx.started = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);

    for (int i = 0; i < TASKPOOL_TASKS; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_pool_submit(&pool, taskpool_slow_worker, &ctx, NULL),
            AppError_NoError
        ));
    }

    
    // Don't manually close the pool - just destroy it directly. 
    // This should abort active tasks but not finish queued tasks.
    IHC_ASSERT(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
    
    const int32_t started = ic_atomic_load(&ctx.started);
    const int32_t completed = ic_atomic_load(&ctx.completed);
    IHC_CHECK(completed <= TASKPOOL_TASKS);
    IHC_CHECK(completed == started);
}

IHC_TEST(verify_task_pool_can_fill_up_and_safely_reject_excess_tasks)
{
    PwTaskPool pool;
    IHC_ASSERT(AppError_eq(pw_task_pool_init(&pool, PW_TASKPOOL_MAX_THREADS), AppError_NoError));
    ic_broadcast broadcast;
    IHC_ASSERT(ic_broadcast_init(&broadcast) == IC_CONCURRENCY_OK);

    for (int i = 0; i < PW_TASKPOOL_MAX_THREADS; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL),
            AppError_NoError
        ));
    }
    ic_thread_sleep(10); // ensure all worker threads have started and are blocking on the broadcast
    for (int i = 0; i < PW_TASKPOOL_MAX_PENDING_TASKS; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL),
            AppError_NoError
        ));
    }

    // This is the real check to validate the test
    AppError err = pw_task_pool_submit(&pool, worker_stuck_until_signaled, &broadcast, NULL);
    IHC_CHECK(AppError_eq(err, AppError_OutOfBounds));

    IHC_ASSERT(ic_broadcast_signal_all(&broadcast) == IC_CONCURRENCY_OK);
    IHC_ASSERT(AppError_eq(pw_task_pool_close(&pool, PwTaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT), AppError_NoError));
    IHC_ASSERT(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
    IHC_ASSERT(ic_broadcast_destroy(&broadcast) == IC_CONCURRENCY_OK);
}

// ==============================================================================
// TASK POOL STRESS TESTS
// ==============================================================================
#if RUN_STRESS_TESTS

#define TASKPOOL_STRESS_THREADS 8
#define TASKPOOL_STRESS_TASKS_PER_THREAD 10000

typedef struct taskpool_stress_submit_ctx
{
    PwTaskPool* pool;
    ic_atomic_i32 submitted;
    ic_atomic_i32 completed;
    ic_atomic_i32 unexpected_errors; // checked on the main thread after joining
} taskpool_stress_submit_ctx;

static void stress_increment_worker(void* arg)
{
    taskpool_stress_submit_ctx* ctx = (taskpool_stress_submit_ctx*)arg;
    ic_atomic_fetch_add(&ctx->completed, 1);
}

static int stress_submitter_thread(void* arg)
{
    taskpool_stress_submit_ctx* ctx = (taskpool_stress_submit_ctx*)arg;

    for (int i = 0; i < TASKPOOL_STRESS_TASKS_PER_THREAD; i++)
    {
        AppError err = pw_task_pool_submit(
            ctx->pool,
            stress_increment_worker,
            ctx,
            NULL
        );

        // Pool saturation is acceptable during stress
        if (AppError_eq(err, AppError_NoError))
        {
            ic_atomic_fetch_add(&ctx->submitted, 1);
        }
        else if (!AppError_eq(err, AppError_OutOfBounds))
        {
            ic_atomic_fetch_add(&ctx->unexpected_errors, 1);
        }
    }

    return 0;
}

IHC_TEST(stress_task_pool_handles_high_contention_submissions)
{
    PwTaskPool pool;

    IHC_ASSERT(AppError_eq(
        pw_task_pool_init(&pool, PW_TASKPOOL_MAX_THREADS),
        AppError_NoError
    ));

    taskpool_stress_submit_ctx ctx = {0};

    ctx.pool = &pool;
    ctx.submitted = ic_make_atomic(0);
    ctx.completed = ic_make_atomic(0);
    ctx.unexpected_errors = ic_make_atomic(0);

    PwTask submitters[TASKPOOL_STRESS_THREADS];

    for (int i = 0; i < TASKPOOL_STRESS_THREADS; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_init(&submitters[i], stress_submitter_thread, &ctx),
            AppError_NoError
        ));
    }

    for (int i = 0; i < TASKPOOL_STRESS_THREADS; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_join(&submitters[i]),
            AppError_NoError
        ));
    }
    IHC_CHECK(ic_atomic_load(&ctx.unexpected_errors) == 0);

    IHC_ASSERT(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT),
        AppError_NoError
    ));

    const int32_t submitted = ic_atomic_load(&ctx.submitted);
    const int32_t completed = ic_atomic_load(&ctx.completed);

    IHC_CHECK(completed == submitted);

    IHC_ASSERT(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

typedef struct shutdown_race_ctx
{
    PwTaskPool* pool;
    ic_atomic_i32 successful_submits;
    ic_atomic_i32 failed_submits;
    ic_atomic_i32 executed;
    ic_atomic_i32 unexpected_errors; // checked on the main thread after joining
} shutdown_race_ctx;

static void shutdown_race_worker(void* arg)
{
    shutdown_race_ctx* ctx = (shutdown_race_ctx*)arg;

    pw_thread_sleep_milliseconds(1);

    ic_atomic_fetch_add(&ctx->executed, 1);
}

static int shutdown_race_submitter(void* arg)
{
    shutdown_race_ctx* ctx = (shutdown_race_ctx*)arg;

    for (int i = 0; i < 10000; i++)
    {
        AppError err = pw_task_pool_submit(
            ctx->pool,
            shutdown_race_worker,
            ctx,
            NULL
        );

        if (AppError_eq(err, AppError_NoError))
        {
            ic_atomic_fetch_add(&ctx->successful_submits, 1);
        }
        else
        {
            // During shutdown or saturation these are valid
            if (!AppError_eq(err, AppError_OutOfBounds) && !AppError_eq(err, AppError_InvalidState))
            {
                ic_atomic_fetch_add(&ctx->unexpected_errors, 1);
            }

            ic_atomic_fetch_add(&ctx->failed_submits, 1);
        }
    }

    return 0;
}

IHC_TEST(stress_task_pool_survives_shutdown_submission_races)
{
    PwTaskPool pool;

    IHC_ASSERT(AppError_eq(
        pw_task_pool_init(&pool, PW_TASKPOOL_MAX_THREADS),
        AppError_NoError
    ));

    shutdown_race_ctx ctx = {0};

    ctx.pool = &pool;
    ctx.successful_submits = ic_make_atomic(0);
    ctx.failed_submits = ic_make_atomic(0);
    ctx.executed = ic_make_atomic(0);
    ctx.unexpected_errors = ic_make_atomic(0);

    PwTask submitters[4];

    for (int i = 0; i < 4; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_init(&submitters[i], shutdown_race_submitter, &ctx),
            AppError_NoError
        ));
    }

    pw_thread_sleep_milliseconds(10);

    IHC_ASSERT(AppError_eq(
        pw_task_pool_close(&pool, PwTaskPoolClose_Abort, TASK_TEST_WAIT_TIMEOUT),
        AppError_NoError
    ));

    for (int i = 0; i < 4; i++)
    {
        IHC_ASSERT(AppError_eq(
            pw_task_join(&submitters[i]),
            AppError_NoError
        ));
    }
    IHC_CHECK(ic_atomic_load(&ctx.unexpected_errors) == 0);

    const int32_t successful = ic_atomic_load(&ctx.successful_submits);
    const int32_t executed = ic_atomic_load(&ctx.executed);

    // Abort guarantees active jobs finish
    IHC_CHECK(executed <= successful);

    IHC_ASSERT(AppError_eq(pw_task_pool_destroy(&pool), AppError_NoError));
}

#define TASKPOOL_LIFECYCLE_ITERATIONS 1000

IHC_TEST(stress_task_pool_survives_repeated_lifecycle_cycles)
{
    for (int iteration = 0; iteration < TASKPOOL_LIFECYCLE_ITERATIONS; iteration++)
    {
        PwTaskPool pool;

        IHC_ASSERT(AppError_eq(
            pw_task_pool_init(&pool, TASKPOOL_THREADS),
            AppError_NoError
        ));

        ic_atomic_i32 counter = ic_make_atomic(0);

        for (int i = 0; i < TASKPOOL_TASKS; i++)
        {
            AppError err = pw_task_pool_submit(
                &pool,
                taskpool_increment,
                &counter,
                NULL
            );

            if (!AppError_eq(err, AppError_NoError))
            {
                IHC_CHECK(AppError_eq(err, AppError_OutOfBounds));
            }
        }

        IHC_ASSERT(AppError_eq(
            pw_task_pool_close(&pool, PwTaskPoolClose_Drain, TASK_TEST_WAIT_TIMEOUT),
            AppError_NoError
        ));

        IHC_ASSERT(AppError_eq(
            pw_task_pool_destroy(&pool),
            AppError_NoError
        ));
    }
}

#endif // RUN_STRESS_TESTS

#endif // IRON_HAMMER_C_TESTS_CONCURRENCY_SIGNAL_TEST_H
