/**        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                   Version 2, December 2004
 *
 * Copyright (C) 2019 Matthias Gatto <uso.cosmo.ray@gmail.com>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#include <yirl/entity.h>
#include <yirl/events.h>
#include <yirl/entity-script.h>
#include <yirl/game.h>

#define NB_MONSTERS 2

#define ATK_BAR_MAX 10

enum {
	BASE_POS,
	DONT_KNOW_POS,
	L_POS_0,
	L_POS_1,
	R_POS_0,
	R_POS_1
};

enum {
	ML_POS,
	MR_POS,
};

static Entity *grp_left;
static Entity *grp_right;
static Entity *grp_down;
static Entity *grp_atk;

static int atk_bar;

static int jmp_power = -1;
static int player_pos;

static int atk_state;
static int atk_dir;

static void *die(Entity *ai)
{
	printf("YOU ARE DEAD !\n");
	ygTerminate();
}

static void print_mob(Entity *mob)
{
	YE_FOREACH(mob, str) {
		printf("%s\n", yeGetString(str));
	}
}

static Entity *check_monster_col(Entity *msp, Entity *pos, int w, int h)
{
	yeAutoFree Entity *ps = ywSizeCreate(w, h, NULL, NULL);
	yeAutoFree Entity *pr = ywRectCreatePosSize(pos, ps, NULL, NULL);

	for (int j = 0; j < yeLen(msp); ++j) {
		Entity *minfo = yeGet(msp, j);
		int mt = yeGetIntAt(minfo, 0);
 		Entity *mpos = yeGet(minfo, 1);
		yeAutoFree Entity *ms;
		yeAutoFree Entity *mr;

		if (mt == 0)
			ms = ywSizeCreate(4, 2, NULL, NULL);
		else
			ms = ywSizeCreate(1, 1, NULL, NULL);
		mr = ywRectCreatePosSize(mpos, ms, NULL, NULL);
		ywPosAddXY(mr, 1, 0);
		if (ywRectCollision(pr, mr))
			return minfo;
	}
	return NULL;
}

static void draw_stuff(Entity *txt, Entity *stuff, Entity *stuff_p)
{
	int x = ywPosX(stuff_p);
	int y = ywPosY(stuff_p);
	int l = yeLen(stuff);
	for (int i = 0; i < l; ++i) {
		/* 3 because 2 case of txt threshold */
		/* (the life string and the |--| on top) */
		/* + 1 because voila */
		yeStringReplaceStrAt(yeGet(txt, y - l + i + 3),
				     yeGetStringAt(stuff, i),
				     x);
	}
}

static void draw_level(Entity *ai, Entity *level)
{
	Entity *txt = yeGet(ai, "text");
	Entity *pjp = yeGet(ai, "pjp");
	Entity *pj = yeGet(yeGet(ai, "pj"), player_pos);
	Entity *monsters = yeGet(ai, "monsters");

	for (int i = 0; i < yeLen(level); ++i) {
		Entity *ll = yeGet(level, i);
		yeSetStringAt(txt, i + 2, yeGetString(ll));
	}

	draw_stuff(txt, pj, pjp);

	if (atk_state) {
		YE_NEW(Array, atk_sprite);
		Entity *pos = yeGet(ai, "atk_p");

		if (atk_dir == L_POS_0) {
			YEntityBlock {
				if (atk_state == 1) {
					atk_sprite = [
						"\\  ",
						" \\-",
						"   ",
						];
				} else if (atk_state == 2) {
					atk_sprite = [
						"   ",
						"---",
						"   ",
						];
				} else {
					atk_sprite = [
						"   ",
						" /-",
						"/  ",
						];
				}
			}
			draw_stuff(txt, atk_sprite, pos);
		} else if (atk_dir == R_POS_0) {
			YEntityBlock {
				if (atk_state == 1) {
					atk_sprite = [
						"  /",
						"-/ ",
						"   "
						];
				} else if (atk_state == 2) {
					atk_sprite = [
						"   ",
						"---",
						"   "
						];
				} else {
					atk_sprite = [
						"   ",
						"-\\ ",
						"  \\"
						];
				}
			}
			draw_stuff(txt, atk_sprite, pos);
		}
	}


	Entity *msp = yeGet(ai, "msp");
	for (int j = 0; j < yeLen(msp); ++j) {
		int mt = yeGetIntAt(yeGet(msp, j), 0);
 		Entity *mpos = yeGet(yeGet(msp, j), 1);
		int mn_sp =  yeGetIntAt(yeGet(msp, j), 2);
		Entity *mon = yeGet(yeGet(monsters, mt), mn_sp);

		draw_stuff(txt, mon, mpos);
	}
	YE_NEW(Array, exit);

	YEntityBlock {
		exit = [ " -- ",
			 "|\\/|",
			 "|/\\|"
			];
	}
	draw_stuff(txt, exit, yeGet(ai, "exitp"));
}

void *ai_action(int nbArgs, void **args)
{
	Entity *ai = args[0];
	Entity *eves = args[1];
	Entity *pjp = yeGet(ai, "pjp");
	Entity *lv = yeGet(ai, "lv");
	int on_land = 0, press_jmp = yevIsKeyUp(eves, ' ');
	int l_down = yevIsGrpDown(eves, grp_left);
	int r_down = yevIsGrpDown(eves, grp_right);
	Entity *msp = yeGet(ai, "msp");
	Entity *l_txt = yeGet(yeGet(ai, "text"), 0);
	Entity *life = yeGetIntAt(ai, "life");
	Entity *atk_pos = yeGet(ai, "atk_p");
	Entity *minfo;
	int py;
	static int x_mv;

	if (r_down) {
		x_mv = 1;
		printf("right\n");
	} else if (l_down) {
		x_mv = -1;
		printf("left\n");
	}

	if ((yevIsGrpUp(eves, grp_right) && !l_down) ||
	    (yevIsGrpUp(eves, grp_left) && !r_down)) {
		x_mv = 0;
	}

	if (atk_state) {
		// '& 3' is '% 4' the hipster way
		atk_state = (atk_state + 1) & 3;
	}

	if (yevIsGrpDown(eves, grp_atk) && atk_bar == ATK_BAR_MAX) {
		atk_bar = 0;
		atk_state = 1;
		if (player_pos >= L_POS_0 && player_pos <= L_POS_1) {
			atk_dir = L_POS_0;
		} else if (player_pos >= R_POS_0 && player_pos <= R_POS_1) {
			atk_dir = R_POS_0;
		} else {
			atk_dir = 0;
			player_pos = DONT_KNOW_POS;
		}
	} else if (atk_bar < ATK_BAR_MAX) {
		++atk_bar;
	}

	if (jmp_power < 0) {
		int px = ywPosX(pjp);
		py = ywPosY(pjp);

		if (jmp_power < -5)
			jmp_power = -5;
		for (int i = -1; i >= jmp_power; --i) {
			Entity *ll = yeGet(lv, py - i);
			if (yeStrDoesRangeContainChr(ll, px, px + 3, '=')) {
				jmp_power = i + 1;
				on_land = 1;
			}
		}
	}

	if (yevIsGrpDown(eves, grp_down)) {
		if (!x_mv)
			player_pos = BASE_POS;
	}

	if (x_mv > 0)
		player_pos = player_pos ==  R_POS_0 ? R_POS_1 : R_POS_0;
	else if (x_mv < 0)
		player_pos = player_pos ==  L_POS_0 ? L_POS_1 : L_POS_0;

	ywPosAddXY(pjp, x_mv, -jmp_power);
	ywPosPrint(pjp);
	if (ywPosX(pjp) < 1) {
		ywPosSetX(pjp, 1);
	} else if (ywPosX(pjp) > 65 - 3) {
		ywPosSetX(pjp, 65 - 3);
	}

	if (ywPosY(pjp) > 29) {
		ywPosSetY(pjp, 29);
		on_land = 1;
	} else if (jmp_power > 0 && ywPosY(pjp) <= 3) {
		jmp_power = 0;
		ywPosSetY(pjp, 2);
	}
	jmp_power -= 1;
	ywPosPrint(pjp);

	if (press_jmp && on_land) {
		x_mv *= 2;
		jmp_power = 3;
	}

	printf("action !\n");
	Entity *mcalls = yeGet(ai, "ms_callback");
	YE_FOREACH(msp, ms_i) {
		int mt = yeGetIntAt(ms_i, 0);

		yesCall(yeGet(mcalls, mt), ai, ms_i);
	}
	if ((minfo = check_monster_col(msp, pjp, 3, 3))) {
		Entity *l = yeGet(ai, "life");
		Entity *mpos = yeGet(minfo, 1);

		yeSubInt(l, 1);
		printf("bad time is gonna happen:( %p %p\n", mpos,
		       pjp);
		if (ywPosX(mpos) > ywPosX(pjp))
			ywPosAddXY(pjp, -3, 0);
		else
			ywPosAddXY(pjp, 3, 0);
		if (yeGetInt(l) < 1)
			return die(ai);
	}
	if (atk_state) {
		ywPosSet(atk_pos, pjp, 0);
		if (atk_dir == L_POS_0) {
			ywPosSubXY(atk_pos, 3, 0);
		} else if (atk_dir == R_POS_0) {
			ywPosAddXY(atk_pos, 3, 0);
		}
		if ((minfo = check_monster_col(msp, atk_pos, 3, 3))) {
			printf("TOUCH TOUCH kokoni TOUCH\n");
			yeRemoveChild(msp, minfo);
		}
	}

	yeSetString(l_txt, "life:");
	for (int i = 0; i < life; ++i)
		yeStringAdd(l_txt, " <3");

	int i;
	Entity *b_txt = yeGet(yeGet(ai, "text"), 34);
	yeSetString(b_txt, "AttackReady: |");
	for (i = 0; i < atk_bar; ++i)
		yeStringAddCh(b_txt, '#');
	for (; i < ATK_BAR_MAX; ++i)
		yeStringAddCh(b_txt, '-');
	yeStringAddCh(b_txt, '|');

	draw_level(ai, lv);
	return (void *)ACTION;
}

static Entity *ai_levels(void)
{
	Entity *r = yeCreateArray(NULL, NULL);

#include "levels.c"
	print_mob(yeGet(r, 0));
	return r;
}

void *ai_init(int nbArgs, void **args)
{
	Entity *ai = args[0];
	yeAutoFree Entity *levels = ai_levels();
	Entity *level = yeGet(levels, 0);
	Entity *pj;
	Entity *monsters;
	Entity *msp;

	printf("ai, yume no ai !\n");
	YEntityBlock {
		ai.pj = [
			[
				" o ",
				"/Y\\",
				"/ \\"
				],
			[
				"<o>",
				" Y ",
				"/ \\"
				],
			[
				" o ",
				"/| ",
				"/|  "
				],
			[
				" o ",
				"/| ",
				" ] "
				],
			[
				" o ",
				" |\\",
				" |\\"
				],
			[
				" o ",
				" |\\",
				" [ "
				]

			];
		ai.monsters = [
			[
				[
					"<==<o>",
					"/  \\  "
					],
				[
					"<o>==>",
					"  /  \\"
					]

				],
			[
				[ "\\^/" ], [ "\\^/" ]
				]
			];
		ai.ms_callback = [];
		ai.text = {
		0:       "",
		1:       "|----------------------------------------------------------------|",
		2-32:    "",
		33:      "|________________________________________________________________|",
		34: ""
		};
		ai.lv = level;
		ai.lvs = levels;
		ai.life = 3;
		ai.background = "rgba: 255 255 255 255";
		ai.fmt = "yirl";
		ai.action = ai_action;
		// monsters pos
		ai.atk_p = [0, 0];
		ai.msp = [];
		ai["text-align"] = "center";
		/* ai["turn-length"] = 300000; */
		ai["turn-length"] = 130000;
	}
	/* I can't use the YEntityBlock here because I need lua callback,
	 * and I don;t have the systax yet to add non tcc functions */
	yeCreateFunctionSimple("ai_bad_mob0_callback", ygGetLuaManager(),
			       yeGet(ai, "ms_callback"));
	yeCreateFunctionSimple("ai_bat_callback", ygGetLuaManager(),
			       yeGet(ai, "ms_callback"));
	atk_bar = ATK_BAR_MAX;
	pj = yeGet(yeGet(ai, "pj"), player_pos);
	print_mob(yeGetByStr(ai, "monsters.0.0"));
	print_mob(yeGetByStr(ai, "monsters.0.1"));
	print_mob(yeGet(levels, 0));
	print_mob(pj);
	monsters = yeGet(ai, "monsters");
	msp = yeGet(ai, "msp");
	/* The Great Replacement */
	for (int i = 0; i < yeLen(level); ++i) {
		Entity *ll = yeGet(level, i);
		int px = yeStrChrIdx(ll, '*');
		if (px > 0) {
			yeStringReplaceCharAt(ll, ' ', px);
			YEntityBlock { ai.pjp = [px, i]; };
		}

		px = yeStrChrIdx(ll, '>');
		if (px > 0) {
			yeStringReplaceCharAt(ll, ' ', px);
			YEntityBlock { ai.exitp = [px, i]; };
		}
		for (int j = 0; j < NB_MONSTERS; ++j) {
			px = yeStrChrIdx(ll, '0' + j);
			if (px > 0) {
				Entity *mp = yeCreateArray(msp, NULL);
				yeStringReplaceCharAt(ll, ' ', px);
				YEntityBlock { mp = [j, [px, i], MR_POS]; };
			}
		}
	}
	draw_level(ai, level);
	void *ret = ywidNewWidget(ai, "text-screen");
	return ret;
}

void *mod_init(int nbArg, void **args)
{
	Entity *mod = args[0];
	Entity *init;

	printf("AI init !");
	init = yeCreateArray(NULL, NULL);
	YEntityBlock {
		init.name = "ai";
		init.callback = ai_init;
		mod.name = "ai";
		mod.starting_widget = "test_ai";
		mod.test_ai = [];
		mod["window size"] = [800, 600];
		mod.test_ai["<type>"] = "ai";
		mod["window name"] = "Asc II";
		mod["pre-load"] = [];
		mod["pre-load"][0] = [];
		mod["pre-load"][0].file = "callback.lua";
		mod["pre-load"][0].type = "lua";
	}
	ywidAddSubType(init);
	printf("%p - %p - %p\n", mod, yeGet(mod, "test_ai"),
		yeGet(yeGet(mod, "test_ai"), "<type>"));
	grp_left = yevCreateGrp(NULL, 'a', Y_LEFT_KEY);
	grp_right = yevCreateGrp(NULL, 'd', Y_RIGHT_KEY);
	grp_down = yevCreateGrp(NULL, 's', Y_DOWN_KEY);
	grp_atk = yevCreateGrp(NULL, 'z', '\n', 'w');
	return mod;
}
