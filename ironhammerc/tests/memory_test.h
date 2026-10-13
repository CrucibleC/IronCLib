#ifndef IRON_HAMMER_C_TESTS_MEMORY_TEST_H
#define IRON_HAMMER_C_TESTS_MEMORY_TEST_H

#include "ic_hammer.h"
#include "ic_memory.h"

IHC_TEST(verify_memory_protects_against_negative_size) 
{
    int* const negative_size_array = IC_MALLOC_ARRAY(int, -5);
    IHC_CHECK(negative_size_array == NULL);
    free(negative_size_array); // in case memory was allocated
}

IHC_TEST(verify_memory_protects_against_zero_size) 
{
    int* const zero_size_array = IC_MALLOC_ARRAY(int, 0);
    IHC_CHECK(zero_size_array == NULL);
    free(zero_size_array); // in case memory was allocated
}

IHC_TEST(verify_memory_protects_against_overflow) 
{
    const size_t max_size = (size_t)-1;
    double* const overflow_array = IC_MALLOC_ARRAY(double, (max_size / sizeof(double)) + 1);
    IHC_CHECK(overflow_array == NULL);
    free(overflow_array); // in case memory was allocated
}

IHC_TEST(verify_memory_allocates_valid_array) 
{
    char* const valid_array = IC_MALLOC_ARRAY(char, 10);
    IHC_CHECK(valid_array != NULL);
    valid_array[0] = 'H';
    valid_array[1] = 'i';
    IHC_CHECK(valid_array[0] == 'H');
    IHC_CHECK(valid_array[1] == 'i');
    free(valid_array);
}

IHC_TEST(verify_memory_handles_every_integer_size_type)
{
    // Negative values of every signed type are rejected, whatever their width
    const signed char negative_schar = -1;
    const long long negative_llong = -1000000000000LL;
    IHC_CHECK(IC_MALLOC_ARRAY(int, negative_schar) == NULL);
    IHC_CHECK(IC_MALLOC_ARRAY(int, negative_llong) == NULL);

    // Plain char is its own type in ISO C (MSVC treats it as signed/unsigned char instead)
    const char char_size = 4;
    int* const from_char = IC_MALLOC_ARRAY(int, char_size);
    IHC_CHECK(from_char != NULL);
    free(from_char);

    // Qualified (const) and unsigned sizes allocate normally
    const unsigned short ushort_size = 4;
    const unsigned long long ullong_size = 4;
    const size_t size_t_size = 4;
    int* const from_ushort = IC_MALLOC_ARRAY(int, ushort_size);
    int* const from_ullong = IC_MALLOC_ARRAY(int, ullong_size);
    int* const from_size_t = IC_MALLOC_ARRAY(int, size_t_size);
    IHC_CHECK(from_ushort != NULL);
    IHC_CHECK(from_ullong != NULL);
    IHC_CHECK(from_size_t != NULL);
    free(from_ushort);
    free(from_ullong);
    free(from_size_t);
}

IHC_TEST(verify_memory_evaluates_size_once_when_supported)
{
#if IC_MALLOC_ARRAY_SINGLE_EVALUATION
    int count = 3;
    int* const array = IC_MALLOC_ARRAY(int, count++);
    IHC_CHECK(array != NULL);
    IHC_CHECK(count == 4); // the fallback would increment twice
    free(array);
#else
    IHC_CHECK(1); // fallback build: size is documented to be evaluated more than once
#endif
}

#endif // IRON_HAMMER_C_TESTS_MEMORY_TEST_H
