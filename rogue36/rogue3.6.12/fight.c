/*
 * All the fighting gets done here
 *
 * @(#)fight.c	3.28 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "curses.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "rogue.h"

int e_levels[] = {
    10,20,40,80,160,320,640,1280,2560,5120,10240,20480,
    40920, 81920, 163840, 327680, 655360, 1310720, 2621440, 0 };

/*
 * fight:
 *	The player attacks the monster.
 */

int
fight(coord *mp, int mn, struct object *weap, int thrown)
{
    struct thing *tp;
    struct linked_list *item;
    int did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL)
    {
	debug("Fight what @ %d,%d", mp->y, mp->x);
	return(0);
    }
    tp = (struct thing *) ldata(item);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    quiet = 0;
    runto(mp, &hero);
    /*
     * Let him know it was really a mimic (if it was one).
     */
    if (tp->t_type == 'M' && tp->t_disguise != 'M' && off(player, ISBLIND))
    {
	msg("Wait! That's a mimic!");
	tp->t_disguise = 'M';
	did_hit = thrown;
    }
    if (did_hit)
    {
	char *mname;

	did_hit = FALSE;
	if (on(player, ISBLIND))
	    mname = "it";
	else
	    mname = monsters[mn-'A'].m_name;
	if (roll_em(&pstats, &tp->t_stats, weap, thrown))
	{
	    did_hit = TRUE;
	    if (thrown)
		thunk(weap, mname);
	    else
		hit(NULL, mname);
	    if (on(player, CANHUH))
	    {
		msg("Your hands stop glowing red");
		msg("The %s appears confused.", mname);
		tp->t_flags |= ISHUH;
		player.t_flags &= ~CANHUH;
	    }
	    if (tp->t_stats.s_hpt <= 0)
		killed(item, TRUE);
	}
	else
	    if (thrown)
		bounce(weap, mname);
	    else
		miss(NULL, mname);
    }
    count = 0;
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */

int
attack(struct thing *mp)
{
    char *mname;

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    quiet = 0;
    if (mp->t_type == 'M' && off(player, ISBLIND))
	mp->t_disguise = 'M';
    if (on(player, ISBLIND))
	mname = "it";
    else
	mname = monsters[mp->t_type-'A'].m_name;
    if (roll_em(&mp->t_stats, &pstats, NULL, FALSE))
    {
	if (mp->t_type != 'E')
	    hit(mname, NULL);
	if (pstats.s_hpt <= 0)
	    death(mp->t_type);	/* Bye bye life ... */
	if (off(*mp, ISCANC))
	    switch (mp->t_type)
	    {
		case 'R':
		    /*
		     * If a rust monster hits, you lose armor
		     */
		    if (cur_armor != NULL && cur_armor->o_ac < 9)
		    {
			if (!terse)
			    msg("Your armor appears to be weaker now. Oh my!");
			else
			    msg("Your armor weakens");
			cur_armor->o_ac++;
		    }
		when 'E':
		    /*
		     * The gaze of the floating eye hypnotizes you
		     */
		    if (on(player, ISBLIND))
			break;
		    if (!no_command)
		    {
			addmsg("You are transfixed");
			if (!terse)
			    addmsg(" by the gaze of the floating eye.");
			endmsg();
		    }
		    no_command += rnd(2)+2;
		    if (no_command > 100 && food_left <= 0)
			death('E');
		when 'A':
		    /*
		     * Ants have poisonous bites
		     */
		    if (!save(VS_POISON))
			if (!ISWEARING(R_SUSTSTR))
			{
			    chg_str(-1);
			    if (!terse)
				msg("You feel a sting in your arm and now feel weaker");
			    else
				msg("A sting has weakened you");
			}
			else
			    if (!terse)
				msg("A sting momentarily weakens you");
			    else
				msg("Sting has no effect");
		when 'W':
		    /*
		     * Wraiths might drain energy levels
		     */
		    if (rnd(100) < 15)
		    {
			int fewer;

			if (pstats.s_exp == 0)
			    death('W'); 	/* All levels gone */
			msg("You suddenly feel weaker.");
			if (--pstats.s_lvl == 0)
			{
			    pstats.s_exp = 0;
			    pstats.s_lvl = 1;
			}
			else
			    pstats.s_exp = e_levels[pstats.s_lvl-1]+1;
			fewer = roll(1, 10);
			pstats.s_hpt -= fewer;
			max_hp -= fewer;
			if (pstats.s_hpt < 1)
			    pstats.s_hpt = 1;
			if (max_hp < 1)
			    death('W');
		    }
		when 'F':
		    /*
		     * Violet fungi stops the poor guy from moving
		     */
		    player.t_flags |= ISHELD;
		    sprintf(monsters['F'-'A'].m_stats.s_dmg,"%dd1",++fung_hit);
		when 'L':
		{
		    /*
		     * Leperachaun steals some gold
		     */
		    int lastpurse;

		    lastpurse = purse;
		    purse -= GOLDCALC;
		    if (!save(VS_MAGIC))
			purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		    if (purse < 0)
			purse = 0;
		    if (purse != lastpurse)
			msg("Your purse feels lighter");
		    remove_monster(&mp->t_pos, find_mons(mp->t_pos.y, mp->t_pos.x));
		    mp = NULL;
		}
		when 'N':
		{
		    struct linked_list *list, *steal;
		    struct object *obj;
		    int nobj;

		    /*
		     * Nymph's steal a magic item, look through the pack
		     * and pick out one we like.
		     */
		    steal = NULL;
		    for (nobj = 0, list = pack; list != NULL; list = next(list))
		    {
			obj = (struct object *) ldata(list);
			if (obj != cur_armor &&
			    obj != cur_weapon &&
			    obj != cur_ring[LEFT] &&
			    obj != cur_ring[RIGHT] && /* Nymph bug fix */
			    is_magic(obj) &&
			    rnd(++nobj) == 0)
				steal = list;
		    }
		    if (steal != NULL)
		    {
			struct object *sobj;

			sobj = (struct object *) ldata(steal);
			remove_monster(&mp->t_pos, find_mons(mp->t_pos.y, mp->t_pos.x));
			mp = NULL;
			if (sobj->o_count > 1 && sobj->o_group == 0)
			{
			    int oc;

			    oc = sobj->o_count;
			    sobj->o_count = 1;
			    msg("She stole %s!", inv_name(sobj, TRUE));
			    sobj->o_count = oc - 1;
			}
			else
			{
			    msg("She stole %s!", inv_name(sobj, TRUE));
			    detach(pack, steal);
			    discard(steal);
			}
			inpack--;
		    }
		}
		otherwise:
		    break;
	    }
    }
    else if (mp->t_type != 'E')
    {
	if (mp->t_type == 'F')
	{
	    pstats.s_hpt -= fung_hit;
	    if (pstats.s_hpt <= 0)
		death(mp->t_type);	/* Bye bye life ... */
	}
	miss(mname, NULL);
    }
    /*
     * Check to see if this is a regenerating monster and let it heal if
     * it is.
     */
    if ((mp != NULL) && (on(*mp, ISREGEN) && rnd(100) < 33))
	mp->t_stats.s_hpt++;
    if (fight_flush)
    {
	flush_type();	/* flush typeahead */
    }
    count = 0;
    status();

    if (mp == NULL)
	return(-1);
    else
	return(0);
}

/*
 * swing:
 *	returns true if the swing hits
 */

int
swing(int at_lvl, int op_arm, int wplus)
{
    int res = rnd(20)+1;
    int need = (21-at_lvl)-op_arm;

    return (res+wplus >= need);
}

/*
 * check_level:
 *	Check to see if the guy has gone up a level.
 */

void
check_level()
{
    int i, add;

    for (i = 0; e_levels[i] != 0; i++)
	if (e_levels[i] > pstats.s_exp)
	    break;
    i++;
    if (i > pstats.s_lvl)
    {
	add = roll(i-pstats.s_lvl,10);
	max_hp += add;
	if ((pstats.s_hpt += add) > max_hp)
	    pstats.s_hpt = max_hp;
	msg("Welcome to level %d", i);
    }
    pstats.s_lvl = i;
}

/*
 * roll_em:
 *	Roll several attacks
 */

int
roll_em(struct stats *att, struct stats *def, struct object *weap, int hurl)
{
    char *cp;
    int ndice, nsides, def_arm;
    int did_hit = FALSE;
    int prop_hplus, prop_dplus;

    prop_hplus = prop_dplus = 0;
    if (weap == NULL)
	cp = att->s_dmg;
    else if (hurl)
	if ((weap->o_flags&ISMISL) && cur_weapon != NULL &&
	  cur_weapon->o_which == weap->o_launch)
	{
	    cp = weap->o_hurldmg;
	    prop_hplus = cur_weapon->o_hplus;
	    prop_dplus = cur_weapon->o_dplus;
	}
	else
	    cp = (weap->o_flags&ISMISL ? weap->o_damage : weap->o_hurldmg);
    else
    {
	cp = weap->o_damage;
	/*
	 * Drain a staff of striking
	 */
	if (weap->o_type == STICK && weap->o_which == WS_HIT
	    && weap->o_charges == 0)
		{
		    strcpy(weap->o_damage,"0d0");
		    weap->o_hplus = weap->o_dplus = 0;
		}
    }
    for (;;)
    {
	int damage;
	int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	if (weap == cur_weapon)
	{
	    if (ISRING(LEFT, R_ADDDAM))
		dplus += cur_ring[LEFT]->o_ac;
	    else if (ISRING(LEFT, R_ADDHIT))
		hplus += cur_ring[LEFT]->o_ac;
	    if (ISRING(RIGHT, R_ADDDAM))
		dplus += cur_ring[RIGHT]->o_ac;
	    else if (ISRING(RIGHT, R_ADDHIT))
		hplus += cur_ring[RIGHT]->o_ac;
	}
	ndice = atoi(cp);
	if ((cp = strchr(cp, 'd')) == NULL)
	    break;
	nsides = atoi(++cp);
	if (def == &pstats)
	{
	    if (cur_armor != NULL)
		def_arm = cur_armor->o_ac;
	    else
		def_arm = def->s_arm;
	    if (ISRING(LEFT, R_PROTECT))
		def_arm -= cur_ring[LEFT]->o_ac;
	    else if (ISRING(RIGHT, R_PROTECT))
		def_arm -= cur_ring[RIGHT]->o_ac;
	}
	else
	    def_arm = def->s_arm;
	if (swing(att->s_lvl, def_arm, hplus+str_plus(&att->s_str)))
	{
	    int proll;

	    proll = roll(ndice, nsides);
	    if (ndice + nsides > 0 && proll < 1)
		debug("Damage for %dd%d came out %d.", ndice, nsides, proll);
	    damage = dplus + proll + add_dam(&att->s_str);
	    def->s_hpt -= max(0, damage);
	    did_hit = TRUE;
	}
	if ((cp = strchr(cp, '/')) == NULL)
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname:
 *	The print name of a combatant
 */

char *
prname(who, upper)
char *who;
int upper;
{
    static char tbuf[80];

    *tbuf = '\0';
    if (who == 0)
	strcpy(tbuf, "you");
    else if (on(player, ISBLIND))
	strcpy(tbuf, "it");
    else
    {
	strcpy(tbuf, "the ");
	strcat(tbuf, who);
    }
    if (upper)
	*tbuf = toupper(*tbuf);
    return tbuf;
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */

void
hit(char *er, char *ee)
{
    char *s = "";

    addmsg(prname(er, TRUE));
    if (terse)
	s = " hit.";
    else
	switch (rnd(4))
	{
	    case 0: s = " scored an excellent hit on ";
	    when 1: s = " hit ";
	    when 2: s = (er == 0 ? " have injured " : " has injured ");
	    when 3: s = (er == 0 ? " swing and hit " : " swings and hits ");
	}
    addmsg(s);
    if (!terse)
	addmsg(prname(ee, FALSE));
    endmsg();
}

/*
 * miss:
 *	Print a message to indicate a poor swing
 */

void
miss(char *er, char *ee)
{
    char *s = "";

    addmsg(prname(er, TRUE));
    switch (terse ? 0 : rnd(4))
    {
	case 0: s = (er == 0 ? " miss" : " misses");
	when 1: s = (er == 0 ? " swing and miss" : " swings and misses");
	when 2: s = (er == 0 ? " barely miss" : " barely misses");
	when 3: s = (er == 0 ? " don't hit" : " doesn't hit");
    }
    addmsg(s);
    if (!terse)
	addmsg(" %s", prname(ee, FALSE));
    endmsg();
}

/*
 * save_throw:
 *	See if a creature save against something
 */
int
save_throw(int which, struct thing *tp)
{
    int need;

    need = 14 + which - tp->t_stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}

/*
 * save:
 *	See if he saves against various nasty things
 */

int
save(int which)
{
    return save_throw(which, &player);
}

/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */

int
str_plus(str_t *str)
{
    if (str->st_str == 18)
    {
	if (str->st_add == 100)
	    return 3;
	if (str->st_add > 50)
	    return 2;
    }
    if (str->st_str >= 17)
	return 1;
    if (str->st_str > 6)
	return 0;
    return str->st_str - 7;
}

/*
 * add_dam:
 *	compute additional damage done for exceptionally high or low strength
 */

 int
 add_dam(str_t *str)
 {
    if (str->st_str == 18)
    {
	if (str->st_add == 100)
	    return 6;
	if (str->st_add > 90)
	    return 5;
	if (str->st_add > 75)
	    return 4;
	if (str->st_add != 0)
	    return 3;
	return 2;
    }
    if (str->st_str > 15)
	return 1;
    if (str->st_str > 6)
	return 0;
    return str->st_str - 7;
}

/*
 * raise_level:
 *	The guy just magically went up a level.
 */

void
raise_level()
{
    pstats.s_exp = e_levels[pstats.s_lvl-1] + 1L;
    check_level();
}

/*
 * thunk:
 *	A missile hits a monster
 */

void
thunk(struct object *weap, char *mname)
{
    if (weap->o_type == WEAPON)
	msg("The %s hits the %s", w_names[weap->o_which], mname);
    else
	msg("You hit the %s.", mname);
}

/*
 * bounce:
 *	A missile misses a monster
 */

void
bounce(struct object *weap, char *mname)
{
    if (weap->o_type == WEAPON)
	msg("The %s misses the %s", w_names[weap->o_which], mname);
    else
	msg("You missed the %s.", mname);
}

/*
 * remove a monster from the screen
 */
void
remove_monster(coord *mp, struct linked_list *item)
{
    mvwaddch(mw, mp->y, mp->x, ' ');
    mvwaddch(cw, mp->y, mp->x, ((struct thing *) ldata(item))->t_oldch);
    detach(mlist, item);
    discard(item);
}

/*
 * is_magic:
 *	Returns true if an object radiates magic
 */

int
is_magic(struct object *obj)
{
    switch (obj->o_type)
    {
	case ARMOR:
	    return obj->o_ac != a_class[obj->o_which];
	when WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	when POTION:
	case SCROLL:
	case STICK:
	case RING:
	case AMULET:
	    return TRUE;
    }
    return FALSE;
}

/*
 * killed:
 *	Called to put a monster to death
 */

void
killed(struct linked_list *item, int pr)
{
    struct thing *tp;
    struct linked_list *pitem, *nexti;

    tp = (struct thing *) ldata(item);
    if (pr)
    {
	addmsg(terse ? "Defeated " : "You have defeated ");
	if (on(player, ISBLIND))
	    msg("it.");
	else
	{
	    if (!terse)
		addmsg("the ");
	    msg("%s.", monsters[tp->t_type-'A'].m_name);
	}
    }
    pstats.s_exp += tp->t_stats.s_exp;
    /*
     * Do adjustments if he went up a level
     */
    check_level();
    /*
     * If the monster was a violet fungi, un-hold him
     */
    switch (tp->t_type)
    {
	case 'F':
	    player.t_flags &= ~ISHELD;
	    fung_hit = 0;
	    strcpy(monsters['F'-'A'].m_stats.s_dmg, "000d0");
	when 'L':
	{
	    struct room *rp;

	    if ((rp = roomin(&tp->t_pos)) == NULL)
		break;
	    if (rp->r_goldval != 0 || fallpos(&tp->t_pos,&rp->r_gold,FALSE))
	    {
		rp->r_goldval += GOLDCALC;
		if (save(VS_MAGIC))
		    rp->r_goldval += GOLDCALC + GOLDCALC
				   + GOLDCALC + GOLDCALC;
		mvwaddch(stdscr, rp->r_gold.y, rp->r_gold.x, GOLD);
		if (!(rp->r_flags & ISDARK))
		{
		    light(&hero);
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    }
	}
    }
    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;
    while (pitem != NULL)
    {
	struct object *obj;

	nexti = next(tp->t_pack);
	obj = (struct object *) ldata(pitem);
	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, pitem);
	fall(pitem, FALSE);
	pitem = nexti;
    }
    /*
     * Get rid of the monster.
     */
    remove_monster(&tp->t_pos, item);
}
