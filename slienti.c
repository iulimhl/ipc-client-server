#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<sys/socket.h>
#include<iostream>
#include <sys/stat.h> 

using namespace std;

#define SERVER_FIFO "server_fifo"
#define CLIENT_FIFO "client_fifo"
#define max_buffer 1024

int main() {
    char comanda[max_buffer],raspuns[max_buffer];
    int fds,fdc;
    mkfifo(CLIENT_FIFO,0666);
    
    fds=open(SERVER_FIFO,O_WRONLY);
    if(fds==-1)
        {
            perror("eroare la deschiderea fisierului in server");
            exit(1);
        }
    
    while(true){
        cout<<"Client:introdu o comanda ";
        cin.getline(comanda,sizeof(comanda));

        if(write(fds,comanda,strlen(comanda))==-1)
        {
            perror("eroare la write");
            exit(1);
        }

        if(strcmp(comanda,"quit")==0)
        {
            cout<<"am primit quit";
            break;
        }

        fdc=open(CLIENT_FIFO,O_RDONLY);
        if(fdc==-1){
            perror("eroare la deschiderea client_fifo");
            exit(1);
        }

        int lungime;
        read(fdc, &lungime, sizeof(lungime));
        read(fdc, raspuns, lungime);
        raspuns[lungime] = '\0';

        cout << "Server: " << raspuns << endl;

        close(fdc);
        
        
    }

    close(fds);
    unlink(CLIENT_FIFO);
    return 0;
}
