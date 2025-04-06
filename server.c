#include <string.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<sys/socket.h>
#include<iostream>
#include <cstring>
#include<fstream>
#include<ctime>
#include<stdbool.h>
#include<utmp.h>
#include <sys/stat.h>
#include <utmp.h>
#include <sys/types.h>

using namespace std;

#define fifo "server_fifo"
#define max_buffer 1024

bool autentificat=false;
string aut_user="";

void login(int sv[2], const char* comanda,bool &autentificat)
{
    if(autentificat)
    {
        
        const char *mesaj="Cineva este deja autentificat";
        int lungime=strlen(mesaj);
        if(write(sv[1],&lungime,sizeof(lungime))==-1)
        {
            perror("eroare la scrierea lungimii rasp");
            exit(1);
        }
        if(write(sv[1],mesaj,lungime)==-1)
        {
            perror("eroare la scrierea rasp");
            exit(1);
        }
            
        write(sv[1],&autentificat,sizeof(autentificat));

    }
    else{
    
        const char *username=comanda+8;
        char user[100];
        strcpy(user,username);
        
        ifstream fisier("users.txt");
        bool gasit=false;

        if(fisier.is_open())
        {
            string linie;
            while(getline(fisier,linie))
            {
                if (linie == user) {
                    gasit = true;
                    break;
                }
            }
            fisier.close();
            
        }
        else
            perror("eroare la deshiderea fisierului");

        const char* raspuns=gasit?"Login reusit":"user nu exista";
        if(gasit)
        { autentificat=true;
            aut_user=user;
        }
        int lungime=strlen(raspuns);

        if(write(sv[1],&lungime,sizeof(lungime))==-1)
        {
            perror("eroare la scrierea lungimii rasp");
            exit(1);
        }


        if(write(sv[1],raspuns,lungime)==-1)
        {
            perror("eroare la scrierea rasp");
            exit(1);
        }
        
        write(sv[1],&autentificat,sizeof(autentificat));
    }

}


void logout(int sv[2], bool &autentificat)
{
    if(!autentificat)
    {
        const char *mesaj="eroare:utilizatorul nu e autentificat";
        int lungime=strlen(mesaj);
        if(write(sv[1],&lungime,sizeof(lungime))==-1)
        {
            perror("eroare la scrierea lungimii rasp");
            exit(1);
        }
        if(write(sv[1],mesaj,lungime)==-1)
        {
            perror("eroare la scrierea  rasp");
            exit(1);
        }
        write(sv[1],&autentificat,sizeof(autentificat));
        return;

    }

    autentificat=false;
    aut_user="";

    const char *mesaj="Logout reusit";
    int lungime=strlen(mesaj);
    if(write(sv[1],&lungime,sizeof(lungime))==-1)
        {
            perror("eroare la scrierea lungimii rasp");
            exit(1);
        }
    if(write(sv[1],mesaj,lungime)==-1)
        {
            perror("eroare la scrierea  rasp");
            exit(1);
        }
    write(sv[1],&autentificat,sizeof(autentificat));
    
    
}

void get_logged_users(int p[2],bool &autentificat)
{
    if(!autentificat)
    {
        const char *mesaj="eroare:utilizatorul nu e autentificat";
        int lungime=strlen(mesaj);
        
        write(p[1],&lungime,sizeof(lungime));
        //verific
        write(p[1],mesaj,lungime);
        write(p[1],&autentificat,sizeof(autentificat));
        return;

    }
    
    setutent();
    struct utmp *u;
    string rezultat;
    
    while((u=getutent())!=NULL){
        if(u->ut_type==USER_PROCESS)
        rezultat += string(u->ut_user) + " | " + string(u->ut_line) + " | " + to_string(u->ut_tv.tv_sec) + "\n";
    }
    endutent();
    
    int lungime=rezultat.length();
    write(p[1],&lungime,sizeof(lungime));
    //verific
    write(p[1],rezultat.c_str(),lungime);
    write(p[1],&autentificat,sizeof(autentificat));
} 

void get_proc_info(int p[2],const char *comanda, bool &autentificat)
{
    if(!autentificat)
    {
        const char *mesaj="eroare:utilizatorul nu e autentificat";
        int lungime=strlen(mesaj);
        write(p[1],&lungime,sizeof(lungime));
        
        write(p[1],mesaj,lungime);
        write(p[1],&autentificat,sizeof(autentificat));
        return;

    }
    
    const char *pid=comanda+16;
    string path="/proc/"+string(pid)+"/status";
    
    ifstream path_file(path);
    if(!path_file)
    {
        const char *mesaj="nu se poate deschide fisierul";
        int lungime=strlen(mesaj);
        write(p[1],&lungime,sizeof(lungime));
        write(p[1],mesaj,lungime);
        write(p[1],&autentificat,sizeof(autentificat));
        return;
    }
    
    string rezultat,linie;
    while(getline(path_file,linie))
    {
        if(linie.find("Name:")==0)
            rezultat+=linie+"\n";
        else if(linie.find("State:")==0)
            rezultat+=linie+"\n";
        else if(linie.find("PPid:")==0)
            rezultat+=linie+"\n";
        else if(linie.find("Uid:")==0)
            rezultat+=linie+"\n";
        else if(linie.find("VmSize:")==0)
        
            rezultat+=linie+"\n";
    }
    path_file.close();
    int lungime=rezultat.length();
    write(p[1],&lungime,sizeof(lungime));
    char rezultat1[lungime+1];
    for(int i=0;i<lungime;i++)
        rezultat1[i]=rezultat[i];
    rezultat1[lungime]='\0';
    write(p[1],rezultat1,lungime);
    write(p[1],&autentificat,sizeof(autentificat));
}


int main() {

    char comanda[max_buffer];
    int fd;

    mkfifo(fifo,0666);
    fd=open(fifo,O_RDONLY);
        if(fd==-1)
        {
            perror("eroare la deschiderea fisierului in server");
            exit(1);
        }
    int sv[2];

    if(socketpair(AF_UNIX,SOCK_STREAM,0,sv)==-1)
    {
        perror("eroare la socket");
        exit(1);
    }

    int p[2];

    if (pipe(p) == -1) {
        perror("eroare la pipe");
        exit(1);
    }

    cout<<"Serverul asteapta coamanda"<<endl;
    
    while(true)
    {
        memset(comanda,0,max_buffer);
        if(read(fd,comanda,max_buffer-1)==-1)
        {
            perror("eroare la read in server");
            exit(1);
        }

        cout<<"Am primit comanda: "<<comanda<<endl;

        if(strncmp(comanda,"login : ",8)==0)
            {
            if(fork()==0)
            {
                close(sv[0]);
                login(sv,comanda,autentificat);
                close(sv[1]);
                exit(0);
            }
            else
            {
                
                int lungime;
                read(sv[0],&lungime,sizeof(lungime));
                read(sv[0],comanda,lungime);
                comanda[lungime]='\0';
                
                cout<<"Raspuns: "<<comanda<<endl;
                int fd_client = open("client_fifo", O_WRONLY);
                if (fd_client == -1) {
                    perror("eroare la deschiderea client_fifo");
                } else {
                    write(fd_client, &lungime, sizeof(lungime));
                    write(fd_client, comanda, lungime);
                    close(fd_client);
                }

                read(sv[0],&autentificat,sizeof(autentificat));
                wait(NULL);
               
            }
        }


        else if(strncmp(comanda,"logout",6)==0)
        {
            if(fork()==0)
            {
                close(sv[0]);
                logout(sv,autentificat);
                close(sv[1]);
                exit(0);
            }
            else   
            {   
                
                int lungime;
                read(sv[0],&lungime,sizeof(lungime));
                read(sv[0],comanda,lungime);
                comanda[lungime]='\0';
                
                
               cout<<"Raspuns: "<<comanda<<endl;
               int fd_client = open("client_fifo", O_WRONLY);
               if (fd_client == -1) {
                    perror("eroare la deschiderea client_fifo");
                } else {
                    write(fd_client, &lungime, sizeof(lungime));
                    write(fd_client, comanda, lungime);
                    close(fd_client);
                }

                read(sv[0],&autentificat,sizeof(autentificat));
                wait(NULL);
                
            }
        }
        else if(strncmp(comanda,"quit",4)==0)
        {
            close(fd);
            close(sv[0]);
            close(sv[1]);
            close(p[0]);
            close(p[1]);
            cout<<"S-a dat quit"<<endl;
            unlink(fifo);
            break;
        }
        
        else if(strncmp(comanda,"get-logged-users",16)==0)
        {
            if(fork()==0)
            {
                close(p[0]);
                get_logged_users(p,autentificat);
                close(p[1]);
                exit(0);
            }
            else
            {
                //close(p[1]);
                int lungime;
                read(p[0],&lungime,sizeof(lungime));
                char buffer[max_buffer];
                read(p[0],buffer,lungime);
                buffer[lungime]='\0';
                read(p[0],&autentificat,sizeof(autentificat));
                cout<<"Rasp: "<<buffer<<endl;
                int fd_client = open("client_fifo", O_WRONLY);
                if (fd_client == -1) {
                    perror("eroare la deschiderea client_fifo");
                } else {
                    write(fd_client, &lungime, sizeof(lungime));
                    write(fd_client, buffer, lungime);
                    close(fd_client);
                }
                wait(NULL);
                        
            }
        } 
        else if(strncmp(comanda,"get-proc-info : ",16)==0)
        {
        if(fork()==0)
            {
            close(p[0]);
            get_proc_info(p,comanda,autentificat);
            close(p[1]);
            exit(0);
            }
        else
        {
        //close(p[1]);
        int lungime;
        read(p[0],&lungime,sizeof(lungime));
        char buffer[max_buffer];
        read(p[0],buffer,lungime);
        buffer[lungime]='\0';
        read(p[0],&autentificat,sizeof(autentificat));
        cout<<"Raspuns: "<<buffer<<endl;
        int fd_client = open("client_fifo", O_WRONLY);
        if (fd_client == -1) {
            perror("eroare la deschiderea client_fifo");
        } else {
            write(fd_client, &lungime, sizeof(lungime));
            write(fd_client, buffer, lungime);
            close(fd_client);
        }
    
        wait(NULL);
        //  close(p[0]);
        }
    } 
        
    else
    {
    const char* mesaj = "nu exista functia";
    int lungime = strlen(mesaj);
    cout << mesaj << endl;

    int fd_client = open("client_fifo", O_WRONLY);
    if (fd_client == -1) {
        perror("eroare la deschiderea client_fifo");
    } 
    else {
        write(fd_client, &lungime, sizeof(lungime));
        write(fd_client, mesaj, lungime);
        close(fd_client);
        }
    }
 }

    return 0;
}
