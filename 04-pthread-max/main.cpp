#include <vector>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

pthread_cond_t cond;
pthread_mutex_t cond_mutex;
int thread_counter = 0;
pthread_mutex_t counter_mutex;
sem_t broadcast_sem;
bool ready = false;

void *bidon(void *arg)
{
    (void) arg;
    pthread_mutex_lock(&counter_mutex);
    --thread_counter;
    if (thread_counter <= 0) {
        sem_post(&broadcast_sem);
    }
    pthread_mutex_unlock(&counter_mutex);
    pthread_mutex_lock(&cond_mutex);
    while(!ready) {
        pthread_cond_wait(&cond, &cond_mutex);
    }
    pthread_mutex_unlock(&cond_mutex);
    return 0;
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&cond_mutex, NULL);
    pthread_mutex_init(&counter_mutex, NULL);
    sem_init(&broadcast_sem, 0, 0);

    std::vector<pthread_t> threads;

    pthread_mutex_lock(&counter_mutex);
    for (;;) {
        pthread_t t;
        int ret = pthread_create(&t, NULL, bidon, NULL);
        if (ret == 0) {
            threads.push_back(t);
        } else {
            std::cout << "max thread" << threads.size();
            break;
        }
    }
    thread_counter = threads.size();
    pthread_mutex_unlock(&counter_mutex);

    sem_wait(&broadcast_sem);

    pthread_mutex_lock(&cond_mutex);
    ready = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&cond_mutex);
    for (pthread_t thread : threads) {
        pthread_join(thread, NULL);
    }

    pthread_mutex_destroy(&cond_mutex);
    pthread_mutex_destroy(&counter_mutex);
    sem_destroy(&broadcast_sem);
    pthread_cond_destroy(&cond);
    return 0;
}
