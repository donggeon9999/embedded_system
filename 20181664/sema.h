#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h>
#include <errno.h>  

#define MODE_CNT 3
#define SEM_KEY (key_t) 0x20
#define SEM_CNT 3

#define key_sem 0
#define mode_sem 1
#define merge_sem 2


 union semun { 
       int val; 
       struct semid_ds *buf; 
       unsigned short *array; 
 }; 


struct sembuf p[SEM_CNT], v[SEM_CNT];
int mode;
int initsem(key_t semkey);
int semlock(int semid, int mode);
int semunlock(int semid, int mode);
void erase_sema(int semid);
void semhandle();

