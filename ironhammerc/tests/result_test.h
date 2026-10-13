#ifndef IRON_HAMMER_C_TESTS_RESULT_TEST_H
#define IRON_HAMMER_C_TESTS_RESULT_TEST_H

#include "ic_hammer.h"
#include "ironclib/ic_result.h"
#include "ironclib/ic_inline.h"
#include "docs/premade/global_result.h"

IC_RESULT_TYPE(CharTest, char, int)

IHC_TEST(verify_generation_of_simple_result_type_and_generated_functions)
{
    const CharTest ok_char = CharTest_ok('c');
    IHC_ASSERT(ok_char.ok);
    IHC_CHECK(ok_char.data.value == 'c');

    const CharTest err_char = CharTest_err(9);
    IHC_ASSERT(!err_char.ok);
    IHC_CHECK(err_char.data.error == 9);
}

IHC_TEST(verify_ok_and_err_functions_work_for_typenum_error) 
{
    const IntResult ok_res = IntResult_ok(7);
    IHC_ASSERT(ok_res.ok);
    IHC_CHECK(ok_res.data.value == 7);

    const IntResult err_res = IntResult_err(AppError_Runtime);
    IHC_ASSERT(!err_res.ok);
    IHC_CHECK(AppError_eq(err_res.data.error, AppError_Runtime));
}

IHC_TEST(verify_optional_accessor_macros_work)
{
    const DoubleResult ok_res = DoubleResult_ok(42.1337);
    IHC_ASSERT(IC_RESULT_IS_OK(ok_res));
    IHC_CHECK(IC_RESULT_VALUE(ok_res) == 42.1337);

    const DoubleResult err_res = DoubleResult_err(AppError_Argument);
    IHC_ASSERT(!IC_RESULT_IS_OK(err_res));
    IHC_CHECK(AppError_eq(IC_RESULT_ERROR(err_res), AppError_Argument));
}

IC_HEADER_FUNC IntResult test_try_propagation(const AppError expected_return_error)
{
    const StringViewResult success_res = StringViewResult_ok("This is good and will not return early");
    IC_TRY_RETURN_ERR_AS(IntResult, success_res);

    const FloatResult fail_res = FloatResult_err(expected_return_error);
    IC_TRY_RETURN_ERR_AS(IntResult, fail_res);

    return IntResult_ok(99);
}

IHC_TEST(verify_try_macro_propagates_error_in_same_error_type_context)
{
    const AppError expected_error = AppError_CorruptData;
    const IntResult err_prop = test_try_propagation(expected_error);

    IHC_ASSERT(!err_prop.ok);
    IHC_CHECK(AppError_eq(expected_error, err_prop.data.error));
}

#if IC_TRY_SINGLE_EVALUATION
static int try_evaluation_count = 0;

static FloatResult counted_failing_call(void)
{
    try_evaluation_count++;
    return FloatResult_err(AppError_Timeout);
}

static IntResult try_with_function_call(void)
{
    IC_TRY_RETURN_ERR_AS(IntResult, counted_failing_call());
    return IntResult_ok(1);
}
#endif

IHC_TEST(verify_try_macro_evaluates_result_once_when_supported)
{
#if IC_TRY_SINGLE_EVALUATION
    try_evaluation_count = 0;
    const IntResult res = try_with_function_call();
    IHC_ASSERT(!res.ok);
    IHC_CHECK(AppError_eq(res.data.error, AppError_Timeout));
    IHC_CHECK(try_evaluation_count == 1); // the fallback would call it twice on the error path
#else
    IHC_CHECK(1); // fallback build: result is documented to be evaluated twice on error
#endif
}

#endif // IRON_HAMMER_C_TESTS_RESULT_TEST_H
