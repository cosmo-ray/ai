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
#include <yirl/game.h>

static void print_mob(Entity *mob)
{
	YE_FOREACH(mob, str) {
		printf("%s\n", yeGetString(str));
	}
}

void *ai_action(int nbArgs, void **args)
{
	printf("action !\n");
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
	Entity *txt;

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
		0:       "life: {ai.life}",
		1:       "|----------------------------------------------------------------|",
		33 :     "|________________________________________________________________|"
		};
		ai.lv = level;
		ai.lvs = levels;
		ai.life = 3;
		ai.background = "rgba: 255 255 255 255";
		ai.fmt = "yirl";
		ai.action = ai_action;
		ai["text-align"] = "center";
		ai["turn-length"] = 200000;
	}

	//printf("%p - %p\n", yeGet(ai, "lv"), yeGet(ai, "lvs"));
	txt = yeGet(ai, "text");
	pj = yeGet(ai, "pj");
	print_mob(yeGetByStr(ai, "monsters.0"));
	print_mob(yeGetByStr(ai, "monsters.1"));
	print_mob(yeGet(levels, 0));
	print_mob(pj);
	for (int i = 0; i < yeLen(level); ++i) {
		Entity *ll = yeGet(level, i);
		int px = yeStrChrIdx(ll, 'P');
		printf("chr: %d\n", px);
		if (px > 0) {
			int pjl = yeLen(pj);
			for (int j = 0; j < pjl; ++j) {
				yeStringReplaceStrAt(yeGet(level, i - pjl + j + 1),
						     yeGetStringAt(pj, j),
						     px);
			}
			YEntityBlock { ai.pjp = [px, i]; };
		}
		yePushAt(txt, ll, i + 2);
	}
	ywPosPrint(yeGet(ai, "pjp"));
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
	yeCreateString("ascii-trail", init, "name");
	yeCreateFunction("ai_init", ygGetManager("tcc"), init, "callback");
	ywidAddSubType(init);
	YEntityBlock {
		mod.starting_widget = "test_ai";
		mod.test_ai = [];
		mod["window size"] = [800, 600];
		mod.test_ai["<type>"] = "ascii-trail";
	}
	printf("%p - %p - %p\n", mod, yeGet(mod, "test_ai"),
		yeGet(yeGet(mod, "test_ai"), "<type>"));
	return mod;
}
