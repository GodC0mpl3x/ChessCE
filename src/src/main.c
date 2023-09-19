/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Standard headers */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

/* Shared libraries */
#include <lib/ce/graphx.h>
#include <lib/ce/fileioc.h>
#include "gfx/all_gfx.h"

#define F(I, S, N) for (I = S; I < N; I++)
#define W(A) while (A)
#define K(A, B) * (int * )(T + A + (B & 8) + S * (B & 7))
#define J(A) K(y + A, b[y]) - K(x + A, u) - K(H + A, t)

#define GRAY_COLOR	0x4A
#define RED_COLOR   0x4B

#define HOME_LOC	121
#define BLOCK_SIZE	26
#define NUM_BLOCKS	8

#define BLACK_COLOR	    0x0A
#define WHITE_COLOR     0x0B
#define SEP_COLOR       0x0C

#define BPIECE_COLOR    gfx_black
#define WPIECE_COLOR	gfx_white
#define BACK_COLOR      gfx_white

#define USER_INPUT	0
#define AI_INPUT	1

/* version info */
#define VERSION		1
#define WHITE 0
#define BLACK 1
#define BLANK 2

#define U 7530
struct _ {int K,V;char X,Y,D;} A[U];           /* hash table, 7530 entries*/

const int V=112,M=136,S=128,I=8e4,C=799;       /* V=0x70=rank mask, M=0x88 */

int N,Q,i;

char O,K,L;
const char w[]={0,1,1,3,-1,3,5,9};
const char o[]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0,  /* step-vector lists */ 
          7,-1,11,6,8,3,6,                                  /* 1st dir. in o[] per piece*/
          6,3,5,7,4,5,3,6};                                 /* initial piece setup      */
char b[129], T[1035];

gfx_image_t *pieces[] = {
    NULL, NULL, black_pawn, black_knight, black_king, black_bishop, black_rook, black_queen, NULL, 
    white_pawn, NULL, white_knight, white_king, white_bishop, white_rook, white_queen
};

char *white_open_moves[] = {
    "g1f3", "c2c4", "d2d4", "e2e4","g2g3", "b1c3"
};

char *black_open_moves[] = {
    "g8f6", "c7c5", "d7d5", "g7g6","e7e5", "b8c6"
};

typedef struct player_struct {
	int8_t row;
	int8_t col;
	int8_t selrow;
	int8_t selcol;
	int8_t testrow;
	int8_t testcol;
	bool draw_selection;
	uint8_t input;
    bool first_move_done;
} player_t;

player_t player[2];

const char me[] = "matt \"mateoconlechuga\" waltz";
const char me2[] = ".bern."
const char them[] = "engine by h.g. muller";
const char new_game[] = "new game";
const char load_game[] = "load game";
const char settings_str[] = "game setup";
const char mode_str[] = "mode";
const char play_as_str[] = "play as";
const char will_start_str[] = "will start";
const char no_str[] = "no";
const char yes_str[] = "yes";
const char human1_str[] = "human v calc";	  // mode 0
const char calc2_str[] = "calc v calc";		// mode 1
const char remote1_str[] = "network match"; // mode 2
const char human2_str[] = "human v human";	 // mode 3
const char black_str[] = "black";
const char white_str[] = "white";
const char arrows_str[] = "use the <> arrows to set";
const char info_str[] = "these settings will only affect a new game";
const char white_turn_str[] = "white's turn";
const char black_turn_str[] = "black's turn";
const char appvar_name[] = "ChessNET";

void fast_exit(void);
int D(int k, int q, int l, int e, int J, int Z, int E, int z, int n);
char get_piece_color(uint8_t row, uint8_t col);
void print_settings(void);
void get_settings(void);
void init_board(void);
void draw_board(void);
void print_home(void);
void game_loop(void);
void draw_logo(void);
void run_game(void);
void game_over(void);
void game_reset(void);
void draw_box(uint8_t c, uint8_t col, uint8_t row);
uint8_t get_player_color(void);
bool check_move(void);
void draw_controls(void);
bool load_save(void);
void save_save(void);
void draw_red_text(char *text, uint16_t x, uint8_t y);

typedef struct settings_struct {
	int8_t mode;
	uint8_t start;
	uint8_t playingas;
} settings_t;

settings_t settings;
uint8_t home_item;
uint8_t settings_item;
uint8_t current_player;
uint8_t play_as;
gfx_image_t *game_logo;
unsigned steps;

void draw_logo(void) {
    unsigned x_val;
	gfx_FillScreen(BACK_COLOR);
    gfx_SetTextScale( 3, 3 );
    gfx_SetTextBGColor(BACK_COLOR);
    gfx_SetTextFGColor(GRAY_COLOR);
    x_val = (320-gfx_GetStringWidth(appvar_name))/2 + 10;
    gfx_PrintStringXY(appvar_name, x_val, 55);
    gfx_PrintStringXY(appvar_name, x_val+2, 55+2);
    gfx_SetTextScale( 1, 1 );
    gfx_TransparentSprite_NoClip(black_king, x_val+60, 95);
    gfx_TransparentSprite_NoClip(white_king, x_val+80, 95);
}

void draw_box(uint8_t c, uint8_t col, uint8_t row) {
	gfx_SetColor(c);
	gfx_Rectangle(15 + BLOCK_SIZE * col, 15 + BLOCK_SIZE * (7 - row), BLOCK_SIZE - 1, BLOCK_SIZE - 1);
}

uint8_t get_player_color(void) {
	return current_player == 0 ? BLACK : WHITE;
}

void in_check(void) {
    draw_red_text("check!", 252, 116);
    gfx_PrintStringXY("[mode]", 252, 140);
    gfx_PrintStringXY("checkmate", 237, 150);
}

const char *a_cols = "abcdefgh";
const char *a_rows = "12345678";

char *compute_move(uint8_t row_from, uint8_t col_from, uint8_t row, uint8_t col, char *move_str) {
	move_str[0] = a_cols[col_from];
	move_str[1] = a_rows[row_from];
	move_str[2] = a_cols[col];
	move_str[3] = a_rows[row];
	move_str[4] = '\0';
    
	return move_str;
}

char get_piece_color(uint8_t row, uint8_t col) {
	unsigned x, curr = 0;
	char h;
    
	for(x = 0; x<col + (7-row)*8; x++) {
		curr = curr+9 & ~8;
	}
	h = b[curr]&15;
    
	// 2-7 is white
	// 9-15 is black

	if((h >= 2 && h <= 7)) {
		h = BLACK;
	} else if((h >= 9 && h <= 15)) {
		h = WHITE;
	} else {
		h = BLANK;
	}
	return h;
}

void run_game(void) {
	uint8_t row, col;
	uint8_t key = 1;
	char move_str[5];
	char *first_move;
    
	int k, *p, check_test;
    
	play_as = get_player_color();
    
	k = 8+8*(play_as == BLACK);
    
	switch(settings.mode) {
	case 0:
		player[settings.playingas ^ 1].input = USER_INPUT;
		player[settings.playingas].input = AI_INPUT;
		break;
	case 1:
		player[0].input = AI_INPUT;
		player[1].input = AI_INPUT;
		break;
	case 2: // FOR PLAYING OVER NETWORK
		player[0].input = USER_INPUT;
		player[1].input = REMOTE_INPUT; // ADDED REMOTE_INPUT
	default:
		player[0].input = USER_INPUT;
		player[1].input = USER_INPUT;
		break;
	}

	gfx_SetTextFGColor( GRAY_COLOR );
	gfx_SetDrawScreen();
    
	draw_board();
	draw_controls();
    
	check_test = 0;
    
	while(key != sk_Clear) {
        
		if (player[current_player].input == AI_INPUT) {
			draw_red_text("thinking...", 239, (240 - 8) / 2);
			N = 0;
            
			// make first move random
			if(!player[current_player].first_move_done) {
				first_move = (current_player) ? white_open_moves[rand()%6] : black_open_moves[rand()%6];
				K=first_move[0]-16*first_move[1]+C;
				L=first_move[2]-16*first_move[3]+C;
			} else {
				D(k, -I, I, Q, 1, 1, O, 8, N);
			}
            
			F(i, 0, U) A[i].K = 0;
			if ((check_test = D(k, -I, I, Q, 1, 1, O, 9, 2)) == I) {
				k ^= 24;
				steps++;
				gfx_SetDrawBuffer();
				draw_board();
				player[current_player].first_move_done = true;
				current_player ^= 1;
				draw_controls();
				gfx_SwapDraw();
				gfx_SetDrawScreen();
				key = 1;
			} else if (check_test == -I/2) {
				in_check();
			}
		}

		if (player[current_player].input == REMOTE_INPUT){
			draw_red_text("waiting...", 239, (240 - 8) / 2);
			N = 0;

			//TODO handle remote player move input
			// Recieve remote string set up as move_str = "{col_from}{row_from}{col_to}{row_to}"

			// K=move_str[0]-16*move_str[1]+C;
			// L=move_str[2]-16*move_str[3]+C;



			//Copied from other game modes, i'm just gonna leave this here and hope it works as expected
			F(i, 0, U) A[i].K = 0;
			if ((check_test = D(k, -I, I, Q, 1, 1, O, 9, 2)) == I) {
				k ^= 24;
				steps++;
				gfx_SetDrawBuffer();
				draw_board();
				player[current_player].first_move_done = true; //Not sure this is needed cause not AI, but it shows in user input too.. hmm
				current_player ^= 1;
				draw_controls();
				gfx_SwapDraw();
				gfx_SetDrawScreen();
				key = 1;
			} else if (check_test == -I/2) {
				in_check();
			}

		}
        
		if ( key ) {
			if (player[current_player].input == USER_INPUT) {
				draw_box(0xE0, player[current_player].col, player[current_player].row);
				if (player[current_player].draw_selection) {
					draw_box(0xE5, player[current_player].selcol, player[current_player].selrow);
				}
			}
		}
		if ((key = os_GetCSC())) {
			draw_box(BACK_COLOR, player[current_player].col, player[current_player].row);
		}
        
		if(key == sk_Del) fast_exit();
        
		/* up pressed */
		if (key == sk_Up) {
			player[current_player].row = (player[current_player].row + 1) % 8;
		}
		/* down pressed */
		if (key == sk_Down) {
			player[current_player].row = player[current_player].row - 1 < 0 ? 7 : player[current_player].row - 1;
		}
		/* right pressed */
		if (key == sk_Right) {
			player[current_player].col = (player[current_player].col + 1) % 8;
		}
		/* left pressed */
		if (key == sk_Left) {
			player[current_player].col = player[current_player].col - 1 < 0 ? 7 : player[current_player].col - 1;
		}
		if (key == sk_Alpha) {
			draw_box(BACK_COLOR, player[current_player].selcol, player[current_player].selrow);
			player[current_player].draw_selection = false;
		}
		if (key == sk_Mode && check_test == -I/2) {
			game_over();
			goto exit;
		}
		if (key == sk_Enter || key == sk_2nd) {
			if (player[current_player].input == USER_INPUT) {
				col = player[current_player].col;
				row = player[current_player].row;
                
				if (player[current_player].draw_selection) {
					compute_move(player[current_player].selrow, player[current_player].selcol, row, col, move_str);
					//TODO:
					// Send move over serial here if connected..?
					// Or do I save that for the end of this entire if statement? 
					// Guess and check 😂
					K=move_str[0]-16*move_str[1]+C;
					L=move_str[2]-16*move_str[3]+C;
					    
					F(i,0,U)A[i].K = 0;
					if(((check_test = D(k,-I,I,Q,1,1,O,9,2)) == I)) {
						player[current_player].draw_selection = false;
						draw_box(BACK_COLOR, player[current_player].selcol, player[current_player].selrow);
						gfx_SetDrawBuffer();
						draw_board();
						player[current_player].first_move_done = true;
						k^=24; current_player ^= 1;
						steps++;
						play_as = get_player_color();
						draw_controls();
						gfx_SwapDraw();
						gfx_SetDrawScreen();
					} else if (check_test == -I/2){
						in_check();
					}
				}
			
				if (get_piece_color(row, col) == get_player_color()) {
					draw_box(BACK_COLOR, player[current_player].selcol, player[current_player].selrow);
					player[current_player].selcol = col;
					player[current_player].selrow = row;
					player[current_player].draw_selection = true;
				}
			}
		}
    }
    
    /* save everything */
	save_save();
exit:
	gfx_SetDrawBuffer();
    
    return;
}

void fast_exit(void) {
    gfx_End();
    prgm_CleanUp();
    exit(1);
}

int D(int k, int q, int l, int e, int J, int Z, int E, int z, int n) {
    int j, r, m = 0, v, d, h, i = 9, F, G;
    char t, p, u, x, y, X, Y, H, B;
    struct _ * a = A;
    
    if(os_GetCSC() == sk_Del) {
        fast_exit();
    }
    
    j = (k * E ^ J) & U - 9;
    W((h = A[++j].K) && h - Z && --i);
    a += i ? j : 0;
    if (a -> K) {
        d = a -> D;
        v = a -> V;
        X = a -> X;
        if (d >= n) {
            if (v >= l | X & S && v <= q | X & 8) return v;
            d = n - 1;
        }
        X &= ~M;
        Y = a -> Y;
        Y = d ? Y : 0;
    } else d = X = Y = 0;
    N++;
    W(d++ < n | z == 8 & N < 128 & d < 98) {
        x = B = X;
        Y |= 8 & Y >> 4;
        m = d > 1 ? -I : e;
        do {
            u = b[x];
            if (u & k) {
                r = p = u & 7;
                j = o[p + 16];
                W(r = p > 2 & r < 0 ? -r : -o[++j]) {
                    A: y = x;F = G = S;
                    do {
                        H = y += r;
                        if (Y & 8) H = y = Y & ~M;
                        if (y & M) break;
                        if (p < 3 & y == E) H = y ^ 16;
                        t = b[H];
                        if (t & k | p < 3 & !(r & 7) != !t) break;
                        i = 99 * w[t & 7];
                        if (i < 0 || E - S && b[E] && y - E < 2 & E - y < 2) m = I;
                        if (m >= l) goto C;
                        if (h = d - (y != z)) {
                            v = p < 6 ? b[x + 8] - b[y + 8] : 0;
                            b[G] = b[H] = b[x] = 0;
                            b[y] = u & 31;
                            if (!(G & M)) {
                                b[F] = k + 6;
                                v += 30;
                            }
                            if (p < 3) {
                                v -= 9 * (((x - 2) & M || b[x - 2] != u) +
                                    ((x + 2) & M || b[x + 2] != u) - 1);
                                if (y + r + 1 & S) {
                                    b[y] |= 7;
                                    i += C;
                                }
                            }
                            v = -D(24 - k, -l - (l > e), m > q ? -m : -q, -e - v - i,
                                J + J(0), Z + J(8) + G - S, F, y, h);
                            v -= v > e;
                            if (z == 9) {
                                if (v != -I & x == K & y == L) {
                                    Q = -e - i;
                                    O = F;
                                    return l;
                                }
                                v = m;
                            }
                            b[G] = k + 38;
                            b[F] = b[y] = 0;
                            b[x] = u;
                            b[H] = t;
                            if (Y & 8) {
                                m = v;
                                Y &= ~8;
                                goto A;
                            }
                            if (v > m) {
                                m = v;
                                X = x;
                                Y = y | S & G;
                            }
                        }
                        t += p < 5;
                        if (p < 3 & 6 * k + (y & V) == S || (u & ~24) == 36 & j == 7 &&
                            G & M && b[G = (x | 7) - (r >> 1 & 7)] & 32 && !(b[G ^ 1] | b[G ^ 2])
                        ) {
                            F = y;
                            t--;
                        }
                    }
                    W(!t);
                }
            }
        }
        W((x = x + 9 & ~M) - B);
        C: if (m > I / 4 | m < -I / 4) d = 99;
        m = m + I ? m : -D(24 - k, -I, I, 0, J, Z, S, S, 1) / 2;
        if (!a -> K | (a -> X & M) != M | a -> D <= d) {
            a -> K = Z;
            a -> V = m;
            a -> D = d;
            A -> K = 0;
            a -> X = X | 8 * (m > q) | S * (m < l);
            a -> Y = Y;
        }
    }
    if (z & 8) {
        K = X;
        L = Y & ~M;
    }
    return m;
}

/**
 * Draws the board in the buffer
 */
void draw_board(void) {
	uint24_t x;
	uint8_t y, i = 0, tmp;
	unsigned curr = 0;
	gfx_image_t *curr_sprite;
    
	/* clear the buffer before we try to draw anything */
	gfx_FillScreen(BACK_COLOR);
	
	/* draw all the lines */
	gfx_SetColor(SEP_COLOR);
	for(y = 14; y <= BLOCK_SIZE * NUM_BLOCKS + 14; y += BLOCK_SIZE) {
		gfx_HorizLine_NoClip(14, y, BLOCK_SIZE * NUM_BLOCKS);
	}
	for(x = 14; x <= BLOCK_SIZE * NUM_BLOCKS + 14; x += BLOCK_SIZE) {
		gfx_VertLine_NoClip(x, 14, BLOCK_SIZE * NUM_BLOCKS + 1);
	}

	/* draw the rectangles */
	for(x = 14; x < BLOCK_SIZE * NUM_BLOCKS + 14; x += BLOCK_SIZE) {
		for(y = 14; y < BLOCK_SIZE * NUM_BLOCKS + 14; y += BLOCK_SIZE) {
			gfx_SetColor((i & 1) ? WHITE_COLOR : BLACK_COLOR);
			gfx_FillRectangle_NoClip(x + 2, y + 2, BLOCK_SIZE - 3, BLOCK_SIZE - 3);
			i++;
		}
		i++;
	}
    
	i = 0;
	for(x = 23; x < BLOCK_SIZE * NUM_BLOCKS + 14; x += BLOCK_SIZE) {
		gfx_SetTextXY(x, 227);
		gfx_PrintChar(a_cols[i++]);
	}

	i = 8;
	for(y = 23; y < BLOCK_SIZE * NUM_BLOCKS + 14; y += BLOCK_SIZE) {
		gfx_SetTextXY(3, y);
		gfx_PrintUInt(i--, 1);
	}

	for(y = 14; y < BLOCK_SIZE * NUM_BLOCKS + 14; y += BLOCK_SIZE) {
		for(x = 14; x < BLOCK_SIZE * NUM_BLOCKS + 14; x += BLOCK_SIZE) {
			curr_sprite = pieces[b[curr] & 15];
			curr = curr+9 & ~8;
			if(curr_sprite)
				gfx_TransparentSprite_NoClip(curr_sprite, x + 5, y + 4);
		}
	}
}

void get_settings(void) {
	uint8_t key = 1;
    
	/* wait until 2nd or enter or clear is pressed before continuing */
	while(key != sk_Enter && key != sk_2nd && key != sk_Clear) {
		if(key) {
			print_settings();
		}
		key = os_GetCSC();

		if(key == sk_Del) fast_exit();
    
		/* down pressed */
		if (key == sk_Down) {
			settings_item = (settings_item + 1) % 3;
			if (settings.mode) {
				settings_item = 2;
			}
		}
		/* up pressed */
		if (key == sk_Up) {
			settings_item = settings_item - 1 < 0 ? 2 : settings_item - 1;
			if (settings.mode) {
				settings_item = 0;
			}
		}
		/* left pressed */
		if (key == sk_Left) {
			if(settings_item == 0) {
				settings.mode = settings.mode - 1 < 0 ? 2 : settings.mode - 1;
			}
		}
		/* right pressed */
		if (key == sk_Right) {
			if(settings_item == 0) {
				settings.mode = (settings.mode + 1) % 3;
			}
		}
		if (key == sk_Right || key == sk_Left) {
			switch(settings_item) {
			case 1:
				settings.playingas ^= 1;
				break;
			case 2:
				settings.start ^= 1;
				break;
			default:
				break;
			}
		}
	}
}

void print_settings(void) {
	char *str = NULL;
	draw_logo();
	gfx_SetTextXY(85, (240 - 8) / 2 + 10);
	gfx_PrintString(settings_item == 0 ? "\x10 " : "");
	gfx_PrintString(mode_str);
	switch(settings.mode) {
	case 0:
		str = human1_str;
		break;
	case 1:
		str = calc2_str;
		break;
	case 2:
		str = remote1_str;
	case 3:
		str = human2_str;
		break;
	default:
		break;
	}

	gfx_PrintStringXY(str, 180, (240 - 8) / 2 + 10);

	gfx_SetTextXY(85, (240 - 8) / 2 + 22);
	gfx_PrintString(settings_item == 1 ? "\x10 " : "");
	gfx_PrintString(play_as_str);
	gfx_PrintStringXY(settings.playingas ? black_str : white_str, 180, (240 - 8) / 2 + 22);
	if (settings.mode) {
		gfx_SetColor( BACK_COLOR );
		gfx_HorizLine_NoClip(85, (240 - 8) / 2 + 26, 320 - 95);
		gfx_SetColor( GRAY_COLOR );
	}

	gfx_SetTextXY(85, (240 - 8) / 2 + 34);
	gfx_PrintString(settings_item == 2 ? "\x10 " : "");
	gfx_PrintString(will_start_str);
	gfx_PrintStringXY(settings.start ? black_str : white_str, 180, (240 - 8) / 2 + 34);
	
	gfx_PrintStringXY(arrows_str, (320 - gfx_GetStringWidth(arrows_str)) / 2, 240 - 16);
	gfx_PrintStringXY(info_str, (320 - gfx_GetStringWidth(info_str)) / 2, 240 - 16 - 16);
	gfx_HorizLine_NoClip(0, 200, 320);
	gfx_SwapDraw();
}

void draw_red_text(char *text, uint16_t x, uint8_t y) {
	gfx_SetTextFGColor( RED_COLOR );
	gfx_PrintStringXY( text, x, y );
	gfx_SetTextFGColor( GRAY_COLOR );
}

void print_home(void) {
	draw_logo();
	gfx_SetTextXY(HOME_LOC, (240 - 8) / 2 + 10);
	gfx_PrintString(home_item == 0 ? "\x10 " : "");
	gfx_PrintString(new_game);

	gfx_SetTextXY(HOME_LOC, (240 - 8) / 2 + 22);
	gfx_PrintString(home_item == 1 ? "\x10 " : "");
	gfx_PrintString(load_game);

	gfx_SetTextXY(HOME_LOC, (240 - 8) / 2 + 34);
	gfx_PrintString(home_item == 2 ? "\x10 " : "");
	gfx_PrintString(settings_str);

	gfx_SetColor( GRAY_COLOR );
	gfx_HorizLine_NoClip(0, 200, 320);
	gfx_PrintStringXY(them, (320 - gfx_GetStringWidth(them)) / 2, 240 - 16);
	gfx_PrintStringXY(me, (320 - gfx_GetStringWidth(me)) / 2, 240 - 16 - 16);
}

void draw_controls(void) {
	gfx_PrintStringXY(current_player ? white_turn_str : black_turn_str, 230, 15);
	gfx_PrintStringXY("steps - ", 230, 49);
	gfx_PrintUInt(steps, 4);
}

void init_board(void) {
    int j;
    
    /* seed randoms */
    srand( rtc_Time() );

    memset(b, 0, sizeof(b));

    /* setup the default board */
    F(i, 0, 8) {
        b[i] = (b[i + V] = o[i + 24] + 40) + 8;
        b[i + 16] = 18;
        b[i + 96] = 9;
        F(j, 0, 8) b[16 * j + i + 8] = (i - 4) * (i - 4) + (j - 3.5) * (j - 3.5);
    }
    F(i, M, 1035) T[i] = rand() >> 9;
}

void game_reset(void) {
	steps = 0;
	current_player = settings.start ^ 1;
    memset(&player[0], 0, sizeof(player_t));
	memset(&player[1], 0, sizeof(player_t));
}

bool load_save(void) {
	ti_var_t save;

	ti_CloseAll();

	save = ti_Open(appvar_name, "r");

	if(save) {
		if (ti_GetC(save) == VERSION) {
			if (ti_Read(b, sizeof(b), 1, save) != 1) {
				goto fail;
			}
            if (ti_Read(T, sizeof(T), 1, save) != 1) {
				goto fail;
			}
			if (ti_Read(&settings, sizeof(settings_t), 1, save) != 1) {
				goto fail;
			}
			if (ti_Read(&player[0], sizeof(player_t), 1, save) != 1) {
				goto fail;
			}
			if (ti_Read(&player[1], sizeof(player_t), 1, save) != 1) {
				goto fail;
			}
			if (ti_Read(&steps, sizeof(unsigned), 1, save) != 1) {
				goto fail;
			}
			if (ti_Read(&current_player, sizeof(unsigned), 1, save) != 1) {
				goto fail;
			}
		} else {
			goto fail;
		}
	} else {
        goto fail;
    }
    
    ti_CloseAll();
    return true;
    
fail:
	ti_CloseAll();
    return false;
}

void save_save(void) {
	ti_var_t save;

	ti_CloseAll();

	draw_red_text("save...", 252, 116);
    
	save = ti_Open(appvar_name, "w");

	if(save) {
		ti_PutC(VERSION, save);
		if (ti_Write(b, sizeof(b), 1, save) != 1) {
			goto err;
		}
		if (ti_Write(T, sizeof(T), 1, save) != 1) {
			goto err;
		}
		if (ti_Write(&settings, sizeof(settings_t), 1, save) != 1) {
			goto err;
		}
		if (ti_Write(&player[0], sizeof(player_t), 1, save) != 1) {
			goto err;
		}
		if (ti_Write(&player[1], sizeof(player_t), 1, save) != 1) {
			goto err;
		}
		if (ti_Write(&steps, sizeof(unsigned), 1, save) != 1) {
			goto err;
		}
		if (ti_Write(&current_player, sizeof(unsigned), 1, save) != 1) {
			goto err;
		}
	}

	ti_CloseAll();
	return;
	
err:
	ti_Delete(appvar_name);
    ti_CloseAll();
}

void game_over(void) {
	uint8_t key;
    
    gfx_SetDrawBuffer();
    draw_board();
    draw_controls();
    draw_red_text(current_player == WHITE ? "white wins!" : "black wins!", 237, (240 - 8) / 2);
    gfx_SwapDraw();
    
    while((key = os_GetCSC()) != sk_Clear && key != sk_2nd && key != sk_Enter);
    
    /* reset the game */
    init_board();
    game_reset();
}

void game_loop(void) {
	uint8_t key = 1;
	home_item = 1;
	
	for(;;) {
		boot_ClearVRAM();
		gfx_SetDrawBuffer();
		gfx_SetTextFGColor( GRAY_COLOR );
		gfx_SetTextBGColor( BACK_COLOR );
	
		do {
			/* wait until 2nd or enter or clear is pressed before continuing */
			while(key != sk_Enter && key != sk_2nd && key != sk_Clear) {
				if(key) {
					print_home();
					gfx_SwapDraw();
				}
				key = os_GetCSC();

				/* down pressed */
				if (key == sk_Down) {
					home_item = (home_item + 1) % 3;
				}
				/* up pressed */
				if (key == sk_Up) {
					home_item = home_item - 1 < 0 ? 2 : home_item - 1;
				}
			}

			if (key == sk_Clear) {
				break;
			}

			if(key == sk_Del) fast_exit();
    
			if (home_item == 2) {
				get_settings();
			}
			key = 1;
		} while(home_item == 2);

		if (key == sk_Clear) {
			break;
		}

		/* clear the stuffs */
		gfx_FillScreen(BACK_COLOR);
		gfx_SwapDraw();

		/* set the palette colors */
		gfx_palette[RED_COLOR] = gfx_RGBTo1555(191, 0, 0);
		gfx_palette[GRAY_COLOR] = gfx_RGBTo1555(127, 127, 127);
		gfx_palette[BLACK_COLOR] = gfx_RGBTo1555(239, 211, 150);
		gfx_palette[WHITE_COLOR] = gfx_RGBTo1555(190, 121, 81);
		gfx_palette[SEP_COLOR] = gfx_RGBTo1555(143, 106, 64);

		/* check if we need to load the save file */
		if (home_item == 1) {
			if(!(load_save())) {
				game_reset();
				init_board();
			}
		} else {
			game_reset();
			init_board();
		}
		run_game();
	}
	boot_ClearVRAM();
}


srl_device_t srl;
bool has_srl_device = false;
uint8_t srl_buf[512];
bool serial_init_data_sent = false;
usb_error_t usb_error;
const usb_standard_descriptors_t *usb_desc;


usb_error_t handle_usb_event(usb_event_t event, void *event_data, usb_callback_data_t *callback_data)
{
    usb_error_t err;
    if ((err = srl_UsbEventCallback(event, event_data, callback_data)) != USB_SUCCESS)
        return err;
    if (event == USB_DEVICE_CONNECTED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE))
    {
        usb_device_t device = event_data;
        USB_connected = true;
        usb_ResetDevice(device);
    }

    if (event == USB_HOST_CONFIGURE_EVENT || (event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)))
    {

        if (has_srl_device)
            return USB_SUCCESS;

        usb_device_t device;
        if (event == USB_HOST_CONFIGURE_EVENT)
        {
            device = usb_FindDevice(NULL, NULL, USB_SKIP_HUBS);
            if (device == NULL)
                return USB_SUCCESS;
        }
        else
        {
            device = event_data;
        }

        srl_error_t error = srl_Open(&srl, device, srl_buf, sizeof srl_buf, SRL_INTERFACE_ANY, 9600);
        if (error)
        {
            gfx_End();
            os_ClrHome();
            printf("Error %d initting serial\n", error);
            while (os_GetCSC())
                ;
            return 0;
            return USB_SUCCESS;
        }
        has_srl_device = true;
    }

    if (event == USB_DEVICE_DISCONNECTED_EVENT)
    {
        usb_device_t device = event_data;
        if (device == srl.dev)
        {
            USB_connected = false;
            srl_Close(&srl);
            has_srl_device = false;
        }
    }

    return USB_SUCCESS;
}

/* CONNECTION FUNCTIONS */
void SendSerial(const char *message) {
    size_t totalBytesWritten = 0;
    size_t messageLength = strlen(message);

    while (totalBytesWritten < messageLength) {
        int bytesWritten = srl_Write(&srl, message + totalBytesWritten, messageLength - totalBytesWritten);

        if (bytesWritten < 0) {
            printf("SRL W ERR");
        }

        totalBytesWritten += bytesWritten;
    }
}


void readSRL()
{
    size_t bytes_read = srl_Read(&srl, in_buffer, sizeof in_buffer);

    if (bytes_read < 0)
    {
        // has_srl_device = false;
        printf("SRL 0B");
    }
    else if (bytes_read > 0)
    {
        in_buffer[bytes_read] = '\0';
        has_unread_data = true;

		// TODO: rip out all the GFX to make it match this game instead

        /* BRIDGE CONNECTED GFX */
        if (strcmp(in_buffer, "bridgeConnected") == 0) {
            bridge_connected = true;
            gfx_SetColor(0x00);
            gfx_FillRectangle(((GFX_LCD_WIDTH - gfx_GetStringWidth("Bridge disconnected!")) / 2), 80, gfx_GetStringWidth("Bridge disconnected!"), 15);
            gfx_SetColor(0x00);
            gfx_PrintStringXY("Bridge connected!", ((GFX_LCD_WIDTH - gfx_GetStringWidth("Bridge connected!")) / 2), 80);
        }
        if (strcmp(in_buffer, "bridgeDisconnected") == 0) {
            bridge_connected = false;
            gfx_SetColor(0x00);
            gfx_FillRectangle(((GFX_LCD_WIDTH - gfx_GetStringWidth("Bridge disconnected!")) / 2), 80, gfx_GetStringWidth("Bridge disconnected!"), 15);
            gfx_SetColor(0x00);
            gfx_PrintStringXY("Bridge disconnected!", ((GFX_LCD_WIDTH - gfx_GetStringWidth("Bridge disconnected!")) / 2), 80);
        }

        /* Internet Connected GFX */
        if (strcmp(in_buffer, "SERIAL_CONNECTED_CONFIRMED_BY_SERVER") == 0) {
            internet_connected = true;
            gfx_SetColor(0x00);
            gfx_FillRectangle(((GFX_LCD_WIDTH - gfx_GetStringWidth("Internet disconnected!")) / 2), 110, gfx_GetStringWidth("Internet disconnected!"), 15);
            gfx_SetColor(0x00);
            gfx_PrintStringXY("Internet connected!", ((GFX_LCD_WIDTH - gfx_GetStringWidth("Internet connected!")) / 2), 110);
        }
        if (strcmp(in_buffer, "internetDisconnected") == 0) {
            internet_connected = false;
            gfx_SetColor(0x00);
            gfx_FillRectangle(((GFX_LCD_WIDTH - gfx_GetStringWidth("Internet disconnected!")) / 2), 110, gfx_GetStringWidth("Internet disconnected!"), 15);
            gfx_SetColor(0x00);
            gfx_PrintStringXY("Internet disconnected!", ((GFX_LCD_WIDTH - gfx_GetStringWidth("Internet disconnected!")) / 2), 110);
        }


        if (strcmp(in_buffer, "ALREADY_CONNECTED") == 0) {
            alreadyConnectedScreen();
        }

        if (strcmp(in_buffer, "USER_NOT_FOUND") == 0) {
            userNotFoundScreen();
        }

        if (startsWith(in_buffer, "YOUR_IP:")) {
            displayIP(in_buffer + strlen("YOUR_IP:"));
        }

        has_unread_data = false;
    }
}

void sendSerialInitData()
{
    serial_init_data_sent = true;
    char init_serial_connected_text_buffer[17] = "SERIAL_CONNECTED";
    SendSerial(init_serial_connected_text_buffer);
}



void main(void) {
	ti_var_t savefile;
	gfx_Begin( gfx_8bpp );
	gfx_SetPalette(all_gfx_pal, sizeof(all_gfx_pal), 0);

	const usb_standard_descriptors_t *usb_desc = srl_GetCDCStandardDescriptors();
    usb_error_t usb_error = usb_Init(handle_usb_event, NULL, usb_desc, USB_DEFAULT_INIT_FLAGS);
    if (usb_error)
    {
        printf("usb init error\n%u\n", usb_error);
        sleep(2);
        return 1;
    }


	//Open and check for multiplayer game data
	appvar = ti_Open("CHESSNET","r");
	if (appvar == 0){
		// No game set to join
	} else {
		// read game data to be used to join a match
		// TODO: figure out a standard for this and implement it
		// I'm expecting it to be something like IP of server to connect to and port...
		// and maybe username of person being played against

	}



	/* enter the main game loop */
	game_loop();

	/* archive the save file */
	savefile = ti_Open(appvar_name,"r");
	if(savefile) {
		ti_SetArchiveStatus(true,savefile);
	}
	ti_CloseAll();
	
	/* close the graphics and return to the OS */
	gfx_End();
	prgm_CleanUp();
}