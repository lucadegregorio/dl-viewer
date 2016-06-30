#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include "sbcvoice.h"

char PC_VOICE_STATUS = 0,PC_VOICE_SPEAKER = 1,ENTRA = 1;
unsigned int COUNT_18HZ = 0, COUNT_FOR18HZ = 0;
unsigned long SUONATI = 0,DIMCAMPIONE = 0;
char far *PUNTABUFFER;

void interrupt (*vecchiohw) ();

unsigned int cerca_frequenza_assegna_DIMCAMPIONE (char far **buffer)
  {
    char far *punta;
    unsigned char divisore;
    unsigned int frequenza;

    if (*(*buffer) != 0x01)
      *buffer += 0x08;

    ++(*buffer);
    punta = (char far *) &DIMCAMPIONE;
    DIMCAMPIONE = 0;
    *(punta++) = *(*buffer)++;
    *(punta++) = *(*buffer)++;
    *(punta++) = *(*buffer)++;
    DIMCAMPIONE -= 2;
    divisore = *(*buffer)++;
    --divisore;
    divisore = ~divisore;
    frequenza = 1000000/divisore;
    divisore = *(*buffer)++;
    ++divisore;
    frequenza /= divisore;

    return (frequenza);
  }

void play_voce (char far *buffer)
  {
    unsigned char high,low;
    unsigned int frequenza;

    PUNTABUFFER = buffer;
    frequenza = cerca_frequenza_assegna_DIMCAMPIONE (&PUNTABUFFER);

    disable ();
    low = (1193180/frequenza) % 256;
    high = (1193180/frequenza) / 256;
    outportb (0x43,0x34);
    outportb (0x40,low);
    outportb (0x40,high);
    SUONATI = 0;
    COUNT_18HZ = frequenza / 18.2;
    COUNT_FOR18HZ = COUNT_18HZ;
    PC_VOICE_STATUS = 1;
    enable ();
  }

void stop_voce (void)
  {
    disable ();
    outportb (0x43,0x34);
    outportb (0x40,0xFF);
    outportb (0x40,0xFF);
    PC_VOICE_STATUS = 0;
    enable ();
  }

void interrupt tick (void)
  {
    if (ENTRA)
      {
	ENTRA = 0;
	if (PC_VOICE_STATUS)
	  {
	    _AH = *PUNTABUFFER;
	    _AH >>= 0x06;
	    _AH &= 0x02;
	    _AL = inportb (0x61);
	    _AL &= 0xFC;
	    _AL |= _AH;
	    if (PC_VOICE_SPEAKER)
	      outportb (0x61,_AL);
	    ++SUONATI;
	    ++PUNTABUFFER;
	    _AX = FP_OFF (PUNTABUFFER);
	    _DX = FP_SEG (PUNTABUFFER);
	    if (!_AX)
	      {
		_DX += 0x1000;
		PUNTABUFFER = MK_FP (_DX,_AX);
	      }
	    if (SUONATI == DIMCAMPIONE)
	      stop_voce ();
	    --COUNT_FOR18HZ;
	    if (!COUNT_FOR18HZ)
	      {
		COUNT_FOR18HZ = COUNT_18HZ;
		vecchiohw ();
	      }
	    else
	      outportb (0x20,0x20);
	  }
	else
	  vecchiohw ();
	ENTRA = 1;
      }
  }

void installa_speaker (void)
  {
    disable ();
    vecchiohw = getvect (0x08);
    setvect (0x08,tick);
    enable ();
  }

void rimuovi_speaker (void)
  {
    disable ();
    stop_voce ();
    setvect (0x08,vecchiohw);
    enable ();
  }

char pc_sound_init (char sb)
  {
    if (sb && sbc_check_card () & 4 && sbc_scan_int () && !ctvm_init())
      {
	ctvm_speaker (0);
	ctvm_speaker (1);
	return (1);
      }
    else
      {
	installa_speaker ();
	return (0);
      }
  }

void pc_sound_terminate (char sb)
  {
    if (sb)
      ctvm_terminate ();
    else
      rimuovi_speaker ();
  }

char pc_sound_status (char sb)
  {
    if (sb)
      return (ct_voice_status);
    else
      return (PC_VOICE_STATUS);
  }

void pc_sound_stop (char sb)
  {
    if (sb)
      ctvm_stop ();
    else
      stop_voce ();
  }

void pc_sound_play (char sb,char far *buffer)
  {
    if (sb)
      ctvm_output (buffer+((VOCHDR far*)buffer)->voice_offset);
    else
      play_voce (buffer+((VOCHDR far*)buffer)->voice_offset);
  }

void pc_sound_speaker (char sb,char come)
  {
    if (sb)
      ctvm_speaker (come);
    else
      PC_VOICE_SPEAKER = come;
  }