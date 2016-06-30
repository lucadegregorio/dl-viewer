#include <dir.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <alloc.h>
#include <bios.h>
#include <io.h>
#include <process.h>
#include <time.h>

char PROGSCADG = 1;
char PROGSCADM = 12;
char PROGSCADA = 95;
char *PROGNAME = "DL VIEWER";
char *PROGVER = "3.3b"; /* rimuovere il fastmode per le versioni non beta */
char *PROGCOPY = "(c) 1994 Davide Tome' & Luca De Gregorio";
char ITEN=1; /* ITalianoENglish */
char TESTMODE=0;
char MININFO=1;
char REGISTRATO = 0;
char BADNAMESN = 1;
char *BADNAMES[2] = {"Pippo"};
char NAME[80] = "UNREGISTERED - NON REGISTRATO";
char ADDRESS[80];
char ZIPCODE[80];
char CITYSTATE[80];
char COUNTRY[80];
char PHONE[80];
float DELAYPARAM;
unsigned int CONTATORE=0;
time_t DATAODIERNA;
char NOXMS = 0;
char NOSOUND = 0;
char NOFONT = 0;
char FASTMODE = 0;

#define COLONNE 4
#define RIGHE 10

#define POSFINESTRAX 7                  /* Posizioni */
#define POSFINESTRAY 6
#define POSTITOLOX 8
#define POSTITOLOY 1
#define POSBARRAX 4
#define POSBARRAY 23
#define POSDELAYX 4
#define POSDELAYY 21
#define POSMEMX 20
#define POSMEMY 21
#define POSSUONOX 38
#define POSSUONOY 21
#define POSMESSAGGIX 55
#define POSMESSAGGIY 21

#define TESTOFINESTRA 0                  /* Colori */
#define ATTRIBEVID 79
#define ATTRIBEVIDTAG 64
#define NEROSUGRIGIO 112
#define NEROSUGRIGIOTAG 116
#define GRIGIOSUBLU 23
#define GRIGIOSCUROSUBLU 21
#define COLOREBARRETTE 39

#define F1 315
#define F2 316
#define F3 317
#define F4 318
#define F5 319
#define F6 320
#define F7 321
#define F8 322
#define F9 323
#define F10 324
#define BS 8
#define BARRA 32
#define ENTER 13
#define ESC 27
#define HOME 327
#define FINE 335
#define SU 328
#define GIU 336
#define PAGSU 329
#define PAGGIU 337
#define SINISTRA 331
#define DESTRA 333
#define INS 338
#define CANC 339
#define MAXNOMIFILE 300

extern char VERSIONE;
extern char FORMATO;
extern char TITOLO [41];
extern char AUTORE [41];
extern char SB;
extern char SUONO;
extern char XMS;

typedef char nomefile[16];

nomefile nomifile [MAXNOMIFILE];
unsigned char PDELAY[13] = {0,5,10,20,30,45,60,80,100,130,160,200,250};
unsigned char POSDELAY = 8;
unsigned char DELAY = 100;

char TESTOPALETTE[48];

char TESTOPALORIGIN[48] = {0,0,0,
			   0,0,42,
			   0,42,0,
			   0,42,42,
			   42,0,0,
			   42,0,42,
			   42,21,0,
			   42,42,42,
			   21,21,21,
			   21,21,63,
			   21,63,21,
			   21,63,63,
			   63,21,21,
			   63,21,63,
			   63,63,21,
			   63,63,63,
			   };

char *ERRMSG[2][11] = {{"FILE NON TROVATO",
			"VERSIONE SCONOSCIUTA",
			"MEMORIA  INSUFFICIENTE",
			"FORMATO NON VALIDO",
			"CARATTERE NON VALIDO",
			"DRIVE ILLEGALE",
			"ANIMAZIONE SCADUTA\0",
			"ERRORE NEL DISCO",
			"ERRORE NEL DISPOSITIVO",
			"NECESSARIA CONVERSIONE",
			"CARICAMENTO  SOSPESO"
			 },
		       {"FILE NOT FOUND",
			"UNKNOWN DL VERSION",
			"MORE MEMORY REQUIRED",
			"ERROR IN FILE DL",
			"INVALID DRIVE LETTER",
			"INVALID DRIVE LETTER",
			"EXPIRED ANIMATION\0",
			"DISK ERROR",
			"DEVICE ERROR",
			"UPGRADING NEEDED",
			"ABORTED!"
		       }};
char *COLORMSG[2][12] = {{"SFONDO",
			  "BARRE DI STATO",
			  "FINESTRE",
			  "EVIDENZIATORE",
			  "\0",
			  "Posizionarsi usando le frecce",
			  "+ Aumento Intensita'",
			  "- Diminuzione Intensita'",
			  "INVIO Accetta Nuovi Colori",
			  "ESC Torna ai Colori Precedenti",
			  "F1 Ripristina Colori Originali",
			  "R   V   B"
			 },
			{ "BACKGROUND",
			  "STATUS BAR",
			  "WINDOWS",
			  "CURSOR",
			  "\0",
			  "Move using arrows",
			  "+ Increase intensity",
			  "- Decrease intensity",
			  "ENTER Accept New Colors",
			  "ESC Restore Previous Colors",
			  "F1 Restore Default Colors",
			  "R   G   B"
			}};

char *MSG[2][21] = {{"VELOCITA'",
		     "MEMORIA    ",
		     "",
		     "",
		     "",
		     "SUONO",
		     "",
		     "Directory Corrente :",
		     "",
		     "CARICAMENTO IN CORSO",
		     "PREMI UN TASTO"
		    },
		    {"SPEED    ",
		     "FREE MEMORY",
		     "",
		     "",
		     "",
		     "SOUND",
		     "",
		     " Current Directory :",
		     "",
		     "LOADING...",
		     "TYPE ANY KEY"
		   }};

char *SOUNDMSG[3] = { "             ",
		      "   SPEAKER   ",
		      "SOUND BLASTER" };

int LASTPOS,POS,TOT,I,TOTALE;
char ERRORE=0;
char NOMEFILE[15] = "\0";


char CONFIGURAZIONE[128];
char KEY[128];
char FINALE[128];

char CONTROLLINO = 0;
char PRIMAVOLTA = 1;
unsigned long big_read (int HANDLE, char huge *p, unsigned long q)
  {
    char huge *provvi=p;
    unsigned long completi,ultimi,totletti=0;
    unsigned int letti=65500;

    completi = q / 65500;
    ultimi = q % 65500;
    while (completi && letti && letti!=0xFFFF)
      {
	letti = _read (HANDLE,(char *) provvi,65500);
	if (letti != 0xFFFF)
	  {
	    provvi += letti;
	    totletti += letti;
	  }
	completi--;
      }
    if (!completi&&letti==65500&&ultimi)
      {
	letti = _read (HANDLE,(char *) provvi,ultimi);
	if (letti !=0xFFFF)
	  totletti += letti;
      }
    return (totletti);
  }

char Controlla_Chiave ()
  {
    FILE *f;
    int fh;
    int c;
    char i,conta;
    char CONTATORE;
    char blocco[1024];

    KEY [strlen(KEY)-3] = 0;
    strcat (KEY,"KEY");
    f = fopen (KEY,"r+t");
    if (f==NULL)
      return (2);
    fgets (NAME,80,f);
    fgets (ADDRESS,80,f);
    fgets (ZIPCODE,80,f);
    fgets (CITYSTATE,80,f);
    fgets (COUNTRY,80,f);
    fgets (PHONE,80,f);
    NAME [strlen(NAME)-1] = 0;
    ADDRESS [strlen(ADDRESS)-1] = 0;
    ZIPCODE [strlen(ZIPCODE)-1] = 0;
    CITYSTATE [strlen(CITYSTATE)-1] = 0;
    COUNTRY [strlen(COUNTRY)-1] = 0;
    PHONE [strlen(PHONE)-1] = 0;
    c = strlen (NAME) + strlen (ADDRESS) + strlen (ZIPCODE) + strlen (CITYSTATE) + strlen (COUNTRY) + strlen (PHONE)+14;
    fclose (f);
    if (c==0)
      return (3);
    fh = _open (KEY,1);
    if (fh==-1)
      return (2);
    if (_read (fh,blocco,c)!=c)
      {
	_close (fh);
	return (3);
      }
    conta=blocco[c-1];
    if (!conta)
      return (3);
    if (_read (fh,blocco,conta)!=conta)
      {
	_close (fh);
	return (3);
      }
    if (_read (fh,&conta,1)!=1)
      {
	_close (fh);
	return (3);
      }
    _close (fh);
    CONTATORE = strlen(NAME)+strlen(ADDRESS)-strlen(PHONE);
    for (i=0;i<strlen(NAME);i++)
      CONTATORE += (NAME[i]*i);
    for (i=0;i<strlen(ADDRESS);i++)
      CONTATORE += (ADDRESS[i]*i);
    for (i=0;i<strlen(ZIPCODE);i++)
      CONTATORE += (ZIPCODE[i]*i);
    for (i=0;i<strlen(CITYSTATE);i++)
      CONTATORE += (CITYSTATE[i]*i);
    for (i=0;i<strlen(COUNTRY);i++)
      CONTATORE += (COUNTRY[i]*i);
    for (i=0;i<strlen(PHONE);i++)
      CONTATORE += (PHONE[i]*i);
    i = 0;
    while (i < BADNAMESN)
      {
	if (strcmp (NAME,BADNAMES[i])==0)
	  return (4);
	i++;
      }
    if (CONTATORE == conta)
      return (1);
    else
      return (3);
  }

void Salva_Schermo ()
  {
    memcpy (MK_FP(0XB800,4000),MK_FP(0XB800,0),4000);
  }

void Ripristina_Schermo ()
  {
    memcpy (MK_FP(0XB800,0),MK_FP(0XB800,4000),4000);
  }

void modo_testo ()
  {
    setta_colore (0,0,0,0);
    memset (MK_FP(0xa000,0),0,64000);
    _AH = 0x00;
    _AL = 0x03;
    geninterrupt (0x10);
  }

int diskready (int errval,int ax, int bp,int si)
  {
    if (ax<0)
      ERRORE = 8;
    else
      ERRORE = 9;
    hardretn(-1);
  }
int breakcontrol ()
  {
  }

int getkey(void)
  {
   int key, lo, hi;

   key = bioskey(0);
   lo = key & 0X00FF;
   hi = (key & 0XFF00) >> 8;
   return((lo == 0) ? hi + 256 : lo);
  }

int upcase (int a)
  {
    if (a <='z' && a >= 'a')
      return (a-32);
    else
      return (a);
  }

void marginecolor (char n)
  {
    _AH = 11;
    _BL = n;
    _BH = 0;
    geninterrupt (0X10);
  }

void cursore (come)
  char come;
    {
      switch (come)
	{
	  case 0 : _CX = 0x2000;
		   break;
	  case 1 : _CX = 0x0607;
		   break;
	}
      _AH = 1;
      geninterrupt (0x10);
    }

void attributizonavideo (char x1,char y1,char x2,char y2,char att,char sfondo)
  {
    char *punta = MK_FP (0xB800,160*(y1-1)+2*(x1-1)+sfondo);
    int co1;
    unsigned char co2;

    for (co1=0;co1<=(y2-y1)*160;co1+=160)
      for (co2=0;co2<=(x2-x1)*2;co2+=2)
	 *(punta+co1+co2) = att;
  }
void clrscr()
  {
    attributizonavideo (1,1,80,25,' ',0);
  }

void Scrivi_Diretto (char x,char y, char *s)
  {
    char *punta = MK_FP (0xB800,160*(y-1)+2*(x-1));
    unsigned char co,co1;

    co1 = strlen (s);
    for (co=0;co<co1*2;co+=2)
       *(punta+co) = *(s++);
  }

void scrividiretto (char x,char y, char *s, char c)
  {
    char *punta;
    unsigned char co,co1;
    if (x==0)
      x = 40-strlen(s)/2;
    punta = MK_FP (0xB800,160*(y-1)+2*(x-1));
    co1 = strlen (s);
    for (co=0;co<co1*2;co+=2)
      {
	 *(punta+co) = *(s++);
	 if (c != 0)
	   *(punta+co+1) = c;
      }
  }

void cornice (char x1, char y1, char x2, char y2)
  {
    attributizonavideo (x1,y1,x1,y2,'³',0);
    attributizonavideo (x2,y1,x2,y2,'³',0);
    attributizonavideo (x1,y1,x2,y1,'Ä',0);
    attributizonavideo (x1,y2,x2,y2,'Ä',0);
    scrividiretto (x1,y1,"Ú",0);
    scrividiretto (x1,y2,"À",0);
    scrividiretto (x2,y1,"¿",0);
    scrividiretto (x2,y2,"Ù",0);
  }


void Scrivi_Help()
  {
    char provvi[16];

    if (ITEN)
      strcpy (provvi,"F1  Help");
    else
      strcpy (provvi,"F1  Istruzioni");
    scrividiretto (POSMESSAGGIX+11-(strlen(provvi)/2),POSMESSAGGIY,provvi,0);
  }

void Pulisci_Messaggio ()
  {
    scrividiretto (POSMESSAGGIX,POSMESSAGGIY,"                      ",0);
  }
void Scrivi_Messaggio (char *p)
  {
    Pulisci_Messaggio();
    scrividiretto (POSMESSAGGIX+11-(strlen(p)/2),POSMESSAGGIY,p,0);
  }

char Mostra_Finestra ()
  {
    int pro2;
    char i1,i2;
    char tot;

    if (CONTROLLINO==0)
      return (0);
    while (nomifile[I][0]==0)
      I = I - 1;
    pro2 = (I/(RIGHE*COLONNE))*(RIGHE*COLONNE);
    I = pro2+1;
    tot = 0;
    for (i2=1;i2<=COLONNE;i2++)
      {
	attributizonavideo (POSFINESTRAX+(i2-1)*17,POSFINESTRAY,POSFINESTRAX+(i2*17)-2,POSFINESTRAY+RIGHE-1,' ',0);
	attributizonavideo (POSFINESTRAX+(i2-1)*17,POSFINESTRAY,POSFINESTRAX+(i2*17)-2,POSFINESTRAY+RIGHE-1,NEROSUGRIGIO,1);
      }
    for (i2=1;i2<=COLONNE;i2++)
      for (i1=1;i1<=RIGHE;i1++)
	{
	  scrividiretto (POSFINESTRAX+(i2-1)*17+1,POSFINESTRAY+i1-1,nomifile[pro2+i1+(i2-1)*RIGHE-1],0);
	  if (nomifile[pro2+i1+((i2-1)*RIGHE)-1][0]!=0)
	    {
	      if (nomifile[pro2+i1+((i2-1)*RIGHE)-1][15]==1)
		attributizonavideo (POSFINESTRAX+(i2-1)*17,POSFINESTRAY+i1-1,POSFINESTRAX+(i2-1)*17+1+14,POSFINESTRAY+i1-1,NEROSUGRIGIOTAG,1);
	      tot++;
	    }
	}
    return (tot);
  }

void Sistema_Nome_File()
  {
    strcpy (NOMEFILE,nomifile[POS-1]);
    *(strchr (NOMEFILE,' '))=0;
    strcat (NOMEFILE,".DL");
  }
void Sistema_Nomifile ()
  {
    int i;
    for (i=0;i<TOTALE;i++)
      if (nomifile[i][15])
	i = TOTALE+5;
    if (i < TOTALE+5)
      nomifile[POS-1][15] = 1;
  }
void Disegna_Cornici ()
  {
    char i;
    cornice (POSFINESTRAX-1,POSFINESTRAY-1,POSFINESTRAX+COLONNE*17-1,POSFINESTRAY+RIGHE);
    for (i=POSFINESTRAX-1+17;i<=POSFINESTRAX-1+(COLONNE*17)-17;i+=17)
      {
	scrividiretto (i,POSFINESTRAY-1,"Â",0);
	scrividiretto (i,POSFINESTRAY+RIGHE,"Á",0);
	attributizonavideo (i,POSFINESTRAY,i,POSFINESTRAY+RIGHE-1,'³',0);
      }
  }
void Prepara_Finestra (char x1, char y1, char x2, char y2, char attr)
  {
    attributizonavideo (x1+1,y1,x2+1,y2+1,GRIGIOSCUROSUBLU,1);
    attributizonavideo (x1,y1,x2,y2,attr,1);
    attributizonavideo (x1+1,y2+1,x2+1,y2+1,'ß',0);
    attributizonavideo (x2+1,y1,x2+1,y1,'Ü',0);
    attributizonavideo (x2+1,y1+1,x2+1,y2,'Û',0);
  }

void Scrivi_Delay ()
  {
    attributizonavideo (POSDELAYX,POSDELAYY,POSDELAYX+11-POSDELAY,POSDELAYY,'Û',0);
    attributizonavideo (POSDELAYX+12-POSDELAY,POSDELAYY,POSDELAYX+11,POSDELAYY,' ',0);
  }
void Scrivi_Suono ()
  {
    scrividiretto (POSSUONOX,POSSUONOY,SOUNDMSG[SUONO],0);
  }
void Scrivi_Memoria ()
  {
    char libera[15];
    long memoria_libera;
    memoria_libera = coreleft ();
      if (XMS)
	memoria_libera += (unsigned long)1024*xms_libera();
    sprintf (libera,"%8lu Bytes",memoria_libera);
    scrividiretto (POSMEMX,POSMEMY,libera,0);
  }

void Muovi_Delay (char i)
  {
    switch (i)
      {
	case 1 : if (POSDELAY < 12)
		   POSDELAY++;
		 break;
	case 2 : POSDELAY = 12;
		 break;
	case 0 : POSDELAY = 8;
		 break;
	case -1: if (POSDELAY > 0)
		   POSDELAY--;
		 break;
	case -2: POSDELAY = 0;
      }
    DELAY = PDELAY[POSDELAY];
  }
void Scrivi_In_Lingua()
  {
    scrividiretto (POSBARRAX+2,POSBARRAY, MSG[ITEN][7],0);
    scrividiretto (POSBARRAX+2,POSBARRAY+1,MSG[ITEN][8],0);
    scrividiretto (POSDELAYX,POSDELAYY-1,MSG[ITEN][0],0);
    scrividiretto (POSMEMX,POSMEMY-1,MSG[ITEN][1],0);
    if (!NOSOUND)
      scrividiretto (POSSUONOX,POSSUONOY-1,MSG[ITEN][5],0);
    Pulisci_Messaggio();
    Scrivi_Help();
  }
void Intestazione ()
  {
    char provvi[80];
    sprintf (provvi,"%s   %s %s",PROGNAME,PROGVER,PROGCOPY);
    scrividiretto (POSTITOLOX+2,POSTITOLOY+1,provvi,0);
    Scrivi_In_Lingua();
    Scrivi_Delay();
    if (!NOSOUND)
      Scrivi_Suono();
  }
void Attiva_Palette_Testo (palette)
  register char *palette;
    {
      register int co=0,pu=0;

      disable ();

      while (inportb (0x03da) & 0x08);

      while (pu<64)
	{
	  if (pu == 6)
	    pu = 20;
	  if (pu == 21)
	    pu = 7;
	  if (pu==8)
	    pu = 56;
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

void Sfuma (char come)
    {
      char co,pu;
      char BACK[48],BACK2[48];

      switch (come)
	{
	  case 0 : memcpy (&BACK,&TESTOPALETTE,48);
		   for (co=0;co<32;co++)
		     {
		       for (pu=0;pu<48;pu++)
		       if (BACK [pu]>1)
			 BACK[pu]-=2;
		       Attiva_Palette_Testo (&BACK);
		     }
		   break;
	  case 1 : for (co=0;co<48;co++)
		     BACK2 [co] = TESTOPALETTE[co]-44;

		   for (co=0;co<22;co++)
		     {
		       for (pu=0;pu<48;pu++)
			 {
			   BACK2 [pu] += 2;
			   if (BACK2 [pu] < 0)
			     BACK [pu] = 0;
			   else
			     BACK [pu] = BACK2 [pu];
			  }
		       Attiva_Palette_Testo (&BACK);
		     }
		   Attiva_Palette_Testo (&TESTOPALETTE);
		   break;
	}
  }

void Prepara_Schermo()
  {
    char i;
    char BACK [48];
    for (i=0;i<24;i++)
      BACK [i] = 0;
    Attiva_Palette_Testo (&BACK);
    marginecolor (1);
    attributizonavideo (1,1,80,25,' ',0);
    attributizonavideo (1,1,80,25,GRIGIOSUBLU,1);
    Prepara_Finestra (POSTITOLOX,POSTITOLOY,80-POSTITOLOX,POSTITOLOY+2,NEROSUGRIGIO);
    Prepara_Finestra (POSFINESTRAX-1,POSFINESTRAY-1,POSFINESTRAX-1+COLONNE*17,POSFINESTRAY+RIGHE,NEROSUGRIGIO);
    Prepara_Finestra (POSBARRAX,POSBARRAY,80-POSBARRAX,POSBARRAY+1,NEROSUGRIGIO);
    Prepara_Finestra (POSDELAYX,POSDELAYY,POSDELAYX+11,POSDELAYY,COLOREBARRETTE);
    Prepara_Finestra (POSMEMX,POSMEMY,POSMEMX+13,POSMEMY,COLOREBARRETTE);
    if (!NOSOUND)
      Prepara_Finestra (POSSUONOX,POSSUONOY,POSSUONOX+12,POSSUONOY,COLOREBARRETTE);
    Prepara_Finestra (POSMESSAGGIX,POSMESSAGGIY,POSMESSAGGIX+21,POSMESSAGGIY,COLOREBARRETTE);
    attributizonavideo (8,1,38,1,7*17,1);
    Disegna_Cornici ();
    Intestazione();
    Scrivi_Help();
    scrividiretto (79-POSBARRAX-(strlen(NAME)),POSBARRAY+1,NAME,0);
    Sfuma (1);
  }

void Visualizza_Short_Info ()
  {
    attributizonavideo (1,POSBARRAY-5,80,POSBARRAY-4,' ',0);
    switch (VERSIONE)
      {
	case 1 : if (strchr (TITOLO,10)!=NULL)
		   *(strchr (TITOLO,10))=0;
		 scrividiretto (POSBARRAX+73-strlen(TITOLO),POSBARRAY-5,TITOLO,0);
		 break;
	case 2 :
	case 3 :
		 scrividiretto (POSBARRAX+73-strlen(TITOLO),POSBARRAY-5,TITOLO,0);
		 scrividiretto (POSBARRAX+69-strlen(AUTORE),POSBARRAY-4,"(c)",0);
		 scrividiretto (POSBARRAX+73-strlen(AUTORE),POSBARRAY-4,AUTORE,0);
		 break;
      }
  }

void errore (char e)
  {
    Scrivi_Messaggio (ERRMSG[ITEN][e-1]);
    sound (800);
    delay ((int) 50*DELAYPARAM);
    nosound();
    while (!kbhit());
    Pulisci_Messaggio();
    Scrivi_Help();
    ERRORE = 0;
  }

void evidenzia (int i,char on)
  {
     char x = POSFINESTRAX+(((i-1)/(RIGHE))%COLONNE)*17;
     char y = POSFINESTRAY+((i-1)%(RIGHE));
     char *punta = MK_FP (0xB800,160*(y-1)+2*(x-1)+1);
     char co,att;

     if (nomifile[POS-1][0]!='\\'&&nomifile[POS-1][0]!=' '&&MININFO&&ERRORE==0)
       {
	 if (on)
	   {
	     Sistema_Nome_File();
	     ERRORE = carica_informazioni(NOMEFILE);
/*	 attributizonavideo (POSBARRAX,POSBARRAY-1,POSBARRAX+74,POSBARRAY-1,' ',0);*/
	     if (ERRORE)
	       errore(ERRORE);
	     else
	       Visualizza_Short_Info();
	     NOMEFILE[0] = 0;
	     libera_memoria();
	  }
	else
	  {
	    attributizonavideo (1,POSBARRAY-5,80,POSBARRAY-4,' ',0);
	  }
       }
/*     else
       attributizonavideo (POSBARRAX,POSBARRAY-1,POSBARRAX+74,POSBARRAY-1,' ',0);*/
     if (nomifile[POS-1][15])
       {
	 if (on)
	   att = ATTRIBEVIDTAG;
	 else
	   att = NEROSUGRIGIOTAG;
       }
     else
       {
	 if (on)
	   att = ATTRIBEVID;
	 else
	   att = NEROSUGRIGIO;
       }
     for (co=0;co<32;co+=2)
       *(punta+co) = att;
  }

char Salva_Configurazione()
  {
    FILE *file;

    file = fopen (CONFIGURAZIONE,"w+b");
    if (file!=NULL)
      {
	fwrite (TESTOPALETTE,24,1,file);
	fwrite (&ITEN,1,1,file);
	fwrite (&MININFO,1,1,file);
	fwrite (&POSDELAY,1,1,file);
	fwrite (&SUONO,1,1,file);
	fwrite (&SB,1,1,file);
	fwrite (&CONTATORE,2,1,file);
	fclose (file);
	return (1);
      }
    else
      {
	return (0);
      }
  }

void Color_Evidenzia (char quale, char si)
  {
     if (si)
       attributizonavideo (26+(quale%3)*4,POSFINESTRAY+(quale/3),26+(quale%3)*4+2,POSFINESTRAY+(quale/3),ATTRIBEVID,1);
     else
       attributizonavideo (26+(quale%3)*4,POSFINESTRAY+(quale/3),26+(quale%3)*4+2,POSFINESTRAY+(quale/3),GRIGIOSUBLU,1);
  }

void Help (char iten)
  {
    char i;

    char *HELPMSG[2][15] = {{"F1  Istruzioni",
			     "F2  Suono (Speaker/SoundBlaster/No)",
			     "F3  Cambio Drive",
			     "F4  Italiano <-> Inglese",
			     "F6  Configurazione Colori",
			     "F9  File Info Attivo/Non Attivo",
			     "F10 Shell DOS",
			     "SPZ Seleziona/Deseleziona File",
			     "DEL Cancella il File",
			     "+   Aumento Velocita'",
			     "-   Diminuzione Velocita'",
			     "*   Massima velocita'",
			     "/   Minima velocita'",
			     "=   Velocita' Standard",
			     "ESC Uscita"
			    },
			   { "F1  Help",
			     "F2  Sound (Speaker/SoundBlaster/No)",
			     "F3  Change Drive",
			     "F4  Italian <-> English",
			     "F6  Color Configuration",
			     "F9  Mini Info ON/OFF",
			     "F10 Shell DOS",
			     "SPC Tag/Untag File",
			     "DEL Delete File",
			     "+   Increase Speed",
			     "-   Decrease Speed",
			     "*   Maximum Speed",
			     "/   Minimum Speed",
			     "=   Default Speed",
			     "ESC Quit"
			   }
			  };
    Salva_Schermo();

    attributizonavideo (1,5,52,22,' ',0);
    attributizonavideo (53,5,80,20,' ',0);
    attributizonavideo (1,5,52,22,GRIGIOSUBLU,1);
    attributizonavideo (53,5,80,20,GRIGIOSUBLU,1);
    attributizonavideo (10,6,12,21,30,1);

    for (i=0;i<15;i++)
      scrividiretto (10,6+i,HELPMSG[iten][i],0);

    if (iten)
      Scrivi_Messaggio ("TYPE ANY KEY");
    else
      Scrivi_Messaggio ("PREMI UN TASTO");

    getkey();
    Ripristina_Schermo();
  }

void ripristinapalettetesto ()
  {
    TESTOPALETTE[0] = 0;
    TESTOPALETTE[1] = 0;
    TESTOPALETTE[2] = 0;
    TESTOPALETTE[3] = 0;
    TESTOPALETTE[4] = 22;
    TESTOPALETTE[5] = 35;
    TESTOPALETTE[6] = 0;
    TESTOPALETTE[7] = 30;
    TESTOPALETTE[8] = 40;
    TESTOPALETTE[9] = 40;
    TESTOPALETTE[10] =40;
    TESTOPALETTE[11] =40;
    TESTOPALETTE[12] =45;
    TESTOPALETTE[13] =0;
    TESTOPALETTE[14] =0;
    TESTOPALETTE[15] =0;
    TESTOPALETTE[16] =10;
    TESTOPALETTE[17] =25;
    TESTOPALETTE[18] =0;
    TESTOPALETTE[19] =25;
    TESTOPALETTE[20] =63;
    TESTOPALETTE[21] =45;
    TESTOPALETTE[22] =45;
    TESTOPALETTE[23] =45;
    TESTOPALETTE[24] =21;
    TESTOPALETTE[25] =21;
    TESTOPALETTE[26] =21;
    TESTOPALETTE[27] =21;
    TESTOPALETTE[28] =21;
    TESTOPALETTE[29] =63;
    TESTOPALETTE[30] =21;
    TESTOPALETTE[31] =63;
    TESTOPALETTE[32] =21;
    TESTOPALETTE[33] =21;
    TESTOPALETTE[34] =63;
    TESTOPALETTE[35] =63;
    TESTOPALETTE[36] =63;
    TESTOPALETTE[37] =21;
    TESTOPALETTE[38] =21;
    TESTOPALETTE[39] =63;
    TESTOPALETTE[40] =21;
    TESTOPALETTE[41] =63;
    TESTOPALETTE[42] =63;
    TESTOPALETTE[43] =63;
    TESTOPALETTE[44] =21;
    TESTOPALETTE[45] =63;
    TESTOPALETTE[46] =63;
    TESTOPALETTE[47] =63;
  }

void Cambia_Configurazione ()
  {
    char i;
    int c;
    char provvi[48];

    Salva_Schermo();
    memcpy (provvi,TESTOPALETTE,48);
    attributizonavideo (1,POSFINESTRAY-1,POSFINESTRAX+COLONNE*17,POSFINESTRAY+RIGHE+1,' ',0);
    attributizonavideo (1,POSFINESTRAY-1,POSFINESTRAX+COLONNE*17,POSFINESTRAY+RIGHE+1,GRIGIOSUBLU,1);
    attributizonavideo (1,POSFINESTRAY-1,POSFINESTRAX+19,POSFINESTRAY+4,30,1);
    for (i=0;i<11;i++)
      scrividiretto (10,POSFINESTRAY+i,COLORMSG[ITEN][i],0);
    for (i=0;i<4;i++)
      scrividiretto (27,POSFINESTRAY+i,COLORMSG[ITEN][11],0);
    i = 0;
    do
      {
	Color_Evidenzia(i,1);
	c=getkey();
	Color_Evidenzia(i,0);
	switch (c)
	  {
	    case SU : if (i>2)
			i=i-3;
		      break;
	    case GIU : if (i<9)
			i=i+3;
		      break;
	    case SINISTRA : if (i>0)
			i--;
		      break;
	    case DESTRA : if (i<11)
			i++;
		      break;
	    case F1 : ripristinapalettetesto();
		      memcpy (provvi,TESTOPALETTE,48);
		      Attiva_Palette_Testo(&provvi);
		      break;
	    case '+' : if (i>5&&i<9)
			 {
			   if (provvi[15+i]<60)
			     {
			       provvi[15+i]+=3;
			       Attiva_Palette_Testo(&provvi);
			     }
			 }
		       else
			 if (provvi[3+i]<60)
			   {
			     provvi[3+i]+=3;
			     if (i<3)
			       {
				 provvi[15]=provvi[3]/3*2;
				 provvi[16]=provvi[4]/3*2;
				 provvi[17]=provvi[5]/3*2;
			       }
			     Attiva_Palette_Testo(&provvi);
			   }
		      break;
	    case '-' : if (i>5&&i<9)
			 {
			   if (provvi[15+i]>2)
			     {
			       provvi[15+i]-=3;
			       Attiva_Palette_Testo(&provvi);
			     }
			 }
		       else
			 if (provvi[3+i]>2)
			   {
			     provvi[3+i]-=3;
                             if (i<3)
			       {
				 provvi[15]=provvi[3]/3*2;
				 provvi[16]=provvi[4]/3*2;
				 provvi[17]=provvi[5]/3*2;
			       }
			     Attiva_Palette_Testo(&provvi);
			   }
		      break;
	  }
      }
    while (c!=ESC&&c!=ENTER);
    if (c==ENTER)
      {
	memcpy (TESTOPALETTE,provvi,48);
      }
    Attiva_Palette_Testo (&TESTOPALETTE);
    Ripristina_Schermo();
  }
char Cambio_Drive ()
  {
    int drivescelto;
    char msg[22];
    if (ITEN)
      strcpy (msg,"ENTER NEW DRIVE");
    else
      strcpy (msg,"INSERIRE NUOVO DRIVE");
    Scrivi_Messaggio (msg);
    drivescelto = upcase (getkey ());
    Pulisci_Messaggio();
    if (drivescelto!=27&&drivescelto!=13)
      {
	if (drivescelto<'A'||drivescelto>'Z')
	  ERRORE = 5;
	else
	  if (setdisk(drivescelto-65) < drivescelto-65)
	    ERRORE = 6;
	  else
	    if (drivescelto-65 != getdisk())
	      ERRORE = 6;
	    else
	      {
		Scrivi_Help();
		return (1);
	      }
      }
    else
      Scrivi_Help();
    return (0);
  }

void Pulisci_nomifile ()
  {
    int i;
    i = 0;
    for (i=0;i<MAXNOMIFILE;i++)
      {
	nomifile[i][0] = 0;
	nomifile[i][15] = 0;
      }
  }

void Carica_Dir ()
  {
    int i;
    struct ffblk cfile;
    int fatto;
    char drive[3];
    char dir[3];
    char name[9];
    char extension[5];


    Pulisci_nomifile();
    i = 0;
    fatto = findfirst ("*.*",&cfile,0x10);
    while (!fatto)
      {
	if (cfile.ff_attrib == 0x10&&!(cfile.ff_name[0]=='.'&&strlen(cfile.ff_name)==1))
	  {
	    strcpy (nomifile[i],"\\");
	    strcat (nomifile[i],cfile.ff_name);
	    i++;
	  }
	else
	  {
	    fnsplit (cfile.ff_name,drive,dir,name,extension);
	    if ((cfile.ff_attrib==0x20||cfile.ff_attrib==0x00) && strcmp(extension,".DL")==0)
	      {
		if (cfile.ff_fsize/1024 < 10000)
		  sprintf (nomifile[i],"%-9s%4dK",name,cfile.ff_fsize/1024);
		else
		  sprintf (nomifile[i],"%-9s %2dMb",name,(cfile.ff_fsize/1024)/1024);
		i++;
	      }
	  }
	fatto = findnext (&cfile);
      }
    TOTALE = 0;
    while (nomifile[TOTALE][0]!=0)
      TOTALE++;
    if (TOTALE==0)
      {
	strcpy (nomifile[0],"  NO ENTRIES");
	TOTALE = 1;
      }
  }

void Ordina_Dir (int s, int d)       /* Quick sort */
    {
      int i,j;
      nomefile x,w;

      i = s;
      j = d;
      strcpy (x,nomifile[(s+d)/2]);
      do
	{
	  while (strcmp (nomifile[i],x)<0)
	    i++;
	  while (strcmp (nomifile[j],x)>0)
	    j--;
	  if (i <= j)
	    {
	      strcpy (w,nomifile[i]);
	      strcpy (nomifile[i],nomifile[j]);
	      strcpy (nomifile[j],w);
	      ++i;
	      --j;
	    }
	 }
      while (i <= j);

      if (s < j)
	Ordina_Dir (s,j);

      if (i < d)
	Ordina_Dir (i,d);
    }
void Scrivi_Directory ()
  {
    char *buffer;
    char *p;

    buffer = malloc (255);
    if (ERRORE==8||ERRORE==9)
      {
	buffer[0] = (getdisk()+65);
	buffer[1] = ':';
	buffer[2] = 0;
      }
    else
      getcwd (buffer,255);
    scrividiretto (POSBARRAX+23,POSBARRAY,"                                              ",0);

    while (strlen(buffer)>45)
      {
	p = strrchr (buffer,92);
	if (p != NULL)
	  memcpy (buffer+7,p,10);
	buffer[3]='.';
	buffer[4]='.';
	buffer[5]='.';
	buffer[6]='.';
      }
    scrividiretto (POSBARRAX+23,POSBARRAY,buffer,0);
    free (buffer);
  }

char Scegli_File_o_Dir ()
  {
    int s;
    int oldI;
    char uscita=0;

    Scrivi_Memoria();
    Scrivi_Delay();
    Scrivi_Directory();
    while (!uscita)
      {
	TOT = Mostra_Finestra();
	if (LASTPOS>I+TOT-1||LASTPOS<I)
	  LASTPOS = I;
	POS = LASTPOS;
	oldI = I;
	while (oldI==I&&!uscita)
	  {
	    evidenzia (POS,1);
	    if (ERRORE)
	      errore (ERRORE);
	    s = 0;
	    s = getkey ();
	    if (CONTROLLINO==0)
	      s=0;
	    switch (s)
	      {
		case SU     : evidenzia (POS,0);
			      if (POS>I)
				POS--;
			      else
				if (I>2)
				  {
				    I -=2;
				    LASTPOS=I+1;
				  }
			      break;
		case PAGSU  : evidenzia (POS,0);
			      if (I>2)
				I -=2;
			      else
				POS = 1;
			      break;
		case GIU    : evidenzia (POS,0);
			      if (POS<I+TOT-1)
				POS++;
			      else
				if (POS%(RIGHE*COLONNE)==0)
				  I += RIGHE*COLONNE;
			      break;
		case PAGGIU : evidenzia (POS,0);
			      if (TOT==RIGHE*COLONNE)
				I += RIGHE*COLONNE;
			      else
				{
				  I = TOTALE;
				  LASTPOS = TOTALE;
				}
			      break;
		case DESTRA : evidenzia (POS,0);
			      if (POS<I+TOT-RIGHE)
				POS += RIGHE;
			      break;
		case SINISTRA:evidenzia (POS,0);
			      if (POS>=I+RIGHE)
				POS -= RIGHE;
			      break;
		case HOME   : evidenzia (POS,0);
			      POS = 1;
			      I = 1;
			      break;
		case FINE   : evidenzia (POS,0);
			      I = TOTALE;
			      LASTPOS = TOTALE;
			      break;
		case ENTER  : LASTPOS = POS;
			      if (nomifile[POS-1][0]=='\\')
				{
				  evidenzia (POS,0);
				  I = 1;
				  LASTPOS = 1;
				  uscita = 1;
				}
			      else
				if (nomifile[POS-1][0]!= ' ')
				  uscita = 2;
			      break;
		case BARRA  : if ((nomifile[POS-1][0]!='\\')&&(nomifile[POS-1][0]!= ' '))
				nomifile[POS-1][15]=(nomifile[POS-1][15]+1)%2;
			      evidenzia (POS,0);
			      if (nomifile[POS][0]!='\\')
				{
				  if (POS<I+TOT-1)
				    POS++;
				  else
				    if (POS==RIGHE*COLONNE)
				      I += RIGHE*COLONNE;
				}
			      break;
		case CANC     : LASTPOS = POS;
			      if (nomifile[POS-1][0]!='\\'&&nomifile[POS-1][0]!=' ')
				{
				  evidenzia (POS,0);
				  uscita = 5;
				}
			      break;
		case ESC    : Salva_Configurazione();
			      evidenzia (POS,0);
			      uscita = 4;
			      break;
		case '+'    : Muovi_Delay (-1);
			      Scrivi_Delay();
			      break;
		case '-'    : Muovi_Delay (1);
			      Scrivi_Delay();
			      break;
		case '='    : Muovi_Delay (0);
			      Scrivi_Delay();
			      break;
		case '*'    : Muovi_Delay (-2);
			      Scrivi_Delay();
			      break;
		case '/'    : Muovi_Delay (2);
			      Scrivi_Delay();
			      break;
		case F2     : if (!NOSOUND)
				{
				  pc_sound_terminate (SB);
				  switch (SUONO)
				    {
				      case 0 : SUONO = 1;
					       SB = pc_sound_init (0);
					       Scrivi_Suono ();
					       break;
				      case 1 : SB = pc_sound_init (1);
					       if (SB)
						 SUONO = 2;
					       else
						 SUONO = 0;
					       Scrivi_Suono ();
					       break;
				      case 2 : SUONO = 0;
					       Scrivi_Suono ();
					       break;
				   }
				}
			      break;
		case F3     : evidenzia (POS,0);
			      uscita = Cambio_Drive ();
			      break;
		case F4     : ITEN = (ITEN+1)%2;
			      Scrivi_In_Lingua();
			      break;
		case F1     : Help(ITEN);
			      break;
		case F6     : Cambia_Configurazione();
			      break;
		case F9     : MININFO = (MININFO+1)%2;
			      if (!MININFO)
                                attributizonavideo (1,POSBARRAY-5,80,POSBARRAY-4,' ',0);
			      break;
		case F10    : evidenzia (POS,0);
			      uscita = 3;
			      break;

	      } /* Fine Switch(s) */
	  } /* Fine While */
      } /*Fine While Principale */
    return (uscita);
  }

void allarme ()
  {
    int i;
    for (i=3;i<40;i++)
      {
	sound (100+(i*i/2));
	delay ((int) 5*DELAYPARAM);
      }
    for (i=40;i>6;i--)
      {
	sound (100+(i*i/2));
	delay ((int) 5*DELAYPARAM);
      }
    for (i=7;i<55;i++)
      {
	sound (100+(i*i/2));
	delay ((int) 4*DELAYPARAM);
      }
    for (i=55;i>10;i--)
      {
	sound (100+(i*i/2));
	delay ((int) 4*DELAYPARAM);
      }
    nosound();
  }
void shell (void) /* Crea una Shell del DOS */
  {
    if (coreleft()<10000)
      ERRORE = 3;
    else
      {
	textbackground (0);
	textcolor (7);
	marginecolor(0);
	modo_testo();
	Attiva_Palette_Testo(&TESTOPALORIGIN);
	gotoxy (1,1);
	if (ITEN)
	  cprintf ("Type EXIT to return in DL VIEWER");
	else
	  cprintf ("EXIT per tornare nel DL VIEWER");
	cursore (1);
	system ("command.com");
	Attiva_Palette_Testo(&TESTOPALETTE);
      }
  }  /* FINE shell */

void Cancella_File()
  {
    char NOMEBAK[15];
    strcpy (NOMEBAK,NOMEFILE);

    NOMEBAK[strlen(NOMEFILE)] = 'K';
    NOMEBAK[strlen(NOMEFILE)-1] = 'A';
    NOMEBAK[strlen(NOMEFILE)-2] = 'B';
    NOMEBAK[strlen(NOMEFILE)+1] = 0;
    if (rename (NOMEFILE,NOMEBAK) == -1)
      {
	sound (1000);
	delay ((int)100*DELAYPARAM);
	nosound ();
      }
  }

void Carica_Configurazione()
  {
    int file;

    if ((file = _open (CONFIGURAZIONE,1))!=-1)
      {
	if (filelength (file) == 31)
	  {
	    _read (file,&TESTOPALETTE,24);
	    _read (file,&ITEN,1);
	    _read (file,&MININFO,1);
	    _read (file,&POSDELAY,1);
	    DELAY = PDELAY[POSDELAY];
	    _read (file,&SUONO,1);
	    _read (file,&SB,1);
	    _read (file,&CONTATORE,2);
	    CONTATORE++;
	  }
	_close (file);
      }
    if (Salva_Configurazione()==0)
      CONTATORE=1000;
    if (!NOFONT)
      assegna_font (0);
  }

void abter (char p)
  {
    if (p)
      modo_testo();
    switch (p)
      {
	case 1: scrividiretto (0,10,"RISCONTRATE ALTERAZIONI",15);
		scrividiretto (0,11,"( ALTERATION  FOUND )",7);
                gotoxy (1,18);
		cursore(1);
		allarme ();
		exit (1);
	case 2: scrividiretto (0,10,"MEMORIA INSUFFICIENTE",15);
		scrividiretto (0,11,"( NOT ENOUGH MEMORY )",7);
		gotoxy (1,18);
		cursore(1);
		exit (2);
	case 3: scrividiretto (0,10,"CHIAVE ALTERATA",15);
		scrividiretto (0,11,"( INVALID KEY )",7);
		gotoxy (1,18);
		cursore(1);
		allarme();
		exit (3);
	case 4: scrividiretto (0,10,"CHIAVE DISABILITATA",15);
		scrividiretto (0,11,"( STOPPED KEY )",7);
		gotoxy (1,18);
		cursore(1);
		allarme();
		exit (4);
      }
  }

char Scrivi_Meglio (char *CHIAMATO, unsigned int chiave)
  {
    unsigned int ACCUMULO=20;
    int file;
    unsigned char conta;
    unsigned long fs;
    char huge *p,huge *base,huge *pend;
    unsigned int huge *finale;

    file = open (CHIAMATO,1);
    if (file!=NULL)
      {
	fs = filelength (file);
	p = farmalloc (fs);
	pend = p+fs-2;
	if (p==NULL)
	  return (2);
	base = p;
	if (big_read (file,p,fs)==-1)
	  return (1);
	close (file);
	ACCUMULO = 0;
	conta = 1;
	do
	  {
	    ACCUMULO += (*p)+(*p)*conta;
	    conta++;
	    p++;
	  }
	while (p!=pend);
	finale = (unsigned int huge *) pend;
	if (chiave)
	  {
            ACCUMULO += (*p)+(*p)*conta;
	    conta++;
	    p++;
	    ACCUMULO += (*p)+(*p)*conta;
	    *finale = chiave;
	  }
	if (*finale != ACCUMULO)
	  return (1);
	else
	  {
	    CONTROLLINO = 1;
	    farfree ((char far *)base);
	    return (0);
	  }
      }
    else
      return (1);
  }
void converti (char grafica)
  {
    char provvi[80];
    char drive[3]="\0";
    char dir[60]="\0";
    char filename[9]="\0";
    char ext[5]="\0";

    fnsplit (CONFIGURAZIONE,drive,dir,filename,ext);
    if (coreleft()<100000)
      ERRORE = 3;
    else
      {
	if (!grafica)
	  {
	    setta_colore (0,0,0,0);
	    memset (MK_FP(0xa000,0),0,64000);
	  }
	sprintf (provvi,"%s%sDL-CONV.EXE",drive,dir);
        gotoxy (8,1);
	spawnl (P_WAIT,provvi,provvi,NOMEFILE,"HIDDEN",NULL);
	if (!grafica)
	  setta_colore (7,0,0,0);
	ERRORE = 0;
      }
  }
void Finale ()
  {
    char provvi[20];

    itoa (CONTATORE,provvi,10);
    if (ITEN)
      execl (FINALE,FINALE,"E",provvi,NULL);
    else
      execl (FINALE,FINALE,"I",provvi,NULL);
  }
void controllascadenza()
  {
    char provvi[80];
    time_t datascadenza;
    datascadenza = (time_t)((PROGSCADA-70)*31536000+(PROGSCADM)*2592000+(PROGSCADG)*86400);
    time (&DATAODIERNA);
    if (DATAODIERNA > datascadenza)
      {
	modo_testo();
	attributizonavideo (0,0,80,3,31,1);
	sprintf (provvi,"%s   %s %s",PROGNAME,PROGVER,PROGCOPY);
	scrividiretto (POSTITOLOX+2,POSTITOLOY+1,provvi,0);
	if (ITEN)
	  {
	    scrividiretto (0,5,"This shareware version of DL VIEWER has expired !",12);
	    scrividiretto (0,7,"Please contact the authors to get a new version.",3);
	  }
	else
	  {
	    scrividiretto (0,5,"Questa versione shareware del DL VIEWER e' scaduta !",12);
	    scrividiretto (0,7,"Contattate gli autori per ottenere una versione recente.",3);
	  }
	scrividiretto (0,10,"GALACTICA BBS : +39-2-29006058 (FIDONET 2:331/358 to DL-SOFT)",15);
	scrividiretto (0,11,"INTERNET : dl-soft@galactica.it",15);
	scrividiretto (0,12,"FAX : +39-2-29006153",15);
	gotoxy (1,20);
	exit (0);
      }
  }

void testdelay ()
  {
    struct time t2,t1;
    long dif,p1;

    gettime (&t1);
    delay (200);
    gettime (&t2);
    dif = t2.ti_hour - t1.ti_hour;
    if (dif<0)
      dif = dif+24;
    dif = dif*360000; /*differenza ore in centesimi*/
    p1 = t2.ti_min - t1.ti_min;
    p1 = p1*6000;
    dif = dif + p1; /*sommo o detraggo minuti in centesimi*/
    dif = dif + (t2.ti_sec - t1.ti_sec)*100; /*sommo o detraggo secondi in centesimi*/
    dif = dif + t2.ti_hund - t1.ti_hund; /*sommo o detraggo centesimi*/
    if (dif==0)
      dif=1; /* per sicurezza */
    DELAYPARAM = (float) 20/dif;
  }

void processaparametro (char *s)
  {
    char provvi[128];
    strcpy (provvi,s);
    if (strcmp ("/NOSOUND",strupr(provvi))==0)
      NOSOUND = 1;
    if (strcmp ("/NOXMS",strupr(provvi))==0)
      NOXMS = 1;
    if (strcmp ("/NOFONT",strupr(provvi))==0)
      NOFONT = 1;
    if (strcmp ("-F",strupr(provvi))==0)
      FASTMODE = 1;
  }
main (int argc, char *argv[])
  {
    char uscita;
    char p[15];
    char drive[3]="\0";
    char dir[60]="\0";
    char filename[9]="\0";
    char ext[5]="\0";
    char chiave;
    char ripristinare;
    char aniresult;
    char i;


    for (i=1;i<argc;i++)
      processaparametro (argv[i]);
    if (NOXMS)
      XMS = 0;
    else
      XMS = esiste_xms();
    modo_testo ();
    if (!NOFONT)
      assegna_font (0);
    cursore(0);
    ripristinapalettetesto();
    strcpy (KEY,argv[0]);
    strcpy (CONFIGURAZIONE,argv[0]);
    CONFIGURAZIONE [strlen(CONFIGURAZIONE)-3] = 0;
    strcat (CONFIGURAZIONE,"CFG");
    strcpy (FINALE,argv[0]);
    FINALE [strlen(FINALE)-3] = 0;
    strcat (FINALE,"DAT");

    if (!FASTMODE)
       {
	abter (Scrivi_Meglio (argv[0],0));
	abter (Scrivi_Meglio (FINALE,45256));
      }
    else
     CONTROLLINO = 1;


    chiave = Controlla_Chiave();
    if (chiave==1)
      REGISTRATO = 1;
    else
      if (chiave==2)
	REGISTRATO = 0;
      else
	abter(chiave);

    harderr(diskready);
    ctrlbrk(breakcontrol);
    testdelay ();
    Carica_Configurazione();
    controllascadenza();
    if (!NOSOUND)
      {
	SB = pc_sound_init (SB);
	if (SUONO>0)
	  SUONO = 1+SB;
	else
	  pc_sound_terminate (SB);
      }
    LASTPOS = 0;
    I = 1;
    putenv ("PROMPT=[DL VIEWER] $p$g");

    aniresult = 0;
    for (i=1;i<argc;i++)
      {
	if (argv[i][0]!='/'&&argv[i][0]!='-')
	  {
	    fnsplit(argv[i],drive,dir,filename,ext);
	    if (drive[0]!=0)
	      {
		if (drive[0]>96)
		  drive[0] -=32;
		setdisk(drive[0]-65);
	      }
	    if (dir[0])
	      {
		if ((strlen(dir)>1)&&(dir[strlen(dir)-1]==92))
		  dir[strlen(dir)-1] = 0;
		chdir (dir);
	      }
	    if (ext[0]==0&&filename[0]!=0)
	      {
		if (chdir (filename)==0)
		  filename[0]=0;
	      }
	    if (filename[0])
	      {
		strcpy (NOMEFILE,filename);
		strcat (NOMEFILE,".DL");
		attributizonavideo (1,1,80,25,' ',0);
		attributizonavideo (1,1,80,25,0,1);
		ERRORE = carica_tutto (NOMEFILE);
		if (ERRORE==0)
		  {
		    if (!aniresult)
		      Sfuma (0);
		    else
		      memset(MK_FP(0XA000,0),0,64000);
		    aniresult = 1;
		    if (!entra_in_grafica ())
		      animazione ();
		  }
		libera_memoria();
		NOMEFILE[0]=0;
	      }
	  }
      } /* fine ciclo for */
    if (aniresult)
      {
	modo_testo ();
	if (!NOFONT)
	  assegna_font (0);
	if (REGISTRATO)
	  {
	    Finale();
	    exit (0);
	  }
	cursore (0);
	aniresult=0;
      }
    Prepara_Schermo();

    do
      {
	Carica_Dir();
	Ordina_Dir(0,TOTALE-1);
	uscita = Scegli_File_o_Dir();
	ripristinare = 0;
	switch (uscita)
	  {
	    case 1 : strcpy (p,nomifile[POS-1]);
		     strrev (p);
		     p[strlen(p)-1]=0;
		     strrev (p);
		     chdir (p);
		     break;
	    case 2 : Sistema_Nomifile();
		     PRIMAVOLTA = 1;
		     for (POS=1;POS<=TOTALE;POS++)
		       {
			 if (nomifile[POS-1][15])
			   {
			     Sistema_Nome_File();
			     if (PRIMAVOLTA)
			       Scrivi_Messaggio (MSG[ITEN][9]);
			     do
			      {
			       ERRORE = carica_tutto (NOMEFILE);
			       if (ERRORE == 10)
				 {
				   converti (PRIMAVOLTA);
				   ERRORE = carica_tutto (NOMEFILE);
				 }
			       if (PRIMAVOLTA)
				 Pulisci_Messaggio();
			       if (ERRORE==0&&CONTROLLINO)
				 {
				   if (PRIMAVOLTA)
				     Sfuma (0);
				   if (!entra_in_grafica ())
				     {
				       if ((aniresult=animazione ())==2)
					 POS=TOTALE;
				     }
				   else
				     POS=TOTALE;
				   ripristinare = 1;
				 }
			       libera_memoria();
			       PRIMAVOLTA = 0;
			      }
			     while (aniresult==3);
			   }
		       }
		     if (ripristinare)
		       {
			 modo_testo ();
			 if (!NOFONT)
			   assegna_font (0);
			 cursore (0);
			 Prepara_Schermo();
		       }
		     break;
	    case 3 : shell();
		     cursore (0);
		     clrscr();
		     if (!NOFONT)
		       assegna_font(0);
		     Prepara_Schermo();
		     break;
	    case 5 : Sistema_Nome_File();
		     Cancella_File();
		     break;
	  }
      }
    while (uscita!=4);
    libera_memoria();
    if (!NOSOUND)
      pc_sound_terminate (SB);
    Finale();
    modo_testo ();
  }
