#include "cacti.h"
#include "err.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#define ACTORS_STARTING_COUNT 256
#define ALLOC_MULTIPLIER 2

#define SYSTEM_CREATE_FAIL (-42)

#define MESSAGE_SENT 0
#define ACTOR_IS_DEAD (-1)
#define ACTOR_DOES_NOT_EXIST (-2)
#define SENDING_MESSAGE_ERROR (-3)

static inline void mutex_init(pthread_mutex_t *mutex) {
    if (pthread_mutex_init(mutex, NULL) != 0) {
        syserr("Mutex init error.\n");
    }
}

static inline void mutex_lock(pthread_mutex_t *mutex) {
    if (pthread_mutex_lock(mutex) != 0) {
        syserr("Mutex locking error.\n");
    }
}

static inline void mutex_unlock(pthread_mutex_t *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) {
        syserr("Mutex unlocking error.\n");
    }
}

static inline void mutex_destroy(pthread_mutex_t *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) {
        syserr("Mutex destroying error.\n");
    }
}

static inline void cond_init(pthread_cond_t *cond) {
    if (pthread_cond_init(cond, NULL) != 0) {
        syserr("Cond init error.\n");
    }
}

static inline void cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    if (pthread_cond_wait(cond, mutex) != 0) {
        syserr("Cond wait error.\n");
    }
}

static inline void cond_signal(pthread_cond_t *cond) {
    if (pthread_cond_signal(cond) != 0) {
        syserr("Cond signal error.\n");
    }
}

static inline void cond_broadcast(pthread_cond_t *cond) {
    if (pthread_cond_broadcast(cond) != 0) {
        syserr("Cond broadcast error.\n");
    }
}

static inline void cond_destroy(pthread_cond_t *cond) {
    if (pthread_cond_destroy(cond) != 0) {
        syserr("Cond destroying error.\n");
    }
}

typedef struct {
    void **arr;
    uint first, last;
    uint curr_size;
    size_t elements_count;
} queue_t;

static queue_t *queue_create() {
    queue_t *new_queue = malloc(sizeof(queue_t));
    if (new_queue == NULL) {
        syserr("Malloc failed.\n");
    }

    new_queue->first = new_queue->last = 0;
    // According to moodle forum, reserving such space is ok.
    // The queue won't be reallocated in case of actor queue
    // and we have only one other queue.
    new_queue->curr_size = ACTOR_QUEUE_LIMIT;
    new_queue->arr = malloc(ACTOR_QUEUE_LIMIT * sizeof(void *));
    new_queue->elements_count = 0;

    if (new_queue->arr == NULL) {
        syserr("Malloc failed.\n");
    }

    return new_queue;
}

static inline void queue_destroy(queue_t **q) {
    free((*q)->arr);
    free(*q);
    *q = NULL;
}

static inline bool queue_is_full(queue_t *q) {
    return q->elements_count == q->curr_size;
}

static inline bool queue_is_empty(queue_t *q) {
    return q->elements_count == 0;
}

static void queue_resize(queue_t *q) {
    q->arr = realloc(q->arr, ALLOC_MULTIPLIER * q->curr_size * sizeof(void *));
    if (q->arr == NULL) {
        syserr("Realloc failed.\n");
    }
    uint old_size = q->curr_size;
    q->curr_size *= ALLOC_MULTIPLIER;

    if (q->first > q->last) {
        uint how_many_to_move = old_size - q->first;
        for (uint i = 0; i < how_many_to_move; i++) {
            q->arr[q->curr_size - 1 - i] = q->arr[old_size - 1 - i];
        }
        q->first = q->curr_size - how_many_to_move;
    }
}

static void queue_push(queue_t *q, void *new_el) {
    if (q == NULL) {
        return;
    } else if (queue_is_full(q)) {
        queue_resize(q);
    }

    if (q->elements_count > 0) {
        q->last = (q->last + 1) % q->curr_size;
    }
    q->arr[q->last] = new_el;
    q->elements_count += 1;
}

// Note: queue_is_empty() should be called first.
static void *queue_pop(queue_t *q) {
    void *ret_val = q->arr[q->first];
    if (q->elements_count > 1) {
        q->first = (q->first + 1) % q->curr_size;
    }
    q->elements_count -= 1;

    return ret_val;
}

typedef struct {
    void *state_ptr;
    role_t *role;
    queue_t *queue;
    pthread_mutex_t mutex;
    bool is_waiting_for_work;
    bool is_dead;
} actor_info_t;

actor_info_t *actors_info = NULL;

pthread_mutex_t actors_mutex;
size_t actors_curr_size = 0;
actor_id_t actors_new_id = 0;
size_t actors_active = 0;
size_t actors_dead = 0;

pthread_t *active_threads;

pthread_cond_t waiting_for_system_end;
bool is_finished = false;
size_t how_many_joined = 0;

static void free_threads() {
    for (int i = 0; i < POOL_SIZE; i++) {
        if (pthread_join(active_threads[i], NULL)) {
            syserr("Thread join failed.\n");
        }
    }
    free(active_threads);
}

_Thread_local actor_id_t curr_actor_id = -1;

static inline bool does_actor_exist(actor_id_t id) {
    return id < actors_new_id && id >= 0;
}

static void destroy_actor_queue(actor_info_t *actor) {
    while (!queue_is_empty(actor->queue)) {
        free(queue_pop(actor->queue));
    }
    queue_destroy(&actor->queue);
}

typedef struct {
    queue_t *queue;
    pthread_mutex_t mutex;
    pthread_cond_t waiting_for_work;
    size_t working_threads_count;
    size_t threads_count;
    bool is_end;
} pool_t;

pool_t *pool = NULL;

static void finish_working() {
    // All actors should by dead by now.
    // We need to free the memory and destroy the pool.
    // Also we should destroy all mutexes.
    for (int i = 0; i < actors_new_id; i++) {
        mutex_destroy(&actors_info[i].mutex);
    }
    free(actors_info);
    actors_info = NULL;
    actors_new_id = 0;

    mutex_lock(&pool->mutex);
    pool->is_end = true;
    cond_broadcast(&pool->waiting_for_work);
    mutex_unlock(&pool->mutex);
}

static void pool_add_waiting_actor(actor_id_t *id_ptr) {
    mutex_lock(&pool->mutex);

    queue_push(pool->queue, id_ptr);
    cond_signal(&pool->waiting_for_work);

    mutex_unlock(&pool->mutex);
}

static void actor_go_die(actor_info_t *actor) {
    mutex_lock(&actor->mutex);
    destroy_actor_queue(actor);
    actor->is_dead = true;
    mutex_unlock(&actor->mutex);

    mutex_lock(&actors_mutex);
    actors_active--;
    actors_dead++;

    if (actors_active == 0) {
        mutex_unlock(&actors_mutex);
        finish_working();
    } else {
        mutex_unlock(&actors_mutex);
    }
}

static actor_id_t actor_create_new(role_t *role) {
    mutex_lock(&actors_mutex);
    if ((size_t) actors_new_id == actors_curr_size) {
        actors_curr_size *= ALLOC_MULTIPLIER;
        if (actors_curr_size > CAST_LIMIT) {
            actors_curr_size = CAST_LIMIT;
        }

        actors_info = realloc(actors_info, actors_curr_size);
        if (actors_info == NULL) {
            syserr("Realloc failed.\n");
        }
    }

    actors_active++;
    actor_id_t new_id = actors_new_id;
    actors_new_id++;
    mutex_unlock(&actors_mutex);

    actor_info_t *new_act = &actors_info[new_id];

    new_act->role = role;
    new_act->is_dead = false;
    mutex_init(&new_act->mutex);
    new_act->state_ptr = NULL;
    new_act->is_waiting_for_work = false;
    new_act->queue = queue_create();

    return new_id;
}

static void actor_spawn(message_t *message) {
    if (actors_new_id < CAST_LIMIT) {
        actor_id_t new_id;
        new_id = actor_create_new(message->data);

        message_t new_message;
        new_message.message_type = MSG_HELLO;
        new_message.nbytes = sizeof(void *);
        new_message.data = (void *) actor_id_self();

        send_message(new_id, new_message);
    }
}

static void actor_work(actor_info_t *actor, message_t *message) {
    switch (message->message_type) {
        case MSG_SPAWN:
            actor_spawn(message);
            break;
        case MSG_GODIE:
            actor_go_die(actor);
            break;
        default:
            // Messages outside of roles are not pushed on the queue.
            (actor->role->prompts)[message->message_type](&actor->state_ptr,
                                                          message->nbytes,
                                                          message->data);
            break;
    }
}

static void pool_actor_work(actor_id_t *id) {
    actor_info_t *curr_act = &actors_info[*id];
    mutex_lock(&curr_act->mutex);
    curr_act->is_waiting_for_work = false;

    if (!curr_act->is_dead) {
        message_t *message = (message_t *) queue_pop(curr_act->queue);
        mutex_unlock(&curr_act->mutex);

        actor_work(curr_act, message);
        mutex_lock(&actors_mutex);
        free(message);
        if (actors_active == 0 || curr_act->is_dead) {
            free(id);
            mutex_unlock(&actors_mutex);
            return;
        } else {
            mutex_unlock(&actors_mutex);
        }

        mutex_lock(&curr_act->mutex);
        // If someone added a message to the queue between taking from it the first one,
        // it is possible that this actor is ready to work, but not queued.
        if (!curr_act->is_waiting_for_work
            && !queue_is_empty(curr_act->queue)) {
            curr_act->is_waiting_for_work = true;
            pool_add_waiting_actor(id);
        } else {
            free(id);
        }
    }

    mutex_unlock(&curr_act->mutex);
}

static bool pool_process_work(actor_id_t *curr_id) {
    // Someone may add a message to actor's queue now.
    pool_actor_work(curr_id);
    if (actors_active == 0) {
        return false;
    }

    mutex_lock(&pool->mutex);
    pool->working_threads_count -= 1;
    curr_actor_id = -1;
    mutex_unlock(&pool->mutex);

    return true;
}

static void pool_destroy() {
    if (pool != NULL) {
        while (!queue_is_empty(pool->queue)) {
            free(queue_pop(pool->queue));
        }
        queue_destroy(&pool->queue);

        mutex_destroy(&pool->mutex);
        cond_destroy(&pool->waiting_for_work);

        free(pool);
        pool = NULL;
    }
}

static void pool_finish_working() {
    pool->threads_count -= 1;
    if (pool->threads_count == 0) {
        mutex_unlock(&pool->mutex);
        pool_destroy();
        mutex_lock(&actors_mutex);
        is_finished = true;
        if (how_many_joined > 0) {
            cond_broadcast(&waiting_for_system_end);
            mutex_unlock(&actors_mutex);
        } else {
            cond_destroy(&waiting_for_system_end);
            mutex_unlock(&actors_mutex);
            mutex_destroy(&actors_mutex);
        }
    } else {
        mutex_unlock(&pool->mutex);
    }
}

static void *pool_worker() {
    curr_actor_id = -1;

    while (true) {
        mutex_lock(&pool->mutex);

        while (queue_is_empty(pool->queue) && !pool->is_end) {
            cond_wait(&pool->waiting_for_work, &pool->mutex);
        }

        if (pool->is_end) {
            break;
        } else {
            actor_id_t *curr_id = (actor_id_t *) queue_pop(pool->queue);
            pool->working_threads_count += 1;
            curr_actor_id = *curr_id;
            mutex_unlock(&pool->mutex);
            if (!pool_process_work(curr_id)) {
                continue;
            }
        }
    }

    // If thread leaves the loop, it owns the semaphore.
    pool_finish_working();

    return NULL;
}

static void pool_create() {
    if (pool == NULL) {
        pool = malloc(sizeof(pool_t));
        if (pool == NULL) {
            syserr("Malloc failed.\n");
        }

        active_threads = malloc(POOL_SIZE * sizeof(pthread_t));

        pool->queue = queue_create();

        mutex_init(&pool->mutex);
        cond_init(&pool->waiting_for_work);

        pool->threads_count = POOL_SIZE;
        pool->working_threads_count = 0;
        pool->is_end = false;

        for (int i = 0; i < POOL_SIZE; i++) {
            if (pthread_create(&active_threads[i], NULL, pool_worker, NULL) != 0) {
                syserr("Creating a new thread failed.\n");
            }
        }
    }
}

actor_id_t actor_id_self() {
    return curr_actor_id;
}

int actor_system_create(actor_id_t *actor, role_t *const role) {
    actors_info = malloc(ACTORS_STARTING_COUNT * sizeof(actor_info_t));
    if (actors_info == NULL) {
        return SYSTEM_CREATE_FAIL;
    }

    if (pthread_mutex_init(&actors_mutex, NULL) != 0) {
        free(actors_info);
        actors_info = NULL;
        return SYSTEM_CREATE_FAIL;
    }

    if (pthread_cond_init(&waiting_for_system_end, NULL) != 0) {
        free(actors_info);
        actors_info = NULL;
        mutex_destroy(&actors_mutex);
        return SYSTEM_CREATE_FAIL;
    }
    actors_curr_size = ACTORS_STARTING_COUNT;
    actors_new_id = 0;
    actors_active = 0;
    actors_dead = 0;

    is_finished = false;
    how_many_joined = 0;

    *actor = actor_create_new(role);

    pool_create();

    message_t new_msg;
    new_msg.message_type = MSG_HELLO;
    new_msg.nbytes = 0;
    new_msg.data = NULL;
    send_message(*actor, new_msg);

    return 0;
}

void actor_system_join(actor_id_t actor) {
    if (does_actor_exist(actor)) {
        mutex_lock(&actors_mutex);
        how_many_joined++;

        while (!is_finished) {
            cond_wait(&waiting_for_system_end, &actors_mutex);
        }
        how_many_joined--;
        if (how_many_joined == 0) {
            cond_destroy(&waiting_for_system_end);
            mutex_unlock(&actors_mutex);
            mutex_destroy(&actors_mutex);
            free_threads();
        } else {
            mutex_unlock(&actors_mutex);
        }
    }
}

static bool add_to_actor_queue(actor_id_t id, void *new_el) {
    if (actors_info[id].queue->elements_count < ACTOR_QUEUE_LIMIT) {
        queue_push(actors_info[id].queue, new_el);
        return true;
    } else {
        return false;
    }
}

int send_message(actor_id_t actor, message_t message) {
    if (!does_actor_exist(actor)) {
        return ACTOR_DOES_NOT_EXIST;
    } else if (actors_active == 0) {
        return SENDING_MESSAGE_ERROR;
    } else {
        actor_info_t *curr_act = &actors_info[actor];
        mutex_lock(&curr_act->mutex);
        if (curr_act->is_dead) {
            mutex_unlock(&curr_act->mutex);
            return ACTOR_IS_DEAD;
        } else {
            message_t *new_msg = malloc(sizeof(message_t));
            if (new_msg == NULL) {
                syserr("Malloc failed\n");
            }

            new_msg->message_type = message.message_type;
            new_msg->nbytes = message.nbytes;
            new_msg->data = message.data;

            if (add_to_actor_queue(actor, new_msg)) {
                if (!curr_act->is_waiting_for_work) {
                    actor_id_t *new_id = malloc(sizeof(actors_new_id));
                    if (new_id == NULL) {
                        syserr("Malloc failed.\n");
                    }

                    *new_id = actor;
                    pool_add_waiting_actor(new_id);
                    curr_act->is_waiting_for_work = true;
                }
                mutex_unlock(&curr_act->mutex);
                return MESSAGE_SENT;
            } else {
                mutex_unlock(&curr_act->mutex);
                free(new_msg);
                return SENDING_MESSAGE_ERROR;
            }
        }
    }
}
