#include "sema.h"

int initsem(key_t semkey) { 
    union semun semunarg; 
    int semid; 



    semid = semget(semkey, SEM_CNT, IPC_CREAT); 
    if(semid == -1) { 
            printf("sema error!\n");    
    } 
    else { 
        semunarg.val = 1;

         int i;
        for (i = 0; i < SEM_CNT; i++) {
            if (semctl(semid, i, SETVAL, semunarg) == -1) {
                perror("initsem"); 
                return -1;
            }

            p[i].sem_num = v[i].sem_num = i;
            //semaphore index store
            p[i].sem_flg = v[i].sem_flg = SEM_UNDO;

            p[i].sem_op = -1;  // The semaphore p() operation
            v[i].sem_op = 1;   // The semaphore v() operation
        }
    } 
    
    return semid; 
}
void erase_sema(int semid)//erase semaphore function
{
    if (semctl(semid, 0, IPC_RMID, 0) == -1) {
        printf("fail to erase sema\n");
    }
    return; 

}



