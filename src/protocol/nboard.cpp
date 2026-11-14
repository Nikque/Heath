/**
 * @file nboard.cpp
 *
 * Chris Welty's NBoard Protocol
 *
 * @date 1998 - 2020
 * @author Richard Delorme
 * @version 4.4
 */

#include "util/options.h"
#include "search/search.h"
#include "util/util.h"
#include "protocol/ui.h"
#include "protocol/play.h"

#include <stdarg.h>

static Log nboard_log[1];

static void nboard_send(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	putchar('\n'); fflush(stdout);

	if (log_is_open(nboard_log)) {
		fprintf(nboard_log->f, "sent> \"");
		va_start(args, format);
		vfprintf(nboard_log->f, format, args);
		va_end(args);
		fprintf(nboard_log->f, "\"\n");
	}
}

static void nboard_fail(const char *format, ...)
{
	va_list args;

	fprintf(stderr, "Error: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	putc('\n', stderr); fflush(stderr);

	if (log_is_open(nboard_log)) {
		fprintf(nboard_log->f, "error> \"");
		va_start(args, format);
		vfprintf(nboard_log->f, format, args);
		va_end(args);
		fprintf(nboard_log->f, "\"\n");
	}
}

static void nboard_send_move(Result *result)
{
	char move[4];

	move_to_string(result->move, WHITE, move);
	nboard_send("=== %s %.2f %.1f", move, 1.0 * result->score, 0.001 * result->time);
}

static void nboard_observer(Result *result)
{
	if (log_is_open(nboard_log)) {
		fprintf(nboard_log->f, "edax> ");
		result_print(result, nboard_log->f);
		putc('\n', nboard_log->f);
	}
	nboard_send("nodestats %llu %.2f", result->n_nodes, 0.001 * result->time);
}

/**
 * @brief initialize edax protocol
 * @param ui user interface
 */
void ui_init_nboard(UI *ui)
{
	Play *play = ui->play;

	// other update...
	play_init(play, &ui->book);
	play->search.options.header = play->search.options.separator = NULL;
	ui->book.search = &play->search;
	book_load_at_startup(&ui->book);
	play->search.id = 1;
	search_set_observer(&play->search, nboard_observer);
	ui->mode = 3;
	play->type = ui->type;

	log_open(nboard_log, options.ui_log_file);
}

/**
 * @brief free resources used by edax protocol
 * @param ui user interface
 */
void ui_free_nboard(UI *ui)
{
	if (ui->book.need_saving) book_save(&ui->book, options.book_file);
	book_free(&ui->book);
	play_free(ui->play);
	log_close(nboard_log);
}

/**
 * @brief Play a move given by the interface.
 *
 * While Edax ponders, a move that leads to the position pondered keeps the pondering (the next "go" goes on with
 * its search); another move stops it. The move that answers Edax's own "go" starts the pondering.
 *
 * @param play Play.
 * @param string Move as a string.
 * @param go_board The position of the last "go" (NULL: none, or already played).
 * @return true if the move has been legally played.
 */
static bool nboard_move(Play *play, const char *string, const Board *go_board)
{
	Move move;
	Board next;
	const Board before = play->board;

	if (parse_move(string, &play->board, &move) == string) return false;

	next = before;
	board_update(&next, &move);
	if (play->state == IS_PONDERING && !board_equal(&next, &play->ponder.board)) play_stop_pondering(play);
	play_update(play, &move);
	// (the NBoard protocol ponders only with -ponder on given: up to v4.5.5-newedax.13 it never pondered)
	if (go_board && board_equal(&before, go_board) && options.ponder_set) play_ponder(play);

	return true;
}

/**
 * @brief Loop event
 * @param ui user interface
 */
void ui_loop_nboard(UI *ui)
{
	char *cmd = NULL, *param = NULL;
	Play *play = ui->play;
	Board go_board;           // the position of the last "go": the move played from it is Edax's own move
	bool go_board_set = false;

	// loop forever
	for (;;) {
		errno = 0;

		if (log_is_open(nboard_log)) {
			play_print(play, nboard_log->f);
		}

		ui_event_wait(ui, &cmd, &param);
		log_print(nboard_log, "received< \"%s %s\"\n", cmd, param);

		if (*cmd == '\0') {

		} else if (string_is(cmd, "nboard")) {
			if (strcmp(param, "1") != 0) nboard_fail("Edax expected \"nboard 1\" protocol");

		} else if (string_is(cmd, "depth")) {
			options.level = string_to_int(param, 21);
			BOUND(options.level, 0, 60, "level");
			options.level_set = true;
			nboard_send("set myname Edax%d", options.level);

		} else if (string_is(cmd, "game")) {
			Game game;
			Board board;
			int lastmove = parse_ggf(&game, param);
			if (lastmove >= 0) {
				game_get_board(&game, 60, &board);
				if (lastmove == PASS)	// https://github.com/okuhara/edax-reversi-AVX/issues/1
					board_pass(&board);
				// the position pondered: the pondering goes on (the next "go" continues its search)
				const bool pondered = (play->state == IS_PONDERING && board_equal(&board, &play->ponder.board));
				if (!pondered) play_stop_pondering(play);
				play->initial_board = board;
				// -keep-hash on, and the same game a few moves later: the hash tables are kept (up to
				// v4.5.5-newedax.13 they were cleaned at each "set game", the interfaces sending the position of
				// each move). The position pondered keeps them in any case: the pondering goes on with them.
				if (pondered || (options.keep_hash && play_board_continues(&play->board, &board))) play_continue(play);
				else play_new(play);
				go_board_set = false;
			} else {
				nboard_fail("Cannot parse game \"%s\"", param);
			}

		} else if (string_is(cmd, "move")) {
			if (!nboard_move(play, param, go_board_set ? &go_board : NULL)) {
				nboard_fail("Cannot parse move \"%s\"", param);
			}
			go_board_set = false;
		} else if (string_is(cmd, "hint")) {

			nboard_send("status Edax is thinking");
			play_hint(play, string_to_int(param, MAX_MOVE));
			nboard_send("status Edax is waiting");

		} else if (string_is(cmd, "go")) {
			nboard_send("status Edax is thinking");
			go_board = play->board;
			go_board_set = true;
			play_go(play, false);
			nboard_send_move(&play->result);
			// this "go" is answered: a ping does not stop anything more
			lock(&ui->event);
			if (--play->nboard_go <= 0) {
				play->nboard_go = 0;
				play->stop_pending = false;
			}
			unlock(&ui->event);
			nboard_send("status Edax is waiting");

		} else if (string_is(cmd, "quit", "eof")) {
			free(cmd); free(param);			
			return;

		} else if (string_is(cmd, "ping")) {
			nboard_send("pong %s", param);

		// switches: "set ponder on|off", "set keep-hash on|off"
		} else if (string_is(cmd, "ponder", "keep-hash")) {
			options_read(cmd, param);
			if (!options.can_ponder) play_stop_pondering(play);

		} else if (string_is(cmd, "learn")) {
			nboard_send("status Edax is learning");
			play_stop_pondering(play);
			play_store(play);
			nboard_send("status Edax is waiting");

		} else if (nboard_move(play, cmd, go_board_set ? &go_board : NULL)) {
			go_board_set = false;

		// error: unknown message
		} else {
			nboard_fail("unknown command \"%s\" \"%s\"", cmd, param);
		}
	}
}
