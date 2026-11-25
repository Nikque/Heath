/**
 * @file gtp.cpp
 *
 * Go Text Protocol
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

#include <ctype.h>

static Log gtp_log[1];
bool ui_book_save_at_end(UI *ui, const bool to_book_file);

static void gtp_observer(Result *result)
{
	if (log_is_open(gtp_log)) {
		result_print(result, gtp_log->f);
		putc('\n', gtp_log->f);
	}
}

/*
 * @brief Preprocess a line according to GTP2.
 *
 * @param line string to process
 */
void gtp_preprocess(char *line)
{
	char *s;

	for (s = line; *line; ++line) {
		if (*line == '\t') *line = ' ';
		if (*line == '#') break;
		if (*line < 32 && *line != '\n') continue;

		*s++ = *line;
	}
	*s = '\0';
}

static void gtp_send(const char *s, int id, bool has_id)
{
	if (has_id) printf("= %d ", id);
	else printf("= ");
	puts(s);
	putchar('\n'); fflush(stdout);

	log_print(gtp_log, "sent> \"%s\"\n", s);
}

static void gtp_fail(const char *s, int id, bool has_id)
{
	if (has_id) printf("? %d ", id);
	else printf("? ");
	puts(s);
	putchar('\n'); fflush(stdout);

	log_print(gtp_log, "error> \"%s\"\n", s);
}

static char *gtp_parse_color(char *s, int *color)
{
	char word[8];

	s = parse_word(s, word, 7);

	if (string_is(word, "black")) {
		*color = BLACK;
	} else if (string_is(word, "b")) {
		*color = BLACK;
	} else if (string_is(word, "white")) {
		*color = WHITE;
	} else if (string_is(word, "w")) {
		*color = WHITE;
	} else {
		*color = EMPTY;
	}

	return s;
}

/**
 * @brief initialize edax protocol
 * @param ui user interface
 */
void ui_init_gtp(UI *ui)
{
	Play *play = ui->play;

	options.verbosity = 0;
	options.info = 0;

	play_init(play, &ui->book);
	ui->book.search = &play->search;
	book_load_at_startup(&ui->book);
	play->search.id = 1;
	search_set_observer(&play->search, gtp_observer);
	ui->mode = 3;
	play->type = ui->type;

	log_open(gtp_log, options.ui_log_file);
}

/**
 * @brief free resources used by edax protocol
 * @param ui user interface
 */
void ui_free_gtp(UI *ui)
{
	ui_book_save_at_end(ui, false);
	book_free(&ui->book);
	play_free(ui->play);
	log_close(gtp_log);
}

/**
 * @brief Loop event
 * @param ui user interface
 */
void ui_loop_gtp(UI *ui)
{
	char *cmd = NULL, *param = NULL, *move;
	Play *play = ui->play;
	int id = 0;
	bool has_id = false;
	int komi, color, size;
	int byo_yomi_time = 0, byo_yomi_stone = 0, main_time = 315576000;

	// loop forever
	for (;;) {
		errno = 0;
		id = 0;
		has_id = false;

		if (log_is_open(gtp_log)) {
			play_print(play, gtp_log->f);
		}

		ui_event_wait(ui, &cmd, &param);

		log_print(gtp_log, "received: \"%s %s\"\n", cmd, param);

		if (isdigit((unsigned char) *cmd)) {
			has_id = true;
			parse_int(cmd, &id);
			parse_command(param, cmd, param, strlen(param) + 1);
		}

		if (*cmd == '\0') {

		} else if (string_is(cmd, "protocol_version")) {
			gtp_send("2", id, has_id);

		} else if (string_is(cmd, "name")) {
			gtp_send("Edax", id, has_id);

		} else if (string_is(cmd, "version")) {
			gtp_send(VERSION_STRING, id, has_id);

		} else if (string_is(cmd, "known_command")) {
			if (string_is(param, "protocol_version", "name", "version", "known_command", "list_commands", "quit", "boardsize", "clear_board", "komi", "play", "genmove", "undo", "time_settings", "time_left", "set_game", "list_games", "loadsgf", "reg_genmove", "showboard")) gtp_send("true", id, has_id);
			else gtp_send("false", id, has_id);

		} else if (string_is(cmd, "list_commands")) {
			gtp_send("protocol_version\nname\nversion\nknown_command\nlist_commands\n"
				"quit\nboardsize\nclear_board\nkomi\nplay\ngenmove\nundo\n"
				"time_settings\ntime_left\nset_game\nlist_games\n"
				"loadsgf\nreg_genmove\nshowboard", id, has_id);

		} else if (string_is(cmd, "quit", "eof")) {
			gtp_send("", id, has_id);
			free(cmd); free(param);
			return;

		} else if (string_is(cmd, "boardsize")) {
			size = string_to_int(param, 0);
			if (size != 8) gtp_fail("unacceptable size", id, has_id);
			else gtp_send("", id, has_id);

		} else if (string_is(cmd, "clear_board")) {
			play_new(play);
			gtp_send("", id, has_id);

		} else if (string_is(cmd, "komi")) {
			komi = string_to_int(param, 0);
			gtp_send("", id, has_id);
			(void) komi; // do nothing of it....

		} else if (string_is(cmd, "play")) {
			move = gtp_parse_color(param, &color);
			if (color == EMPTY) {
				gtp_fail("syntax error (wrong or missing color)", id, has_id);
				continue;
			} else if (color != play->player) {
				if (play_must_pass(play)) {
					play_move(play, PASS);
				} else {
					gtp_fail("wrong color", id, has_id);
					continue;
				}
			 }

			if (play_user_move(play, move)) {
				gtp_send("", id, has_id);
			} else {
				gtp_fail("illegal move", id, has_id);
			}

		} else if (string_is(cmd, "genmove", "reg_genmove")) {
			const bool update = string_is(cmd, "genmove");
			char m[4];
			// reg_genmove leaves the game as it is: its board, its player, its moves and its clocks
			const Board saved_board = play->board;
			const int saved_player = play->player;
			unsigned char saved_time[sizeof play->time];
			memcpy(saved_time, play->time, sizeof play->time);
			gtp_parse_color(param, &color);
			if (color == EMPTY) {
				gtp_fail("syntax error (wrong or missing color)", id, has_id);
				continue;
			} else if (color != play->player) {
				if (!play_must_pass(play)) {
					gtp_fail("wrong color", id, has_id);
					continue;
				} else if (update) {
					play_move(play, PASS);
				} else { // the pass is not played: the position after it is searched
					board_pass(&play->board);
					play->player ^= 1;
				}
			}
			if (play_is_game_over(play)) {
				gtp_send("pass", id, has_id);
			} else {
				play_go(play, update);
				if (play->result.move == PASS) gtp_send("pass", id, has_id);
				else gtp_send(move_to_string(play->result.move, WHITE, m), id, has_id);
			}
			if (!update) {
				play->board = saved_board;
				play->player = saved_player;
				memcpy(play->time, saved_time, sizeof play->time);
			}

		// optional GTP commands but needed or supported by Quarry
		} else if (string_is(cmd, "undo")) {
			if (play->i_game <= 0) {
				gtp_fail("cannot undo", id, has_id);
			} else {
				play_undo(play);
				gtp_send("", id, has_id);
			}

		} else if (string_is(cmd, "time_settings")) {
			char *s;
			int t_main = main_time, t_byo_yomi = byo_yomi_time, n_byo_yomi = byo_yomi_stone; // (a missing number keeps its value)
			errno = 0;
			s = parse_int(param, &t_main);
			s = parse_int(s, &t_byo_yomi);
			s = parse_int(s, &n_byo_yomi);
			// numbers of seconds or stones from 0 to INT_MAX: else the settings are kept
			if (errno == ERANGE || t_main < 0 || t_byo_yomi < 0 || n_byo_yomi < 0) {
				gtp_fail("syntax error (time settings)", id, has_id);
				continue;
			}
			main_time = t_main; byo_yomi_time = t_byo_yomi; byo_yomi_stone = n_byo_yomi;
			if (byo_yomi_stone > 0) {
				options.play_type = EDAX_TIME_PER_MOVE;
			} else {
				options.play_type = EDAX_TIME_PER_GAME;
			}
			options.time = 1000LL * main_time; // (in a long long: more than 2147483 s overflows an int)
			gtp_send("", id, has_id);

		} else if (string_is(cmd, "time_left")) {
			char *s;
			double t = 0.0;
			int n = 0;
			s = gtp_parse_color(param, &color);
			if (color == EMPTY) {
				gtp_fail("syntax error (wrong or missing color)", id, has_id);
				continue;
			}
			s = parse_real(s, &t);
			errno = 0;
			s = parse_int(s, &n); // (a missing number is 0)
			// seconds from 0 to INT_MAX (as time_settings) and a number of stones from 0: else the clock is kept
			if (errno == ERANGE || !(t >= 0.0 && t <= (double) INT_MAX) || n < 0) {
				gtp_fail("syntax error (time left)", id, has_id);
				continue;
			}
			t *= 1000;
			options.level = 60;
			if (options.play_type == EDAX_TIME_PER_MOVE) {// time_per_move ?
				if (n == 0) play->time[color].left = byo_yomi_stone > 0 ? (t + byo_yomi_time * 1000.0) / byo_yomi_stone : t;
				else play->time[color].left = t / n;
			} else { // time_per_game
				play->time[color].left = t;
			}
			gtp_send("", id, has_id);

		} else if (string_is(cmd, "set_game")) {
			if (string_is(param, "Othello")) gtp_send("", id, has_id);
			else gtp_fail("unsupported game", id, has_id);

		} else if (string_is(cmd, "list_games")) {
			gtp_send("Othello", id, has_id);

		// regression subset
		} else if (string_is(cmd, "loadsgf")) {
			if (play_load(play, param)) {
				gtp_send("", id, has_id);
			} else {
				gtp_fail(play->error_message, id, has_id);
			}

		// optional commands
		} else if (string_is(cmd, "showboard")) {
			printf("= ");
			if (has_id) printf("%d\n", id);
			board_print(&play->board, play->player, stdout);
			putchar('\n'); fflush(stdout);

		// error: unknown message
		} else {
			gtp_fail("unknown command", id, has_id);
		}
	}
}
