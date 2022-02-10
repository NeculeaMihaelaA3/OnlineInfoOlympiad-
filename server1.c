#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <time.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

/* portul folosit */
#define PORT 2024

char* sql;
char *err_msg = 0;
int ok;
/* codul de eroare returnat de anumite apeluri */
extern int errno;
int nr_probleme = 4, lower = 1, number;
char cerinta[400] = "";
char sir[400] = "gata\n";
char rezolvare[1024];
char fis[100] = "rezolvare";
int ordine;
int cordine; //copie la ordine
char exe[100] = "";  int l=0; //pt executabil
int pipes[2]; //pt comunicare
char buff[400] = ""; //pt pipe 
int punctaj;
char in[100] = "";
char bufi[400] = "";
char liniuta[400] = ""; 
char lin[400] = "";
double elapsed1, timpd;
int poti_participa;
char *my_args2[5];  //argumentele pentru apelul exectabilului
char copie_input[400];
FILE *p;
int nota;
char copie_fis[100] = "";
char copie_sir[100] = "";
char nota_sir[100] = ""; int len=0;//pentru nota
char notare_fis[100] = "";
char copie_rezolvare[1024] = "";
int nr_linii; //nr linii din rezolvare
time_t start, end;  //timpul pentru verificarea vitezei de executiei
double elapsed;
char  in_fis[100];

int nrlinii(char rez[400])
{
    int nr = 0;
    for(int i=0; i <strlen(rez); i++)
        if(rez[i] == '\n')
           nr++;
    return nr;
}

void nr_max(char x[100])
{
    char y[100] = "";
    char rasp[30] = ""; int l=0;
    strcpy(y, x); int nr; 
    for(int i=0; i<strlen(y); i++)
    {  
       if(y[i] >= '0' && y[i]<='9')
       {
           rasp[l++] = y[i];
       }
    }
    strcpy(x, rasp);
}

void argumente(char input[400])
{
    strcpy(my_args2[0], exe);
    char *p;
    int l = 1;
    p = strtok(input, " "); 
    printf("input: %s \n ", input);
    while(p)
    {
        printf("intra aici in argumente\n");
        strcpy(my_args2[l], p);
        l++;
        p = strtok(NULL, " ");
    }
    my_args2[l] = NULL;

    //hai sa afisam argumentele
    for(int i=0;i<l;i++)
       printf("%s ", my_args2[i]);
    printf("\n");
}

int main ()
{
    srand(time(0)); //pentru alegerea random a numarului problemei
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    char msg[100];		//mesajul primit de la client
    char msgrasp[100]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }

    // servim in mod concurent clientii
    while (1)
    {
		ordine++; //ordinea clientului
        cordine++;
    	int client;
    	int length = sizeof (from);

    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);

    	// acceptam un client (stare blocanta pina la realizarea conexiunii) 
    	client = accept(sd, (struct sockaddr *) &from, &length);

    	// eroare la acceptarea conexiunii de la un client 
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}

    	int pid;
    	if ((pid = fork()) == -1) 
        {
    		close(client);
    		continue;
    	} 
        else if (pid > 0) //tatal
        { 
    		close(client);
    		while(waitpid(-1,NULL,WNOHANG));
    		continue;
    	} else if (pid == 0) { //child
    		close(sd);
    		// s-a realizat conexiunea, se astepta mesajul */
    		bzero (msg, 100);
    		//fflush (stdout);

            //alegem o problema random
            srand(time(0)); // pentru rand
            number = (rand() % (nr_probleme - lower +1)) + lower;
            //printf("%d\n", number);
    		//citim din fisier problema respectiva

            //deschidem fis cu informatii pentru a lua nr max de persoane
            FILE *h;
            if((h=fopen("informatii.txt", "r")) < 0)
            { printf("Eroare la deschiderea fisierului cu informatii\n"); }
            char prima_lin_info[100] = "";
            fgets(prima_lin_info, 400, h); //luam prima linie din fis cu info
            // printf("prima linie: %s\n", prima_lin_info);

            nr_max(prima_lin_info);
            printf("Nr max de persoane care pot participa este: %s \n", prima_lin_info);
            int nr_persoane = atoi(prima_lin_info);
            // printf("ca numar %d \n", nr_persoane);
            fclose(h);


            if(ordine <= nr_persoane) 
            {
                poti_participa = 1; //ii scriem clientului daca poate participa
                if(write (client, &poti_participa, sizeof(poti_participa)) <= 0)
                {
                    perror ("[server]Eroare la write() catre client cerina.\n");
                    continue; // continuam sa ascultam 
                }

                //ii scriem clientului nr sau de ordine
                if(write (client, &ordine, sizeof(ordine)) <= 0)
                {
                    perror ("[server]Eroare la write() catre client cerina.\n");
                    continue; // continuam sa ascultam 
                }
                 
                //deschidem fisierul cu probleme ca sa luam problema cu nr de ordine ales random
                FILE* t;
                char line[400];
                t = fopen("probleme.txt", "r"); //deschidem fisierul cu probleme
                while(fgets(line, 400, t)) //luam cate o linie din fisier
                {
                    if((line[0]-'0') == number)  //daca dam de numarul problemei alese
                    {
                        strcpy(cerinta, line);
                    } 
                }
                fclose(t);

                printf("Cerinta ce trebuie trimisa este:\n%s\n", cerinta); //afisam cerinta

                char s[100] = ""; int lun=0;
                int copie_number = number;

                //transformare number din string in char, adica numarul problemei
                while(copie_number)
                {
                    lun++;
                    for(int i=lun; i >= 0; i--)
                        s[i] = s[i-1];
                    s[0] = copie_number%10 + '0';
                    copie_number = copie_number/10;
                }   

                strcpy(in,"input");
                strcat(in, s);
                strcat(in, ".txt");   //numele fis de input
                strcpy(in_fis, in);
                //deschidere fisier de input

                FILE* deschid;
                if((deschid=fopen(in, "r")) < 0)
                { printf("Eroare la deschiderea fisierului cu input ul\n"); }
                
                fgets(liniuta, 400, deschid);
                strcpy(copie_input, liniuta); //luam inputul
                fclose(deschid);

                char out[100] = "out";
                strcat(out, s);
                strcat(out,".txt");
                //printf("fisierul de ouput si input ar trb sa fie: %s %s\n", out, in);

                //pregatim mesajul de raspuns 
                bzero(msgrasp,100);
                strcat(msgrasp,msg);

                //printf("[server]Trimitem cerinta problemei inapoi...%s\n", msgrasp);
                
                FILE *q;
                //char lin[400] = ""; 
                int number = 3;
                int timp = 0;
                int nr_linii_necesare = 0;

                if((q=fopen("informatii.txt", "r")) < 0)
                { printf("Eroare la deschiderea fisierului cu informatii\n"); }
                
                fgets(lin, 400, q);
                fgets(lin, 400, q);  //luam al doilea rand din fisier pt a trimite modalitatea de notare
                //printf("NOTARE: %s\n", lin);

                //ii scriem clientului informatiile despre notare
                if(write(client, lin, 400) <= 0)
                {
                    perror ("[server]Eroare la write() catre client informati notare\n");
                    continue;		/// continuam sa ascultam 
                }

                while(fgets(lin, 400, q)) //luam cate o linie din fisier
                {
                    if((lin[0]-'0') == number) //daca dam de numarul problemei alese
                    {
                        for(int i=3; i < strlen(lin)-1 && lin[i] != ','; i++)
                        {
                            
                            timp = timp*10 + (lin[i] - '0'); //luam timpul
                        }

                        for(int k=0; k<strlen(lin)-1; k++)
                        {
                            if(lin[k] == ',') //am dat de , deci urmeaza nr de linii
                            {
                                for(int j=k+3; j<strlen(lin)-1; j++)
                                {
                                    nr_linii_necesare = nr_linii_necesare*10 + (lin[j] - '0'); //luam nr de linii
                                    
                                }
                            }
                        }
                    }
                }

                printf("Timpul pentru rezolvare(in minute): %d\n", timp);
                printf("Nr de linii max la aceasta problema este: %d\n", nr_linii_necesare);

                timpd = timp*60;
                printf("Timpul (secunde) de rezolvare : %f \n", timpd);
                fclose(q);
                
                //trimitem prima data timpul pentru rezolvare
                if(write (client, &timp, sizeof(timp)) <= 0)
                {
                    perror ("[server]Eroare la write() catre client cerina.\n");
                    continue; /// continuam sa ascultam 
                }

                //scriem catre client cerinta pentru problema
                if(write (client, cerinta, 400) <= 0)
                {
                    perror ("[server]Eroare la write() catre client cerina.\n");
                    continue; // continuam sa ascultam 
                }
                else
                    printf ("[server]Cerinta a fost trasmisa cu succes.\n");
            

                //scriem clientul continutul fisierului de input
                if(write (client, liniuta, 400) <= 0)
                {
                    perror ("[server]Eroare la write() catre client cerina.\n");
                    continue;		/// continuam sa ascultam 
                }

                
                //deschidem fisier de output
                if((deschid=fopen(out, "r")) < 0)
                { printf("Eroare la deschiderea fisierului cu input ul\n"); }

                fgets(liniuta, 400, deschid);
                fclose(deschid);
                printf("Continutul fis de output este: %s\n", liniuta);


                //scriem clientul continutul fisierului de output
                if(write (client, liniuta, 400) <= 0)
                {
                    perror ("[server]Eroare la write() catre client cerina.\n");
                    continue;		/// continuam sa ascultam 
                }

                //citim timpul in care a rezolvat clientul
                if(read(client, &elapsed1, sizeof(elapsed1)) < 0)
                { 
                    printf("eroare la read de la client\n"); 
                    return errno; 
                }
                //printf("Clientul a rezolvat problema in %f secunde.\n", elapsed1);

                //citim rezolvarea problemei transmisa de client
                if(read(client, rezolvare, 1024) < 0)
                {
                    perror ("[server]Eroare la read() de la client.\n");
                    return errno;
                }

                printf("S-a citit rezolvarea: \n %s \n", rezolvare); //afisam rezolvarea problemei
                printf("Clientul a rezolvat problema in %f secunde.\n", elapsed1);
                
                strcpy(copie_rezolvare, rezolvare);
                //rezolvare este: %s\n", rezolvare);
                nr_linii = nrlinii(copie_rezolvare);
                printf("Nr de linii din rezolvare este: %d \n", nr_linii);

                //cream in sir numele fisierul in care scriem rezolvarea
                char sir[100] = ""; int l=0;
                while(ordine)
                {
                    l++;
                    for(int i=l; i >= 0; i--)
                    sir[i] = sir[i-1];
                    sir[0] = ordine%10 + '0';
                    ordine = ordine/10;
                }


                // printf("%s\n", sir);
                strcat(fis, sir);
                strcat(fis, ".c");

                printf("Numele fisierului.c este: %s\n", fis); //numele fisierului
                strcpy(exe, fis); //executabilul e acelasi
                exe[strlen(fis)-2] = '\0'; //doar ca stergem ultimele 2 caractere, adica ".c"
                printf("Numele executabilului este: %s\n", exe); //numele executabilului

            
                FILE* f;
                if((f=fopen(fis,"w")) < 0) //creare si deschidere fisier pentru salvarea rezolvarii
                {
                    printf("Eroare la deschidere/creare fisier in care scriem problema! \n");
                    return 3;
                }

                if(fputs(rezolvare, f) < 0) //scriere in fisier a rezolvarii
                {
                    printf("eroare la punere in fisier\n");
                }
                fclose(f); //inchidem fisierul dupa ce am scris in el
        

                //FACEM FORK CA SA CREAM EXECUTABILUL 
                if(pipe(pipes) < 0) //ne vom folosi de pipe
                {
                    printf("eroare la pipe\n");
                    return 4;
                }

                printf("linia din input : %s \n", copie_input);
                // argumente(copie_input);
               
                start = time(NULL); //AICIII
                pid_t pid = fork();
                if(pid < 0)
                {
                    printf("Eroare la fork! \n");
                    return 1;
                }
                else if(pid == 0) //child -- in child facem alt fork pt a crea executabilul cu execlp !!!!!!!!!!!
                {

                    dup2(pipes[1], STDOUT_FILENO); //duplicam ca iesirea sa se puna in pipes[1] (capatul de scriere)
                    close(pipes[0]);
                    close(pipes[1]);

                    char *my_args[5];  //argumentele pentru executarea executabilului
                    strcpy(my_args[0], exe);
                    //strcpy(my_args[1], );
                    strcpy(my_args[1], in_fis);
                    my_args[2] = NULL;
                    //argumente(liniuta);

                    pid_t pid2 = fork();
                    if(pid2 < 0)
                    {
                        printf("erroare la fork2 \n");
                        return 6;
                    }
                    else if(pid2 == 0) //child
                    {   
                        // gcc fisier.c -o exe
                       
                        execlp("gcc", "gcc", fis, "-o", exe, NULL); //aici cream executabilul
                        exit(0);
                    }
                    else  //parinte
                    {
                        wait(NULL); //asteptam fiul
                        execv(exe, my_args);  //rulam executabilul
                    }

                    exit(0);
                }
                else  //parinte
                {
                    wait(NULL); //asteptam fiul

                    end = time(NULL); //timpul cand a terminat de executat
                    elapsed = difftime(end, start); //diferenta de timp dintre start si end pentru timpul de executie
                    printf("Timpul de executie al rezolvarii: %f secunde.\n", elapsed);
                  
                    // printf("Ar trebui sa avem executabil! :) \n");
                    close(pipes[1]);
                    if(read(pipes[0], buff, sizeof(buff)) < 0)
                    {
                        printf("eroare citire\n");
                        return 5;
                    }
                    printf("raspuns: %s\n", buff);
                
                    close(pipes[0]);
                    // printf("atoi de buff: %d \n", atoi(buff));
                    
                    //luam din nou raspunsul din fisierul de output
                    if((p=fopen(out, "r")) < 0)
                    {
                        printf("Eroare la deschidere\n");
                    }

                    if(fgets(bufi, 100, p) < 0)
                    {
                        printf("Eroare la bufi\n");
                    }

                    fclose(p);
                    printf("Cu ce trebuie comparat raspunsul: %s\n", bufi);

                    //verificare raspunsului prin comparare 
                    //printf("lungimea raspunsului: %lu \n", strlen(buff));

                    if(strlen(buff) == 0)
                    {
                        printf("Programul trimis nu ruleaza! :( primiti doar punctul din oficiu \n");
                        nota = 1; 
                    }
                    else if(atoi(buff) == atoi(bufi)) //comparam cu rezultatul din fis
                    {
                        printf("Ati raspuns corect! :) \n");
                        nota = 10;

                        if(elapsed1 != 0 && elapsed1 < timpd) //a rezolvat inainte sa se termine timpul
                        { 
                            nota++; 
                            printf("Primiti un punct in plus pentru ca ati scris o rezolvare corecta intr-un timp mai scurt.\n");
                        }

                        if(nr_linii <= nr_linii_necesare)
                        {
                            nota++; 
                            printf("Primiti un punct in plus pentru ca rezolvarea e mai scurta decat era maximul(%d).\n", nr_linii_necesare);
                        }
                    }
                    else 
                    {
                        printf("Nu ati raspuns corect! Dar primiti nota 5 pentru ca ruleaza programul. \n");
                        nota = 5;

                    }
                    
                    printf("Nota clientului cu nr de ordine %d este %d\n", cordine, nota);
                    if(write(client, &nota, sizeof(nota)) < 0)
                    {
                        printf("Eroare la trimis okay ul\n");
                        return 6;
                    }

                    FILE* opn; //pt fisierul cu note
                    if((opn=fopen("note.txt", "a+")) < 0)
                    { printf("eroare la deschiderea fis cu note\n"); }
                    
                    strcpy(copie_fis, fis); strcpy(copie_sir, sir); //copii la numele fis si la sir(ordinea ca sir de caractere)
                    strcpy(notare_fis, "");
                    strcpy(notare_fis, copie_sir); //oridine
                    strcat(notare_fis, " ");
                    strcat(notare_fis, copie_fis); //numele fisierului in care am salvat
                    strcat(notare_fis, " ");
      
                    //transformam nota in sir 
                    //char nota_sir[100] = ""; int len=0;
                    while(nota)
                    {
                        len++;
                        for(int i=len; i >= 0; i--)
                            nota_sir[i] = nota_sir[i-1];
                        nota_sir[0] = nota%10 + '0';
                        nota = nota/10;
                    }
                    strcat(notare_fis, nota_sir);
                    strcat(notare_fis, "\n");

                    if(fputs(notare_fis,opn) < 0)
                    { printf("eroare la scriere in fisierul cu note\n"); }
                    fclose(opn);

                    /*printf("timp inceput: %ld\n", start);
                    printf("timp end: %ld\n", end);*/
                }
                
            }
            else
            {
               // printf("Nu mai poti participa la olimpiada!! \n");
                poti_participa = 0; //ii scriem clientului daca poate participa
                if(write (client, &poti_participa, sizeof(poti_participa)) <= 0)
                {
                    perror ("[server]Eroare la write() catre client cerina.\n");
                    continue; // continuam sa ascultam 
                }
            }

    		// am terminat cu acest client
    		close (client);
    		exit(0);
    	}

    }		

    return 0;
}			

