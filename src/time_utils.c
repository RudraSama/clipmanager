#include <stdint.h>
#include <time.h>
#include <time_utils.h>

int64_t get_timestamp(void) {
	return (int64_t)time(NULL);
}

uint64_t get_miliseconds(void) {
	struct timespec ts = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &ts);

	uint64_t nsec = ts.tv_nsec / 1000000;
	uint64_t sec = (uint64_t)ts.tv_sec * 1000;
	return nsec + sec;
}
