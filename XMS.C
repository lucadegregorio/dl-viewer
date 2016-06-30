#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned int (far *xms) (void);
static struct
  {
    unsigned long lunghezza;
    unsigned int  handles;
    unsigned long indirizzos;
    unsigned int  handled;
    unsigned long indirizzod;
  } MOVE;

void inizializza_xms ()
  {
    unsigned int seg,offset;

    _AX = 0x4310;
    geninterrupt (0x2f);
    seg = _ES;
    offset = _BX;
    xms = MK_FP (seg,offset);
  }

char esiste_xms ()
  {
    unsigned char al;

    _AX = 0x4300;
    geninterrupt (0x2f);
    al=_AL;
    if (al == 0x80)
      {
	inizializza_xms ();
	return (1);
      }
    else
      return (0);
  }

unsigned int versione_xms ()
  {
    _AH = 0x00;
    xms ();
    return (_AX);
  }

unsigned int xms_libera ()
  {
    _AH = 0x08;
    xms ();
    return (_DX);
  }

unsigned int alloca_blocco (unsigned int quanta)
  {
    unsigned int ax,dx;

    _AH = 0x09;
    _DX = quanta;
    xms ();
    ax = _AX;
    dx = _DX;
    if (ax)
      return (dx);
    else
      return (0);
  }

unsigned int rilascia_blocco (unsigned int handle)
  {
    _AH = 0x0A;
    _DX = handle;
    xms ();
    return (_AX);
  }

unsigned int sposta_in_xms (unsigned long lunghezza,char far *indirizzo,unsigned int handle)
  {
    MOVE.lunghezza = lunghezza;
    MOVE.handles = 0;
    MOVE.indirizzos = (long) indirizzo;
    MOVE.handled = handle;
    MOVE.indirizzod = 0;

    _DS = FP_SEG (&MOVE);
    _SI = FP_OFF (&MOVE);
    _AH = 0x0B;
    xms ();
    return (_AX);
  }

unsigned int sposta_da_xms (unsigned long lunghezza,char far *indirizzo,unsigned int handle)
  {
    MOVE.lunghezza = lunghezza;
    MOVE.handles = handle;
    MOVE.indirizzos = 0;
    MOVE.handled = 0;
    MOVE.indirizzod = (long) indirizzo;

    _DS = FP_SEG (&MOVE);
    _SI = FP_OFF (&MOVE);
    _AH = 0x0B;
    xms ();
    return (_AX);
  }


