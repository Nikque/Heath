/**
 * @file sample.cpp
 *
 * Sample held-out (test split: hash % 32 == 0) positions of a data file as OBF lines (the values go to stderr).
 * usage: sample <file.bin> <n> <seed>  > out.obf
 */
#include <cerrno>

#include "evtool.hpp"

using namespace evtool;

int main(int argc, char **argv)
{
	if (argc < 4) die("usage: sample <file.bin> <n> <seed>");
	FILE *f = xfopen(argv[1], "rb");
	char *end;
	errno = 0;
	const long long n = strtoll(argv[2], &end, 10);	// (the records kept: at most n, in memory)
	if (end == argv[2] || *end != '\0' || errno == ERANGE || n < 1 || (unsigned long long) n > SIZE_MAX / sizeof (Rec)) die("bad n %s", argv[2]);
	const uint64_t seed = strtoull(argv[3], nullptr, 10);
	uint64_t k = 0;	// the test records of the file (as many as there are, beyond 2^31 too)
	Rec *res = (Rec *) xmalloc((size_t) n * sizeof *res), r;
	setvbuf(f, nullptr, _IOFBF, 1 << 24);
	while (fread(&r, sizeof r, 1, f) == 1) {
		const uint64_t h = mix64(r.P * 0x9E3779B97F4A7C15ULL ^ r.O);
		if (h % 32) continue;
		if (k < (uint64_t) n) res[k] = r;
		else { const uint64_t j = mix64(h ^ seed ^ k) % (k + 1); if (j < (uint64_t) n) res[j] = r; }
		++k;
	}
	if (ferror(f)) die("cannot read %s", argv[1]);
	const uint64_t kept = k < (uint64_t) n ? k : (uint64_t) n;	// the smaller of n and the test records
	for (uint64_t i = 0; i < kept; ++i) {
		char s[65];
		for (int x = 0; x < 64; ++x) s[x] = (res[i].P >> x) & 1 ? 'X' : (res[i].O >> x) & 1 ? 'O' : '-';
		s[64] = 0;
		printf("%s X;\n", s); fprintf(stderr, "%d\n", res[i].v);
	}
	return 0;
}
