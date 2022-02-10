#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
//CLIENT

extern int errno;
int port; //portul de conectare la server
int timp;
char fis[100];
int ordine;
char rezolvare[400];
int ok;
int nota;
int timp; //timpul pentru rezolvare
char rasp[100];
char liniuta[400];
char liniuta2[400];

//pentru masurarea timpului de scriere
time_t start, end;
time_t start1, end1; //pentru scrierea rezolvarii
double elapsed, elapsed1;
int terminate = 1;
int secunde;
int poti_participa;
char info_notare[400] = "";
int ordine;
char nume_fis[100]="fisier";

int main(int argc, char *argv[])
{
    int sd; //descriptul de socket
    struct sockaddr_in server; //structura folosita pt conectare
    char msg[400];		// mesajul trimis

    if(argc != 3)
    {
        printf("Adresa server, port!!\n");
        return -1;
    }

    //stabilim portul
    port = atoi(argv[2]);

    //cream socketul
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket! \n");
        return errno;
    }

    //umplem structura pt realizarea conexiunii
    //familia socketului
    server.sin_family = AF_INET;
    //adresa IP a serverului
    server.sin_addr.s_addr = inet_addr(argv[1]);
    //portul de conectare
    server.sin_port = htons(port);

    //ne conectam la server
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    printf("Numarul meu de ordine este: %d\n", ordine);
    
    //citim daca putem participa
    if(read(sd, &poti_participa, sizeof(poti_participa)) < 0)
    {
        perror("[client] eroare la read() de la server pentru timp cerinta \n");
        return errno;
    }

    //citim nr nostru de ordine
    if(read(sd, &ordine, sizeof(ordine)) < 0)
    {
        perror("[client] eroare la read() de la server pentru timp cerinta \n");
        return errno;
    }
    
    char s[100] = ""; int lun=0;

    //transformare ordine din string in char, adica numarul problemei
    while(ordine)
    {
        lun++;
        for(int i=lun; i >= 0; i--)
            s[i] = s[i-1];
        s[0] = ordine%10 + '0';
        ordine = ordine/10;
    }  

    if(poti_participa == 1)
    {  
        //citim informatii despre notare
        if(read(sd, info_notare, 400) < 0)
        {
            perror("[client] eroare la read() de la server pentru informatii notare \n");
            return errno;
        }

        //afisare informatii notare
        printf("MODALITATE DE NOTARE: \n %s \n", info_notare);

        //citirea timpului
        if(read(sd, &timp, sizeof(timp)) < 0)
        {
            perror("[client] eroare la read() de la server pentru timp cerinta \n");
            return errno;
        }
        printf("Timpul pe care il aveti petru rezolvat problema este: %d (minute)\n", timp);

        //citirea problemei - citire de la server
        if(read(sd, msg, 400) < 0)
        {
            perror("[client] eroare la read() de la server pentru cerinta \n");
            return errno;
        }
        
        //citire continutului fis in input
        if(read(sd, liniuta, 400) < 0)
        { printf("eroare la read fis de input din client\n"); return errno; }
       
        //citim continutul fis de output
        if(read(sd, liniuta2, 400) < 0)
        { printf("eroare la read fis de input din client\n"); return errno; }
       
        //afisam cerinta si continutul fis de input si output
        printf("[client] Cerinta: %s", msg);
        printf("Continutul fis de input este: %s", liniuta);
        printf("Continutul fis de output este: %s", liniuta2);

        printf("Vreti sa introduceti rezolvare? y/n...\n");
        fflush(stdout);
        bzero(rasp, 100);
        read(0, rasp, 400);


        FILE *f;
        if(strcmp(rasp, "y\n") == 0)
        {
            //scriere rezolvarii
            printf("Introduceti rezolvarea problemei(aveti la dispozitie timpul specificat mai sus) \n");
            printf("Trebuie sa scrieti 'gata' cand ati terminat de scris si vreti sa trimiteti.\n");
            secunde = 60*timp; //calcularea secundelor pt scrierea rezolvarii
            fflush(stdout);
            start = time(NULL);

            //cream fisierul in care scriem problema
            
           // char nume_fis[100]="fisier";
            strcat(nume_fis, s);
            strcat(nume_fis, ".txt");
            printf("Numele fis este: %s\n", nume_fis);

            if((f=fopen(nume_fis, "w")) < 0)
            {
                printf("Eroare la deschidere fisierului fisier.txt\n");
                return 3;
            }

            int ok = 1;
            start1 = time(NULL);
            while(terminate && ok)
            {
                end = time(NULL);
                elapsed = difftime(end, start); //diferenta de timp dintre start si end

                if(elapsed >= secunde)  //nr de secunde
                    terminate = 0;
                else 
                {
                    bzero(rezolvare, 400);
                    read(0, rezolvare, 400);
                    if(strcmp(rezolvare, "gata\n") == 0) //daca a scris gata, adica vrea sa trimita rezolvarea
                    {
                        ok = 0; 
                        end1 = time(NULL);
                        elapsed1 = difftime(end1, start1); //timpul in care a scris rezolvarea
                    } 
                    if(ok)
                    {   
                        if(fputs(rezolvare,f) < 0) //scriem in fisier 
                        {
                            printf("Eroare la scris in fisier");
                        }
                    }
                }
            }
            fclose(f);
        }
        else
        { printf("Nu doriti sa participati la olimpiada! \n"); exit(0); }
        

        printf("Ati scris rezolvarea in %f secunde! \n", elapsed1);

        //trebuie trimisa aceasta valoare serverului pentru a face corectura
        if(write(sd, &elapsed1, sizeof(elapsed1)) < 0) //trimitem nr de secunde
        {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }

        
        //deschidem acum pt citire
        if((f=fopen(nume_fis, "r")) < 0)
        {
            printf("Eroare la deschidere fisierului fisier.txt\n");
            return 3;
        }
        
        char line[15]="";
        bzero(rezolvare, 400); //facem rezolvarea 0
        while(fgets(line, 400, f)) //luam cate o linie din fisier
        {
            strcat(rezolvare, line);
        }
        fclose(f);

        //trimitem serverului rezolvarea
        if(write(sd, rezolvare, 400) < 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }
    
        //citirea unui eventual raspuns
        if(read(sd, &nota, sizeof(nota)) < 0)
        {
            printf("Eroare la a citi ok\n");
            return 6;
        }

        printf("Punctajul tau este: %d \n", nota);
        //inchidem conexiunea
    }
    else
    {
        printf("Nu mai poti participa la olimpiada!! s-a depasit nr max de participanti\n");
    }

    close(sd);
    
    return 0;
}

