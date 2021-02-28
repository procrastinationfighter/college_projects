#include "cacti.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define MSG_CREATE_ACT 1
#define MSG_GIVE_ID_AND_SEND 2
#define MSG_SUM_ROWS 3
#define MSG_COUNT 4
#define MSG_FREE 5

#define MICROSECONDS_IN_MILlISECOND 1000

#define NOT_USED(x) (void)(x)

role_t matrix_role;

typedef struct {
    long column;
    bool is_first;
    actor_id_t next_id;
    void *temp_create_state;
    void *temp_adding_state;
} actor_state;

typedef struct {
    actor_id_t first_id;
    int how_many_to_create;
    int column;
} create_state;

typedef struct {
    long **numbers;
    long **times;
    int rows_count;
    int columns_count;
    long sum;
    int row_no;
} adding_state;

message_t create_message(message_type_t type, size_t nbytes, void *data) {
    message_t mess;
    mess.message_type = type;
    mess.nbytes = nbytes;
    mess.data = data;
    return mess;
}

void hello(void **stateptr, size_t nbytes, void *data) {
    actor_state *act_state = malloc(sizeof(actor_state));
    if (act_state == NULL) {
        fprintf(stderr, "Malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    if (nbytes == 0) {
        act_state->is_first = true;
    } else {
        act_state->is_first = false;
        send_message((actor_id_t) data,
                     create_message(MSG_GIVE_ID_AND_SEND, sizeof(actor_id_t), (void *) actor_id_self()));
    }

    act_state->temp_adding_state = act_state->temp_create_state = NULL;

    *stateptr = act_state;
}

void create_actors(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(nbytes);
    actor_state *my_state = *stateptr;
    create_state *create_st = data;

    my_state->column = create_st->column;

    create_st->column += 1;

    if (create_st->column < create_st->how_many_to_create) {
        send_message(actor_id_self(),
                     create_message(MSG_SPAWN, matrix_role.nprompts, &matrix_role));
        my_state->temp_create_state = create_st;
    } else {
        my_state->next_id = create_st->first_id;
        // All actors created. Start adding.
        send_message(create_st->first_id,
                     create_message(MSG_SUM_ROWS, 0, NULL));
        free(create_st);
    }
}

void give_id_and_send_forward(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(nbytes);

    actor_state *my_state = *stateptr;

    my_state->next_id = (actor_id_t) data;
    send_message(my_state->next_id,
                 create_message(MSG_CREATE_ACT, sizeof(create_state), my_state->temp_create_state));
    my_state->temp_create_state = NULL;
}

void sum_rows(void **stateptr, size_t nbytes, void *data) {
    actor_state *my_state = *stateptr;
    adding_state *add_state;

    if (nbytes == 0) {
        data = my_state->temp_adding_state;
        my_state->temp_adding_state = NULL;
    }

    add_state = data;

    if (my_state->is_first) {
        if (add_state->row_no != -1) {
            printf("%ld\n", add_state->sum);
        }
        add_state->row_no += 1;
        add_state->sum = 0;

        if (add_state->row_no == add_state->rows_count) {
            send_message(actor_id_self(),
                           create_message(MSG_FREE, 0, NULL));
            return;
        }
    }

    usleep(add_state->times[add_state->row_no][my_state->column] * MICROSECONDS_IN_MILlISECOND);
    add_state->sum += add_state->numbers[add_state->row_no][my_state->column];
    send_message(my_state->next_id,
                 create_message(MSG_SUM_ROWS, sizeof(adding_state), add_state));
}

void count(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(nbytes);

    // Starts whole process, should be sent only to the first actor.
    actor_state *my_state = *stateptr;
    my_state->temp_adding_state = data;

    create_state *cr_state = malloc(sizeof(create_state));
    if (cr_state == NULL) {
        fprintf(stderr, "Malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    cr_state->column = 0;
    cr_state->first_id = actor_id_self();
    cr_state->how_many_to_create = ((adding_state *) data)->columns_count;

    send_message(actor_id_self(),
                 create_message(MSG_CREATE_ACT, sizeof(create_state), cr_state));
}

void free_mem(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(nbytes);
    NOT_USED(data);

    if (*stateptr != NULL) {
        actor_state *my_state = *stateptr;
        send_message(my_state->next_id,
                     create_message(MSG_FREE, 0, NULL));
        free(*stateptr);
        *stateptr = NULL;
        send_message(actor_id_self(),
                     create_message(MSG_GODIE, 0, NULL));
    }
}

int main(){
    long **numbers, **times;
    int n, m;
    scanf("%d", &n);
    scanf("%d", &m);

    act_t prompts[] = {hello, create_actors, give_id_and_send_forward, sum_rows, count, free_mem};
    matrix_role.nprompts = 6;
    matrix_role.prompts = prompts;

    numbers = malloc(n * sizeof(long *));
    times = malloc(n * sizeof(long *));

    if (numbers == NULL || times == NULL) {
        fprintf(stderr, "Malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        numbers[i] = malloc(m * sizeof(long));
        times[i] = malloc(m * sizeof(long));

        if (numbers[i] == NULL || times[i] == NULL) {
            fprintf(stderr, "Malloc failed.\n");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < m; j++) {
            scanf("%ld %ld", &numbers[i][j], &times[i][j]);
        }
    }

    adding_state add_state;
    add_state.numbers = numbers;
    add_state.times = times;
    add_state.rows_count = n;
    add_state.columns_count = m;
    add_state.sum = 0;
    add_state.row_no = -1;

    actor_id_t first_act;

    int res = actor_system_create(&first_act, &matrix_role);

    if (res == 0) {
        send_message(first_act,
                     create_message(MSG_COUNT, sizeof(adding_state), &add_state));
        actor_system_join(first_act);
    } else {
        fprintf(stderr, "Error in creating the actors' system.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        free(numbers[i]);
        free(times[i]);
    }

    free(numbers);
    free(times);

    return 0;
}