/**
 * @file label.cpp
 *
 * Label collected positions: dedupe the records of <in.raw>, pick n at random, search each with Edax
 * (exactly by default) and write them with their score.
 * usage: label <in.raw> <out.bin> <n> <threads> [-seed s] [-level L (default 60 = exact)] [-eval file (default eval_orig.dat)]
 */
#define main edax_original_main
#include "all.cpp"
#undef main
#include <omp.h>
#include <atomic>
#include <cerrno>

#include "evtool.hpp"

using evtool::Rec;
using evtool::mix64;

static int cmp_rec(const void *a, const void *b)
{
	const Rec *x = (const Rec *) a, *y = (const Rec *) b;
	if (x->P != y->P) return x->P < y->P ? -1 : 1;
	if (x->O != y->O) return x->O < y->O ? -1 : 1;
	return 0;
}

static int level = 60;

static int solve(Search *search, Board b)
{
	int sign = 1;
	if (!board_get_moves(&b)) {
		board_pass(&b);
		if (!board_get_moves(&b)) {	// game over (from the original player's view: b is swapped)
			const int o = bit_count(b.player), p = bit_count(b.opponent), e = 64 - o - p;
			return p > o ? p - o + e : p < o ? p - o - e : 0;
		}
		sign = -1;
	}
	search_cleanup(search);
	search_set_board(search, &b, BLACK);
	search_set_level(search, level, search->eval.n_empties);
	search_set_game_time(search, TIME_MAX);
	search_run(search);
	return sign * search->result->score;
}

int main(int argc, char **argv)
{
	if (argc < 5) evtool::die("usage: label <in.raw> <out.bin> <n> <threads> [-seed s] [-level L] [-eval file]");
	const char *in = argv[1], *outname = argv[2];
	char *end;
	errno = 0;
	long long n_want = strtoll(argv[3], &end, 10);
	if (end == argv[3] || *end != '\0' || errno == ERANGE || n_want < 1) evtool::die("bad n %s (1 or more records to label)", argv[3]);
	const int nt = atoi(argv[4]);
	unsigned long long seed = 1;
	const char *eval_file = "eval_orig.dat";
	auto value = [&](int &i) -> char * { if (i + 1 >= argc) evtool::die("missing value of %s", argv[i]); return argv[++i]; };
	for (int i = 5; i < argc; ++i) {
		if (!strcmp(argv[i], "-seed")) seed = strtoull(value(i), nullptr, 10);
		else if (!strcmp(argv[i], "-level")) level = atoi(value(i));
		else if (!strcmp(argv[i], "-eval")) eval_file = value(i);
		else evtool::die("unknown option %s", argv[i]);
	}
	if (nt < 1) evtool::die("bad threads %s", argv[4]);

	options.n_task = 1;
	options.hash_table_size = 20;
	options.verbosity = 0;
	options.eval_file = (char *) eval_file;
	bit_init();
	edge_stability_init();
	statistics_init();
	eval_open(options.eval_file);
	search_global_init();

	FILE *f = evtool::xfopen(in, "rb");
	long long n = evtool::rec_count(f, in);
	if ((unsigned long long) n > SIZE_MAX / sizeof (Rec)) evtool::die("%s holds too many records", in);
	Rec *R = (Rec *) evtool::xmalloc((size_t) n * sizeof *R);
	long long n_read = 0;
	while (n_read < n && evtool::rec_read(R + n_read, (size_t) (n - n_read), f, in, &n_read) > 0) {}
	if (n_read != n) evtool::die("cannot read %s (the file changed)", in);
	fclose(f);
	FILE *g = evtool::xfopen(outname, "wb");
	for (long long i = 0; i < n; ++i) R[i].v = 0;
	qsort(R, n, sizeof *R, cmp_rec);
	long long u = 0;
	for (long long i = 0; i < n; ++i) if (u == 0 || cmp_rec(R + i, R + u - 1)) R[u++] = R[i];
	for (long long i = u - 1; i > 0; --i) {
		const long long j = (long long) (mix64(seed * 0x9E3779B97F4A7C15ULL + i) % (unsigned long long) (i + 1));
		const Rec t = R[i]; R[i] = R[j]; R[j] = t;
	}
	if (n_want > u) n_want = u;
	fprintf(stderr, "%s: %lld records, %lld unique, labelling %lld\n", in, n, u, n_want);

	const double t0 = omp_get_wtime();
	std::atomic<long long> done{0};	// the records labelled (read by thread 0 for the progress)
	#pragma omp parallel num_threads(nt)
	{
		Search search;
		search_init(&search);
		search.options.verbosity = 0;
		long long k;
		#pragma omp for schedule(dynamic, 8)
		for (k = 0; k < n_want; ++k) {
			Board b; b.player = R[k].P; b.opponent = R[k].O;
			R[k].v = (signed char) solve(&search, b);
			const long long now = ++done;
			if (omp_get_thread_num() == 0 && k % 5000 == 0) fprintf(stderr, "%lld / %lld, %.0f s\n", now, n_want, omp_get_wtime() - t0);
		}
		search_free(&search);
	}
	const bool written = fwrite(R, sizeof *R, (size_t) n_want, g) == (size_t) n_want;
	if (fclose(g) != 0 || !written) evtool::die("cannot write %s", outname);
	printf("%s: %lld labelled in %.0f s\n", outname, n_want, omp_get_wtime() - t0);
	return 0;
}
