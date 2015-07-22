#predmet: Principy programovacích jazyků a OOP - IPP
#autor: xcasla03@stud.fit.vutbr.cz, Martin Caslava
#projekt: skript na determinizaci konecneho automatu
#prog jazyk: Python 3
#hodnoceni: 9/10


#!/usr/bin/python3
# -*- coding: utf-8 -*-
#DKA:xcasla03

""" zpracovani parametru - OK
    nacitani ze souboru a ze stdin
    rozparsovani konecneho automatu
    ulozeni KA do pameti
    algoritmus pro odstraneni epsilon prechodu
    algoritmus pro determinizaci
            """
import sys
import getopt

validace = 0
#promenne pro vystupy
vystup_no_epsilon = []
vystup_no_determinism = []
vystup_validace = []

#promenna s finalnim vystupem
vystup_final = ''

vstup_stdin = ''
#fronta celkoveho vyctu vsech stavu
Q_state_name = []
#fronta celkoveho vyctu vsech symbolu
Q_symbol_name = []
#fronta pouzitych stavu - odkud
Q_odkud = []
#fronta pouzitych stavu - kam
Q_kam = []
#fronta pouzitych vstupnich symbolu
Q_symbol = []
#fronta koncovych stavu
Q_koncovy_stav = []
#pocatecni stav
Start_state = ''

#fronty pro vysledny automat bez epsilon prechodu
Q_odkud_bez_eps = []
Q_kam_bez_eps = []
Q_symbol_bez_eps = []

#fronty pro vysledny automat bez determinismus
Q_odkud_bez_determ = []
Q_kam_bez_determ = []
Q_symbol_bez_determ = []

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# FUNKCE: TISK NAPOVEDY
# funkce tiskne text napovedy
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def print_help():
  help_text="""
  IPP projekt 2 - determinizace konecneho automatu
  autor: Martin Caslava - xcasla03@stud.fit.vutbr.cz
  -- help : tiskne napovedu programu
  --input=filename : zadany vstupni textovy soubor v UTF-8 s popisem KA
  --output=filename : textovy vystupni soubor v UTF-8 s popisem ekvivalentniho KA
  -e, --no-epsilon-rules : pouhe odstraneni epsilon pravidel, nelze kombinovat s parametrem -d
  -d, --determinization : provede determinizaci KA bez generovani nedostupnych stavu, nelze kombinovat s -e
  -i, --case-insensitive : neni bran ohled na velikost znaku pri provnavani znaku ci stavu"""
  print (help_text)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# FUNKCE: ZPRACOVANI ARGUMENTU
# funkce zpracuje zadane argumenty 
# vraci promenne podle kterych se spousti jednotlive funkce programu
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def Parse_arguments(vstup, output, no_epsilon, determinization, case_insensitiv):
    try:
        opts, args = getopt.getopt(sys.argv[1:], "edi", ["help", "input=", "output=", "no-epsilon-rules", "determinization", "case-insensitive"])
    except (getopt.GetoptError):
        sys.stderr.write ("Error 1: Program arguments problem!")
        sys.exit(1)
        
    for options, param_text in opts:
      #--help
        if options == "--help":
            if no_epsilon!=0 or determinization !=0 or case_insensitiv!=0 or vstup!=None or output!=None:
                sys.stderr.write("Error 1: Program arguments problem!")
                sys.exit(1)
            #tisk napovedy  
            print_help()
            sys.exit(0)
      #-e, --no-epsilon-rules
        elif options in ("-e", "--no-epsilon-rules"):
            if no_epsilon==0:
                no_epsilon=1
            else:
                sys.stderr.write("Error 1: Program arguments problem!")
                sys.exit(1)
      #-d, --determinization
        elif options in ("-d", "--determinization"):
            if determinization==0:
                determinization=1
            else:
                sys.stderr.write("Error 1: Program arguments problem!")
                sys.exit(1)
      #-i, --case-insensitive
        elif options in ("-i", "--case-insensitive"):
            if case_insensitiv==0:
                case_insensitiv=1
            else:
                sys.stderr.write("Error 1: Program arguments problem!")
                sys.exit(1)
      #--input
        elif options == "--input":
            if vstup == None:
                vstup=param_text
            else:
                sys.stderr.write("Error 1: Program arguments problem!")
                sys.exit(1)
      #--output
        elif options == "--output":
            if output == None:
                output=param_text
            else:
                sys.stderr.write("Error 1: Program arguments problem!")
                sys.exit(1)
      #neznamy argument          
        else:
            assert False, "Error 1: Unknown argument!"
 
    if no_epsilon==1 and determinization==1:
        sys.stderr.write("Error 1: Program arguments problem!")
        sys.exit(1)
    if vstup==None:
        vstup="stdin"  
    if output==None:
        output="stdout"
    return (vstup, output, no_epsilon, determinization, case_insensitiv)
  
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# FUNKCE: NACTI KONECNY AUTOMAT
# funkce ktera nacita do pameti konecny automat ze souboru nebo ze stdin
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def Read_machine(vstup):
  
  global Start_state
  global case_insensitiv
  global vystup_final
  global vystup_validace
  global Q_state_name
  global Q_symbol_name
  global Q_koncovy_stav
  vystup_final = ''

  #pocatecni stav
  pocatecni_stav = ''
  #koncovy stav
  koncovy_stav = ''
  #nejaky ty pomocny promenny:D
  current_state = None
  comment = None
  error = None
  pom_zavorka = None
  konec_cteni_symbolu = None 
  konec_cteni_pravidel = None
  start_uvozovka = None
  end_uvozovka = None
  muzu_cist_mezery = None
  symbol_znak = None
  nactena_carka = None
  pom_stav = None
  muzu_cist_carky = None
  iterator = 0  
  iterator2 =0

  pocitadlo_uvozovek = 0
  pocet_carek = 0
  muzu_cist_komentare = 0
  
  vstupni_retezec = ''
  pravidla = ''  
  odkud = ''
  kam = ''
  symbol = ''  
  stav_odkud = ''
  stav_kam = ''
  stav_symbol = ''  
  odkud_kam_name = ''
  state_name = ''#nazev stavu KA
  symbol_name= ''#nazev prechodu  
  symbol_kam = ''
  odkud_name = ''
  kam_name = ''
  symbol_name = ''
  zdvojeni = ''#pomocny retezec pro zdvojeni 
  char = ''
  pred = ''
  
  #typy zavorek
  oteviraci_kul_zavorka = None#pocatecni kulata zavorka
  uzaviraci_kul_zavorka = None#koncova kulata zavorka
  oteviraci_sloz_zavorka = None#pocatecni zobackova zavorka
  uzaviraci_sloz_zavorka = None#uzaviraci zobackova zavorka
  
  #cteni ze souboru nebo ze stdin
  if vstup: 
	  
      if vstup != "stdin":
        #otevre soubor
        file = open(vstup)
        while 1:
          line = file.readline()
          if not line:break
          vstupni_retezec += line
      else:
        vstup_stdin = input()    
      current_state = "STAVY"#prvni stav
    
      while 1:
          #nacte jeden znak ze souboru
          if vstup != "stdin":
            if iterator2 < len(vstupni_retezec):
              char = vstupni_retezec[iterator2]
            if case_insensitiv == 1:
              vstupni_retezec = vstupni_retezec.lower()
            iterator2 += 1
          else:
            if iterator < len(vstupni_retezec):
              char = vstupni_retezec[iterator]
            iterator += 1
          
          #pokud se nejedna o znak konec cyklu
          #konec pro cteni ze souboru
          if vstup != "stdin":
            if iterator2 == (len(vstupni_retezec)+1):
              break 
          #konec pro cteni ze stdin
          else:
             if iterator == (len(vstupni_retezec)+1):
              break
       
          #zjisti jestli neni pred krizkem uvozovka, jedinej zpusob co me napadl jak cist krizky
          if char == '#' and pred == 1:
            muzu_cist_komentare = 1
            pred = ''
          else:
            pred = ''

          if char == '\'' and pred == '':
            pred = 1
          #Mechanismus na preskakovani komentaru
          if char == '\n' and comment == 1:
              comment = None	       
          
          if char == '#' and comment == None and muzu_cist_komentare == 0:
              comment = 1      
              
          #pokud se nepreskakuje komentar
          if comment == None:
              muzu_cist_komentare = 0
#stavova masina na parsovani vstupnihou souboru a ulozeni KA do pameti
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#1) NACITANI STAVU   
#   - nacita nazvy stavu a po jednom je uklada do fronty Q_state_name
#   - vysledky uklada paralelne do tri front predstavujici trojrozmerne pole
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
              if current_state == "STAVY":
                  muzu_ulozit_stav = 1
                  #preskakuje bile znaky
                  if not char.isspace():         
                      #kontrola zda nebyla nactena ukoncovaci slozena zavorka pred oteviraci
                      if char == '}' and oteviraci_sloz_zavorka == None:
                          sys.stderr.write("Error 40: File format problem!")
                          sys.exit(40)           
                      #pokud neni nactena kulata zavorka a nacteny znak == (
                      if oteviraci_kul_zavorka == None and char == '(':
                          error = 1
                          oteviraci_kul_zavorka = 1           
                      #pokud byla nactena oteviracu kul zavorka, oteviraci sloz zavorka nactena nebyla a znak == {  
                      elif oteviraci_kul_zavorka == 1 and oteviraci_sloz_zavorka == None and char == '{':
                          error = 1
                          oteviraci_sloz_zavorka = 1           
                      #byly nacteny obe oteviraci zavorky a nacitej nazvy stavu  
                      elif oteviraci_kul_zavorka == 1 and oteviraci_sloz_zavorka == 1:
                      #nacita nazev stavu
                          error = 1
                          #pokud neni nactena uzaviraci sloz zavorka nebo carka
                          if char != '}' and char != ',':
       	                       #uloz nazev
                               state_name += char
                          else:
       	                       error = 1
       	                       #skontroluje zda se nahodou nejedna o duplicitni stav
                               for state in Q_state_name:
                                if state == state_name:
                                  muzu_ulozit_stav = 0
                               #pokud stav neni duplicitni tak ho ulozi
                               if muzu_ulozit_stav == 1:
                                Q_state_name.append(state_name)
       	                       #vymaz nazev stavu
       	                       state_name = ''
       	                       #pokud byla nactena uzaviraci slozena zavorka
       	                       if char == '}':
       	                           uzaviraci_sloz_zavorka = 1
       	                           #zmen stav
       	                           current_state = "SYMBOLY"
       	                           oteviraci_sloz_zavorka = None
       	                           uzaviraci_sloz_zavorka = None         
                      #defaultni error
                      elif error == None:
                          sys.stderr.write("Error 40: File format problem!4")
                          sys.exit(40)  
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#2) NACITANI VSTUPNICH SYMBOLU
# - nacte svechny vstupni symboly v jednom retezci
# - retezec pak parsuje na jednotlive symboly a po jednom je vklada do fronty Q_symbol_name
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                 
              if current_state == "SYMBOLY":
                  #zacatek cteni prechodu
                  if pom_zavorka != None:
                  
                      if not char.isspace() or konec_cteni_symbolu == None or muzu_cist_mezery == None:
                          
                          if not char.isspace():
                            if char == ',' and uzaviraci_sloz_zavorka == 1 and konec_cteni_symbolu == None:
                              konec_cteni_symbolu = 1 

                          else:
                             uzaviraci_sloz_zavorka = None
                          
                          #nacitam znaky prechodu
                          if oteviraci_sloz_zavorka == 1 and konec_cteni_symbolu == None:
                              symbol_name += char
                          
                          #zacatek cteni
                          if char == '{' and oteviraci_sloz_zavorka == None:
                             oteviraci_sloz_zavorka = 1
                             
                          if char == '}' and uzaviraci_sloz_zavorka == None:
                             uzaviraci_sloz_zavorka = 1
                       
                      #konec cteni prechodu 
                      if konec_cteni_symbolu == 1:
                          current_state = "PRAVIDLA"
                          symbol_name = symbol_name[:-1]
                          oteviraci_sloz_zavorka = None
                          uzaviraci_sloz_zavorka = None
                          start_uvozovka = None
                          end_uvozovka = None
                          muzu_cist_mezery = None
                          
                          for c in symbol_name:
                              #zacatek parsovani jednotlivych prechodu
                              if not c.isspace() or muzu_cist_mezery == 1:                                
                                  
                                  #pocita zdvojene uvozovky
                                  if pocitadlo_uvozovek == 2:
                                      pocitadlo_uvozovek = 0
                                      Q_symbol_name.append(zdvojeni)
                                      zdvojeni = ''
                                  
                                  #pokud znak neni uvozovka a predchozi znak byl povoleny znak pro symbol -> chyba! symbolmuze obsahovat pouze jeden uvozovek
                                  if c != '\'' and symbol_znak != None:
                                      sys.stderr.write("Error 40: File format problem!4")
                                      sys.exit(40)                                      
                                      
                                  #pokud nacte druhou koncovou uvozovku
                                  if c == '\'' and start_uvozovka == 1 and end_uvozovka == None:
                                      end_uvozovka = 1
                                      muzu_cist_mezery = None
                                      start_uvozovka = None
                                      end_uvozovka = None
                                      zdvojeni += c
                                      pocitadlo_uvozovek = pocitadlo_uvozovek + 1
                                      c= ''
                                      prechod_znak = None
                                  
                                  #nacte cokoliv krome uvozovky
                                  if start_uvozovka == 1 and end_uvozovka == None:
                                      pocitadlo_uvozovek = 0
                                      Q_symbol_name.append(c)
                                      prechod_znak = c
                                      zdvojeni = ''
                                  
                                  #nactena prvni uvozovka
                                  if c == '\'' and start_uvozovka == None:
                                      start_uvozovka = 1
                                      muzu_cist_mezery = 1
                  pom_zavorka = 1            
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#3) NACITANI PRAVIDEL
# - nacita pravidla ve tvaru ODKUD - SYMBOL - KAM
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
              if current_state == "PRAVIDLA":
                  if char == ',' and uzaviraci_sloz_zavorka == 1:
                      konec_cteni_pravidel = 1
                      current_state = "POCATECNI STAV"
                  else:                      
                      uzaviraci_sloz_zavorka = None    
                      
                  if char == '{' and oteviraci_sloz_zavorka == None:
                      oteviraci_sloz_zavorka = 1
                      char = ''
                  
                  if char == '}' and uzaviraci_sloz_zavorka == None and start_uvozovka != 1:    
                      uzaviraci_sloz_zavorka = 1   
                  
                  #dokud se nenacte prvni znak preskakuj mezery
                  if oteviraci_sloz_zavorka == 1 and konec_cteni_pravidel == None:
                      if not char.isspace() or muzu_cist_carky!= None:
                          muzu_cist_mezery = 1
                          
                          if char == ',' and start_uvozovka == 1:
                              odkud_kam_name += char                         
                              char = ''                     
                      				           
                          if char == ',' or char == '}' and start_uvozovka != 1:
                              #nactene jedno cele pravidlo, nutne rozdelit na (odkud - symbol - kam)
                              odkud_kam_name = odkud_kam_name.strip()  
                              muzu_cist_mezery = None   
                              start_uvozovka = None
                              end_uvozovka = None  
                                                        
                              pom_stav = None
                              stav_symbol = ''
                              stav_odkud = ''
                              
                              if stav_symbol != '' and stav_odkud != '':
                                  Q_odkud.append(stav_odkud)
                                  Q_symbol.append(stav_symbol)
                              if pravidla != '':
                                  odkud,kam = pravidla.split("->")
                                  for x in odkud:
                                      if x == '\'':
                                          pom_stav = 1
                                      if pom_stav != 1:    
                                          stav_odkud += x
                                      else:
                                          stav_symbol += x  
                                  Q_kam.append(kam)
                                  pravidla = ''
                              for c in odkud_kam_name:
                                  if not c.isspace() or muzu_cist_mezery != None:
                                      pravidla += c
                                      
                                      if c == '\'' and end_uvozovka == None and start_uvozovka == 1:
                                          end_uvozovka = 1
                                          muzu_cist_mezery = None
                                          start_uvozovka = None
                                          c = ''
                                      
                                      if c == '\'' and start_uvozovka == None:
                                          start_uvozovka = 1        
                                          muzu_cist_mezery = 1   
                              if pravidla != '':
                                odkud,kam = pravidla.split("->") 
                              for x in odkud:
                                  if x == '\'':
                                      pom_stav = 1
                                  if pom_stav != 1:    
                                      stav_odkud += x
                                  else:
                                      stav_symbol += x      
                              Q_kam.append(kam)
                              Q_odkud.append(stav_odkud)
                              Q_symbol.append(stav_symbol)   
                              pravidla = ''   
                                                
                              odkud_kam_name = ''
                              muzu_cist_mezery = None
                          else:    
                              start_uvozovka = None
                              odkud_kam_name += char
                                                                                      				      
                          if char == '\'' and start_uvozovka == None:
                              start_uvozovka = 1 
                              muzu_cist_carky = 1				                                                                                     
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#4) NACITANI POCATECNIHO STAVU
# - funkce nacte pocatecni stav do promenne pocatecni_stav
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
              if current_state == "POCATECNI STAV":
                  if pocet_carek < 2:
                      if char == ',':   
                          pocet_carek = pocet_carek + 1              
                      if not char.isspace() and char != ',':
                          pocatecni_stav += char
                  else:
                      Start_state = pocatecni_stav
                      current_state = "KONCOVY STAV" 
                      oteviraci_sloz_zavorka = None
                      uzaviraci_sloz_zavorka = None 
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#5) NACITANI KONCOVEHO STAVU
# - nacte vsechny koncove stavy do fronty Q_koncovy_stav
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
              if current_state == "KONCOVY STAV":
                  if char == '{' and oteviraci_sloz_zavorka == None:
                      oteviraci_sloz_zavorka = 1
                  if char == '}' and oteviraci_sloz_zavorka == 1 and uzaviraci_sloz_zavorka == None:   
                      uzaviraci_sloz_zavorka = 1
                  
                  if uzaviraci_sloz_zavorka == 1:
                      current_state = "KONEC CTENI"
                      if koncovy_stav != '':
                          Q_koncovy_stav.append(koncovy_stav)                         
                  
                  if oteviraci_sloz_zavorka == 1 and uzaviraci_sloz_zavorka == None:    
                      if not char.isspace() and char != ',':
                          if char != '{':
                              koncovy_stav += char
                      else:
                          if koncovy_stav != '':
                              Q_koncovy_stav.append(koncovy_stav)
                          koncovy_stav = ''       
#~~~~~~~~~~~~~~~~~~~~~~~
#5) KONEC CTENI
#~~~~~~~~~~~~~~~~~~~~~~
              if current_state == "KONEC CTENI":

                  if uzaviraci_kul_zavorka == 1:
                    if not char.isspace():
                      sys.stderr.write("Error 40: File format problem!")
                      sys.exit(40) 

                  if char == ')':
                      uzaviraci_kul_zavorka = 1
      
      if vstup != "stdin":
        #zavre soubor  
        file.close()
      
      #kontrola zda byla nactena uzaviraci kulata zavorka
      if uzaviraci_kul_zavorka != 1:
        sys.stderr.write("Error 40: File format problem!")
        sys.exit(40)  

      #overeni semantickych chyb
      #1) pokud je vstupni abeceda prazdna
      if not Q_symbol_name:
          sys.stderr.write("Error 41: Program semantic semantic problem!")
          sys.exit(41)
      
      exist = 0
      
      if Q_odkud[0] != '':
        #2) overeni zda symbol se vyskytuje ve vstupni abecede
        for s in Q_symbol:
          s = s.strip()
          exist = 0
          #pokud se nejedena o epsilon
          if s != '\'\'':
            for s2 in Q_symbol_name:
              s2 = '\'' + s2 + '\''
              if s == s2:
                exist = 1
            if exist == 0:
              sys.stderr.write("Error 41: Program semantic semantic problem!")
              sys.exit(41)
      
      exist = 0

      if Q_odkud[0] != '':
        #3) overeni zda pouzite stavy se vyskytuji v mnozine vsech stavu
        for o in Q_odkud:
            exist = 0
          #pokud se nejedena o epsilon
            for o2 in Q_state_name:
              if o == o2:
                exist = 1
            if exist == 0:
              sys.stderr.write("Error 41: Program semantic semantic problem!")
              sys.exit(41)

      exist = 0
      #4) overeni zda koncove stavy se vyskytuji v mnozine vsech stavu
      for f in Q_koncovy_stav:
          exist = 0
        #pokud se nejedena o epsilon
          for f2 in Q_state_name:
            if f == f2:
              exist = 1
          if exist == 0:
            sys.stderr.write("Error 41: Program semantic semantic problem!")
            sys.exit(41)

      exist = 0
      #5) overeni zda pocatecni stav se vyskytuje v mnozine vsech stavu
      for s in Q_state_name:
        if s == pocatecni_stav:
              exist = 1
      if exist == 0:
        sys.stderr.write("Error 41: Program semantic semantic problem!")
        sys.exit(41)

#pomocne kontrolni vypisy          
      sipka = '->'
      i = 0

      #pokud se ma jen validovat
      i = 0
      end = ''
      x = len(Q_odkud)
      while i < x:
        vystup_validace.append(Q_odkud[i] + ' ' + Q_symbol[i] + ' ' + sipka + ' ' + Q_kam[i] + ',')
        i += 1
      vystup_validace.sort()

      no_duplicates = ''
      no_duplicates = set(vystup_validace)
      vystup_validace = list(no_duplicates)
      vystup_validace.sort()


      no_duplicates = ''
      no_duplicates = set(Q_koncovy_stav)
      Q_koncovy_stav = list(no_duplicates)
      Q_koncovy_stav.sort()

      no_duplicates = ''
      no_duplicates = set(Q_state_name)
      Q_state_name = list(no_duplicates)
      Q_state_name.sort()

      no_duplicates = ''
      no_duplicates = set(Q_symbol_name)
      Q_symbol_name = list(no_duplicates)
      Q_symbol_name.sort()

      #vytvoreni vystupu
      vystup_final += '(' + '\n'
      vystup_final += '{'
      for st in Q_state_name:
        vystup_final += st + ',' + ' '
      vystup_final = vystup_final[:-2]
      vystup_final += '}' + ',' + '\n'

      vystup_final += '{'
      for st in Q_symbol_name:
        vystup_final += '\'' + st + '\'' + ',' + ' '
      vystup_final = vystup_final[:-2]
      vystup_final += '}' + ',' + '\n'

      vystup_final += '{' + '\n'
      #pokud je prazdna fronta pravidel
      if Q_odkud[0] == '':
        vystup_final += '}' + ',' + '\n'
      else:
        #vlozi zvalidovane stavy
        for vystup in vystup_validace:
          vystup_final += vystup + '\n'
        vystup_final = vystup_final[:-2] 
        vystup_final += '\n' + '}' + ',' + '\n'

      vystup_final += Start_state + ',' + '\n'  
      vystup_final += '{'

      #pokud jsou koncove stavy prazdne
      if not Q_koncovy_stav:
        vystup_final += '}' + '\n'
      else:
        for st in Q_koncovy_stav:
          vystup_final += st + ',' + ' '
        vystup_final = vystup_final[:-2]
        vystup_final += '}' + '\n'

      vystup_final += ')'
      


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# FUNKCE NA ODSTRANENI EPSILON PRECHODU
# funkce odstrani epsilon prechody a vysledny automat ulozi do front Q_bez_eps
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def Remove_epsilon ():

  #pomocna fronta na vkladani koncovych stavu u epsilon prechodu
  Q_epsilon = []
  
  Q_pom_eps = ''
  global vystup_final
  global vystup_no_epsilon
  vystup_final = ''

  x = len(Q_odkud)
  #pomocne kontrolni vypisy          
  sipka = '->'
  hledany_stav = ''

  #overit zda se dane stavy a symboly nachazi v celkovych vyctech
  #cyklus pres vsechny stavy
  for stav in Q_state_name:
    i = 0
    hledany_stav = stav
    while i < x:
      #pokud ctene pravidlo ma jako pocatecni stav cteny stav
      if Q_odkud[i].startswith(hledany_stav):

        #pokud pravidlo obsahuje epsilon prechod
        if Q_symbol[i] == '\'\'':
          #ulozi do specialni fronty koncovy stav pravidla s epsilon prechodem
          Q_epsilon.append(Q_kam[i])

          #ulozi stav ze ktereho vychazi epsilon prechod
          Q_pom_eps = Q_odkud[i]
          Q_pom_eps2 = ''
          druha_shoda = 0
          Q_pom_eps3 = []
          x = 0
          y = len(Q_odkud)      

          #logika ktera resi vicenasobne epsilon prechody
          while x < y:
            if Q_odkud[x] == Q_pom_eps and Q_symbol[x] == '\'\'':
              Q_pom_eps2 = Q_kam[x]
            x += 1
          
          x = 0
          y = len(Q_odkud)
          while x < y:
            if Q_odkud[x] == Q_pom_eps2 and Q_symbol[x] == '\'\'':
              druha_shoda = 1
              Q_pom_eps2 = Q_kam[x]
              x = -1
            if Q_odkud[x] == Q_pom_eps2 and Q_symbol[x] != '\'\'':
              if druha_shoda == 1:
                Q_pom_eps3.append(Q_pom_eps + Q_symbol[x] + sipka +Q_kam[x])
            x += 1 

          no_duplicates = ''
          no_duplicates = set(Q_pom_eps3)
          Q_pom_eps3 = list(no_duplicates)
          odkud = ''
          symb = ''
          kam = ''
          for pom in Q_pom_eps3:
            pom = pom.split('->')
            kam = pom[1]
            od = pom[0].split('\'')
            odkud = od[0]
            symb = '\'' + od[1] + '\''
      
            Q_odkud_bez_eps.append(odkud)
            Q_symbol_bez_eps.append(symb)
            Q_kam_bez_eps.append(kam)           


        else:
          Q_odkud_bez_eps.append(Q_odkud[i])
          Q_kam_bez_eps.append(Q_kam[i])
          Q_symbol_bez_eps.append(Q_symbol[i])
      i = i+1

    #pokud neni specialni fronta pro epsilon prechody prazdna projdou se pravidla znovu ale tentokrat s úrvkama frotny Q_epsilon
    if Q_epsilon:
      for epsilon in Q_epsilon:
        i = 0
        hledany_stav = epsilon

        while i < x:
          #pokud se pravidlo shoduje
          if Q_odkud[i].startswith(hledany_stav):
            if Q_symbol[i] != '\'\'':
              Q_odkud_bez_eps.append(stav)
              Q_kam_bez_eps.append(Q_kam[i])
              Q_symbol_bez_eps.append(Q_symbol[i])
          i = i+1 
    del Q_epsilon[:]
  x = len(Q_odkud_bez_eps)
  i = 0

  while i < x:
    vystup_no_epsilon.append(Q_odkud_bez_eps[i] + ' ' + Q_symbol_bez_eps[i] + ' ' + sipka + ' ' + Q_kam_bez_eps[i] + ',')
    i += 1

  no_duplicates = ''
  no_duplicates = set(vystup_no_epsilon)
  vystup_no_epsilon = list(no_duplicates)
  vystup_no_epsilon.sort()

  Q_state_name.sort()
  Q_symbol_name.sort()

  #vytvoreni vystupu
  vystup_final += '(' + '\n'
  vystup_final += '{'
  for st in Q_state_name:
    vystup_final += st + ',' + ' '
  vystup_final = vystup_final[:-2]
  vystup_final += '}' + ',' + '\n'

  vystup_final += '{'
  for st in Q_symbol_name:
    vystup_final += '\'' + st + '\'' + ',' + ' '
  vystup_final = vystup_final[:-2]
  vystup_final += '}' + ',' + '\n'

  vystup_final += '{' + '\n'
  #pokud je prazdna fronta pravidel
  if Q_odkud[0] == '':
    vystup_final += '}' + ',' + '\n'
  else:
    #vlozi stavy bez epsilon prechodu
    for vystup in vystup_no_epsilon:
      vystup_final += vystup + '\n'
    vystup_final = vystup_final[:-2]
    vystup_final += '\n' + '}' + ',' + '\n'

  vystup_final += Start_state + ',' + '\n'  
  vystup_final += '{'

  #pokud jsou koncove stavy prazdne
  if not Q_koncovy_stav:
    vystup_final += '}' + '\n'
  else:
    for st in Q_koncovy_stav:
      vystup_final += st + ',' + ' '
    vystup_final = vystup_final[:-2]
    vystup_final += '}' + '\n'

  vystup_final += ')'

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# FUNKCE NA ODSTRANENI NETERMINISMU
# funkce ktera odstrani nedeterministicke stavy
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def Remove_determinism ():

  global Start_state
  global vystup_final
  global Q_state_name
  global Q_symbol_name
  Q_spojeni_pom = []

  Q_pom1 = []  

  sipka = '->'
  pocet_vyskytu = 0
  stav_existuje = 0
  Q_spojeni = []
  rozdelene_spojeni = ''
  pom_kam = ''

  #fronta vsech stavu ve ktere bude prvni stav ten ktery je pocatecni
  Q_state_name_start_firts = []

  #vlozi pocatecni stav na prvni misto aby se mi lip pracovalo
  for st in Q_state_name:
    if st == Start_state:
      Q_state_name_start_firts.append(st)

  for st in Q_state_name:
    if st != Start_state:
      Q_state_name_start_firts.append(st)

  i = 0
  x = len(Q_odkud_bez_eps)
  je_koncovy = 0

  for st in Q_state_name_start_firts:
    i = 0
    x  = len(Q_odkud_bez_eps)
    pocitadlo = 0 
    je_koncovy = 0
    
    for k in Q_koncovy_stav:
      if st == k:
        je_koncovy = 1
        

    if st == Start_state or je_koncovy:
      je_koncovy = 0
      for sym in Q_symbol_name:
        i = 0
        pocet_vyskytu = 0
        del Q_spojeni[:]
        pom_kam = ''
        sym = '\'' + sym + '\''
        while i < x:
        
          #1) z pocatecniho stavu ihned utvori vazby a spojene stavy
          if Q_symbol_bez_eps[i] == sym and Q_odkud_bez_eps[i].startswith(st):
            pocet_vyskytu += 1
            pom_kam = Q_kam_bez_eps[i]
            Q_spojeni.append(Q_kam_bez_eps[i])
          i += 1    
        if pocet_vyskytu < 2:
      
          if st != '' and sym != '' and pom_kam != '':
            #ulozi stav jak je
            Q_odkud_bez_determ.append(st)
            Q_symbol_bez_determ.append(sym)
            Q_kam_bez_determ.append(pom_kam)
        else:
          Q_spojeni.sort()
          spojeny_stav = ''
          for spoj in Q_spojeni:
            spojeny_stav += spoj + '_'
          spojeny_stav = spojeny_stav[:-1]
          
          #ulozi spojeny stav
          if st != '' and sym != '' and spojeny_stav != '':
            Q_odkud_bez_determ.append(st)
            Q_symbol_bez_determ.append(sym)
            Q_kam_bez_determ.append(spojeny_stav)

  #bude overovat nove vznikle stavy
  for st in Q_kam_bez_determ:
    stav_existuje = 0
    for st2 in Q_state_name:  
      if st == st2:
        stav_existuje = 1
        #if st != Start_state
    #pokud takto novy stav jeste neni uloz v celkovem vyctu stavu
    if stav_existuje == 0:
      #prida stav
      Q_state_name.append(st)

      #rozgenerovani nove vznikleho stavu a overeni jeho prechodu
      rozdelene_spojeni = st.split('_')
      
      #overim kam vedou jednotlive dilci stavy ze spojeni
      del Q_spojeni[:]
      x = len(Q_odkud_bez_eps)

      for sym in Q_symbol_name:
        i = 0

        del Q_spojeni_pom[:]
        sym = '\'' + sym + '\''
        for rozdeleni in rozdelene_spojeni:
          i = 0
          pocet_vyskytu = 0
          while i < x:
            #pokud najde shodu
            if Q_odkud_bez_eps[i] == rozdeleni and Q_symbol_bez_eps[i] == sym:

              stav_existuje = 0
              for s in Q_spojeni_pom:
                if s == Q_kam_bez_eps[i]:
                  stav_existuje = 1
              if stav_existuje == 0:
                Q_spojeni_pom.append(Q_kam_bez_eps[i])
            i += 1
            pocet_vyskytu += 1

        Q_spojeni_pom.sort()
        spojeny_stav = ''
        for s in Q_spojeni_pom:
          spojeny_stav += s + '_'
        spojeny_stav = spojeny_stav[:-1]


        if st != '' and spojeny_stav != '' and sym != '':
          Q_odkud_bez_determ.append(st)
          Q_kam_bez_determ.append(spojeny_stav)
          Q_symbol_bez_determ.append(sym)

  i = 0
  x = len(Q_odkud_bez_determ)
  y = len (Q_koncovy_stav)
  koncovy = ''
  result = 0
  pocet_zdvojeni = 0
  neukladat_zdvojeni = 0

  while i < x:
    if i < y:
      koncovy = Q_koncovy_stav[i]
    if Q_odkud_bez_determ[i] == koncovy:
      result = Q_kam_bez_determ[i].find('_')
      if result != -1:
        pocet_zdvojeni += 1
    i += 1

  del Q_state_name[:]
  Q_koncovy_stav_novy = []
  
  i = 0
  result = 0

  #vytvori nove kmoncove stavy
  for st in Q_koncovy_stav:
    while i < x:
      if Q_odkud_bez_determ[i] == koncovy and pocet_zdvojeni >= 2:
        neukladat_zdvojeni = 1
      if neukladat_zdvojeni != 1:

        result = Q_odkud_bez_determ[i].find(st)
        if result != -1:
          Q_koncovy_stav_novy.append(Q_odkud_bez_determ[i])
        result = 0

        vystup_no_determinism.append(Q_odkud_bez_determ[i] + ' ' + Q_symbol_bez_determ[i] + ' ' + sipka + ' ' + Q_kam_bez_determ[i] + ',')
        Q_state_name.append(Q_odkud_bez_determ[i])
        Q_state_name.append(Q_kam_bez_determ[i])
      neukladat_zdvojeni = 0
      i += 1  

  no_duplicates = set(Q_state_name)
  Q_state_name = list(no_duplicates)
  Q_state_name.sort()

  no_duplicates = set(Q_koncovy_stav_novy)
  Q_koncovy_stav_novy = list(no_duplicates)
  Q_koncovy_stav_novy.sort()

  vystup_no_determinism.sort() 


  if Q_odkud[0] != '':
    vystup_final = ''

    #vytvoreni vystupu
    vystup_final = '(' + '\n'
    vystup_final += '{'
    for st in Q_state_name:
      vystup_final += st + ',' + ' '
    vystup_final = vystup_final[:-2]
    vystup_final += '}' + ',' + '\n'
  
    vystup_final += '{'
    for st in Q_symbol_name:
      vystup_final += '\'' + st + '\'' + ',' + ' '
    vystup_final = vystup_final[:-2]
    vystup_final += '}' + ',' + '\n'
    vystup_final += '{' + '\n'

    #vlozi zdeterminizovana pravidla
    for vystup in vystup_no_determinism:
      vystup_final += vystup + '\n'
    vystup_final = vystup_final[:-2]

    vystup_final += '\n' + '}' + ',' + '\n'
    vystup_final += Start_state + ',' + '\n'
    vystup_final += '{'

    for st in Q_koncovy_stav_novy:
      vystup_final += st + ',' + ' '
    vystup_final = vystup_final[:-2]

    vystup_final += '}' + '\n'
    vystup_final += ')'
    
##~~~~~~~~~~~~~~~
# HLAVNI PROGRAM
#~~~~~~~~~~~~~~~~
if __name__ == "__main__":
  #ulozi do promennych hodnoty podle nactenych argumentu
  (vstup, output, no_epsilon, determinization, case_insensitiv)=Parse_arguments(vstup=None, output=None, no_epsilon=0, determinization=0, case_insensitiv=0) 
  #provede odstraneni epsilon prechodu

  if no_epsilon == 1:
    Read_machine(vstup)
    Remove_epsilon()
    if output != 'stdout':
      file = open(output,'w')
      file.write(vystup_final)
    else:
      print (vystup_final)
    sys.exit(0)

  #provede determinizaci
  if determinization == 1:
    Read_machine(vstup)
    Remove_epsilon()
    Remove_determinism()
    if output != 'stdout':
      file = open(output,'w')
      file.write(vystup_final)
    else:
      print (vystup_final)
    sys.exit(0)
  
  if no_epsilon !=1 and determinization != 1:
    Read_machine(vstup)
    if output != 'stdout':
      file = open(output,'w')
      file.write(vystup_final)
    else:
      print (vystup_final)
    sys.exit(0)
