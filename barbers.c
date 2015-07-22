//predmet: Operacni systemy
//autor: xcasla03@stud.fit.vutbr.cz, Martin Caslava
//projekt: synchronizace paralelnich procesu - problem spiciho holice
//prog jazyk: C
//hodnoceni: 14/15

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define budik_holice 0 //semafor ktery hlida zvonek ktery pri prichodu zakaznika do cekarny budi holice
#define holic_volny 1 //semafor hlidajici stav holice nestriha/striha
#define dvere_cekarny 2 //semafor ktery hlida dvere od cekarny
#define hotovo 3 //semafor ktery posila ostrihaneho zakaznika domu
#define zakaznik_odesel 4 //semafor ktery rika ze zakaznik odesel a muze jit zontrolovat cekarnu
#define cekani_customerReady 5 //semafor ktery nastavi zakaznik_odesel na zelenou - zakaznik uz odesel a holic muze zkontrolovat cekarnu
#define blok_akce 6 //semafor ktery zablokuje vypis akce dalsimu procesu
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** FUNKCE NA INICIALIZACI SEMAFORU **/
void f_init_sem(int sem_ID,int sem_num,int val) //fuknce na inicializaci semaforu
{
  union sem_num 
	{
    	 int val;
    	 struct semid_ds *buf;
    	 unsigned short *array;
  	}struct_sem;
  struct_sem.val=val;
  semctl(sem_ID,sem_num,SETVAL,struct_sem); 
} 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** FUNKCE NA DEALOKACI SEMAFORU **/
int f_dealokace_semaforu(int sem_ID)
{
  union sem_num;
  return semctl(sem_ID, 1, IPC_RMID);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** FUNKCE ZAKAZNIK ODESEL - UMOZNUJE HOLICI KONTROLOVAT CEKARNU **/
void f_zakaznik_odesel(int sem_ID,struct sembuf *semafor)
{ 
//nastavi zakaznik_odesel na zelenou - zakaznik uz odesel a holic muze zkontrolovat cekarnu
 semafor->sem_num=zakaznik_odesel;
 semafor->sem_op=1;//zelenou
 semafor->sem_flg=0;
 semop(sem_ID,semafor,1);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** FUNKCE HOLIC **/
void f_holic(int sem_ID,struct sembuf *semafor,int doba_obsluhy,int *obs_zid, int *akc,FILE *soubor)//pocet obsazenych zidli a cislo akce se predava odkazem
{ 
setbuf(soubor,NULL);//zajisti aby se nemichaly cisla radku
//nastavi budiku holice cervenou - holic zjistuje jestli je nekdo v cekarne a ceka az dostane od zakaznika zelenou//

  semafor->sem_num=blok_akce;//cervena
  semafor->sem_op=1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);
  fprintf(soubor, "%d: barber: checks\n",*akc=*akc+1);
  semafor->sem_num=blok_akce;//zelena
  semafor->sem_op=1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);

  semafor->sem_num=budik_holice;//cervena
  semafor->sem_op=-1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);

//nastavi dverim cekarny cervenou - zamkne holicstvi(osetreni deadloku), uvolni se zidle pro dalsiho zakaznika 
  semafor->sem_num=dvere_cekarny;//cervena
  semafor->sem_op=-1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);
  *obs_zid=*obs_zid-1;//uvolnila se jedna zidle pro dalsiho zakaznikama

//nastavi prvnimu zakaznikovi cekajicimu na radu zelenou - vezme ho strihat
  semafor->sem_num=blok_akce;//cervena
  semafor->sem_op=-1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);
  fprintf(soubor, "%d: barber: ready\n",*akc=*akc+1);
  semafor->sem_num=blok_akce;//zelena
  semafor->sem_op=1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);

  semafor->sem_num=holic_volny;//zelena
  semafor->sem_op=1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);

//nastavi dverim cekarny zelenou - odemkne holicstvi aby mohli chodit dalsi zakaznici
  semafor->sem_num=dvere_cekarny;
  semafor->sem_op=1;//zelena
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);
  
//holic striha
  if (doba_obsluhy > 0)//osetreni chyby deleni nulou, ceka se jen pri nenulovem case
{
  int c = random() % (doba_obsluhy);
  usleep(c*1000);//simuluje dobu strihani
}
  semafor->sem_num=cekani_customerReady;
  semafor->sem_op=-1;//cervena
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);

  semafor->sem_num=blok_akce;//cervena
  semafor->sem_op=-1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);
  fprintf(soubor, "%d: barber: finished\n",*akc=*akc+1);
  semafor->sem_num=blok_akce;//zelena
  semafor->sem_op=1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);


//nastavi hotovo na zelenou - zakaznik je ostrihan a muze odejit
  semafor->sem_num=hotovo;
  semafor->sem_op=1;//zelena
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);

//nastavi zakaznik_odesel na cervenou - zakaznik jeste neodesel a holic nemuze zkontrolovat cekarnu
  semafor->sem_num=zakaznik_odesel;
  semafor->sem_op=-1;//cervena
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** FUNKCE ZAKAZNIK **/
void f_zakaznik(int sem_ID,struct sembuf *semafor,int *obs_zid, int *akc,int pocet_zidli,int cis_zak,FILE *soubor)//pocet obsazenych zidli a cislo akce se predava odkazem
{
setbuf(soubor,NULL);//zajisti aby se nemichaly cisla radku
semafor->sem_num=blok_akce;//cervena
semafor->sem_op=-1;
semafor->sem_flg=0;
semop(sem_ID,semafor,1);
fprintf(soubor, "%d: customer %d: created\n", *akc=*akc+1, cis_zak);
semafor->sem_num=blok_akce;//zelena
semafor->sem_op=1;
semafor->sem_flg=0;
semop(sem_ID,semafor,1);

//nastavi dverim cekarny cervenou - zamkne cekarnu(osetreni deadlocku) a jde zkontrolovat jestli je volna zidle//
  semafor->sem_num=dvere_cekarny;//cervena
  semafor->sem_op=-1;
  semafor->sem_flg=0;
  semop(sem_ID,semafor,1);
 
if(*obs_zid < pocet_zidli) //kdyz je volna zidle (pocet volnych zidli < pocet zidli celkem) sedne si a zvysi pocet obsazenych zidli
	{
		//nastavi budiku holice zelenou - probudi holice a ten zacne strihat//

		semafor->sem_num=blok_akce;//cervena
  		semafor->sem_op=-1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);
		fprintf(soubor, "%d: customer %d: enters\n", *akc=*akc+1, cis_zak);
 		semafor->sem_num=blok_akce;//zelena
  		semafor->sem_op=1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);

  		*obs_zid=*obs_zid+1;//zvysi pocet obsazenych zidli
  		semafor->sem_num=budik_holice;//zelena 
  		semafor->sem_op=1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);

		//nastavi dverim cekarny zelenou - otevre dvere holicstvi aby mohli chodit dalsi zakaznici
  		semafor->sem_num=dvere_cekarny;//zelena 
  		semafor->sem_op=1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);   

		//nastavi stavu holice cervenou - zakaznik ceka az prijde na radu
  		semafor->sem_num=holic_volny;//cervena 
  		semafor->sem_op=-1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1); 

		semafor->sem_num=blok_akce;//cervena
  		semafor->sem_op=-1;
 		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);
		fprintf(soubor, "%d: customer %d: ready\n", *akc=*akc+1, cis_zak);
 		semafor->sem_num=blok_akce;//zelena
 		semafor->sem_op=1;
  		semafor->sem_flg=0;
 		semop(sem_ID,semafor,1);

		semafor->sem_num=cekani_customerReady;
  		semafor->sem_op=1;//zelena
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);

		//nastavi hotovo na cervenou - zacina strihani zakaznika
  		semafor->sem_num=hotovo;
        	semafor->sem_op=-1;//cervena
        	semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);

		semafor->sem_num=blok_akce;//cervena
  		semafor->sem_op=-1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);
		fprintf(soubor, "%d: customer %d: served\n", *akc=*akc+1, cis_zak);
 		semafor->sem_num=blok_akce;//zelena
  		semafor->sem_op=1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);

  	}
else //kdyz neni volna zidle zakaznik vstoupi a odejde
	{
		semafor->sem_num=blok_akce;//cervena
  		semafor->sem_op=-1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);
		fprintf(soubor, "%d: customer %d: enters\n", *akc=*akc+1, cis_zak);
		semafor->sem_num=blok_akce;//zelena
  		semafor->sem_op=1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);

		semafor->sem_num=blok_akce;//cervena
  		semafor->sem_op=-1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);
  		fprintf(soubor, "%d: customer %d: refused\n", *akc=*akc+1, cis_zak);
		semafor->sem_num=blok_akce;//zelena
  		semafor->sem_op=1;
  		semafor->sem_flg=0;
  		semop(sem_ID,semafor,1);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** sdilena struktura SHM_struct **/
typedef struct 
{
int obsazene_zidle;//pocet obsazenych zidi v cekarne cekarna->akce
int akce;//citac akcini
int posledni_zakaznik;//indetifikator posledniho zakaznika
}SHM_struct;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** ZACATEK MAINU **/
int main (int argc, char *argv[])
{
	setbuf(stdout,NULL);//zajisti aby se nemichaly cisla radku	
	pid_t zakaznik_pid;//pid procesu zakaznik  
	pid_t holic_pid;//pid procesu holic
	srandom(time(NULL));
	SHM_struct *cekarna; //vytvoreni promenne cekarna ktera je datoveho typu SHM_struct
	char *SHM_pointer;//ukazatel na sdilenou pamet
	int SHM_velikost;//velikost sdilene pameti 
	int SHM_segment_ID;//id segment sdilene pameti, predava se funkci shmat(), ktera vraci ukazatel na sdilenou pamet

	/** NASTAVENI SEMAFORU **/
	int sem_ID;//id semaforu
  	struct sembuf semafor;//struktura semafor
	sem_ID=semget(IPC_PRIVATE,7,IPC_CREAT|0666);//ziskani id pro semafor
	f_init_sem(sem_ID,budik_holice,0); //znameni od zakaznika pro holice (budik) - holic se probudi a zacina strihat 
  	f_init_sem(sem_ID,holic_volny,0); //znameni od holice pro zakaznika - holic striha/nestriha 
  	f_init_sem(sem_ID,dvere_cekarny,1); //otevreni/zavreni dveri od cekarny-NA ZACATKU NASTAVEN NA 1!!! aby mohl projit prvni zakaznik
 	f_init_sem(sem_ID,hotovo,0); //posila ostrihaneho zakaznika domu
	f_init_sem(sem_ID,zakaznik_odesel,0); //znameni pro holice ze zakaznik odesel a muze kontrolovat cekarnu
	f_init_sem(sem_ID,cekani_customerReady,0);//nastavi zakaznik_odesel na zelenou - zakaznik uz odesel a holic muze zkontrolovat cekarnu
	f_init_sem(sem_ID,blok_akce,1); //zablokuje vypis akce dalsimu procesu

	/** PRACE SE SDILENOU PAMETI **/
	SHM_velikost = sizeof(SHM_struct);//zjisteni velikosti pro alokaci sdilene pameti
	SHM_segment_ID = shmget(IPC_PRIVATE, SHM_velikost, IPC_CREAT | 0666);//alokace sdileneho pametoveho segmentu
	if ((SHM_segment_ID = shmget(IPC_PRIVATE, SHM_velikost, IPC_CREAT | 0666)) < 0)//osetreni chyby alokace
	{
		fprintf(stderr, "Chyba: nepodarilo se alokovat sdilenou pamet!\n");
		shmctl(SHM_segment_ID, IPC_RMID, NULL);//dealokace sdilene pameti
		return(EXIT_FAILURE);
	}
	SHM_pointer = (char *) shmat(SHM_segment_ID, NULL, 0);//ziskani ukazatele na sdilenou pamet
	cekarna = (SHM_struct *) SHM_pointer;//sdileni struktury SHM_struct
	/** ZPRACOVANI PARAMETRU **/
	if ( argc != 6 ) //pokud je vice parametru nez 5 (+ nazev programu), tak chyba
	{
		fprintf (stderr, "Chyba: nespravne zadane parametry!\n");
		return (EXIT_FAILURE);
	}
	for (int PocNumArg = 1; PocNumArg<5; PocNumArg++) //cyklus pres pocet parametru
	{
		int arg_cis=strlen(argv[PocNumArg]);//zjisti delku retezce parametru
		for (int i = 0; i<arg_cis; i++)//cyklus v parametru pres cislice
		{
			if (!isdigit(argv[PocNumArg][i])) //test na numeriku prvnich 4 parametru, testuje v poradi [parametr][znak]
			{
				fprintf (stderr, "Chyba: nespravne zadane parametry!\n");
				return (EXIT_FAILURE);
			}
		}
	}
	int pocet_zidli = atoi(argv[1]);
	int zakaznik_gentime = atoi(argv[2]); 
	int doba_obsluhy = atoi(argv[3]);
	int pocet_zakazniku = atoi(argv[4]); 
	if (pocet_zakazniku == 0)//pokud je pocet zakazniku 0 tak hned zkonci
	{
		shmdt(SHM_pointer);//odpojeni sdilene pameti
		shmctl(SHM_segment_ID, IPC_RMID, NULL);//dealokace sdilene pameti
		f_dealokace_semaforu(sem_ID);//dealokace semaforu
		exit(EXIT_SUCCESS);
	}
	int pocet_procesu = pocet_zakazniku+1;
	FILE *soubor;//promena soubor
	if (*argv[5] == *"-")
	{
		soubor=stdout;//vypis na stdout
	}
	else
	{
		soubor=fopen(argv[5],"w");//zapis do souboru
	}
	pid_t pid_pole[pocet_procesu]; //pole pro ukladani pidu jednotlivych procesu, podle kterych se bude cekat na jejich ukonceni
	cekarna->obsazene_zidle = 0; //pocet obsazenych zidli - na zacatku 0
	cekarna->posledni_zakaznik = 0;//identifikator posledniho zakaznika - na zacatku 0
	cekarna->akce=0;//pocitadlo akci - na zacatku 0
	holic_pid = fork();//vytvori proces holice
	if (holic_pid == 0)
	/*tohle dela holic*/
	{
		while ((cekarna->posledni_zakaznik == 0) || (cekarna->obsazene_zidle > 0))//pokud neni obslouzen posledni zakaznik nebo jsou lidi v cekarne tak
		{
			f_holic(sem_ID,&semafor,doba_obsluhy,&cekarna->obsazene_zidle,&cekarna->akce,soubor);//zavola se funkce holice
		}
		exit(EXIT_SUCCESS);//ukonci proces holice
	}
	else if (holic_pid > 0)
	/*ulozi holicovo pid do pole, tohle dela hlavni proces*/
	{
		pid_pole[0] = holic_pid; 
		/*Cyklus na vytvoreni zakazniku*/
		for (int i=1;i<pocet_zakazniku+1;i++)
		{
			if(zakaznik_gentime > 0 )//osetreni chyby deleni nulou, ceka se jen pri nenulovem case
			{
				int r = random() % (zakaznik_gentime+1);//vygeneruje nahodnou hodnotu r v rozmezi 2. parametru
				usleep(r*1000);//usni po nahodnou dobu r[ms]
			}
			zakaznik_pid = fork();//vytvori proces zakaznika
			if (zakaznik_pid > 0)
			/*ulozi zakaznikovo pid do pole, tohle dela hlavni proces*/
			{ 
				pid_pole[i] = zakaznik_pid;
			}	
			else if (zakaznik_pid == 0)
				/*tohle dela zakaznik*/
				{
					f_zakaznik(sem_ID,&semafor,&cekarna->obsazene_zidle,&cekarna->akce,pocet_zidli,i,soubor);//zavola se funkce zakaznika
					if (pocet_zakazniku == i)
					{	
						cekarna->posledni_zakaznik = 1; //posl. zakaznik = 1 - byl obslouzen posledni zakaznik
					}
					f_zakaznik_odesel(sem_ID,&semafor);
					exit(EXIT_SUCCESS);//ukonci proces zakaznika
				}
			else
			{
				fprintf(stderr, "Chyba: vytvareni zakaznika! %d\n", i);
				return(EXIT_FAILURE);
			}
		}
	}
	else
	{
		fprintf(stderr, "Chyba: vytvareni holice!");
		return(EXIT_FAILURE);
	}	
	/*cyklus cekani na ukonceni vsech procesu, tohle dela hlavni proces*/
	for (int i = 0; i < pocet_procesu; i++)
	{
		if ((waitpid(pid_pole[i], NULL, 0)) < 0)
		{
			kill(pid_pole[i],SIGTERM);
			fprintf(stderr, "Chyba: ukonceni procesu!\n");
			return(EXIT_FAILURE);
		}    
	}
	shmdt(SHM_pointer);//odpojeni sdilene pameti
	shmctl(SHM_segment_ID, IPC_RMID, NULL);//dealokace sdilene pameti
	fclose(soubor);//uzavreni souboru
	f_dealokace_semaforu(sem_ID);//dealokace semaforu
	exit(EXIT_SUCCESS);
/** KONEC MAINU **/
}
