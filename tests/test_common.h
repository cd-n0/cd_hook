#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_
#if defined(_MSC_VER)
#define NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define NO_INLINE __attribute__((noinline))
#endif
#endif // !TEST_COMMON_H_
