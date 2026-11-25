/**
 * @file w2dat2.cpp
 *
 * Convert weights dumped by evsearch (-dump: <cfg>.pat, <cfg>.scl, <cfg>_<ply>.w) for a configuration made of
 * Edax's 46 features (+ optional "mob", "mobp" or both) into an Edax eval.dat of form 1; the mobility weights are appended
 * to the file ("MOBW" then 61 x 64 shorts: side to move [0..31], opponent [32..63]), as read by eval2 (with mob and mobp,
 * the weights of mobp are added to those of the side to move). Every weight is rounded to 16 bits as et_short does.
 * A configuration with patterns of the extra groups of the 64-slot evaluation (ET_X_GROUP) gives a file of form 2
 * with all the extra groups (the groups out of the configuration have weights 0), read by a build with EVAL_SLOTS=64.
 * usage: w2dat2 <template eval.dat> <dir> <cfg> <out.dat> [version x.y.z] [-interp]
 *   -interp: a weight without samples at a ply (<cfg>_<ply>.n) is interpolated linearly between the nearest plies of
 *            the same parity that have samples; a side without such a ply counts as 0 (Buro 1997).
 */
#include <cmath>

#include "evtool.hpp"

using namespace evtool;

int main(int argc, char **argv)
{
	if (argc < 5) die("usage");
	bool interp = false;
	if (!strcmp(argv[argc - 1], "-interp")) { interp = true; --argc; }
	et_init();
	ET_Eval *e = et_read(argv[1]);
	const char *dir = argv[2], *cfg = argv[3];
	char name[1024];
	int n_inst, nw, sc[32] = {0};
	evtool::xname(name, "%s/%s.pat", dir, cfg);
	FILE *f = xfopen(name, "rb");
	xfread(&n_inst, 4, 1, f, name); xfread(&nw, 4, 1, f, name);
	if (n_inst < 1 || n_inst > 256 || nw < 1) die("bad instances in %s", name);
	int *in = (int *) xmalloc(n_inst * sizeof *in), *io = (int *) xmalloc(n_inst * sizeof *io);
	unsigned char (*isq)[16] = (unsigned char (*)[16]) xmalloc(n_inst * 16);
	for (int j = 0; j < n_inst; ++j) {
		xfread(in + j, 4, 1, f, name); xfread(io + j, 4, 1, f, name); xfread(isq[j], 1, 16, f, name);
		int size = 1;
		for (int i = 0; i < in[j] && i < 16; ++i) size *= 3;
		if (in[j] < 1 || in[j] > 16 || io[j] < 1 || io[j] > nw - size) die("bad instances in %s", name);
		for (int i = 0; i < in[j]; ++i) if (isq[j][i] > 63) die("bad instances in %s", name);
	}
	fclose(f);
	evtool::xname(name, "%s/%s.scl", dir, cfg);
	if ((f = fopen(name, "rb"))) {
		const size_t r = fread(sc, 4, 32, f);	// 11 scalar features of evsearch: (used, offset) each
		fclose(f);
		if (r < 4 || r % 2) die("bad scalar features in %s", name);
	}
	// mob (both sides: 64 weights) and mobp (side to move only: 32 weights, the same index as the first 32 of mob): with
	// both, the weights of mobp are added to the first 32 of mob (the same evaluation)
	const int has_mob = sc[0] | sc[2], mob_ofs = sc[1], mobp_ofs = sc[3];
	for (int i = 4; i < 32; i += 2) if (sc[i]) die("only the mob / mobp scalars are supported");
	if ((sc[0] && (mob_ofs < 1 || mob_ofs > nw - 64)) || (sc[2] && (mobp_ofs < 1 || mobp_ofs > nw - 32))) die("bad scalar features in %s", name);
	if (sc[0] && sc[2]) printf("mob and mobp: the weights of mobp are added to the weights of mob of the side to move\n");
	static short mob[61][64];
	double *all = (double *) xmalloc((size_t) 54 * nw * sizeof *all), *w;
	double maxdiff = 0;
	for (int ply = 2; ply <= 53; ++ply) {
		evtool::xname(name, "%s/%s_%02d.w", dir, cfg, ply);
		f = xfopen(name, "rb");
		xfread(all + (size_t) ply * nw, 8, nw, f, name);
		fclose(f);
	}
	if (interp) {
		double *cnt = (double *) xmalloc((size_t) 54 * nw * sizeof *cnt);
		long long filled = 0;
		for (int ply = 2; ply <= 53; ++ply) {
			evtool::xname(name, "%s/%s_%02d.n", dir, cfg, ply);
			if (!(f = fopen(name, "rb"))) { fprintf(stderr, "cannot open %s\n", name); return 1; }
			xfread(cnt + (size_t) ply * nw, 8, nw, f, name);
			fclose(f);
		}
		for (int a = 1; a < nw; ++a) for (int ply = 2; ply <= 53; ++ply) {
			if (cnt[(size_t) ply * nw + a] > 0) continue;
			int p0 = ply - 2, p1 = ply + 2;
			while (p0 >= 2 && !(cnt[(size_t) p0 * nw + a] > 0)) p0 -= 2;
			while (p1 <= 53 && !(cnt[(size_t) p1 * nw + a] > 0)) p1 += 2;
			const bool has0 = p0 >= 2, has1 = p1 <= 53;
			if (!has0 && !has1) continue;
			const double v0 = has0 ? all[(size_t) p0 * nw + a] : 0, v1 = has1 ? all[(size_t) p1 * nw + a] : 0;
			all[(size_t) ply * nw + a] = v0 + (v1 - v0) * (ply - p0) / (p1 - p0);
			++filled;
		}
		free(cnt);
		printf("%lld weights interpolated\n", filled);
	}
	int covered[13] = {0};
	static short xw[61][13][59049];
	bool has_x = false;
	for (int ply = 2; ply <= 53; ++ply) {
		w = all + (size_t) ply * nw;
		short *ew = e->w[ply];
		ew[ET_NW - 1] = et_short(w[0]);
		for (int j = 0; j < n_inst; ++j) {
			if (j > 0 && io[j] == io[j - 1]) continue;
			const int n = in[j];
			uint64_t set = 0;
			for (int i = 0; i < n; ++i) set |= 1ULL << isq[j][i];
			int found = 0;
			for (int ft = 0; ft < ET_NF && !found; ++ft) {
				const ET_F2X *x = ET_FEAT + ft;
				uint64_t es = 0;
				for (int i = 0; i < x->n; ++i) es |= 1ULL << x->x[i];
				if (x->n != n || es != set) continue;
				int pw[16], size = 1;
				for (int i = 0; i < n; ++i) {
					int k = 0;
					while (x->x[k] != isq[j][i]) ++k;
					pw[i] = 1;
					for (int m = k + 1; m < n; ++m) pw[i] *= 3;
					size *= 3;
				}
				const int cl = ET_CLASS[ft];
				char *seen = (char *) xcalloc(ET_PSIZE[cl], 1); // (up to newedax.11 calloc, its result not checked)
				for (int v = 0; v < size; ++v) {
					int y = v, xe = 0;
					for (int i = n - 1; i >= 0; --i) { xe += (y % 3) * pw[i]; y /= 3; }
					const short q = et_short(w[io[j] + v]);
					const int pi = ET_PACK[cl][xe];
					if (seen[pi]) { const double d = fabs((double) ew[pi + ET_POFS[cl]] - q); if (d > maxdiff) maxdiff = d; }
					seen[pi] = 1;
					ew[pi + ET_POFS[cl]] = q;
				}
				free(seen);
				covered[cl] = 1; found = 1;
			}
			for (int g = 0; g < 13 && !found; ++g) {
				uint64_t es = 0;
				for (int i = 0; i < ET_X_GROUP[g].n; ++i) es |= 1ULL << ET_X_GROUP[g].x[i];
				if (ET_X_GROUP[g].n != n || es != set) continue;
				int pw[16];
				for (int i = 0; i < n; ++i) {
					int k = 0;
					while (ET_X_GROUP[g].x[k] != isq[j][i]) ++k;
					pw[i] = 1;
					for (int m = k + 1; m < n; ++m) pw[i] *= 3;
				}
				for (int v = 0; v < ET_X_SIZE[g]; ++v) {
					int y = v, xe = 0;
					for (int i = n - 1; i >= 0; --i) { xe += (y % 3) * pw[i]; y /= 3; }
					xw[ply][g][xe] = et_short(w[io[j] + v]);
				}
				has_x = found = 1;
			}
			if (!found) { fprintf(stderr, "instance %d is not an Edax feature\n", j); return 1; }
		}
		for (int i = 0; i < 64; ++i) mob[ply][i] = et_short((sc[0] ? w[mob_ofs + i] : 0) + (sc[2] && i < 32 ? w[mobp_ofs + i] : 0));
	}
	for (int c = 0; c < 12; ++c) if (c < 10 && c != 4 && c != 7 && c != 5 && c != 6 && c != 3 && c != 0 && c != 9 && c != 8 && !covered[c]) { fprintf(stderr, "Edax class %d is not in the configuration\n", c); return 1; }
	for (int ply = 0; ply < 2; ++ply) { memcpy(e->w[ply], e->w[ply + 2], sizeof e->w[0]); memcpy(mob[ply], mob[ply + 2], sizeof mob[0]); }
	for (int ply = 54; ply < ET_NPLY; ++ply) { memcpy(e->w[ply], e->w[ply - 2], sizeof e->w[0]); memcpy(mob[ply], mob[ply - 2], sizeof mob[0]); }
	if (argc > 5) {	// version x.y.z of the evaluation file
		unsigned v = 0, r = 0, b = 0;
		sscanf(argv[5], "%u.%u.%u", &v, &r, &b);
		e->version = v; e->release = r; e->build = b;
	}
	if (has_x) {	// form 2: the groups of Edax, the extra groups, the mobility group (docs/port/DESIGN-format.md 3.4)
		for (int ply = 0; ply < 2; ++ply) memcpy(xw[ply], xw[ply + 2], sizeof xw[0]);
		for (int ply = 54; ply < ET_NPLY; ++ply) memcpy(xw[ply], xw[ply - 2], sizeof xw[0]);
		static const double coefficient[6] = { -0.10026799, 0.31027733, -0.57772603, 0.07585621, 1.16492647, 5.4171698 };
		const int n_group = 13 + 13 + (has_mob ? 1 : 0), size = 96 + n_group * 20 + 4;
		unsigned int n_weights = ET_NW + (has_mob ? ET_NMOB : 0);
		for (int g = 0; g < 13; ++g) n_weights += ET_X_SIZE[g];
		unsigned char *x = (unsigned char *) xcalloc(size, 1);
		const uint16_t form = 2, hsize = (uint16_t) size, n_plies = ET_NPLY, first_ply = 0, groups = (uint16_t) n_group;
		memcpy(x, "NEDXEVAL", 8); memcpy(x + 8, &form, 2); memcpy(x + 10, &hsize, 2);
		memcpy(x + 16, &e->version, 4); memcpy(x + 20, &e->release, 4); memcpy(x + 24, &e->build, 4); memcpy(x + 28, &e->date, 8);
		memcpy(x + 36, &n_plies, 2); memcpy(x + 38, &first_ply, 2); memcpy(x + 40, &n_weights, 4); memcpy(x + 44, &groups, 2);
		memcpy(x + 48, coefficient, 48);
		et_groups(x + 96);
		for (int g = 0; g < 13; ++g) {
			unsigned char *y = x + 96 + (13 + g) * 20;
			y[0] = (unsigned char) ET_X_GROUP[g].n; y[1] = g == 12 ? 4 : g == 11 ? 8 : g == 10 ? 4 : g >= 8 ? 4 : g == 7 ? 2 : g == 6 ? 4 : g == 5 ? 8 : g == 4 ? 2 : 4;
			memcpy(y + 4, &ET_X_SIZE[g], 4);
			memcpy(y + 8, ET_X_GROUP[g].x, ET_X_GROUP[g].n);
		}
		if (has_mob) memcpy(x + 96 + 26 * 20, ET_MOBILITY_GROUP, 20);
		uint32_t crc = crc32c_update(0, x, size - 4);
		memcpy(x + size - 4, &crc, 4);
		// written as <out.dat>.tmp, then put in the place of out.dat: a failure leaves out.dat as it was
		char tmp[1024];
		evtool::xname(tmp, "%s.tmp", argv[4]);
		f = xfopen(tmp, "wb");
		bool ok = fwrite(x, 1, size, f) == (size_t) size;
		crc = 0;
		for (int ply = 0; ply < ET_NPLY; ++ply) {
			const auto put = [&](const void *p, size_t n) { crc = crc32c_update(crc, (const unsigned char *) p, n * 2); ok = ok && fwrite(p, 2, n, f) == n; };
			put(e->w[ply], ET_NW);
			for (int g = 0; g < 13; ++g) put(xw[ply][g], ET_X_SIZE[g]);
			if (has_mob) put(mob[ply], ET_NMOB);
		}
		ok = ok && fwrite(&crc, 4, 1, f) == 1 && fwrite("EEND", 1, 4, f) == 4;
		if (fclose(f) != 0 || !ok) { remove(tmp); die("cannot write %s", argv[4]); }
		replace_file(tmp, argv[4]);
		printf("%s written, form 2 (largest difference between symmetric entries: %.0f)\n", argv[4], maxdiff);
		return 0;
	}
	// form 1, and the mobility weights after it: "MOBW" then 61 x 64 shorts (same byte order as the file)
	et_write(e, argv[4], has_mob ? mob : nullptr);
	printf("%s written (largest difference between symmetric entries: %.0f)\n", argv[4], maxdiff);
	if (has_mob) printf("mobility weights appended\n");
	return 0;
}
