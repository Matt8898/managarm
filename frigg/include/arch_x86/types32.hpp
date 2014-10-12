
static_assert(sizeof(short) == 2, "Unexpected architecture");
static_assert(sizeof(int) == 4, "Unexpected architecture");
static_assert(sizeof(long long) == 8, "Unexpected architecture");
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

static_assert(sizeof(void *) == 4, "Unexpected architecture");
typedef uint32_t uintptr_t;

typedef uint32_t size_t;

