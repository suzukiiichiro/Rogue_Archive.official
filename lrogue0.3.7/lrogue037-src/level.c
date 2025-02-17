/* level.c */

#ifndef __CYGWIN__
#include <ncurses.h>
#else
#include <ncurses/ncurses.h>
#endif
#include "rogue.h"
#include "level.h"
#include "message.h"
#include "monster.h"
#include "object.h"
#include "pack.h"
#include "random.h"
#include "room.h"
#include "score.h"
#include "trap.h"

#define swap(x,y) {t = x; x = y; y = t;}

int cur_level = 0;
int max_level = 1;
int cur_room;
char *new_level_message = 0;
short party_room = NO_ROOM;
short r_de;

long level_points[MAX_EXP_LEVEL] = {
		  10L,
		  20L,
		  40L,
		  80L,
		 160L,
		 320L,
		 640L,
		1300L,
		2600L,
		5200L,
	   10000L,
	   20000L,
	   40000L,
	   80000L,
	  160000L,
	  320000L,
	 1000000L,
	 3333333L,
	 6666666L,
	  MAX_EXP,
	99900000L
};

char random_rooms[MAXROOMS + 1] = { 3,7,5,2,0,6,1,4,8 };

extern boolean being_held, wizard, detect_monster;
extern boolean see_invisible;
extern int bear_trap;
extern short levitate, extra_hp, less_hp;
extern int party_counter;
extern room rooms[];
extern unsigned short dungeon[DROWS][DCOLS];
extern trap traps[];
extern fighter rogue;

void make_room(int, int, int, int);
void add_mazes(void);
void mix_random_rooms(void);
int connect_rooms(int, int);
void fill_out_level(void);
int same_row(int, int);
void put_door(room *, int, int *, int *);
int same_col(int, int);
void draw_simple_passage(int, int, int, int, int);
void hide_boxed_passage(int, int, int, int, int);
void make_maze(int, int, int, int, int, int);
void fill_it(int, boolean);
boolean mask_room(int, int *, int *, unsigned short);
void recursive_deadend(int, int *, int, int);
int get_exp_level(long);

void make_level(void)
{
	int i, j;
	int must_exist1 = 0;
	int must_exist2 = 0;
	int must_exist3 = 0;
	boolean big_room;

	if (cur_level < LAST_DUNGEON)
	{
		cur_level++;
	}
	if (cur_level > max_level)
	{
		max_level = cur_level;
	}
	must_exist1 = get_rand(0, 5);

	switch(must_exist1)
   	{
	case 0:
		must_exist1 = 0;
		must_exist2 = 1;
		must_exist3 = 2;
		break;
	case 1:
		must_exist1 = 3;
		must_exist2 = 4;
		must_exist3 = 5;
		break;
	case 2:
		must_exist1 = 6;
		must_exist2 = 7;
		must_exist3 = 8;
		break;
	case 3:
		must_exist1 = 0;
		must_exist2 = 3;
		must_exist3 = 6;
		break;
	case 4:
		must_exist1 = 1;
		must_exist2 = 4;
		must_exist3 = 7;
		break;
	case 5:
		must_exist1 = 2;
		must_exist2 = 5;
		must_exist3 = 8;
		break;
	}
	big_room = ((cur_level == party_counter) && rand_percent(1));
	if (big_room)
   	{
		make_room(BIG_ROOM, 0, 0, 0);
	}
   	else
   	{
		for (i = 0; i < MAXROOMS; i++)
	   	{
			make_room(i, must_exist1, must_exist2, must_exist3);
		}
	}
	if (!big_room)
   	{
		add_mazes();

		mix_random_rooms();

		for (j = 0; j < MAXROOMS; j++)
	   	{

			i = random_rooms[j];

			if (i < (MAXROOMS - 1))
		   	{
				connect_rooms(i, i + 1);
			}
			if (i < (MAXROOMS - 3))
		   	{
				connect_rooms(i, i + 3);
			}
			if (i < (MAXROOMS - 2))
		   	{
				if (rooms[i + 1].is_room & R_NOTHING)
			   	{
					if (connect_rooms(i, i + 2))
				   	{
						rooms[i+1].is_room = R_CROSS;
					}
				}
			}
			if (i < (MAXROOMS - 6))
		   	{
				if (rooms[i + 3].is_room & R_NOTHING)
			   	{
					if (connect_rooms(i, i + 6))
				   	{
						rooms[i + 3].is_room = R_CROSS;
					}
				}
			}
			if (is_all_connected())
		   	{
				break;
			}
		}
		fill_out_level();
	}
	if (!has_amulet() && (cur_level >= AMULET_LEVEL))
   	{
		put_amulet();
	}
}

void make_room(int rn, int r1, int r2, int r3)
{
	int left_col = 0;
	int right_col = 0;
	int top_row = 0;
	int bottom_row = 0;
	int width, height;
	int row_offset, col_offset;
	int i, j;
	unsigned short ch;

	switch(rn)
   	{
	case 0:
		left_col = 0;
		right_col = COL1 - 1;
		top_row = MIN_ROW;
		bottom_row = ROW1 - 1;
		break;
	case 1:
		left_col = COL1 + 1;
		right_col = COL2 - 1;
		top_row = MIN_ROW;
		bottom_row = ROW1 - 1;
		break;
	case 2:
		left_col = COL2 + 1;
		right_col = DCOLS - 1;
		top_row = MIN_ROW;
		bottom_row = ROW1 - 1;
		break;
	case 3:
		left_col = 0;
		right_col = COL1 - 1;
		top_row = ROW1 + 1;
		bottom_row = ROW2 - 1;
		break;
	case 4:
		left_col = COL1 + 1;
		right_col = COL2 - 1;
		top_row = ROW1 + 1;
		bottom_row = ROW2 - 1;
		break;
	case 5:
		left_col = COL2 + 1;
		right_col = DCOLS - 1;
		top_row = ROW1 + 1;
		bottom_row = ROW2 - 1;
		break;
	case 6:
		left_col = 0;
		right_col = COL1 - 1;
		top_row = ROW2 + 1;
		bottom_row = DROWS - 2;
		break;
	case 7:
		left_col = COL1 + 1;
		right_col = COL2 - 1;
		top_row = ROW2 + 1;
		bottom_row = DROWS - 2;
		break;
	case 8:
		left_col = COL2 + 1;
		right_col = DCOLS - 1;
		top_row = ROW2 + 1;
		bottom_row = DROWS - 2;
		break;
	case BIG_ROOM:
		top_row = get_rand(MIN_ROW, MIN_ROW + 5);
		bottom_row = get_rand(DROWS - 7, DROWS - 2);
		left_col = get_rand(0, 10);
		right_col = get_rand(DCOLS - 11, DCOLS - 1);
		rn = 0;
		goto B;
	}
	height = get_rand(4, (bottom_row - top_row + 1));
	width = get_rand(7, (right_col - left_col - 2));

	row_offset = get_rand(0, ((bottom_row - top_row) - height + 1));
	col_offset = get_rand(0, ((right_col - left_col) - width + 1));

	top_row += row_offset;
	bottom_row = top_row + height - 1;

	left_col += col_offset;
	right_col = left_col + width - 1;

	if ((rn != r1) && (rn != r2) && (rn != r3) && rand_percent(40))
	{
		goto END;
	}
B:
	rooms[rn].is_room = R_ROOM;

	for (i = top_row; i <= bottom_row; i++)
   	{
		for (j = left_col; j <= right_col; j++)
	   	{
			if ((i == top_row) || (i == bottom_row))
		   	{
				ch = HORWALL;
			}
		   	else
			{
			   	if (((i != top_row) && (i != bottom_row)) &&
				    ((j == left_col) || (j == right_col)))
			   	{
					ch = VERTWALL;
				}
				else
				{
					ch = FLOOR;
				}
			}
			dungeon[i][j] = ch;
		}
	}
END:
	rooms[rn].top_row = top_row;
	rooms[rn].bottom_row = bottom_row;
	rooms[rn].left_col = left_col;
	rooms[rn].right_col = right_col;
}

int connect_rooms(int room1, int room2)
{
	int row1, col1, row2, col2, dir;

	if ((!(rooms[room1].is_room & (R_ROOM | R_MAZE))) ||
		(!(rooms[room2].is_room & (R_ROOM | R_MAZE))))
	{
		return(0);
	}
	if (same_row(room1, room2) &&
		(rooms[room1].left_col > rooms[room2].right_col))
	{
		put_door(&rooms[room1], LEFT, &row1, &col1);
		put_door(&rooms[room2], RIGHT, &row2, &col2);
		dir = LEFT;
	}
	else
	{
		if (same_row(room1, room2) &&
		    (rooms[room2].left_col > rooms[room1].right_col))
		{
			put_door(&rooms[room1], RIGHT, &row1, &col1);
			put_door(&rooms[room2], LEFT, &row2, &col2);
			dir = RIGHT;
		}
		else
		{
			if (same_col(room1, room2) &&
			    (rooms[room1].top_row > rooms[room2].bottom_row))
			{
				put_door(&rooms[room1], UP, &row1, &col1);
				put_door(&rooms[room2], DOWN, &row2, &col2);
				dir = UP;
			}
			else
			{
				if (same_col(room1, room2) &&
				    (rooms[room2].top_row > rooms[room1].bottom_row))
				{
					put_door(&rooms[room1], DOWN, &row1, &col1);
					put_door(&rooms[room2], UP, &row2, &col2);
					dir = DOWN;
				}
				else
				{
					return(0);
				}
			}
		}
	}

	do
	{
		draw_simple_passage(row1, col1, row2, col2, dir);
	} while (rand_percent(4));

	rooms[room1].doors[dir / 2].oth_room = room2;
	rooms[room1].doors[dir / 2].oth_row = row2;
	rooms[room1].doors[dir / 2].oth_col = col2;

	rooms[room2].doors[(((dir + 4) % DIRS) / 2)].oth_room = room1;
	rooms[room2].doors[(((dir + 4) % DIRS) / 2)].oth_row = row1;
	rooms[room2].doors[(((dir + 4) % DIRS) / 2)].oth_col = col1;

	return(1);
}

void clear_level(void)
{
	unsigned int i, j;

	for (i = 0; i < MAXROOMS; i++)
	{
		rooms[i].is_room = R_NOTHING;
		for (j = 0; j < 4; j++)
		{
			rooms[i].doors[j].oth_room = NO_ROOM;
		}
	}

	for (i = 0; i < MAX_TRAPS; i++)
	{
		traps[i].trap_type = NO_TRAP;
	}
	for (i = 0; i < DROWS; i++)
	{
		for (j = 0; j < DCOLS; j++)
		{
			dungeon[i][j] = NOTHING;
		}
	}
	detect_monster = see_invisible = 0;
	being_held = 0;
	bear_trap = 0;
	party_room = NO_ROOM;
	rogue.row = rogue.col = -1;
	clear();
}

void put_door(room *rm, int dir, int *row, int *col)
{
	int wall_width;

	wall_width = (rm->is_room & R_MAZE) ? 0 : 1;

	switch(dir)
	{
	case UP:
	case DOWN:
		*row = ((dir == UP) ? rm->top_row : rm->bottom_row);
		do
		{
			*col = get_rand(rm->left_col + wall_width,
			                rm->right_col - wall_width);
		} while (!(dungeon[*row][*col] & (HORWALL | TUNNEL)));
		break;
	case RIGHT:
	case LEFT:
		*col = (dir == LEFT) ? rm->left_col : rm->right_col;
		do
		{
			*row = get_rand(rm->top_row + wall_width,
			                rm->bottom_row - wall_width);
		} while (!(dungeon[*row][*col] & (VERTWALL | TUNNEL)));
		break;
	}
	if (rm->is_room & R_ROOM)
	{
		dungeon[*row][*col] = DOOR;
	}
	if ((cur_level > 2) && rand_percent(HIDE_PERCENT))
	{
		dungeon[*row][*col] |= HIDDEN;
	}
	rm->doors[dir / 2].door_row = *row;
	rm->doors[dir / 2].door_col = *col;
}

void draw_simple_passage(int row1, int col1, int row2, int col2, int dir)
{
	int i;
	int middle;
	int t;

	if ((dir == LEFT) || (dir == RIGHT))
	{
		if (col1 > col2)
		{
			swap(row1, row2);
			swap(col1, col2);
		}
		middle = get_rand(col1 + 1, col2 - 1);
		for (i = col1 + 1; i != middle; i++)
		{
			dungeon[row1][i] = TUNNEL;
		}
		for (i = row1; i != row2; i += (row1 > row2) ? -1 : 1)
		{
			dungeon[i][middle] = TUNNEL;
		}
		for (i = middle; i != col2; i++)
		{
			dungeon[row2][i] = TUNNEL;
		}
	}
	else
	{
		if (row1 > row2)
		{
			swap(row1, row2);
			swap(col1, col2);
		}
		middle = get_rand(row1 + 1, row2 - 1);
		for (i = row1 + 1; i != middle; i++)
		{
			dungeon[i][col1] = TUNNEL;
		}
		for (i = col1; i != col2; i += (col1 > col2) ? -1 : 1)
		{
			dungeon[middle][i] = TUNNEL;
		}
		for (i = middle; i != row2; i++)
		{
			dungeon[i][col2] = TUNNEL;
		}
	}
	if (rand_percent(HIDE_PERCENT))
	{
		hide_boxed_passage(row1, col1, row2, col2, 1);
	}
}

int same_row(int room1, int room2)
{
	return((room1 / 3) == (room2 / 3));
}

int same_col(int room1, int room2)
{
	return((room1 % 3) == (room2 % 3));
}

void add_mazes(void)
{
	int i, j;
	int start;
	int maze_percent;

	if (cur_level > 1)
	{
		start = get_rand(0, (MAXROOMS - 1));
		maze_percent = (cur_level * 5) / 4;

		if (cur_level > 15)
		{
			maze_percent += cur_level;
		}
		for (i = 0; i < MAXROOMS; i++)
		{
			j = ((start + i) % MAXROOMS);
			if (rooms[j].is_room & R_NOTHING)
			{
				if (rand_percent(maze_percent))
				{
					rooms[j].is_room = R_MAZE;
					make_maze(get_rand(rooms[j].top_row + 1, rooms[j].bottom_row - 1),
					          get_rand(rooms[j].left_col + 1, rooms[j].right_col - 1),
					          rooms[j].top_row, rooms[j].bottom_row,
					          rooms[j].left_col, rooms[j].right_col);
					hide_boxed_passage(rooms[j].top_row, rooms[j].left_col,
					                   rooms[j].bottom_row, rooms[j].right_col,
					                   get_rand(0, 2));
				}
			}
		}
	}
}

void fill_out_level(void)
{
	unsigned int i;
	int rn;

	mix_random_rooms();

	r_de = NO_ROOM;

	for (i = 0; i < MAXROOMS; i++)
	{
		rn = random_rooms[i];
		if ((rooms[rn].is_room & R_NOTHING) ||
		    ((rooms[rn].is_room & R_CROSS) && coin_toss()))
		{
			fill_it(rn, 1);
		}
	}
	if (r_de != NO_ROOM)
	{
		fill_it(r_de, 0);
	}
}

void fill_it(int rn, boolean do_rec_de)
{
	int i;
	int tunnel_dir;
	int door_dir;
	int drow, dcol;
	int target_room;
	int rooms_found = 0;
	int srow, scol;
	int t;
	static int offsets[4] = {-1, 1, 3, -3};
	boolean did_this = 0;

	for (i = 0; i < 10; i++)
	{
		srow = get_rand(0, 3);
		scol = get_rand(0, 3);
		t = offsets[srow];
		offsets[srow] = offsets[scol];
		offsets[scol] = t;
	}
	for (i = 0; i < 4; i++)
	{

		target_room = rn + offsets[i];

		if (((target_room < 0) || (target_room >= MAXROOMS)) ||
		    (!(same_row(rn, target_room) || same_col(rn, target_room))) ||
		    (!(rooms[target_room].is_room & (R_ROOM | R_MAZE))))
		{
			continue;
		}
		if (same_row(rn, target_room))
		{
			tunnel_dir = (rooms[rn].left_col < rooms[target_room].left_col) ?
			             RIGHT : LEFT;
		}
		else
		{
			tunnel_dir = (rooms[rn].top_row < rooms[target_room].top_row) ?
		                 DOWN : UP;
		}
		door_dir = ((tunnel_dir + 4) % DIRS);
		if (rooms[target_room].doors[door_dir / 2].oth_room != NO_ROOM)
		{
			continue;
		}
		if (((!do_rec_de) || did_this) ||
		    (!mask_room(rn, &srow, &scol, TUNNEL)))
		{
			srow = (rooms[rn].top_row + rooms[rn].bottom_row) / 2;
			scol = (rooms[rn].left_col + rooms[rn].right_col) / 2;
		}
		put_door(&rooms[target_room], door_dir, &drow, &dcol);
		rooms_found++;
		draw_simple_passage(srow, scol, drow, dcol, tunnel_dir);
		rooms[rn].is_room = R_DEADEND;
		dungeon[srow][scol] = TUNNEL;

		if ((i < 3) && (!did_this))
		{
			did_this = 1;
			if (coin_toss())
			{
				continue;
			}
		}
		if ((rooms_found < 2) && do_rec_de)
		{
			recursive_deadend(rn, offsets, srow, scol);
		}
		break;
	}
}

void recursive_deadend(int rn, int *offsets, int srow, int scol)
{
	int i;
	int de;
	int drow, dcol;
	int tunnel_dir;

	rooms[rn].is_room = R_DEADEND;
	dungeon[srow][scol] = TUNNEL;

	for (i = 0; i < 4; i++)
	{
		de = rn + offsets[i];
		if (((de < 0) || (de >= MAXROOMS)) ||
		    (!(same_row(rn, de) || same_col(rn, de))))
		{
			continue;
		}
		if (!(rooms[de].is_room & R_NOTHING))
		{
			continue;
		}
		drow = (rooms[de].top_row + rooms[de].bottom_row) / 2;
		dcol = (rooms[de].left_col + rooms[de].right_col) / 2;
		if (same_row(rn, de))
		{
			tunnel_dir = (rooms[rn].left_col < rooms[de].left_col) ?
			             RIGHT : LEFT;
		}
		else
		{
			tunnel_dir = (rooms[rn].top_row < rooms[de].top_row) ?
			             DOWN : UP;
		}
		draw_simple_passage(srow, scol, drow, dcol, tunnel_dir);
		r_de = de;
		recursive_deadend(de, offsets, drow, dcol);
	}
}

boolean mask_room(int rn, int *row, int *col, unsigned short mask)
{
	int i, j;

	for (i = rooms[rn].top_row; i <= rooms[rn].bottom_row; i++)
	{
		for (j = rooms[rn].left_col; j <= rooms[rn].right_col; j++)
		{
			if (dungeon[i][j] & mask)
			{
				*row = i;
				*col = j;
				return(1);
			}
		}
	}
	return(0);
}

void make_maze(int r, int c, int tr, int br, int lc, int rc)
{
	char dirs[4];
	int i, t;

	dirs[0] = UP;
	dirs[1] = DOWN;
	dirs[2] = LEFT;
	dirs[3] = RIGHT;

	dungeon[r][c] = TUNNEL;

	if (rand_percent(33))
	{
		for (i = 0; i < 10; i++)
		{
			int t1, t2;

			t1 = get_rand(0, 3);
			t2 = get_rand(0, 3);

			swap(dirs[t1], dirs[t2]);
		}
	}
	for (i = 0; i < 4; i++)
	{
		switch(dirs[i])
		{
		case UP:
			if (((r - 1) >= tr) &&
				(dungeon[r - 1][c] != TUNNEL) &&
				(dungeon[r - 1][c - 1] != TUNNEL) &&
				(dungeon[r - 1][c + 1] != TUNNEL) &&
				(dungeon[r - 2][c] != TUNNEL))
			{
				make_maze((r - 1), c, tr, br, lc, rc);
			}
			break;
		case DOWN:
			if (((r + 1) <= br) &&
				(dungeon[r + 1][c] != TUNNEL) &&
				(dungeon[r + 1][c - 1] != TUNNEL) &&
				(dungeon[r + 1][c + 1] != TUNNEL) &&
				(dungeon[r + 2][c] != TUNNEL))
			{
				make_maze((r + 1), c, tr, br, lc, rc);
			}
			break;
		case LEFT:
			if (((c - 1) >= lc) &&
				(dungeon[r][c - 1] != TUNNEL) &&
				(dungeon[r - 1][c - 1] != TUNNEL) &&
				(dungeon[r + 1][c - 1] != TUNNEL) &&
				(dungeon[r][c - 2] != TUNNEL))
			{
				make_maze(r, (c - 1), tr, br, lc, rc);
			}
			break;
		case RIGHT:
			if (((c + 1) <= rc) &&
				(dungeon[r][c + 1] != TUNNEL) &&
				(dungeon[r - 1][c + 1] != TUNNEL) &&
				(dungeon[r + 1][c + 1] != TUNNEL) &&
				(dungeon[r][c + 2] != TUNNEL))
			{
				make_maze(r, (c + 1), tr, br, lc, rc);
			}
			break;
		}
	}
}

void hide_boxed_passage(int row1, int col1, int row2, int col2, int n)
{
	int i, j;
	int t;
	int row, col;
	int row_cut, col_cut;
	int h, w;

	if (cur_level > 2)
	{
		if (row1 > row2)
		{
			swap(row1, row2);
		}
		if (col1 > col2)
		{
			swap(col1, col2);
		}
		h = row2 - row1;
		w = col2 - col1;

		if ((w >= 5) || (h >= 5))
		{
			row_cut = ((h >= 2) ? 1 : 0);
			col_cut = ((w >= 2) ? 1 : 0);

			for (i = 0; i < n; i++)
			{
				for (j = 0; j < 10; j++)
				{
					row = get_rand(row1 + row_cut, row2 - row_cut);
					col = get_rand(col1 + col_cut, col2 - col_cut);
					if (dungeon[row][col] == TUNNEL)
					{
						dungeon[row][col] |= HIDDEN;
						break;
					}
				}
			}
		}
	}
}

void put_player(int nr)		/* try not to put in this room */
{
	int rn = nr;
	int misses;
	int row, col;

	for (misses = 0; ((misses < 2) && (rn == nr)); misses++)
	{
		gr_row_col(&row, &col, (FLOOR | TUNNEL | OBJECT | STAIRS));
		rn = get_room_number(row, col);
	}
	rogue.row = row;
	rogue.col = col;

	if (dungeon[rogue.row][rogue.col] & TUNNEL)
	{
		cur_room = PASSAGE;
	}
	else
	{
		cur_room = rn;
	}
	if (cur_room != PASSAGE)
	{
		light_up_room(cur_room);
	}
	else
	{
		light_passage(rogue.row, rogue.col);
	}
	wake_room(get_room_number(rogue.row, rogue.col), 1, rogue.row, rogue.col);
	if (new_level_message)
	{
		message(new_level_message, 0);
		new_level_message = 0;
	}
	mvaddch(rogue.row, rogue.col, rogue.fchar);
}

int drop_check(void)
{
	if (wizard)
	{
		return(1);
	}
	if (dungeon[rogue.row][rogue.col] & STAIRS)
	{
		if (levitate)
		{
			message("You're floating in the air!", 0);
			return(0);
		}
		return(1);
	}
	message("I see no way down.", 0);
	return(0);
}

int check_up(void)
{
	if (!wizard)
	{
		if (!(dungeon[rogue.row][rogue.col] & STAIRS))
		{
			message("I see no way up.", 0);
			return(0);
		}
	        if (!has_amulet())
		{
                        message("Your way is magically blocked.",0);
                        return(0);
                }
	}
	new_level_message = "You feel a wrenching sensation in your gut.";
	if (cur_level == 1)
	{
		win();
	}
	else
	{
		cur_level -= 2;
		return(1);
	}
	return(0);
}

void add_exp(int e, boolean promotion)
{
	char mbuf[40];
	int new_exp;
	int i, hp;

	rogue.exp_points += e;

	if (rogue.exp_points >= level_points[rogue.exp - 1])
	{
		new_exp = get_exp_level(rogue.exp_points);
		if (rogue.exp_points > MAX_EXP)
		{
			rogue.exp_points = MAX_EXP + 1;
		}
		for (i = rogue.exp + 1; i <= new_exp; i++)
		{
			sprintf(mbuf, "Welcome to experience level %d!", i);
			message(mbuf, 0);
			if (promotion)
			{
				hp = hp_raise();
				rogue.hp_current += hp;
				rogue.hp_max += hp;
			}
			rogue.exp = i;
			print_stats(STAT_HP | STAT_EXP);
		}
	}
	else
	{
		print_stats(STAT_EXP);
	}
}

int get_exp_level(long e)
{
	int i;

	for (i = 0; i < (MAX_EXP_LEVEL - 1); i++)
	{
		if (level_points[i] > e)
		{
			break;
		}
	}
	return(i + 1);
}

int hp_raise(void)
{
	int hp;

	hp = (wizard ? 10 : get_rand(3, 10));
	return(hp);
}

void show_average_hp(void)
{
	char mbuf[80];
	int real_average;
	int effective_average;

	if (rogue.exp == 1)
	{
		real_average = effective_average = 0.00;
	}
	else
	{
		real_average = 
			((rogue.hp_max - extra_hp - INIT_HP) + less_hp) / (rogue.exp - 1);
		effective_average = (rogue.hp_max - INIT_HP) / (rogue.exp - 1);

	}
	sprintf(mbuf, "R-Hp: %.2d, E-Hp: %.2d (!: %d, V: %d)", real_average,
		effective_average, extra_hp, less_hp);
	message(mbuf, 0);
}

void mix_random_rooms(void)
{
	unsigned int i;
	short t;
	short x, y;

	for (i = 0; i < (3 * MAXROOMS); i++)
	{
		do
		{
			x = get_rand(0, (MAXROOMS-1));
			y = get_rand(0, (MAXROOMS-1));
		} while (x == y);
		swap(random_rooms[x], random_rooms[y]);
	}
}
