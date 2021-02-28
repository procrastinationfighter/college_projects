#include "cacti.h"
#include <stdlib.h>
#include <stdio.h>

#define MSG_CALCULATE 1
#define MSG_GIVE_ID 2
#define MSG_FACTORIAL 3

#define NOT_USED(x) (void)(x)

typedef struct {
    unsigned long long curr_result;
    int target;
    int curr_num;
} fact_state;

role_t factorial_role;

message_t create_message(message_type_t type, size_t nbytes, void *data) {
    message_t mess;
    mess.message_type = type;
    mess.nbytes = nbytes;
    mess.data = data;
    return mess;
}

void hello(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(stateptr);

    if (nbytes > 0) {
        send_message((actor_id_t) data,
                     create_message(MSG_GIVE_ID, sizeof(actor_id_t), (void *) actor_id_self()));
    }
}

void calculate(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(nbytes);

    fact_state *state = data;
    state->curr_result *= state->curr_num;
    if (state->curr_num < state->target) {
        *stateptr = state;
        send_message(actor_id_self(),
                     create_message(MSG_SPAWN, factorial_role.nprompts, &factorial_role));
    } else {
        printf("%llu\n", state->curr_result);
        free(state);
        send_message(actor_id_self(),
                     create_message(MSG_GODIE, 0, NULL));
    }
}

void give_id_and_send_forward(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(nbytes);

    fact_state *my_state = *stateptr;

    my_state->curr_num += 1;

    send_message((actor_id_t) data,
                 create_message(MSG_CALCULATE, sizeof(fact_state), my_state));
    send_message(actor_id_self(),
                 create_message(MSG_GODIE, 0, NULL));
}

void factorial(void **stateptr, size_t nbytes, void *data) {
    NOT_USED(stateptr);
    NOT_USED(nbytes);

    fact_state *state = malloc(sizeof(fact_state));

    int target = *((int *) data);
    if (target == 0) {
        printf("%d", 1);
        free(state);
        send_message(actor_id_self(),
                     create_message(MSG_GODIE, 0, NULL));
    }
    else {
        state->curr_num = 1;
        state->target = target;
        state->curr_result = 1;

        send_message(actor_id_self(),
                     create_message(MSG_CALCULATE, sizeof(fact_state), state));
    }
}

int main(){
    factorial_role.nprompts = 4;
    act_t prompts[] = {hello, calculate, give_id_and_send_forward, factorial};
    factorial_role.prompts = prompts;

    int n;
    scanf("%d", &n);

    actor_id_t first;
    int res = actor_system_create(&first, &factorial_role);
    if (res == 0) {
        send_message(first,
                     create_message(MSG_FACTORIAL, sizeof(int), &n));
        actor_system_join(first);
    } else {
        printf("Creating system failed.\n");
    }
	return 0;
}
