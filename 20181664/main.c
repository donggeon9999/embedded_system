#include "main.h"
//#include "sema.h"
#include "device.h"
#include "sema.h"
#include "io.h"
// #include "merge.h"

#define SHM_KEY_1 (key_t) 0x10
#define SHM_KEY_2 (key_t) 0x20


int io_shmid, merge_shmid;

int semid;

int main() { 

    semid= initsem(SEM_KEY);// semaphore 설정 및 관련 값들 초기화 함수
    if(semid == -1)
    {
        printf("semaphore error\n");//semaphore 생성 실패
        return 1;
    }

    open_devices();// device driver open 
    pid_t pid = make_fork(); // main process 에서 fork() 진행하고 pid 를 받아옴 

    if(pid == MAIN_PID) // main process 가 진행하는 부분
    {

        printf("Main Process ===== %d\n", pid); 
        main_process();// main process 가 진행하는 함수

        
         // 진행이 모두 종료 되면 shared memory 해제 및 삭제, semaphore 삭제
        shmdt((char *)io_shmaddr);  
        shmctl(io_shmid, IPC_RMID, (struct shmid_ds *)NULL); 
        shmctl(merge_shmid, IPC_RMID, (struct shmid_ds *)NULL); 
        erase_sema(semid);

    }
    else if(pid == IO_PID)// main process에서 fork 된 io process 가 진행하는 부분
    {
        pid_t pid;

        printf("IO Process =====%d\n", pid); 
        switch(pid = fork()) { // io process 내에서 fork 를 한번 더 해주었다
            case -1: 
                perror("fork"); 
                exit(1); 
                break; 
            case 0: // Mode를 바꾸어주는 process
                io_shmaddr = (struct input_buff *)shmat(io_shmid, NULL, 0); //shared memory에 연결
                mode_io(semid,io_shmaddr,io_shmid,merge_shmid);
                shmdt((struct input_buff *)io_shmaddr); // shared memory 해제
                break;
            default: //io 입력을 받아주는 process
                put_io(semid,io_shmaddr);
                shmdt((struct input_buff *)io_shmaddr); // shared memory 해제
                shmdt((char *)merge_shmaddr); // shared memory 해제
                break;
        }
        
    }
     else if(pid == MERGE_PID)// merge 를 진행하는 process
    {
        // printf("MERGE Process =====%d\n", pid); 
        
        merge_func(semid);

        shmdt((char *)merge_shmaddr); // shared memory 해제
    }

    return;
}

void main_process()// main process 가 진행하는 함수
{
    int prev_mode = PUT;
    initialize_put();// 시작할 때, led 1번만 켜져 있도록 설정
    while(1)
    {        
        //semlock(semid, io_sem);
        semop(semid, &p[mode_sem], 1);//io process 와의 shared memory 동시 접근을 막기 위한 semaphore 감소
        //printf("in main_process \n");
        if(io_shmaddr->mode == PUT){// PUT 모드일 경우 진행하는 부분
            
            if(io_shmaddr->is_changed)//모드가 바뀌거나 key-value를 저장했을 때의 상태 변화를 감지
            {
                io_shmaddr->is_changed = 0; // 값을 초기화 하고
                printf("mode is PUT\n");
                printf("mode is changed to  PUT\n");
                initialize(); // fnd lcd off
                initialize_put(); // number 1 led on 
            }
            //만약 key input이거나 value input 상황일 때 led 출력 수행
            if(io_shmaddr->key_value_mode == KEY_INPUT||io_shmaddr->key_value_mode == VALUE_INPUT)
                led_put(semid,io_shmaddr,0);


        }
        else if(io_shmaddr->mode == GET){// GET 모드일 경우 진행하는 부분
            //printf("mode is GET\n");
            if(io_shmaddr->is_changed)
            {// 모드가 바뀌거나 key-value를 저장했을 때의 상태 변화를 감지
                io_shmaddr->is_changed = 0;
                printf("mode is changed to GET\n");
                initialize(); // fnd lcd off
                initialize_put(); // number 1 led on 
            }
            if(io_shmaddr->key_value_mode == KEY_INPUT)//GET mode 에서 KEY input을 받는 경우에 led 출력 수행
                led_put(semid,io_shmaddr,0);

            
        }
        else if(io_shmaddr->mode == MERGE){
                //printf("mode is MERGE\n");
            if(io_shmaddr->is_changed)
            {
                io_shmaddr->is_changed = 0;
                printf("mode is changed to MERGE\n");
                initialize(); // fnd lcd off
                initialize_put();// number 1 led on
            }
            //merge_process();
        }
        else if(io_shmaddr->mode == MODE_EXIT){ // EXIT mode -> program exit
            io_shmaddr->is_changed = 0;
            initialize(); // fnd lcd off
            printf("what is value: %d\n",io_shmaddr->mode); 
            semop(semid, &v[mode_sem], 1);
            kill_child(io_pid);//  program exit 할 때, child process들을 kill
            kill_child(merge_pid);
            return;
        }
        semop(semid, &v[mode_sem], 1);
    }
}
void kill_child(pid_t pid)
{

    //kill ' IO' process
    if (kill(pid, SIGKILL) == -1) {
        perror("Error while killing input process!");
    } else {
        waitpid(pid, NULL, 0);
    }
}

int make_fork(){// fork io process & merge process
    int i,j;
    pid_t pid;
    io_shmid = shmget(SHM_KEY_1, sizeof(struct input_buff), IPC_CREAT|0644); //io-main shared memory generate
    merge_shmid = shmget(SHM_KEY_2, 3*sizeof(struct merge_buff), IPC_CREAT|0644); // io-merge shared memory generate
    if(io_shmid == -1) { 
        perror("io_shmget"); 
        printf("io error");
    } 
    if(merge_shmid == -1) { 
        perror("merge_shmget"); 
        printf("merge error");
    } 
    

    switch(io_pid = fork()) { 
        case -1: 
            perror("fork"); 
            exit(1); 
            break; 
        case 0: //IO process
            io_shmaddr = (struct input_buff *)shmat(io_shmid, NULL, 0); // io-main shared memory connect to io_proccess
            merge_shmaddr= (struct merge_buff *)shmat(merge_shmid, NULL, 0); // io-merge shared memory connect to io_process
            io_shmaddr->mode = PUT; // initialize mode is PUT
            io_shmaddr->buf_size = 0; // initialize value size
            io_shmaddr->is_changed = 1;// initialize mode change
            io_shmaddr->key_idx = 0; //initialize key idx

            merge_shmaddr->input_cnt = 0;// initialize memory table count in memory
            merge_shmaddr->storage_cnt = 0; // initialize storage table count in memory
            merge_shmaddr->need_merge = false; // initialize merge flag
            merge_shmaddr->need_store = false; // initialize store flag
            
            io_shmaddr->get_request = false;// initialize get flag
            
            
            return IO_PID;
            
        default: //Main Process
            
            switch(merge_pid = fork()){
                case -1: 
                            perror("fork"); 
                            exit(1); 
                            break; 
                case 0: // Merge Process
                    merge_shmaddr = (struct merge_buff *)shmat(merge_shmid, NULL, 0); // io-merge shared memory connect to merge_process
                            return MERGE_PID; 
                default:
                    break;
            }
            io_shmaddr = (struct input_buff *)shmat(io_shmid, NULL, 0); // io-main shared memory connect to main process

            io_shmaddr->mode = PUT;
           

            return MAIN_PID; 
    } 


}


