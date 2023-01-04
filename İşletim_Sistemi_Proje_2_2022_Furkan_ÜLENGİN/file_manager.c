#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX_THREADS 6
#define MAX_FILES 10

typedef struct { // file list için tip tanımlanması
    char name[256];
} file_t;
// file listesi ve thread listesi oluşturulması
file_t file_list[MAX_FILES];
pthread_t thread_list[MAX_THREADS];
pthread_mutex_t file_list_lock;

// fonksiyon parametreleri
void *client_handler();
void create_file(char *name);
void delete_file(char *name);
void read_file(char *name);
void write_file(char *name, char *data);

int main() {
    int i;
     // named pipelar oluşturulur.
    mkfifo("mesaj_yaz",0666);
    mkfifo("mesaj_oku",0666);
    pthread_mutex_init(&file_list_lock, NULL);
    // Threadleri işlemek için ileti dizileri oluşturun
    for (i = 0; i < MAX_THREADS; i++) {
        pthread_create(&thread_list[i], NULL, client_handler,NULL);
    }
    // threadlerin bitmesi beklenir
    for (i = 0; i < MAX_THREADS; i++) {
        pthread_join(thread_list[i], NULL);
    }
    pthread_mutex_destroy(&file_list_lock);
    return 0;
}
void *client_handler(){
    char buffer[256];
    int mesaj_oku;
    int mesaj_yaz;
    mesaj_oku=open("mesaj_oku",O_RDONLY);//file_clienttan gelen verilerin okunması için okuma modunda açılan named pipe
    mesaj_yaz = open("mesaj_yaz",O_WRONLY);//file_clienta mesaj göndermek için yazma modunda açılan named pipe
    while(1){
        read(mesaj_oku, buffer, 256);//named pipe okunur.
        char *command = strtok(buffer, " "); // veriler parçalara ayrılır komut-dosyaAdı-Mesaj
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, "\0");
	pthread_mutex_lock(&file_list_lock);
	
        if (strcmp(command, "create") == 0) {
        create_file(arg1);
        } else if (strcmp(command, "delete") == 0) {
        delete_file(arg1);
        } else if (strcmp(command, "read") == 0) {
        read_file(arg1);
        } else if (strcmp(command, "write") == 0) {
        write_file(arg1, arg2);
        } else if (strcmp(command, "exit") == 0) {
           pthread_mutex_unlock(&file_list_lock);
           write(mesaj_yaz,"file manager cikis yapildi",27);
           close(mesaj_yaz);
            exit(0);
        }
        pthread_mutex_unlock(&file_list_lock);
    }
}

void create_file(char *name){
    FILE *dosya;
    int i;
    int mesaj_yaz;
    mesaj_yaz=open("mesaj_yaz",O_WRONLY);
    // Dosyanın zaten var olup olmadığını kontrol edin
    for (i = 0; i < MAX_FILES; i++) {
        if (strcmp(file_list[i].name, name) == 0) {   
            write(mesaj_yaz,"Error: File already exists",27);
            close(mesaj_yaz);
            return;
        }
    }
    // Dosya listesinde boş yuva bulun
    for (i = 0; i < MAX_FILES; i++) {
        if (strlen(file_list[i].name) == 0) {
            strcpy(file_list[i].name, name);
            dosya=fopen(name,"w");
            fclose(dosya);
            write(mesaj_yaz,"dosya oluşturuldu",27);// file_clienta mesaj gönderilir.
            close(mesaj_yaz);
            printf("File created: %s\n", name);
            return;
        }
    }
}
void delete_file(char *name){
    int i;
    int mesaj_yaz;
    mesaj_yaz=open("mesaj_yaz",O_WRONLY);
    // Dosyanın zaten var olup olmadığını kontrol edin
    for (i = 0; i < MAX_FILES; i++) {
        if (strcmp(file_list[i].name, name) == 0) {
            // dosyayı sistemden silin
            remove(name);
            // Dosya listesinden dosya adını temizle
            memset(file_list[i].name, 0, sizeof(file_list[i].name));
            write(mesaj_yaz,"dosya silindi",27);
            close(mesaj_yaz);
            printf("File deleted: %s\n", name);
            return;
        }
        
    }
  printf("File not deleted.");
}
void read_file(char *name){
    int i;
    char buffer[256];
    int mesaj_yaz;
    mesaj_yaz=open("mesaj_yaz",O_WRONLY);
	// Dosyanın zaten var olup olmadığını kontrol edin
	for (i = 0; i < MAX_FILES; i++) {
		if (strcmp(file_list[i].name, name) == 0) {
			// okumak için dosyayı açın
			int fd = open(name, O_RDONLY);
			if (fd < 0) {
				printf("Error: Unable to open file for reading\n");
				return;
			}
	  	
	  	int bytes_written = read(fd, buffer, sizeof(buffer));// dosya içeriği okunur
	  	if (bytes_written < 0) {
		    printf("Error: Unable to read to file\n");
		    close(fd);
		    return;
	  	}
	  	printf("File readed: %s\n", name);
	  	printf("içerik: %s\n", buffer);
	  	write(mesaj_yaz,buffer,256);// dosya içeriği file_client a gönderilir
            	close(mesaj_yaz);
 	        // dosyayı kapatın
		close(fd);
		return;
		}
	}	

	printf("Error: File not found\n");
}
void write_file(char *name, char *data){
    int i;
    int mesaj_yaz;
    mesaj_yaz=open("mesaj_yaz",O_WRONLY);
	// Dosyanın zaten var olup olmadığını kontrol edin
	for (i = 0; i < MAX_FILES; i++) {
		if (strcmp(file_list[i].name, name) == 0) {
			// yazmak için dosyayı açın
			int fd = open(name, O_WRONLY | O_APPEND);
			if (fd < 0) {
				printf("Error: Unable to open file for writing\n");
				return;
			}
	  	// dosyaya veriyi yazın
	  	int bytes_written = write(fd, data, strlen(data));
	  	if (bytes_written < 0) {
		    printf("Error: Unable to write to file\n");
		    close(fd);
		    return;
	  	}
 	        // dosyayı kapatın
 	        write(mesaj_yaz,"dosya veri yazıldı",27);
                close(mesaj_yaz);
		close(fd);
		printf("File written: %s\n", name);
		return;
		}
	}	

	printf("Error: File not found\n");     
}
