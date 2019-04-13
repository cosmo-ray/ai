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

/*
 * I don't think prefix all function by ai was usefull, but 'ai'
 * mean love in japaness, and I find that cute
 * plus I have "ai senshi" singing in my head right now
 */

#include <yirl/entity.h>
#include <yirl/events.h>
#include <yirl/game.h>

#define NB_MONSTERS 2

static Entity *grp_left;
static Entity *grp_right;
static int jmp_power = -1;

static void print_mob(Entity *mob)
{
	YE_FOREACH(mob, str) {
		printf("%s\n", yeGetString(str));
	}
}

static void draw_level(Entity *ai, Entity *level)
{
	Entity *txt = yeGet(ai, "text");
	Entity *pjp = yeGet(ai, "pjp");
	Entity *pj = yeGet(ai, "pj");
	Entity *monsters = yeGet(ai, "monsters");

	for (int i = 0; i < yeLen(level); ++i) {
		Entity *ll = yeGet(level, i);
		yeSetStringAt(txt, i + 2, yeGetString(ll));
		/* yePushAt(txt, ll, i + 2); */
	}

	int px = ywPosX(pjp);
	int py = ywPosY(pjp);
	int pjl = yeLen(pj);
	for (int i = 0; i < pjl; ++i) {
		yeStringReplaceStrAt(yeGet(txt, py - pjl + i + 3),
				     yeGetStringAt(pj, i),
				     px);
	}
	for (int j = 0; j < NB_MONSTERS; ++j) {
		int mt = yeGetIntAt(yeGet(yeGet(ai, "msp"), j), 0);
 		Entity *mpos = yeGet(yeGet(yeGet(ai, "msp"), j), 1);
		Entity *mon = yeGet(monsters, mt);

		px = ywPosX(mpos);
		py = ywPosY(mpos);
		if (px > 0) {
			int mjl = yeLen(mon);
			for (int k = 0; k < mjl; ++k) {
				yeStringReplaceStrAt(yeGet(txt, py - mjl + k + 3),
						     yeGetStringAt(mon, k),
						     px);
			}
		}
	}
}

void *ai_action(int nbArgs, void **args)
{
	Entity *ai = args[0];
	Entity *eves = args[1];
	Entity *pjp = yeGet(ai, "pjp");
	Entity *lv = yeGet(ai, "lv");
	int on_land = 0;
	int py;
	static int x_mv;

	if (yevIsGrpUp(eves, grp_right) ||
	    yevIsGrpUp(eves, grp_left)) {
		x_mv = 0;
	}

	if (yevIsGrpDown(eves, grp_right)) {
		x_mv = 1;
		printf("right\n");
	} else if (yevIsGrpDown(eves, grp_left)) {
		x_mv = -1;
		printf("left\n");
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
		ywPosSetY(pjp, 2);
	}
	jmp_power -= 1;
	ywPosPrint(pjp);

	if (yevIsKeyUp(eves, ' ') && on_land) {
		x_mv *= 2;
		jmp_power = 3;
	}

	printf("action !\n");
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
			" O ",
			"/Y\\",
			"/ \\"
			];
		ai.monsters = [
			[
				"<==<o>",
				"/  \\"
				],
			[
				"\\^/"
				]
			];
		ai.text = {
		0:       "life: <3 <3 <3",
		1:       "|----------------------------------------------------------------|",
		2-32:    "",
		33 :     "|________________________________________________________________|"
		};
		ai.lv = level;
		ai.lvs = levels;
		ai.life = 3;
		ai.background = "rgba: 255 255 255 255";
		ai.fmt = "yirl";
		ai.action = ai_action;
		// monsters pos
		ai.msp = [];
		ai["text-align"] = "center";
		/* ai["turn-length"] = 300000; */
		ai["turn-length"] = 130000;
	}

	pj = yeGet(ai, "pj");
	print_mob(yeGetByStr(ai, "monsters.0"));
	print_mob(yeGetByStr(ai, "monsters.1"));
	print_mob(yeGet(levels, 0));
	print_mob(pj);
	monsters = yeGet(ai, "monsters");
	msp = yeGet(ai, "msp");
	/* The Great Replacement */
	for (int i = 0; i < yeLen(level); ++i) {
		Entity *ll = yeGet(level, i);
		int px = yeStrChrIdx(ll, '*');
		printf("chr: %d\n", px);
		if (px > 0) {
			YEntityBlock { ai.pjp = [px, i]; };
		}
		for (int j = 0; j < NB_MONSTERS; ++j) {
			Entity *mon = yeGet(monsters, j);
			px = yeStrChrIdx(ll, '0' + j);
			if (px > 0) {
				Entity *mp = yeCreateArray(msp, NULL);
				YEntityBlock { mp = [j, [px, i]]; };
			}
		}
	}
	draw_level(ai, level);
	ywPosPrint(yeGet(ai, "pjp"));
	ywPosPrint(yeGet(yeGet(yeGet(ai, "msp"), 0), 1));
	ywPosPrint(yeGet(yeGet(yeGet(ai, "msp"), 1), 1));
	printf("%p\n", yeGet(yeGet(ai, "text"), 0));
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
	}
	ywidAddSubType(init);
	printf("%p - %p - %p\n", mod, yeGet(mod, "test_ai"),
		yeGet(yeGet(mod, "test_ai"), "<type>"));
	grp_left = yevCreateGrp(NULL, 'a', Y_LEFT_KEY);
	grp_right = yevCreateGrp(NULL, 'd', Y_RIGHT_KEY);
	return mod;
}
