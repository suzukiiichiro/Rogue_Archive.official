/*
    trader.c - Anything to do with trading posts
     
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    Based on "Advanced Rogue"
    Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rogue.h"

#define EFFECTIVE_PURSE ((player.t_ctype==C_PALADIN)?(9 * purse / 10) : purse)

/*
    do_post()
        Buy and sell things in a trading post
*/

void
display_postinfo(void)
{
    wclear(hw);
    wstandout(hw);
    mvwaddstr(hw, 0, COLS / 2 - 30,
        "Welcome to Friendly Fiend's Flea Market" );
    wstandend(hw);
    wclrtoeol(hw);
    trans_line();   
}

void
do_post(void)
{
    int bad_letter = FALSE;

    player.t_trans = 0;

    for (;;)
    {
        if (!open_market())
            return;

        display_postinfo();

        if (bad_letter)
        {
            bad_letter = FALSE;
            wstandout(hw);
            mvwaddstr(hw, 7, 0, "Type 'i' or 'I' to inventory, "
                                "'l' to leave, 'b' to buy, or 's' to sell");
            wstandend(hw);
        }

        mvwaddstr(hw, 6, 0, "Do you wish to buy, sell, inventory, or leave?");
        wrefresh(hw);

        switch(wgetch(hw))
        {
            case 'b':
                mvwaddstr(hw, 7, 0,
                    "Lets go into the buying section of the store...");
                touchwin(hw);
                wrefresh(hw);
                buy_it('\0', ISNORMAL);
                break;

            case 's':
                mvwaddstr(hw, 7, 0,
                    "Lets go into the selling section of the store...");
                touchwin(hw);
                wrefresh(hw);
                sell_it();
                break;

            case 'i':
                inventory(pack, '*');
                break;

            case 'I':
                wclear(hw);
                wrefresh(hw);
                inventory(pack, 0);
                msg(" ");
                msg("");
                break;

            case 'l':
                wclear(hw);
                wrefresh(hw);
                return;

            default:
                bad_letter = TRUE;
                break;
        }
    }
}

void
buy_it(int itemtype, int flags)
{
    int i;
    int blessed = flags & ISBLESSED;
    int cursed = flags & ISCURSED;
    int is_spell = flags & SCR_MAGIC;
    int array_size; /* # of items within type */
    int which_type; /* Which type to buy */
    int which_one;  /* Which one within type */
    int plus_or_minus = 0;  /* for magic items */
    const struct magic_item   *magic_array = NULL;
    struct linked_list  *item;
    struct object *obj;
    char buf[2 * LINELEN];

buy_more:

    display_postinfo();

    do
    {
        array_size = 0;

        if (itemtype == '\0')
        {
            mpos = 0;
	    wmove(hw,10,0);
	    wclrtoeol(hw);
            mvwaddstr(hw, 11, 0, "WHAT\tTYPE\n! Potion\n? Scroll\n"
                                 "= Ring\n/ Stick\n] Armor\n) Weapon\n: Food");

            if (wizard)
                mvwaddstr(hw, 19, 0, ", Artifact");

            mvwaddstr(hw, 9, 0, "What type of item do you want? ");
            wclrtoeol(hw);
            touchwin(hw);
            wrefresh(hw);
            itemtype = wgetch(hw);
        }

        switch(itemtype)
        {
            case POTION:
                which_type = TYP_POTION;
                array_size = maxpotions;
                magic_array = p_magic;
                break;

            case SCROLL:
                which_type = TYP_SCROLL;
                array_size = maxscrolls;
                magic_array = s_magic;
                break;

            case FOOD:
                which_type = TYP_FOOD;
                array_size = maxfoods;
                magic_array = fd_data;
                break;

            case WEAPON:
                which_type = TYP_WEAPON;
                array_size = maxweapons;
                break;

            case ARMOR:
                which_type = TYP_ARMOR;
                array_size = maxarmors;
                break;

            case RING:
                which_type = TYP_RING;
                array_size = maxrings;
                magic_array = r_magic;
                break;

            case STICK:
                which_type = TYP_STICK;
                array_size = maxsticks;
                magic_array = ws_magic;
                break;

            case ARTIFACT:
                if (!wizard)
                {
                    itemtype = '\0';
                    continue;
                }

                which_type = TYP_ARTIFACT;
                array_size = maxartifact;
                break;

            case ESCAPE:
                return;

            default:
                wstandout(hw);
                mvwaddstr(hw, 10, 0, "We don't stock any of those.");
                wstandend(hw);
                itemtype = '\0';
                continue;
        }
    }
    while (array_size == 0);

    which_one = array_size;

    do
    {
        const struct magic_item   *m_item;

        display_postinfo();

        mpos = 0;
        sprintf(buf, "Which kind of %s do you wish to have (* for list)? ",
            things[which_type].mi_name);

        mvwaddstr(hw, 9, 0, buf);
        
        touchwin(hw);
        wrefresh(hw);
        buf[0] = '\0';

        switch (get_string(buf, hw))
        {
            case QUIT:
            case ESCAPE:
                itemtype = '\0';
                goto buy_more;
        }

        if (buf[0] == '*')      /* print list */
        {
            add_line(" ID  BASECOST NAME");

            switch (which_type)
            {
                case TYP_RING:
                case TYP_POTION:
                case TYP_STICK:
                case TYP_SCROLL:
                case TYP_FOOD:

                    for(i=0,m_item=magic_array; i < array_size; i++, m_item++)
                        if (!is_spell && m_item->mi_worth > 0)
                        {
                            sprintf(buf, "%3d) %8d %s", i, m_item->mi_worth,
                                m_item->mi_name);
                            add_line(buf);
                        }
                    break;

                case TYP_ARMOR:
                    for (i = 0; i < array_size; i++)
                        if (!is_spell && armors[i].a_worth > 0)
                        {
                            sprintf(buf, "%3d) %8d %s", i, armors[i].a_worth,
                                armors[i].a_name);

                            add_line(buf);
                        }
                    break;

                case TYP_WEAPON:
                    for (i = 0; i < array_size; i++)
                        if (!is_spell && weaps[i].w_worth > 0)
                        {
                            sprintf(buf, "%3d) %8d %s", i, weaps[i].w_worth,
                                weaps[i].w_name);
                            add_line(buf);
                        }
                    break;

                case TYP_ARTIFACT:
                    for (i = 0; i < array_size; i++)
                    {
                        sprintf(buf, "%3d) %8d %s", i, arts[i].ar_worth,
                            arts[i].ar_name);
                        add_line(buf);
                    }
                    break;

                default:
                    add_line("What a strange type.");
            }

            end_line();
            touchwin(hw);
            wrefresh(hw);
            continue;
        }

        if (isdigit((int)buf[0]))
            which_one = atoi(buf);
        else
            switch (which_type)
            {
                case TYP_RING:
                case TYP_POTION:
                case TYP_STICK:
                case TYP_SCROLL:
                case TYP_FOOD:
                    for (i=0,m_item=magic_array; i < array_size; i++, m_item++)
                        if (strcmp(buf, m_item->mi_name) == 0)
                            which_one = i;
                    break;

                case TYP_ARMOR:
                    for (i = 0; i < array_size; i++)
                        if (strcmp(buf, armors[i].a_name) == 0)
                            which_one = i;
                    break;

                case TYP_WEAPON:
                    for (i = 0; i < array_size; i++)
                        if (strcmp(buf, weaps[i].w_name) == 0)
                            which_one = i;
                    break;

                case TYP_ARTIFACT:
                    for (i = 0; i < array_size; i++)
                        if (strcmp(buf, arts[i].ar_name) == 0)
                            which_one = i;
                    break;

                default:
                    msg("What a strange type.");
            }

        if (which_one < 0 || which_one >= array_size)
        {
            wstandout(hw);
            mvwaddstr(hw, 10, 0, "Type the name or an ID number.");
            wstandend(hw);
        }
    }
    while (which_one < 0 || which_one >= array_size);

    item = new_item(sizeof *obj);
    obj = OBJPTR(item);

    if (which_type == TYP_ARTIFACT)
    {
        new_artifact(which_one, obj);
        add_pack(item, NOMESSAGE);
        itemtype = '\0';
        goto buy_more;
    }

    obj->o_type = itemtype;
    obj->o_which = which_one;
    obj->o_mark[0] = '\0';
    obj->o_group = 0;
    obj->o_count = 1;
    obj->o_weight = 0;
    obj->o_dplus = obj->o_hplus = 0;
    obj->o_worth = 0;

    if (!is_spell)
    {
        plus_or_minus = -100;

        do
        {
            mvwaddstr(hw, 10, 0, "Do you want the cursed, blessed, or normal"
                                 " version? (c, b, n) [n]");
            touchwin(hw);
            wrefresh(hw);

	    blessed = cursed = FALSE;
            switch (wgetch(hw))
            {
                case ESCAPE:
                    discard(item);
                    itemtype = '\0';
                    goto buy_more;

                case 'c':
                    cursed = TRUE;
                    plus_or_minus = 0;
                    break;

                case 'b':
                    blessed = TRUE;
                    plus_or_minus = 0;
                    break;

                case 'n':
                case ' ':
                    plus_or_minus = 0;
                    break;

                default:
                    wstandout(hw);
                    mvwaddstr(hw,11,0,"Type 'c' for cursed, 'b' for blessed, "
                                      "or 'n' for normal");
                    wstandend(hw);
            }
        }
        while (plus_or_minus == -100);
    }

    /* else used blessed, cursed from flags parameter */

    if (which_type == TYP_WEAPON)
        init_weapon(obj, which_one);

    obj->o_flags |= ISKNOW;

    if (cursed)
    {
        plus_or_minus = -(rnd(2) + 1);
        obj->o_flags |= ISCURSED;
    }
    else if (blessed)
    {
        plus_or_minus = (rnd(3) + 1);
        obj->o_flags |= ISBLESSED;
    }
    else
    {
        plus_or_minus = 0;
        obj->o_flags |= ISNORMAL;
    }

    switch (which_type)
    {
        case TYP_WEAPON:
            obj->o_hplus += plus_or_minus;
            obj->o_dplus += plus_or_minus;
            break;

        case TYP_ARMOR:
            obj->o_weight = armors[which_one].a_wght;
            obj->o_ac = armors[which_one].a_class - plus_or_minus;
            break;

        case TYP_STICK:
            fix_stick(obj);
            break;

        case TYP_RING:
            obj->o_ac = plus_or_minus;
            break;

        case TYP_SCROLL:
        case TYP_POTION:
            obj->o_weight = things[which_type].mi_wght;
            break;

        case TYP_FOOD:
		    break;
			
        default:
            msg("That's a strange thing to try to own.");
			discard(item);
            itemtype = '\0';
            goto buy_more;
    }

    obj->o_worth = get_worth(obj) * (luck + level / 15 + 1);
    describe_it(obj);

    if (!wizard && obj->o_worth > EFFECTIVE_PURSE)
    {
        wstandout(hw);
        mvwaddstr(hw, 12, 0, "Unfortunately, you can't afford it.");
        wstandend(hw);
        wclrtoeol(hw);
        touchwin(hw);
	wrefresh(hw);
/*		wait_for(' ');*/
		while (wgetch(hw) != ' ') ;
		discard(item);
        itemtype = '\0';
        goto buy_more;
    }

    mvwaddstr(hw, 12, 0, "Do you want it? [y] ");
    wclrtoeol(hw);
    touchwin(hw);
    wrefresh(hw);

    switch (wgetch(hw))
    {
        case ESCAPE:
        case 'n':
            msg("");
			discard(item);
            itemtype = '\0';
            goto buy_more;
    }

    /* The hero bought the item here */

    mpos = 0;

    if (add_pack(item, NOMESSAGE) && !is_spell)
    {
	    if (!wizard)
		{
		    purse -= obj->o_worth; /* take his money */
            ++player.t_trans;
		}
		
        trans_line();   /* show remaining deals */

        switch(which_type)
        {
            case TYP_RING:
            case TYP_STICK:
            case TYP_SCROLL:
            case TYP_POTION:
                know_items[which_type][which_one] = TRUE;
        }
    }
}

/*
    sell_it()
        Sell an item to the trading post
*/

void
sell_it(void)
{
    struct object *obj;
    struct linked_list  *item;
    char buf[2 * LINELEN];

    wclear(cw);

    if ((item = get_item("sell", 0)) == NULL)
        return;

    obj = OBJPTR(item);
    msg("");
    display_postinfo();
    touchwin(hw);
    wrefresh(hw);

    if ((obj->o_type == ARTIFACT) || (obj->o_worth = get_worth(obj)) == 0)
    {
        mpos = 0;
        msg("We don't buy those.");

        if (is_wearing(R_ADORNMENT) && rnd(10) < 4)
            msg("How about that %s ring instead?", r_stones[R_ADORNMENT]);

        return;
    }

    describe_it(obj);
    mvwaddstr(hw, 12, 0, "Do you want to sell it? [n] ");
    touchwin(hw);
    wrefresh(hw);

    switch( wgetch(hw) )
    {
        case 'y':
            break;
        default:
            msg("");
            if (is_wearing(R_ADORNMENT))
                msg("How about that %s ring instead?",
                    r_stones[R_ADORNMENT]);
            return;
    }

    rem_pack(obj);
    purse += obj->o_worth; /* give him his money */
    ++player.t_trans;

    sprintf(buf, "Sold %s.  Hit space to continue.",
        inv_name(obj, LOWERCASE));
    discard(item);

    mvwaddstr(hw, 13, 0, buf);
    touchwin(hw);
    wrefresh(hw);
    wait_for(' ');
}

/*
    describe_it()
        Laud or condemn the object
*/

extern char *inv_name();

void
describe_it(struct object *obj)
{
    static char *cursed_d[] =
    {
        "worthless hunk of junk",
        "shoddy piece of trash",
        "piece of rusty garbage",
        "example of terrible workmanship",
        "cheap hack"
    };

    static char *normal_d[] =
    {
        "journeyman's piece",
        "fine deal",
        "great bargain",
        "good find",
        "real value",
        "piece of honest workmanship",
        "steal",
        "purchase worth making",
        "inexpensive product"
    };

    static char *blessed_d[] =
    {
        "magnificant masterpiece",
        "quality product",
        "exceptional find",
        "unbeatable value",
        "rare beauty",
        "superior product",
        "well-crafted item"
    };

    char    *charp;
    char    buf[2 * LINELEN];

    if (obj->o_flags & ISBLESSED)
        charp = blessed_d[rnd(sizeof(blessed_d) / sizeof(char *))];
    else if (obj->o_flags & ISCURSED)
        charp = cursed_d[rnd(sizeof(cursed_d) / sizeof(char *))];
    else
        charp = normal_d[rnd(sizeof(normal_d) / sizeof(char *))];

    sprintf(buf, "It's a%s %s worth %d pieces of gold.",
        vowelstr(charp), charp, obj->o_worth);

    mvwaddstr(hw, 10, 0, inv_name(obj, TRUE));
    wclrtoeol(hw);
    mvwaddstr(hw, 11, 0, buf);
    wclrtoeol(hw);
}

/*
    open_market()
        Retruns TRUE when ok do to transacting
*/

int
open_market(void)
{
    int maxtrans = is_wearing(R_ADORNMENT) ? MAXPURCH + 4 : MAXPURCH;

    if (wizard || player.t_trans < maxtrans || (level == 0))
        return(TRUE);
    else
    {
        msg("The market is closed. The stairs are that-a-way.");
        return(FALSE);
    }
}

/*
    get_worth()
        Calculate an objects worth in gold
*/

int
get_worth(struct object *obj)
{
    long    worth = 0;
    int wh = obj->o_which;
    int blessed = obj->o_flags & ISBLESSED;
    int cursed = obj->o_flags & ISCURSED;

    switch (obj->o_type)
    {
        case FOOD:
            if (wh < maxfoods)
            {
                worth = obj->o_count * fd_data[wh].mi_worth;
                if (blessed)
                    worth *= 2;
            }
            break;

        case WEAPON:
            if (wh < maxweapons)
            {
                worth = weaps[wh].w_worth;
                worth *= obj->o_count * (2 +
                        (4 * obj->o_hplus +
                        4 * obj->o_dplus));

                if (obj->o_flags & ISSILVER)
                    worth *= 2;

                if (obj->o_flags & ISPOISON)
                    worth *= 2;

                if (obj->o_flags & ISZAPPED)
                    worth += 20 * obj->o_charges;
            }
            break;

        case ARMOR:
            if (wh < maxarmors)
            {
                int plusses = armors[wh].a_class - obj->o_ac;

                worth = armors[wh].a_worth;

                if (plusses > 0)
                    worth *= (1 + (10 *
                        (armors[wh].a_class - obj->o_ac)));
            }
            break;

        case SCROLL:
            if (wh < maxscrolls)
                worth = s_magic[wh].mi_worth;
            break;

        case POTION:
            if (wh < maxpotions)
                worth = p_magic[wh].mi_worth;
            break;

        case RING:
            if (wh < maxrings)
            {
                worth = r_magic[wh].mi_worth;
                worth += obj->o_ac * 40;
            }
            break;

        case  STICK:
            if (wh < maxsticks)
            {
                worth = ws_magic[wh].mi_worth;
                worth += 20 * obj->o_charges;
            }
            break;

        case ARTIFACT:
            if (wh < maxartifact)
                worth = arts[wh].ar_worth;
            break;

        default:
            worth = 0;
    }

    if (obj->o_flags & ISPROT)  /* 300% more for protected */
        worth *= 3;

    if (blessed)        /* 250% more for blessed */
        worth = 5 * worth / 2;
    else if (cursed)    /* half for cursed */
        worth /= 2;

    if (obj->o_flags & (CANRETURN | ISOWNED))
        worth *= 4;
    else if (obj->o_flags & CANRETURN)
        worth *= 2;
    else if (obj->o_flags & ISLOST)
        worth /= 3;

    return(max(0, worth)); /* anything is worth at least one gold piece */
}

/*
    trans_line()
        Show how many transactions the hero has left
*/

void
trans_line(void)
{
    char buf[2 * LINELEN];
    int adorned = is_wearing(R_ADORNMENT);

    if (level == 0 && purse > 0)
        sprintf(buf, "You still have %d pieces of gold left.", purse);
    else if (purse == 0)
        sprintf(buf, "You have no money left.");
    else if (!wizard)
        sprintf(buf, "You have %d transactions and %d gold pieces remaining.",
            max(0, (adorned ? MAXPURCH + 4 : MAXPURCH) - player.t_trans),
            EFFECTIVE_PURSE);
    else
        sprintf(buf, "You have infinite transactions remaining.");

    mvwaddstr(hw, LINES - 2, 0, buf);
}
