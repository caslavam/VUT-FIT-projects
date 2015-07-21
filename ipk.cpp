/*
predmet: pocitacove komunikace a site - IPK
projekt: jednoduchy ftp klient
autor: xcasla03@stud.fit.vutbr.cz, Martin Caslava
hodnoceni: 10/10
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <regex.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <locale.h>

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

using namespace std;

//nejaky pomocny promenny
int konec = 0;
int konec2 = 0;

int mySocket;   // prvni ridici Socket
int mySocket2; //druhy socket na ktery posilam pozadavem LIST
int port = 21;  //port, standartne 21 (upravit na nacitani jineho portu z url)
int new_port; //novy port ktery pro druhy socket
int adrIndex = 11;		//pocitadlo pro znaky v adrese
int passIndex; //index pro pozice zjistovani hesla
int lomitko = 0;		//pozice lomitek v nazvu souboru
int delka_msg_path; //delka zpravy s cestou
int msg_index = 0; //pomocny index zpravy
char resp; 			//prijimany znak ze serveru



string test; //vysledny string s adresarema
string path;           //cesta
string buff; //buffer pro cteni odpovedi
string num; 
string epsv; //zprava epsc
string msg_path; //zprava obsahujici cestu k adresari
string list;//LIST zprava
string user; //zoprava user
string password; //zprava s heslem
string adresaServeru;	//adresa serveru
string zadanaAdresa;	//cela adresa zadavana jako argument
string response; 		//odpoved ze serveru
string adresa_heslo; //url obsahujici usera a heslo
string adr_user;//pomocna promenna pro user
string adr_password;//pomocna promenna pro heslo

/*********************************************
******funkce na zpracovani zadavane adresy***
********************************************/
void parsujAdresu(string zadanaAdresa)
{
  //rozpoznani zda bylo zadano http://
  if (zadanaAdresa.compare(0,6, "ftp://") == 0) //pokud adresa zacina na http://
  {
    //posun index adresy na sedmy znak (preskocim http://)
    adrIndex = 6;
  }
  else
  {
    //jinak cti od zacatku adresy
    adrIndex = 0;
  }
  /*cteni adresy hosta*/
  while(adrIndex < zadanaAdresa.length()) //nacti hosta
  {
    string tmpAdresa;
    tmpAdresa.clear();
    tmpAdresa += zadanaAdresa.at(adrIndex);
    
    //pokud adresa pokracuje cestou
    if (zadanaAdresa.compare(adrIndex,1, "/") == 0)
      break;
    //pokud cesta pokracuje cislem portu
      if (zadanaAdresa.compare(adrIndex,1, ":") == 0) 
	break;
      
      adresaServeru += tmpAdresa;
      adrIndex++;
  }
  /*cteni cisla portu*/
  if (zadanaAdresa.compare(adrIndex,1, ":") == 0)
  {
    adrIndex++; //posune index na prvni cislici portu
    string tmpPort;
    tmpPort.clear();
    
    while(adrIndex < zadanaAdresa.length())
    {
      //pokud je cislo
      if (isdigit(zadanaAdresa.at(adrIndex)))
	//pridej cislici portu
      tmpPort = tmpPort + zadanaAdresa.at(adrIndex);
      //pokud je prazdny znak - break
      if (isspace(zadanaAdresa.at(adrIndex)))
	break;
      //pokud pokracuje cesta - break
	if (zadanaAdresa.compare(adrIndex,1, "/") == 0)
	  break;
	adrIndex++;
    }
    port = atoi(tmpPort.c_str());
  }
  /*cteni cesty*/
  if (adrIndex < zadanaAdresa.length())
  {
    //vymaze z path defaultne zadane lomitko
    path.clear();
    //pokud je jeste co cist
    while (adrIndex < zadanaAdresa.length())
    {  
      string tmpPath;
      tmpPath.clear();
      tmpPath += zadanaAdresa.at(adrIndex);
      path += tmpPath;
      adrIndex++;
    }
  }
} 
/**************************************************************
 *******************odeslani dotazu na server*****************
 *************************************************************/
int sent_msg(int mySocket, string msg, int msglen)
{
   if(write(mySocket, msg.c_str(), msglen) == -1)
    {
      cerr << "nelze odeslat pozadavek" << endl;
      return EXIT_FAILURE;
    }
}

/********************************************************
**********cteni odpovedi ze serveru*********************
/******************************************************/
int read_server_response(int mySocket, string buff)
{
   response.clear();
   read(mySocket, &resp, 1);
   msg_index++;
   response.append(&resp, 1);
   test.append(&resp);
   return 0;
}

/***************
******MAIN******
****************/
int main(int argc, char *argv[])
{    
   //kontrola argumentu
   if (argc != 2)
   {
     cerr << "spatne zadane zadane argumenty programu" << endl;
     return EXIT_FAILURE;
   }       
   //ziska adresu serveru z argumentu
   zadanaAdresa = argv[1];
   //nahrazeni mezer v argumentu
   if(zadanaAdresa.find(" ") != string::npos)
   {
     int pocPoz = zadanaAdresa.find(" ");//pocatecni pozice vyhledavaneho podretezce
     zadanaAdresa.replace(pocPoz, 1, "%20");//nahrazeni
   }
  
  //pokud URL obsahuje v parametrech heslo a jmeno
   std::size_t found=zadanaAdresa.find("@");
   if (found!=std::string::npos)
   {
     adresa_heslo =  zadanaAdresa;
     zadanaAdresa.clear();
     found = found + 1;
    
     while(found < adresa_heslo.length()) //nacti hosta
     {
       string tmp_zadana_adresa;
       tmp_zadana_adresa.clear();
       tmp_zadana_adresa += adresa_heslo.at(found);
       zadanaAdresa += tmp_zadana_adresa;
       found++;
     }
    
     if (adresa_heslo.compare(0,6, "ftp://") == 0) //pokud adresa zacina na http://
     {
       //posun index adresy na sedmy znak (preskocim http://)
       passIndex = 6;
     }
     else
       passIndex = 0;
    
     //parsovani usera
     while(adresa_heslo.at(passIndex) != ':') 
     {
       string tmp_user;      
       tmp_user.clear();
       tmp_user += adresa_heslo.at(passIndex);
       adr_user += tmp_user;
       passIndex++;
     }
    
     passIndex++;
     //parsovani hesla
     while(adresa_heslo.at(passIndex) != '@') 
     {
	string tmp_pass;      
	tmp_pass.clear();
	tmp_pass += adresa_heslo.at(passIndex);
	adr_password += tmp_pass;
	passIndex++;
      }
      //user a password nesmi byt prazdne
      if( (adr_password == "") || (adr_user == "") )
      {	
	cerr << "zadana URL obsahuje prazdne heslo nebo uzivatele" << endl;
	return EXIT_FAILURE;
      }
    }
  
    parsujAdresu(zadanaAdresa);         //parsovani adresy
    hostent *host;              	// host
    struct sockaddr_in serverSock;     // socket pro server
  
    // Vytvoreni prvniho socketu
    if ((mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
      cerr << "nelze vytvorit socket" << endl;
      return EXIT_FAILURE;
    }
    
    //kontrola zadane url adresy
    if ((host = gethostbyname(adresaServeru.c_str())) == NULL)
    {
      cerr << "spatna adresa" << endl;
      return EXIT_FAILURE;
    }

    //zaplneni struktury sockaddr_in
    serverSock.sin_family = AF_INET;
    serverSock.sin_port = htons(port);
    memcpy(&(serverSock.sin_addr), host->h_addr, host->h_length);
  
    // pripojeni prvniho socketu
    if (connect(mySocket, (sockaddr *)&serverSock, sizeof(serverSock)) == -1)
    {
      cerr << "nelze navaz spojeni" << endl;
      return EXIT_FAILURE;
    }

    //pokud neni nacteny heslo z url pouzije defaultne USER A PASS
    if(adresa_heslo == "")
    {  
      user = "USER anonymous\r\n";
      password = "PASS secret\r\n";
    } 
    //jinak vlozi nacteny heslo a uzivatele z url
    else
    {      
      user = "USER ";
      user.append(adr_user);
      user.append("\r\n");
      
      password = "PASS ";
      password.append(adr_password);
      password.append("\r\n");
    }  
    epsv = "EPSV\r\n";  
    list = "LIST\r\n";

    //zjisteni delky vsech zprav
    int delka_list = list.length();
    int delka_epsv = epsv.length();
    int delka_user = user.length();
    int delka_password = password.length();
  
    //odeslani zpravy USER      
    sent_msg(mySocket, user, delka_user);
    read_server_response(mySocket, buff); 
  
    //odeslani zpravy PASS 
    sent_msg(mySocket, password, delka_password);  
    read_server_response(mySocket, buff);

    //odeslani zpravy EPSV )
    int pom=0; 
    int i;
    int k=0;
   
    //hack na cteni viceradkove odpoved - cte tak dlouho dokud odpoved neobsahuje "|)"
    string end_msg;
    sent_msg(mySocket, epsv, delka_epsv);
   
    while(1)
    { 
      read_server_response(mySocket, buff);    
      size_t found = test.find("|)");    
      if(found != std::string::npos)
	break;
    }   
    msg_index = msg_index - 7;

    //vyparsuje nove cislo portu z prijate odpovedi   
    for(int i=0;i<5;i++)
    { 
	num += test.at(msg_index+i);
    } 
   
    //prevede na int
    new_port = atoi(num.c_str());
   
    struct sockaddr_in serverSock2;     // socket pro server2 - mozna to neni potreba, ale nevim
    
    // Vytvoreni druheho socketu
    if ((mySocket2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
      cerr << "nelze vytvorit socket" << endl;
      return EXIT_FAILURE;
    }
  
    //toto nejspis taky nebude potreba
    if ((host = gethostbyname(adresaServeru.c_str())) == NULL)
    {
      cerr << "spatna adresa" << endl;
      return EXIT_FAILURE;
    }
  
    //zaplneni struktury sockaddr_in pro druhy socket
    serverSock2.sin_family = AF_INET;
    serverSock2.sin_port = htons(new_port);
    memcpy(&(serverSock2.sin_addr), host->h_addr, host->h_length);

    // pripojeni druheho socketu
    if (connect(mySocket2, (sockaddr *)&serverSock2, sizeof(serverSock2)) == -1)
    {
      cerr << "nelze navaz spojeni s druhym socketem" << endl;
      return EXIT_FAILURE;
    }

    //odeslani pozadavku list na starej (prvni) socket    
    if(path.empty())//pokud neexistuje cesta k souboru posli jenom LIST
    {  
      sent_msg(mySocket, list, delka_list);
    }  
    else//jinak do zpravy pridej cestu
    {  
      msg_path.append("LIST ");
      msg_path.append(path);
      msg_path.append("\r\n");
      delka_msg_path = msg_path.length();
      sent_msg(mySocket, msg_path, delka_msg_path);
    }  
    
    test.clear();
    int j = 0;
    int index = 0;
    
    //zase hack pro cteni odpovedi obsahujici vypis adresare
    while(1)
    {
      read_server_response(mySocket2, buff);
      if(isspace(test[j]) && konec2 == 1)
	break;
      else
	konec2 = 0;
      if(test[j] == '\n')
      {	
	konec2 = 1;
      }	
      j++;
      index++;
    }
    
    //smazat posledni prazdnej radek-wtf??
    test = test.erase(test.find_last_not_of(' '));  
    //vypise obsah adresare
    cout << test;
   
    //pouzavirat sockety
    close(mySocket);
    close(mySocket2);
    
    return 0; 
}
