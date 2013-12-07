/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

static t_symbol *bendin_sym;
static t_symbol *ctlin_sym;
static t_symbol *midiclkin_sym;
static t_symbol *midiin_sym;
static t_symbol *midirealtimein_sym;
static t_symbol *notein_sym;
static t_symbol *pgmin_sym;
static t_symbol *polytouchin_sym;
static t_symbol *sysexin_sym;
static t_symbol *touchin_sym;

void inmidi_byte(int portno, int byte)
{
    t_atom at[2];
    if (midiin_sym->s_thing)
    {
        SETFLOAT(at, byte);
        SETFLOAT(at+1, portno + 1);
        pd_list(midiin_sym->s_thing, 0, 2, at);
    }
}

void inmidi_sysex(int portno, int byte)
{
    t_atom at[2];
    if (sysexin_sym->s_thing)
    {
        SETFLOAT(at, byte);
        SETFLOAT(at+1, portno + 1);
        pd_list(sysexin_sym->s_thing, 0, 2, at);
    }
}

void inmidi_noteon(int portno, int channel, int pitch, int velo)
{
    if (notein_sym->s_thing)
    {
        t_atom at[3];
        SETFLOAT(at, pitch);
        SETFLOAT(at+1, velo);
        SETFLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(notein_sym->s_thing, &s_list, 3, at);
    }
}

void inmidi_controlchange(int portno, int channel, int ctlnumber, int value)
{
    if (ctlin_sym->s_thing)
    {
        t_atom at[3];
        SETFLOAT(at, ctlnumber);
        SETFLOAT(at+1, value);
        SETFLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(ctlin_sym->s_thing, &s_list, 3, at);
    }
}

void inmidi_programchange(int portno, int channel, int value)
{
    if (pgmin_sym->s_thing)
    {
        t_atom at[2];
        SETFLOAT(at, value + 1);
        SETFLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list(pgmin_sym->s_thing, &s_list, 2, at);
    }
}

void inmidi_pitchbend(int portno, int channel, int value)
{
    if (bendin_sym->s_thing)
    {
        t_atom at[2];
        SETFLOAT(at, value);
        SETFLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list(bendin_sym->s_thing, &s_list, 2, at);
    }
}

void inmidi_aftertouch(int portno, int channel, int value)
{
    if (touchin_sym->s_thing)
    {
        t_atom at[2];
        SETFLOAT(at, value);
        SETFLOAT(at+1, (channel + (portno << 4) + 1));
        pd_list(touchin_sym->s_thing, &s_list, 2, at);
    }
}

void inmidi_polyaftertouch(int portno, int channel, int pitch, int value)
{
    if (polytouchin_sym->s_thing)
    {
        t_atom at[3];
        SETFLOAT(at, pitch);
        SETFLOAT(at+1, value);
        SETFLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(polytouchin_sym->s_thing, &s_list, 3, at);
    }
}

void inmidi_clk(double timing)
{

    static t_float prev = 0;
    static t_float count = 0;
    t_float cur,diff;

    if (midiclkin_sym->s_thing)
    {
        t_atom at[2];
        diff =timing - prev;
        count++;
   
        if (count == 3)
        {  /* 24 count per quoter note */
             SETFLOAT(at, 1 );
             count = 0;
        }
        else SETFLOAT(at, 0);

        SETFLOAT(at+1, diff);
        pd_list(midiclkin_sym->s_thing, &s_list, 2, at);
        prev = timing;
    }
}

void inmidi_realtimein(int portno, int SysMsg)
{
    if (midirealtimein_sym->s_thing)
    {
        t_atom at[2];
        SETFLOAT(at, portno);
        SETFLOAT(at+1, SysMsg);
        pd_list(midirealtimein_sym->s_thing, &s_list, 2, at);
    }
}

void e_midi_setup(void)
{
    bendin_sym = gensym("#bendin");
    ctlin_sym = gensym("#ctlin");
    midiclkin_sym = gensym("#midiclkin");
    midiin_sym = gensym("#midiin");
    midirealtimein_sym = gensym("#midirealtimein");
    notein_sym = gensym("#notein");
    pgmin_sym = gensym("#pgmin");
    polytouchin_sym = gensym("#polytouchin");
    sysexin_sym = gensym("#sysexin");
    touchin_sym = gensym("#touchin");
}
