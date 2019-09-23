#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* gcc ... -lpthread */
/* or gcc ... -pthread */
int max_square = 0;
int dead = 0;
char *** deadends = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    char ** b_;
    int current_x;
    int current_y;
    int m_;
    int n_;
}data_t;

typedef struct{
    pthread_t th;
    int move_;
}val;

int moves(char ** board, int m, int n){
    int total = 0;
    for (int i = 0; i<m; i++){
        for (int j = 0; j<n; j++){
            if (board[i][j] == 'X'){
            total += 1;
            }
        }
    }
    return total;
}

void printboard(char *** boards, pthread_t mytid, int bound, int m, int n){
    for (int i = 0; i< dead; i++){
        int size = moves(deadends[i],m,n);
        if (bound != 0 && size < bound){
            continue;
        }
        for(int j = 0; j<m; j++){
            for (int k = 0; k<n; k++){
                if (j == 0 && k == 0){
                    printf("THREAD %ld: > ", mytid);
                }
                else if (j != 0 && k == 0){
                    printf("THREAD %ld:   ", mytid);
                }
                if(deadends[i][j][k] == '\0'){
                    if (k == n-1){
                        printf(".\n");
                    }
                    else{
                        printf(".");
                    }
                }
                else if(deadends[i][j][k] == 'X'){
                    if (k == n-1){
                        printf("S\n");
                    }
                    else{
                        printf("S");
                    }
                }
            }
        }
    }
}

int tour(char ** board, int m, int n){
    int isit = 1;
    for (int i = 0; i<m; i++){
        for (int j = 0; j<n; j++){
            if (board[i][j] == '\0'){
                isit = 0;
                break;
            }
        }
    }
    return isit;
}



int possiblemove(char ** board, int x, int y, int m, int n){
    int all = 0;
    if (x -1 >= 0 && y-2 >= 0 && board[x-1][y-2] == '\0'){
        all += 1;
    }
    if (x-2 >= 0 && y-1 >= 0  && board[x-2][y-1] == '\0'){
        all += 1;
    }
    if (x-2 >= 0 && y+1 < n && board[x-2][y+1] == '\0'){
        all += 1;
    }
    if (x-1 >= 0 && y+2 < n && board[x-1][y+2] == '\0'){
        all += 1;
    }
    if (x+1 < m && y+2 < n && board[x+1][y+2] == '\0'){
        all += 1;
    }
    if (x+2 < m && y+1 < n && board[x+2][y+1] == '\0'){
        all += 1;
    }
    if (x+2 < m && y-1 >= 0 && board[x+2][y-1] == '\0'){
        all += 1;
    }
    if (x+1 < m && y-2 >= 0 && board[x+1][y-2] == '\0'){
        all += 1;
    }
    return all;
}

// x= row/m; y = column/n
// one move at a time...
void nextmove(char ** board, int x, int y, int m, int n, int * update){
    if (x -1 >= 0 && y-2 >= 0 && board[x-1][y-2] == '\0'){
        board[x-1][y-2] = 'X';
        update[0] = x-1;
        update[1] = y-2;
        return;
    }
    if (x-2 >= 0 && y-1 >= 0  && board[x-2][y-1] == '\0'){
        board[x-2][y-1] = 'X';
        update[0] = x-2;
        update[1] = y-1;
        return;
    }
    if (x-2 >= 0 && y+1 < n && board[x-2][y+1] == '\0'){
        board[x-2][y+1] = 'X';
        update[0] = x-2;
        update[1] = y+1;
        return;
    }
    if (x-1 >= 0 && y+2 < n && board[x-1][y+2] == '\0'){
        board[x-1][y+2] = 'X';
        update[0] = x-1;
        update[1] = y+2;
        return;
    }
    if (x+1 < m && y+2 < n && board[x+1][y+2] == '\0'){
        board[x+1][y+2] = 'X';
        update[0] = x+1;
        update[1] = y+2;
        return;
    }
    if (x+2 < m && y+1 < n && board[x+2][y+1] == '\0'){
        board[x+2][y+1] = 'X';
        update[0] = x+2;
        update[1] = y+1;
        return;
    }
    if (x+2 < m && y-1 >= 0 && board[x+2][y-1] == '\0'){
        board[x+2][y-1] = 'X';
        update[0] = x+2;
        update[1] = y-1;
        return;
    }
    if (x+1 < m && y-2 >= 0 && board[x+1][y-2] == '\0'){
        board[x+1][y-2] = 'X';
        update[0] = x+1;
        update[1] = y-2;
        return;
    }
    return;
}


void * move( void * arg )
  {
    data_t * s = (data_t *)arg;

    pthread_t mytid = pthread_self();
    char ** temp = s->b_;
    int x = s->current_x;
    int y = s->current_y;
    int m = s->m_;
    int n = s->n_;
    int z=0;
    int rc = 0;
    z++;
    rc++;
    int die = 0;
    if (x == 0 && y == 0){
        printf("THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n", mytid, m, n);
    }

    char ** copy = calloc(m, sizeof(char*));
    for (int q = 0; q<m; q++){
        copy[q] = calloc(n, sizeof(char));
    }
    for (int i = 0; i< m; i++){
        for (int j =0; j<n; j++){
            copy[i][j] = temp[i][j];
        }
    }

    for (int c = 0; c<m; c++){
        free(temp[c]);
    }
    free(temp);

    int possible = possiblemove(copy, x, y, m, n);
    int move_num = moves(copy, m, n);
    // if 0, this is deadend already. Go back.
    if (possible == 0){
        int all_moves = moves(copy, m, n);
        if (all_moves>max_square){
            pthread_mutex_lock(&mutex);
                max_square=all_moves;
            pthread_mutex_unlock(&mutex);
        }

        int isittour = tour(copy, m, n);
        if (isittour){
            printf("THREAD %ld: Sonny found a full knight's tour!\n", mytid);
        }
        else{
        printf("THREAD %ld: Dead end after move #%d\n", mytid, all_moves);
        if (dead == 0){
            deadends = calloc(1, sizeof(char**));
            deadends[0] = calloc(m, sizeof(char*));
            for (int i=0; i<m; i++){
                deadends[0][i] = calloc(n, sizeof(char));
            }
            for (int i = 0; i<m; i++){
                for (int j = 0; j<n; j++){
                    deadends[0][i][j] = copy[i][j];
                }
            }
        }
        else{
            deadends = realloc(deadends, (dead+1)*sizeof(char**));
            deadends[dead] = calloc(m, sizeof(char*));
            for (int i=0; i<m; i++){
                deadends[dead][i] = calloc(n, sizeof(char));
            }
            for (int i = 0; i<m; i++){
                for (int j = 0; j<n; j++){
                deadends[dead][i][j] = copy[i][j];
                }
            }
        }
        dead ++;
        }

        val * return_value = malloc(sizeof(val *));
        return_value->th = mytid;
        return_value->move_ = all_moves;
        // return values to parent thread
        for (int c = 0; c<m; c++){
            free(copy[c]);
        }
        free(copy);
        free(s);
        pthread_exit(return_value);
    }

    // make the next move
    else if (possible == 1){
        int * update = calloc(2, sizeof(int));
        nextmove(copy, x, y, m, n, update);
        x = update[0];
        y = update[1];

        int current = possible;
        while (current ==1){
            current = possiblemove(copy, x, y, m, n);
            if (current == 0){break;}
            else if (current == 1){
                nextmove(copy, x, y, m, n, update);
                x = update[0];
                y = update[1];
            }
            else if (current > 1) {break;}
        }


        if (current == 0){ 
            int all_moves = moves(copy, m, n);
            if (all_moves>max_square){max_square=all_moves;}

            int isittour = tour(copy, m, n);
            if (isittour){
                printf("THREAD %ld: Sonny found a full knight's tour!\n", mytid);
            }
            else{
                printf("THREAD %ld: Dead end after move #%d\n", mytid, all_moves);
                if (dead == 0){
                    deadends = calloc(1, sizeof(char**));
                    deadends[0] = calloc(m, sizeof(char*));
                    for (int i=0; i<m; i++){
                        deadends[0][i] = calloc(n, sizeof(char));
                    }
                    for (int i = 0; i<m; i++){
                        for (int j = 0; j<n; j++){
                            deadends[0][i][j] = copy[i][j];
                        }
                    }
                 }
                else{
                    deadends = realloc(deadends, (dead+1)*sizeof(char**));
                    deadends[dead] = calloc(m, sizeof(char*));
                    for (int i=0; i<m; i++){
                        deadends[dead][i] = calloc(n, sizeof(char));
                    }
                    for (int i = 0; i<m; i++){
                        for (int j = 0; j<n; j++){
                            deadends[dead][i][j] = copy[i][j];
                        }
                    }
                }
            dead ++;
        }
            val * return_value = malloc(sizeof(val *));
            return_value->th = mytid;
            return_value->move_ = all_moves;
            for (int c = 0; c<m; c++){
                free(copy[c]);
            }
            free(copy);
            free(s);
            pthread_exit(return_value);
        }


        else if(current > 1){
            int move_num1 = moves(copy, m, n);
            printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", mytid, current, move_num1);
            pthread_t * threads = calloc(8, sizeof(pthread_t));
            int counter = 0;
            if (x -1 >= 0 && y-2 >= 0 && copy[x-1][y-2] == '\0'){
                pthread_t thread;
                data_t * data = malloc(sizeof(data_t));
                data->current_x=x-1;
                data->current_y=y-2;
                data->m_ = m;
                data->n_ = n;
                char **tmp = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp[i][j] = copy[i][j];
                    }
                }
                tmp[x-1][y-2]='X';
                data->b_ = tmp;

                rc = pthread_create( &thread, NULL, move, data);
                threads[counter] = thread;
                counter++;

                #ifdef NO_PARALLEL
                val * r;
                z= pthread_join(thread, (void **)&r);
                z++;
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r->th, r->move_);
                if(r->move_ >= die){die = r->move_;}
                free(r);                
                #endif
            }
            if (x-2 >= 0 && y-1 >= 0  && copy[x-2][y-1] == '\0'){
                pthread_t thread1;
                data_t * data1 = malloc(sizeof(data_t));
                data1->current_x=x-2;
                data1->current_y=y-1;
                data1->m_ = m;
                data1->n_ = n;
                char **tmp1 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp1[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp1[i][j] = copy[i][j];
                    }
                }
                tmp1[x-2][y-1]='X';
                data1->b_ = tmp1;
                
                rc = pthread_create( &thread1, NULL, move, data1);
                threads[counter] = thread1;
                counter++;
                #ifdef NO_PARALLEL
                val * r1;
                z= pthread_join(thread1, (void **)&r1);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r1->th, r1->move_);
                if(r1->move_ >= die){die = r1->move_;}
                free(r1);
                #endif
            }
            if (x-2 >= 0 && y+1 < n && copy[x-2][y+1] == '\0'){
                pthread_t thread2;
                data_t * data2 = malloc(sizeof(data_t));
                data2->current_x=x-2;
                data2->current_y=y+1;
                data2->m_ = m;
                data2->n_ = n;
                char **tmp2 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp2[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp2[i][j] = copy[i][j];
                    }
                }
                tmp2[x-2][y+1]='X';
                data2->b_ = tmp2;
               
                rc = pthread_create( &thread2, NULL, move, data2);
                threads[counter] = thread2;
                counter++;
                #ifdef NO_PARALLEL
                val * r2;
                z= pthread_join(thread2, (void **)&r2);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r2->th, r2->move_);
                if(r2->move_ >= die){die = r2->move_;}
                free(r2);               
                #endif
            }
            if (x-1 >= 0 && y+2 < n && copy[x-1][y+2] == '\0'){
                pthread_t thread3;
                data_t * data3 = malloc(sizeof(data_t));
                data3->current_x=x-1;
                data3->current_y=y+2;
                data3->m_ = m;
                data3->n_ = n;
                char **tmp3 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp3[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp3[i][j] = copy[i][j];
                    }
                }
                tmp3[x-1][y+2]='X';
                data3->b_ = tmp3;
                
                rc = pthread_create( &thread3, NULL, move, data3);
                threads[counter] = thread3;
                counter++;
                #ifdef NO_PARALLEL
                val * r3;
                z= pthread_join(thread3, (void **)&r3);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r3->th, r3->move_);
                if(r3->move_ >= die){die = r3->move_;}
                free(r3);
                #endif
            }
            if (x+1 < m && y+2 < n && copy[x+1][y+2] == '\0'){
                pthread_t thread4;
                data_t * data4 = malloc(sizeof(data_t));
                data4->current_x=x+1;
                data4->current_y=y+2;
                data4->m_ = m;
                data4->n_ = n;
                char **tmp4 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp4[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp4[i][j] = copy[i][j];
                    }
                }
                tmp4[x+1][y+2]='X';
                data4->b_ = tmp4;
                
                rc = pthread_create( &thread4, NULL, move, data4);
                threads[counter] = thread4;
                counter++;
                #ifdef NO_PARALLEL
                val * r4;
                z= pthread_join(thread4, (void **)&r4);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r4->th, r4->move_);
                if(r4->move_ >= die){die = r4->move_;}
                free(r4);
                #endif            
            }
            if (x+2 < m && y+1 < n && copy[x+2][y+1] == '\0'){
                pthread_t thread5;
                data_t * data5 = malloc(sizeof(data_t));
                data5->current_x=x+2;
                data5->current_y=y+1;
                data5->m_ = m;
                data5->n_ = n;
                char **tmp5 = calloc(m, sizeof(char*));
                for (int q= 0; q<m; q++){
                    tmp5[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp5[i][j] = copy[i][j];
                    }
                }
                tmp5[x+2][y+1]='X';
                data5->b_ = tmp5;
                
                rc = pthread_create( &thread5, NULL, move, data5);
                threads[counter] = thread5;
                counter++;

                #ifdef NO_PARALLEL
                val * r5;
                z= pthread_join(thread5, (void **)&r5);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r5->th, r5->move_);
                if(r5->move_ >= die){die = r5->move_;}
                free(r5);         
                #endif  
            }
            if (x+2 < m && y-1 >= 0 && copy[x+2][y-1] == '\0'){
                pthread_t thread6;
                data_t * data6 = malloc(sizeof(data_t));
                data6->current_x=x+2;
                data6->current_y=y-1;
                data6->m_ = m;
                data6->n_ = n;
                char **tmp6 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp6[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp6[i][j] = copy[i][j];
                    }
                }
                tmp6[x+2][y-1]='X';
                data6->b_ = tmp6;
               
                rc = pthread_create( &thread6, NULL, move, data6);
                threads[counter] = thread6;
                counter++;
                #ifdef NO_PARALLEL
                val * r6;
                z= pthread_join(thread6, (void **)&r6);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r6->th, r6->move_);
                if(r6->move_ >= die){die = r6->move_;}
                free(r6);
                #endif 
            }
            if (x+1 < m && y-2 >= 0 && copy[x+1][y-2] == '\0'){
                pthread_t thread7;
                data_t * data7 = malloc(sizeof(data_t));
                data7->current_x=x+1;
                data7->current_y=y-2;
                data7->m_ = m;
                data7->n_ = n;
                char **tmp7 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp7[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp7[i][j] = copy[i][j];
                    }
                }
                tmp7[x+1][y-2]='X';
                data7->b_ = tmp7;
               
                rc = pthread_create( &thread7, NULL, move, data7);
                threads[counter] = thread7;
                counter++;
                #ifdef NO_PARALLEL
                val * r7;
                 z= pthread_join(thread7, (void **)&r7);
                 printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r7->th, r7->move_);
                if(r7->move_ >= die){die = r7->move_;}
                 free(r7);
                #endif 
            }
            #ifndef NO_PARALLEL
                for (int h = 0; h<8; h++){
                    if (threads[h] == '\0'){
                        break;
                    }
                    val * rr;
                    z= pthread_join(threads[h], (void **)&rr);
                    printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, rr->th, rr->move_);
                    if(rr->move_ >= die){die = rr->move_;}
                    free(rr);
                } 
            #endif
            free(threads);
        }
        free(update);
    }

    else{
        printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", mytid, possible, move_num);
        pthread_t * threads = calloc(8, sizeof(pthread_t));
        int counter = 0;
        if (x -1 >= 0 && y-2 >= 0 && copy[x-1][y-2] == '\0'){
                pthread_t thread;
                data_t * data = malloc(sizeof(data_t));
                data->current_x=x-1;
                data->current_y=y-2;
                data->m_ = m;
                data->n_ = n;
                char **tmp = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp[i][j] = copy[i][j];
                    }
                }
                tmp[x-1][y-2]='X';
                data->b_ = tmp;
               
                rc = pthread_create( &thread, NULL, move, data);
                threads[counter] = thread;
                counter++;
                #ifdef NO_PARALLEL
                val * r;
                z= pthread_join(thread, (void **)&r);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r->th, r->move_);
                if(r->move_ >= die){die = r->move_;}
                free(r);                
                #endif
        }
        if (x-2 >= 0 && y-1 >= 0  && copy[x-2][y-1] == '\0'){
                pthread_t thread1;
                data_t * data1 = malloc(sizeof(data_t));
                data1->current_x=x-2;
                data1->current_y=y-1;
                data1->m_ = m;
                data1->n_ = n;
                char **tmp1 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp1[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp1[i][j] = copy[i][j];
                    }
                }
                tmp1[x-2][y-1]='X';
                data1->b_ = tmp1;
                
                rc = pthread_create( &thread1, NULL, move, data1);
                threads[counter] = thread1;
                counter++;

                #ifdef NO_PARALLEL
                val * r1;                
                z= pthread_join(thread1, (void **)&r1);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r1->th, r1->move_);
                if(r1->move_ >= die){die = r1->move_;}
                free(r1);
                #endif
        }
        if (x-2 >= 0 && y+1 < n && copy[x-2][y+1] == '\0'){
                pthread_t thread2;
                data_t * data2 = malloc(sizeof(data_t));
                data2->current_x=x-2;
                data2->current_y=y+1;
                data2->m_ = m;
                data2->n_ = n;
                char **tmp2 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp2[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp2[i][j] = copy[i][j];
                    }
                }
                tmp2[x-2][y+1]='X';
                data2->b_ = tmp2;
                
                rc = pthread_create( &thread2, NULL, move, data2);
                threads[counter] = thread2;
                counter++;
                #ifdef NO_PARALLEL
                val * r2;
                z= pthread_join(thread2, (void **)&r2);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r2->th, r2->move_);
                if(r2->move_ >= die){die = r2->move_;}
                free(r2);               
                #endif
        }
        if (x-1 >= 0 && y+2 < n && copy[x-1][y+2] == '\0'){
                pthread_t thread3;
                data_t * data3 = malloc(sizeof(data_t));
                data3->current_x=x-1;
                data3->current_y=y+2;
                data3->m_ = m;
                data3->n_ = n;
                char **tmp3 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp3[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp3[i][j] = copy[i][j];
                    }
                }
                tmp3[x-1][y+2]='X';
                data3->b_ = tmp3;
                
                rc = pthread_create( &thread3, NULL, move, data3);
                threads[counter] = thread3;
                counter++;

                #ifdef NO_PARALLEL
                val * r3;
                z= pthread_join(thread3, (void **)&r3);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r3->th, r3->move_);
                if(r3->move_ >= die){die = r3->move_;}
                free(r3);
                #endif
        }
        if (x+1 < m && y+2 < n && copy[x+1][y+2] == '\0'){
                pthread_t thread4;
                data_t * data4 = malloc(sizeof(data_t));
                data4->current_x=x+1;
                data4->current_y=y+2;
                data4->m_ = m;
                data4->n_ = n;
                char **tmp4 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp4[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp4[i][j] = copy[i][j];
                    }
                }
                tmp4[x+1][y+2]='X';
                data4->b_ = tmp4;
                
                rc = pthread_create( &thread4, NULL, move, data4);
                threads[counter] = thread4;
                counter++;

                #ifdef NO_PARALLEL
                val * r4;
                z =pthread_join(thread4, (void **)&r4);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r4->th, r4->move_);
                if(r4->move_ >= die){die = r4->move_;}
                free(r4);

                #endif            
        }
        if (x+2 < m && y+1 < n && copy[x+2][y+1] == '\0'){
                pthread_t thread5;
                data_t * data5 = malloc(sizeof(data_t));
                data5->current_x=x+2;
                data5->current_y=y+1;
                data5->m_ = m;
                data5->n_ = n;
                char **tmp5 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp5[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp5[i][j] = copy[i][j];
                    }
                }
                tmp5[x+2][y+1]='X';
                data5->b_ = tmp5;
                
                rc = pthread_create( &thread5, NULL, move, data5);
                threads[counter] = thread5;
                counter++;
                #ifdef NO_PARALLEL
                val * r5;
                z= pthread_join(thread5, (void **)&r5);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r5->th, r5->move_);
                if(r5->move_ >= die){die = r5->move_;}
                free(r5);         
                #endif  
        }
        if (x+2 < m && y-1 >= 0 && copy[x+2][y-1] == '\0'){
                pthread_t thread6;
                data_t * data6 = malloc(sizeof(data_t));
                data6->current_x=x+2;
                data6->current_y=y-1;
                data6->m_ = m;
                data6->n_ = n;
                char **tmp6 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp6[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp6[i][j] = copy[i][j];
                    }
                }
                tmp6[x+2][y-1]='X';
                data6->b_ = tmp6;
                
                rc = pthread_create( &thread6, NULL, move, data6);
                threads[counter] = thread6;
                counter++;
                #ifdef NO_PARALLEL
                val * r6;
                z= pthread_join(thread6, (void **)&r6);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r6->th, r6->move_);
                if(r6->move_ >= die){die = r6->move_;}
                free(r6);
                #endif 
        }
       if (x+1 < m && y-2 >= 0 && copy[x+1][y-2] == '\0'){
                pthread_t thread7;
                data_t * data7 = malloc(sizeof(data_t));
                data7->current_x=x+1;
                data7->current_y=y-2;
                data7->m_ = m;
                data7->n_ = n;
                char **tmp7 = calloc(m, sizeof(char*));
                for (int q = 0; q<m; q++){
                    tmp7[q] = calloc(n, sizeof(char));
                }
                for (int i = 0; i< m; i++){
                    for (int j =0; j<n; j++){
                        tmp7[i][j] = copy[i][j];
                    }
                }
                tmp7[x+1][y-2]='X';
                data7->b_ = tmp7;

                rc = pthread_create( &thread7, NULL, move, data7);
                threads[counter] = thread7;
                counter++;
                #ifdef NO_PARALLEL
                val * r7;
                z= pthread_join(thread7, (void **)&r7);
                printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, r7->th, r7->move_);
                if(r7->move_ >= die){die = r7->move_;}
                free(r7);
                #endif 
        }
        #ifndef NO_PARALLEL
        for (int h = 0; h<8; h++){
            if(threads[h] == '\0'){
                break;
            }
            val * rr;
            z= pthread_join(threads[h], (void **)&rr);
            printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", mytid, rr->th, rr->move_);
            if(rr->move_ >= die){die = rr->move_;}
            free(rr);
        } 
        #endif
        free(threads);
    }

    val * finished_all_child = malloc(sizeof(val *));
    finished_all_child->th = mytid;
    finished_all_child->move_ = die;
        // return values to parent thread
    free(s);
    pthread_exit(finished_all_child);
}


int main(int argc, char** argv) {
    // error checking
    if (argc < 3 && argc < 4){
      fprintf(stderr,"ERROR: Invalid argument(s)\n");
      fprintf(stderr,"USAGE: a.out <m> <n> [<x>]\n");
      return EXIT_FAILURE;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);

    if (m < 3 || n < 3){
        fprintf(stderr,"ERROR: Invalid argument(s)\n");
        fprintf(stderr,"USAGE: a.out <m> <n> [<x>]\n");
        return EXIT_FAILURE;
    }

    int rc=0;
    pthread_t tid1;

    int num = 0;
    if (argc == 4){
        num = atoi(argv[3]);
        if (num > m*n){
            fprintf(stderr,"ERROR: Invalid argument(s)\n");
            fprintf(stderr,"USAGE: a.out <m> <n> [<x>]\n");
            return EXIT_FAILURE;
        }
    }
    // allocate dynamic array of size m*n
    // first possibility

    char ** board = calloc(m, sizeof(char*));
    for (int x = 0; x<m; x++){
        board[x] = calloc(n, sizeof(char));
    }

    board[0][0] = 'X';
    data_t * data = malloc(sizeof(data_t));
    data->b_ = board;
    data->current_x = 0;
    data->current_y = 0;
    data->m_ = m;
    data->n_ = n;

    rc = pthread_create( &tid1, NULL, move, data);
    if ( rc != 0 )
      {
        fprintf( stderr, "ERROR: pthread_create() failed (%d): %s\n",
                 rc, strerror( rc ) );
        return EXIT_FAILURE;
    }

    #ifdef NO_PARALLEL
    val * first;
    rc = pthread_join(tid1, (void **)&first);
       // printf("THREAD %ld: Thread [%ld] joined (return %d)\n", me, first->th, first->move_);
        pthread_t me = first->th;
        free(first);
    #endif

    #ifndef NO_PARALLEL
    val * first;
    rc = pthread_join(tid1, (void **)&first);
       // printf("THREAD %ld: Thread [%ld] joined (return %d)\n", me, first->th, first->move_);
    pthread_t me = first->th;
    free(first);

    #endif

    printf("THREAD %ld: Best solution(s) found visit %d squares (out of %d)\n", me, max_square, m*n);
    printf("THREAD %ld: Dead end boards:\n", me);
    if (argc == 4){
        printboard(deadends, me, num, m, n);
    }
    else{
        printboard(deadends, me, 0, m, n);
    }

    if (deadends != NULL){
        for (int i = 0; i< dead; i++){
            for (int j = 0; j<m; j++ ){
                free(deadends[i][j]);
            }
            free(deadends[i]);
        }
        free(deadends);
    }
    return EXIT_SUCCESS;
}