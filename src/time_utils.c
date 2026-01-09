#include <stdint.h>
#include <time.h>
#include <time_utils.h>

int64_t get_timestamp(void) {
	return (int64_t)time(NULL);
}
