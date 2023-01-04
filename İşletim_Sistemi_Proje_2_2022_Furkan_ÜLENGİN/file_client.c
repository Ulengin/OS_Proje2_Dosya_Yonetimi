#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

pthread_mutex_t file_list_lock;

int main(){
    char buffer[256];
    char mesaj[256];
    int mesaj_yaz;
    int mesaj_oku;
    int fx;
    mesaj_oku = open("mesaj_oku",O_WRONLY);// file_manager e veri gönderebilmek için yazma modunda açılan pipe
    mesaj_yaz = open("mesaj_yaz",O_RDONLY);// file_managerden gelen cevapları okumak için okuma modunda açılan pipe
    while (1) {
    // menu
        printf("Menu:\n");
        printf("create\n");
        printf("delete\n");
        printf("read\n");
        printf("write\n");
        printf("exit\n");
        printf("Enter your choice: ");
        // kullanıcadan veriler alınır
        fgets(buffer, 256, stdin);
        buffer[strcspn(buffer, "\n")] = 0; 
        
        write(mesaj_oku, buffer, 256);//alınan veriler file_manager e gönderilir.
        read(mesaj_yaz,mesaj,256);// file_managerden mesajlar alınır.
    	printf("%s\n",mesaj);// mesajlar ekrana bastırılır.
    	
    }
    close(mesaj_yaz);// açılan pipelar kapatılır.
    close(mesaj_oku);
    return 0;
}
