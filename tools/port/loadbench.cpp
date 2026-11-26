// Load a book file several times and print the CPU time of each load (user and kernel time of the thread,
// cycles of the thread). Built against the sources of a tree: /I<tree>\src.
// usage: loadbench <book file> <loads> [save file [form]]
#define main edax_original_main
#include "all.cpp"
#undef main
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

static bool thread_times(double *user, double *kernel, unsigned long long *cycles)
{
	FILETIME c, e, k, u;
	ULARGE_INTEGER x, y;
	if (!GetThreadTimes(GetCurrentThread(), &c, &e, &k, &u)) return false;
	x.LowPart = u.dwLowDateTime; x.HighPart = u.dwHighDateTime;
	y.LowPart = k.dwLowDateTime; y.HighPart = k.dwHighDateTime;
	*user = x.QuadPart * 1e-7;
	*kernel = y.QuadPart * 1e-7;
	return QueryThreadCycleTime(GetCurrentThread(), cycles) != 0;
}

int main(int argc, char **argv)
{
	if (argc < 3) return 1;
	bit_init();
	options.verbosity = 0;
	options.info = false;
	char *end;
	errno = 0;
	const long n = strtol(argv[2], &end, 10);
	if (*end || n < 1 || n > INT_MAX || errno == ERANGE) return 1;
	if (argc > 4) options.book_format = atoi(argv[4]); // form of the saved file
	for (int i = 0; i < n; ++i) {
		Book b = {};
		double u0, k0, u1, k1;
		unsigned long long c0, c1;
		if (!thread_times(&u0, &k0, &c0)) return 1;
		const bool ok = book_load(&b, argv[1]);
		if (!thread_times(&u1, &k1, &c1)) return 1;
		PROCESS_MEMORY_COUNTERS pmc;
		if (!ok || !GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc)) return 1;
		printf("load %d ok %d nodes %u buckets %d user %.3f kernel %.3f peak_MiB %.1f gcycles %.3f\n", i, ok, b.n_nodes, b.n, u1 - u0, k1 - k0, pmc.PeakWorkingSetSize / 1048576.0, (c1 - c0) * 1e-9);
		if (argc > 3 && i == 0) {
			if (!thread_times(&u0, &k0, &c0)) return 1;
			const bool saved = book_save(&b, argv[3]);
			if (!saved) return 1;
			if (!thread_times(&u1, &k1, &c1)) return 1;
			printf("save ok %d user %.3f kernel %.3f gcycles %.3f\n", saved, u1 - u0, k1 - k0, (c1 - c0) * 1e-9);
		}
		book_free(&b);
	}
	return 0;
}
