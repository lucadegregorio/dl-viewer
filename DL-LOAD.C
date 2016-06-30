#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <string.h>
#include <alloc.h>
#include <dir.h>
#include <time.h>

#define MAXIMMAGINI 1000
#define MAXSUONI       9

typedef char SCHERMO [64000];
typedef char STRINGA [80];
typedef char PALETTE [768];

extern char NOMEFILE [15];
extern unsigned char DELAY;
extern int getkey ();
extern void Muovi_Delay ();
extern void Scrivi_Memoria ();
extern void shell ();
extern void modo_testo ();
extern char ITEN;
extern float DELAYPARAM;
extern time_t DATAODIERNA;
extern char NOSOUND;

extern char pc_sound_init (void);
extern void pc_sound_terminate (char);
extern char pc_sound_status (char);
extern void pc_sound_stop (char);
extern void pc_sound_play (char,char far *);
extern void pc_sound_speaker (char,char);

int HANDLE;

char VERSIONE;
char FORMATO;                   /*  0 = 320x200, 1 = 640x480 */
time_t SCADENZA;
unsigned int CRC;
/* char FUTURE [44];*/
char TITOLO [41];
char AUTORE [41];
unsigned int NUMEROIMMAGINI;
unsigned int NUMEROSEQUENZAVISIVA;
unsigned int NUMEROSUONI;
PALETTE PALETTECOLORI;

int X [MAXIMMAGINI];
int Y [MAXIMMAGINI];
char *SCHERMOVIRTUALE [MAXIMMAGINI],*XMS_BUFFER;
unsigned int XMS_HANDLE [MAXIMMAGINI];
char far *SUONOVIRTUALE [MAXSUONI];

unsigned int  *SEQUENZAVISIVA = NULL;
unsigned char TIPOSUONO;

char SB=1,SUONO=1,XMS=0;

char far *DESTINAZIONE = NULL;

char controlla_chiave (char *s, int codice)
  {
    int file;
    char *p,*p1;
    long lunghezza;
    int conte;
    unsigned int ACCUMULO=0;
    unsigned int CONTROLLO;

    file = _open (s,1);
    if (file==-1)
      return (0);
    lunghezza = filelength (file);
    p = malloc ((unsigned int)lunghezza);
    if (p==NULL)
      return (0);
    if (_read (file,p,lunghezza)!=lunghezza)
      return (0);
    p1 = memchr (p,26,lunghezza);
    if (p1==NULL)
      return (0);
    for (conte=0;conte < (p1-p)+1;conte++)
      {
	ACCUMULO += p[conte]*(conte+1);
      }
    ACCUMULO += codice;

    memcpy (&CONTROLLO,(p1+2)+(*(p1+1)),2);
    if (CONTROLLO != ACCUMULO)
      return (0);
    return (1);

  }

void grafica_media_256_colori (void)
  {
    _AH = 0;
    _AL = 0x13;
    geninterrupt (0x10);
  }

void setta_colore (colore,rosso,verde,blu)
  char colore,verde,blu,rosso;
    {
      _AH = 0x10;
      _AL = 0x10;
      _BH = 0;
      _BL = colore;
      _DH = rosso;
      _CH = verde;
      _CL = blu;
      geninterrupt (0x10);
    }

void setta_palette (void)
  {
    register char *palette;
    register int co=0,pu=0;

    palette = PALETTECOLORI;

    disable ();

    while (inportb (0x03da) & 0x08);

    while (pu<256)
      {
	if (!(pu%35))
	  while (!(inportb (0x03da) & 0x08));

	outportb (0x03c8,pu);
	outportb (0x03c9,palette [co]);
	++co;
	outportb (0x03c9,palette [co]);
	++co;
	outportb (0x03c9,palette [co]);
	++co;
	++pu;
      }
    enable ();
  }

char entra_in_grafica (void)
  {
    unsigned int conta;
    char sfondo [3],co;

    grafica_media_256_colori ();
    assegna_font (1);

    if (strlen (TITOLO))
      {
	setta_colore (7,0,0,0);
	gotoxy (21-(strlen (TITOLO)/2),8);
	printf ("%s",TITOLO);

	gotoxy (21-(strlen (AUTORE)/2),10);
	printf ("%s",AUTORE);

	for (conta=63;conta>0&&(!kbhit());conta-=3)
	  setta_colore (7,45+(conta/4),conta,conta);

	for (conta=1;(conta<100)&&(!kbhit());conta++)
	  delay ((int)(DELAYPARAM*15));

	for (co=45;co>=0&&(!kbhit());co-=2)
	  setta_colore (7,co,0,0);

	gotoxy (1,8);
	printf ("                                         ");
	gotoxy (1,10);
        printf ("                                         ");
      }

    sfondo [0] = PALETTECOLORI[0]-63;
    sfondo [1] = PALETTECOLORI[1]-63;
    sfondo [2] = PALETTECOLORI[2]-63;
    for (conta=1;conta<64&&(!kbhit());conta+=1)
      {
	++sfondo [0];
	++sfondo [1];
	++sfondo [2];
	if ((sfondo [0] > 0) || (sfondo [1] > 0) || (sfondo [2] > 0))
	  setta_colore (0,sfondo[0]*(sfondo[0]>0),sfondo[1]*(sfondo[1]>0),sfondo[2]*(sfondo[2]>0));
      }

    if ((kbhit ()) && (getkey ()==27))
      return (1);
    setta_palette ();
    return (0);
  }

void decodifica_titolo_autore (void)
  {
    char co=0;

    TITOLO [40] = 0;
    AUTORE [40] = 0;
    while (co < strlen (TITOLO))
      {
	TITOLO [co] = ~TITOLO [co];
	++co;
      }
    co = 0;
    while (co < strlen (AUTORE))
      {
	AUTORE [co] = ~AUTORE [co];
	++co;
      }
  }

unsigned int calcola_crc (void)
  {
    unsigned char i;
    unsigned int crc;

    crc = 0;
    for (i=0;i<strlen(AUTORE);i++)
      crc += AUTORE[i]*(i+1);
    return (crc);
  }

char carica_informazioni (nomefile)
  STRINGA nomefile;
    {
      char dove [138];
      char errore=0;

      HANDLE = _open (nomefile,1);
      if (HANDLE >= 0)
	if (_read (HANDLE,&dove,138)==-1)                /*     Header     */
	  errore = 4;
	else
	  {
	    memcpy (&VERSIONE,&dove,1);
	    switch (VERSIONE)
	      {
		case 1 : memcpy (&TITOLO        ,&dove[1] ,20);
			 strcpy (AUTORE,"\0");
			 break;
		case 2 : memcpy (&TITOLO        ,&dove[2] ,20);
			 memcpy (&AUTORE        ,&dove[22],20);
			 break;
		case 3 : memcpy (&TITOLO        ,&dove[52],40);
			 memcpy (&AUTORE        ,&dove[92],40);
			 memcpy (&NUMEROSUONI   ,&dove[136],2);
			 break;
		default: errore = 2;
			 break;
	      }
	    if (!errore)
	      decodifica_titolo_autore ();
	  }
      else
	errore = 1;
      _close (HANDLE);
      return (errore);
  }

char carica_immagine (unsigned int quale)
  {
    unsigned long dimensione = (unsigned long)X[quale]*(unsigned long)Y[quale];
    unsigned int dim_blocco;

    dim_blocco = (unsigned int)(dimensione)/1024;
    if (dimensione%1024)
      ++dim_blocco;

    if (XMS)
      XMS_HANDLE [quale] = alloca_blocco (dim_blocco);
    if (XMS && XMS_HANDLE [quale])
      {
        if ((kbhit ()) && (getkey () == 27))
	  return (11);
	if (_read (HANDLE,XMS_BUFFER,X [quale]*Y [quale])==-1)
	  return (4);
	if (dimensione % 2)
	  ++dimensione;
	sposta_in_xms (dimensione,XMS_BUFFER,XMS_HANDLE[quale]);
	Scrivi_Memoria ();
      }
    else
      {
	XMS_HANDLE [quale] = 0;
	SCHERMOVIRTUALE [quale] = malloc (X [quale]*Y [quale]);
	if (SCHERMOVIRTUALE [quale] != NULL)
	  {
	    if ((kbhit ()) && (getkey () == 27))
	      return (11);
	    if (_read (HANDLE,SCHERMOVIRTUALE [quale],X [quale]*Y [quale])==-1)
	      return (4);
	    Scrivi_Memoria ();
	  }
	else
	  return (3);
      }
    return (0);
  }

char carica_versione_3 (void)
  {
    char esci,errore;
    unsigned int co,daleggere;
    unsigned long dimensione;
    char huge *punta;

    decodifica_titolo_autore ();
    if (NUMEROIMMAGINI < 1)
      return (4);
    if (NUMEROSEQUENZAVISIVA < 1)
      return (4);
    if (XMS)
      {
	XMS_BUFFER = malloc (64000);
	if (XMS_BUFFER == NULL)
	  return (3);
      }
    for (co=0;co<NUMEROIMMAGINI;co++)                   /*     Immagini    */
      {
	if (_read (HANDLE,&X [co],2)==-1)
	  return (4);
	if (_read (HANDLE,&Y [co],2)==-1)
	  return (4);
	errore = carica_immagine (co);
	if (errore)
	  return (errore);
      }
    SEQUENZAVISIVA = malloc (2*NUMEROSEQUENZAVISIVA);
    if (SEQUENZAVISIVA == NULL)
      return (3);
    if ((kbhit ()) && (getkey () == 27))
      return (11);
    if (_read (HANDLE,SEQUENZAVISIVA,2*NUMEROSEQUENZAVISIVA)==-1)/*Sequenza V.*/
      return (4);
    if (NOSOUND)
      return (0);
    for (co=0;co<NUMEROSUONI;co++)                   /*     Campioni    */
      {
	if (_read (HANDLE,&dimensione,4)==-1)
	  return (4);
	if (_read (HANDLE,&TIPOSUONO,1)==-1)
	  return (4);
	SUONOVIRTUALE [co] = farmalloc (dimensione);
	if (SUONOVIRTUALE [co] != NULL)
	  {
	    punta = SUONOVIRTUALE [co];
	    esci = 0;
	    do
	      {
		if (dimensione > 64000)
		  {
		    daleggere = 64000;
		    dimensione -= 64000;
		  }
		else
		  {
		    daleggere = dimensione;
		    esci = 1;
		  }
                if ((kbhit ()) && (getkey () == 27))
		  return (11);
		if (_read (HANDLE,(char far*)punta,daleggere)==-1)
		  return (4);

		punta += daleggere;
	      }
	    while (!esci);
	    Scrivi_Memoria ();
	  }
	else
	  return (3);
      }
    return (0);
  }

char carica_tutto (nomefile)
  STRINGA nomefile;
    {
      char dove [906];
      char errore;
      long memoria_libera;

      memoria_libera = coreleft ();
      if (XMS)
	memoria_libera += (unsigned long)1024*xms_libera();
      HANDLE = _open (nomefile,1);
      if (HANDLE >= 0)
	{
	  if (filelength (HANDLE) <= memoria_libera||NOSOUND)
	    {
	      if (_read (HANDLE,&dove,906)==-1)        /*     Header     */
		errore = 4;
	      else
		{
		  memcpy (&VERSIONE,&dove,1);
		  switch (VERSIONE)
		    {
		      case 1 :
		      case 2 : errore = 10;
			       break;
		      case 3 : memcpy (&FORMATO             ,&dove[1] ,1);
			       memcpy (&SCADENZA            ,&dove[2] ,4);
			       memcpy (&CRC                 ,&dove[6] ,2);
			       memcpy (&TITOLO              ,&dove[52],40);
			       memcpy (&AUTORE              ,&dove[92],40);
			       memcpy (&NUMEROIMMAGINI      ,&dove[132],2);
			       memcpy (&NUMEROSEQUENZAVISIVA,&dove[134],2);
			       memcpy (&NUMEROSUONI         ,&dove[136],2);
			       memcpy (&PALETTECOLORI       ,&dove[138],768);
			       if ((SCADENZA != 0) && (SCADENZA < DATAODIERNA))
				 errore = 7;
			       else
				 if ((CRC) && CRC != calcola_crc ())
				   errore = 4;
				 else
				   errore = carica_versione_3 ();
			       break;
		      default: errore = 2;
			       break;
		    }
		}
	    }
	  else
	    errore = 3;
	  _close (HANDLE);
	}
      else
	errore = 1;

      return (errore);
    }

void libera_memoria (void)
  {
    unsigned int co;

    for (co=0;co<NUMEROIMMAGINI;co++)
      if (SCHERMOVIRTUALE [co] != NULL)
	{
	  free (SCHERMOVIRTUALE [co]);
	  SCHERMOVIRTUALE [co] = NULL;
	}
    for (co=0;co<NUMEROSUONI;co++)
      if (SUONOVIRTUALE [co] != NULL)
	{
	  farfree (SUONOVIRTUALE [co]);
	  SUONOVIRTUALE [co] = NULL;
	}
    if (SEQUENZAVISIVA != NULL)
      {
	free (SEQUENZAVISIVA);
	SEQUENZAVISIVA = NULL;
      }
    if (XMS)
      {
        for (co=0;co<NUMEROIMMAGINI;co++)
	  if (XMS_HANDLE [co] != 0)
	    {
	      rilascia_blocco (XMS_HANDLE[co]);
	      XMS_HANDLE[co] = 0;
	    }
	if (XMS_BUFFER != NULL)
	  {
	    free (XMS_BUFFER);
	    XMS_BUFFER = NULL;
	  }
      }
    Scrivi_Memoria ();
  }

void scrivi_carattere (char c,unsigned char colore)
  {
    _AL = c;
    _AH = 0x0A;
    _BH = 0;
    _BL = colore;
    _CX = 1;
    geninterrupt (0x10);
  }

void scrivi_stringa (unsigned int x, unsigned int y,char stringa[45],
							  unsigned char colore)
  {
    unsigned char i=0;

    while (stringa[i] != 0)
      {
	gotoxy (x++,y);
	scrivi_carattere (stringa[i++],colore);
      }
  }

void fall_circle (modo,x,y,sfasax,sfasay,dimx,dimy,quale)
  char modo;
  int *x,*y,*sfasax,*sfasay,*dimx,*dimy;
  unsigned int quale;
    {
      *sfasax = 0;
      *sfasay = 0;
      if (modo)
	{
          if (*x > 320)
	    *x = -X[quale];
	  if (*x < -X[quale])
	    *x = 320;
	  if (*y > 200)
	    *y = -Y[quale];
	  if (*y < -Y[quale])
	    *y = 200;
	}
      else
	{
	  if (*x > 320)
	    *x = 320;
	  if (*x < -X[quale])
	    *x = -X[quale];
	  if (*y > 200)
	    *y = 200;
	  if (*y < -Y[quale])
	    *y = -Y[quale];
	}

      if (*x > 320 - X[quale])
	*dimx = 320-*x;
      else
	if (*x < 0)
	  {
	    *dimx = *x + X[quale];
	    *sfasax = -*x;
	  }
	else
	  *dimx = X[quale];

      if (*y > 200 - Y[quale])
	*dimy = 200-*y;
      else
	if (*y < 0)
	  {
	    *dimy = *y + Y[quale];
	    *sfasay = -*y;
	  }
	else
	  *dimy = Y[quale];
    }

void range_auto (modo,x,y,sfasax,sfasay,dimx,dimy,quale)
  char modo;
  int *x,*y,*sfasax,*sfasay,*dimx,*dimy;
  unsigned int quale;
    {
      if (modo)
	{
          *x = 160 - X[quale]/2;
	  *y = 100 - Y[quale]/2;
	}
      else
	{
	  if (*x > 320 - X[quale])
	    *x = 320 - X[quale];
	  if (*x < 0)
	    *x = 0;
	  if (*y > 200 - Y[quale])
	    *y = 200 - Y[quale];
	  if (*y < 0)
	    *y = 0;
	}
      *sfasax = 0;
      *dimx = X [quale];
      *sfasay = 0;
      *dimy = Y [quale];
    }

char animazione (void)
  {
    unsigned char esci = 0,modo=1,keyboard_on=1,pwd_ok=0,key_ok=0,shiftx=0,shifty=0;
    unsigned int co=0,pu,quale,saltaqui,quanti=0,decimi,suono=0xFFFF,
		 xt=1,yt=1,coloret=1,x1,y1,x2,y2,colore,car,pwd,password;
    int x=0,y=0,sfasax,dimx,sfasay,dimy;
    unsigned long dimensione;
    char stringa [45];
    char nome_chiave [15];

    DESTINAZIONE = MK_FP (0XA000,0);
    while (!esci)
      {
	quale = *(SEQUENZAVISIVA+co);
	switch (quale)
	  {
	    case 0xFFFF : ++co;                                 /* [ Ciclo */
			  saltaqui = co;
			  quanti = *(SEQUENZAVISIVA+co);
			  break;
	    case 0xFFFE : if (quanti)                           /* ] Ciclo */
			    {
			      co = saltaqui;
			      --quanti;
			    }
			  break;
	    case 0xFFFD : ++co;                                 /* D  Delay */
			  decimi = *(SEQUENZAVISIVA+co);
			  for (pu=0;(pu<decimi)&&(!kbhit ());pu++)
			    {
			      if (!NOSOUND)
				{
				  if (suono!=0xFFFF && !pc_sound_status (SB))
				    pc_sound_play (SB,SUONOVIRTUALE[suono]);
				}
			      delay ((int)10*DELAYPARAM);
			    }
			  break;
	    case 0xFFFC : esci = 1;                             /* E Fine DL */
			  break;
	    case 0xFFFB : esci = 1;                             /* WE Wait e esci */
	    case 0xFFFA : if (!NOSOUND)
			    while (pc_sound_status(SB) && !kbhit ()); /* W wait */
			  break;
	    case 0xFFF9 : Muovi_Delay (-1);                     /* Vel + */
			  break;
	    case 0xFFF8 : Muovi_Delay (1);                      /* Vel - */
			  break;
	    case 0xFFF7 : Muovi_Delay (-2);                     /* Vel * */
			  break;
	    case 0xFFF6 : Muovi_Delay (2);                      /* Vel / */
			  break;
	    case 0xFFF5 : Muovi_Delay (0);                      /* Vel = */
			  break;
	    case 0xFFF4 : ++co;                                 /* Cls */
			  x1 = *(SEQUENZAVISIVA+co);
                          ++co;
			  y1 = *(SEQUENZAVISIVA+co);
                          ++co;
			  x2 = *(SEQUENZAVISIVA+co);
                          ++co;
			  y2 = *(SEQUENZAVISIVA+co);
                          ++co;
			  colore = *(SEQUENZAVISIVA+co);
			  DESTINAZIONE = MK_FP (0XA000,320*y1+x1);
			  for (pu=0;pu<y2-y1+1;pu++)
			    memset (DESTINAZIONE+320*pu,colore,x2-x1+1);
			  break;
	    case 0xFFF3 : ++co;
			  suono = *(SEQUENZAVISIVA+co);       /* Sound Blaster */
			  if (!NOSOUND)
			    {
			      if (pc_sound_status (SB))
				pc_sound_stop (SB);
			      pc_sound_play (SB,SUONOVIRTUALE[suono]);
			      suono = 0xFFFF;
			    }
			  break;
	    case 0xFFF2 : ++co;
			  if (!NOSOUND)
			    if (suono != *(SEQUENZAVISIVA+co))
			      {
				suono = *(SEQUENZAVISIVA+co);       /* Sound Blaster */
				if (pc_sound_status (SB))               /* Loop */
				  pc_sound_stop (SB);
				pc_sound_play (SB,SUONOVIRTUALE[suono]);
			      }
			  break;
	    case 0xFFF1 : shiftx = 1;                         /*  X+  */
			  if (modo == 1)
			    modo = 0;
			  break;
	    case 0xFFF0 : shiftx = 2;                         /*  X-  */
			  if (modo == 1)
			    modo = 0;
			  break;
	    case 0xFFEF : shifty = 1;                         /*  Y+  */
			  if (modo == 1)
			    modo = 0;
			  break;
	    case 0xFFEE : shifty = 2;                         /*  Y-  */
			  if (modo == 1)
			    modo = 0;
			  break;
	    case 0xFFED : keyboard_on = 1;                    /*  K+  */
			  break;
	    case 0xFFEC : keyboard_on = 0;                    /*  K-  */
			  break;
	    case 0xFFEB : ++co;                            /* #<nome_dl> */
			  pu = 0;
			  while (*(SEQUENZAVISIVA+co))
			    {
			      NOMEFILE [pu] = *(SEQUENZAVISIVA+co);
			      ++pu;
			      ++co;
			    }
			  NOMEFILE [pu] = 0;
			  strcat (NOMEFILE,".DL");
			  esci = 3;
			  break;
	    case 0xFFEA : ++co;                               /* (X,Y) */
			  x = *(SEQUENZAVISIVA+co);
			  ++co;
			  y = *(SEQUENZAVISIVA+co);
			  if (modo == 1)
			    modo = 0;
			  break;
	    case 0xFFE9 : ++co;                               /* <X,Y> */
			  xt = *(SEQUENZAVISIVA+co)+1;
			  ++co;
			  yt = *(SEQUENZAVISIVA+co)+1;
                          ++co;
			  coloret = *(SEQUENZAVISIVA+co)+1;
			  break;
	    case 0xFFE8 : ++co;                            /* "<stringa> */
			  pu = 0;
			  while (*(SEQUENZAVISIVA+co))
			    {
			      stringa [pu] = *(SEQUENZAVISIVA+co);
			      ++pu;
			      ++co;
			    }
			  stringa [pu] = 0;
			  if (xt > 41 - strlen (stringa))
			    xt = 41 - strlen (stringa);
			  scrivi_stringa (xt,yt,stringa,coloret-1);
			  break;
	    case 0xFFE7 : while (!kbhit())                   /* Getch */
			    if (!NOSOUND)
			      if (suono!=0xFFFF && !pc_sound_status (SB))
				pc_sound_play (SB,SUONOVIRTUALE[suono]);
			  getkey ();
			  break;
	    case 0xFFE6 : ++co;                              /* VB(...) */
			  x1 = *(SEQUENZAVISIVA+co);
                          ++co;
			  y1 = *(SEQUENZAVISIVA+co);
                          ++co;
			  x2 = *(SEQUENZAVISIVA+co);
                          ++co;
			  y2 = *(SEQUENZAVISIVA+co);
                          ++co;
			  colore = *(SEQUENZAVISIVA+co);
			  break;
	    case 0xFFE5 : break;                              /* VB- */
	    case 0xFFE4 : break;                              /* VBR+ */
	    case 0xFFE3 : break;                              /* VBR- */
	    case 0xFFE2 : modo = 0;                           /* F */
			  break;
	    case 0xFFE1 : modo = 1;                           /* A */
			  break;
	    case 0xFFE0 : modo = 2;                           /* R */
			  break;
	    case 0xFFDF : modo = 3;                           /* O */
			  break;
	    case 0xFFDE : ++co;				      /* Password */
			  pwd = *(SEQUENZAVISIVA+co);
			  if (!pwd_ok)
			    {
			      scrivi_stringa (xt,yt,"Password :",coloret-1);
			      pu = 1;
			      password = 0;
			      do
				{
				  car = upcase(getkey());
				  if ((car != 13) && (car != 27))
				    password += car*pu;
				  ++pu;
				}
			      while (car != 13 && car != 27 && pu <= 8);
			      if (password != pwd)
				esci = 1;
			      else
				pwd_ok = 1;
			    }
			  break;
	    case 0xFFDD : ++co;                            /* @<chiave>,key */
			  pu = 0;
			  while (*(SEQUENZAVISIVA+co))
			    {
			      nome_chiave [pu] = *(SEQUENZAVISIVA+co);
			      nome_chiave [pu] = ~nome_chiave [pu];
			      ++pu;
			      ++co;
			    }
			  ++co;
			  if (!key_ok)
			    {
			      nome_chiave [pu] = 0;
			      strcat (nome_chiave,".DLK");
			      if (!controlla_chiave (nome_chiave,*(SEQUENZAVISIVA+co)))
				{
				  sleep (3);
				  memset (MK_FP (0XA000,0),0,64000);
				  setta_colore (7,45,45,45);
				  setta_colore (8,60,60,60);

				  if (!ITEN)
				    {
				      scrivi_stringa (1,5,"Quest'animazione Š protetta con chiave.",7);
				      scrivi_stringa (1,7,"Per la registrazione contattare :",7);
				    }
				  else
				    {
				      scrivi_stringa (1,5,"This animation is Key-protected.",7);
				      scrivi_stringa (1,7,"For registering get in touch with :",7);
				    }
                                  scrivi_stringa (20-(strlen (AUTORE)/2),9,AUTORE,8);
				  sleep (3);
				  esci = 1;
				}
			      else
				key_ok = 1;
			    }
			  break;
	    case 0xFFDC : shiftx = 0;                            /* X */
			  break;
	    case 0xFFDB : shifty = 0;                            /* Y */
			  break;
		default : if (!NOSOUND)
			    if (suono!=0xFFFF && !pc_sound_status (SB))
			      pc_sound_play (SB,SUONOVIRTUALE[suono]);
			  if (quale >= 0 && quale < NUMEROIMMAGINI)
			    {
			      delay ((int) DELAY*DELAYPARAM);
			      switch (shiftx)
				{
				  case 1 : ++x;
					   break;
				  case 2 : --x;
					   break;
				}
			      switch (shifty)
				{
				  case 1 : ++y;
					   break;
				  case 2 : --y;
					   break;
				}
			      switch (modo)
				{
				  case 0 : fall_circle (0,&x,&y,&sfasax,&sfasay,&dimx,&dimy,quale);
					   break;
				  case 1 : range_auto (1,&x,&y,&sfasax,&sfasay,&dimx,&dimy,quale);
					   break;
				  case 2 : range_auto (0,&x,&y,&sfasax,&sfasay,&dimx,&dimy,quale);
					   break;
				  case 3 : fall_circle (1,&x,&y,&sfasax,&sfasay,&dimx,&dimy,quale);
					   break;
				}
			      DESTINAZIONE = MK_FP (0XA000,320*(y+sfasay)+x+sfasax);
			      dimensione = (unsigned long)X[quale]*(unsigned long)Y[quale];
			      if (dimensione % 2)
				++dimensione;
			      if (XMS && XMS_HANDLE [quale])
				{
				  sposta_da_xms (dimensione,XMS_BUFFER,XMS_HANDLE[quale]);
                                  for (pu=0;pu<dimy;pu++)
				    memcpy (DESTINAZIONE+320*pu,XMS_BUFFER+(pu+sfasay)*X[quale]+sfasax,dimx);
				}
			      else
				for (pu=0;pu<dimy;pu++)
				  memcpy (DESTINAZIONE+320*pu,SCHERMOVIRTUALE[quale]+(pu+sfasay)*X[quale]+sfasax,dimx);
			    }
			  break;
	  }
	++co;
	co %= NUMEROSEQUENZAVISIVA;
	if (kbhit())
	  {
	    car = getkey ();
	    if (keyboard_on)
	      switch (car)
		{
		  case '+': Muovi_Delay (-1);
			    break;
		  case '-': Muovi_Delay (1);
			    break;
		  case '=': Muovi_Delay (0);
			    break;
		  case '*': Muovi_Delay (-2);
			    break;
		  case '/': Muovi_Delay (2);
			    break;
		  case 27 : esci = 2;
			    break;
		  case 13 :
		  case 32 : esci = 1;
			    break;
		  case 316: if (!NOSOUND)
			      {
				if (SUONO)
				  SUONO = 0;
				else
				  SUONO = SB + 1;
				if (!SUONO)
				  pc_sound_speaker (SB,0);
				else
				  pc_sound_speaker (SB,1);
			      }
			    break;
		  case 324: if (!NOSOUND)
			      if (pc_sound_status (SB))
				pc_sound_stop (SB);
			    modo_testo ();
			    shell ();
			    grafica_media_256_colori ();
			    setta_palette ();
			    break;
		}
	  }
      }
    if (!NOSOUND)
      if (pc_sound_status (SB))
	pc_sound_stop (SB);

    return (esci);
  }