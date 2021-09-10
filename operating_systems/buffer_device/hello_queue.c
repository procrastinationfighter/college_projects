#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <minix/ds.h>
#include <minix/ioctl.h>
#include <sys/ioc_hello_queue.h>
#include "hello_queue.h"

#define ALLOC_MULT 2

static int hello_q_open(devminor_t minor, int access, endpoint_t user_endpt);
static int hello_q_close(devminor_t minor);
static ssize_t hello_q_read(devminor_t minor, u64_t position, endpoint_t endpt,
                    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static ssize_t hello_q_write(devminor_t minor, u64_t position, endpoint_t endpt,
	                cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static int hello_q_ioctl(devminor_t minor, unsigned long request, endpoint_t endpt,
	                cp_grant_id_t grant, int flags, endpoint_t user_endpt, cdev_id_t id);

static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int);
static int lu_state_restore(void);

static struct chardriver hello_q_tab = {
        .cdr_open = hello_q_open,
        .cdr_close = hello_q_close,
        .cdr_read = hello_q_read,
        .cdr_write = hello_q_write,
        .cdr_ioctl = hello_q_ioctl
};

char *buffer;

uint32_t buffer_size = DEVICE_SIZE;  // Size of allocated memory.
uint32_t buffer_len;                  // Size of the queue.

static void exit_fail(const char *str) {
    printf("%s\n", str);
    if (buffer != NULL) {
        free(buffer);
    }

    exit(1);
}

static void reset_queue() {
    static char xyz[3] = {'x', 'y', 'z'};

    if (buffer != NULL) {
        free(buffer);
    }

    buffer_size = DEVICE_SIZE;
    buffer_len = DEVICE_SIZE;

    buffer = (char *) malloc(sizeof(char) * buffer_size);
    if (buffer == NULL) {
        exit_fail("Hello queue: reset queue malloc");
    }

    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = xyz[i % 3];
    }
}

static void remove_every_third() {
    uint32_t new_ptr = 0, old_ptr = 0;

    while (old_ptr < buffer_len) {
        if (old_ptr % 3 != 2) {
            buffer[new_ptr] = buffer[old_ptr];
            new_ptr++;
        } 
        old_ptr++;
    }

    buffer_len = new_ptr;
}

static void realloc_buffer(uint32_t new_len) {
    while (new_len > buffer_size) {
        buffer_size *= ALLOC_MULT;
    }
            
    buffer = realloc(buffer, buffer_size);
    if (buffer == NULL) {
        exit_fail("Hello queue: realloc greater");
    }
}

static int hello_q_open(devminor_t UNUSED(minor), int UNUSED(access), endpoint_t UNUSED(user_endpt)) {
    return OK;
}

static int hello_q_close(devminor_t UNUSED(minor)) {
    return OK;
}

static ssize_t hello_q_read(devminor_t UNUSED(minor), u64_t UNUSED(position), endpoint_t endpt,
                    cp_grant_id_t grant, size_t size, int UNUSED(flags), cdev_id_t UNUSED(id)) {

    if (size == 0 || buffer_len == 0) {
        if (buffer_len <= buffer_size / 4 && buffer_size > 1) {
            buffer = realloc(buffer, buffer_size / ALLOC_MULT);
            if (buffer == NULL) {
                exit_fail("Hello queue: read realloc");
            }

            buffer_size /= ALLOC_MULT;
        }

        return 0;
    }
    
    int ret;
    int remainder = buffer_len - size;

    size = (size > buffer_len) ? buffer_len : size;

    if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) buffer, size)) != OK) {
        return ret;
    }

    buffer_len = (remainder > 0) ? remainder : 0;

    for (int i = 0; i < buffer_len; i++) {
        buffer[i] = buffer[size + i];
    }

    if (buffer_len <= buffer_size / 4 && buffer_size > 1) {
        buffer = realloc(buffer, buffer_size / ALLOC_MULT);
        if (buffer == NULL) {
            exit_fail("Hello queue: read realloc");
        }

        buffer_size /= ALLOC_MULT;
    }

    return size;
}

static ssize_t hello_q_write(devminor_t UNUSED(minor), u64_t UNUSED(position), endpoint_t endpt,
	                cp_grant_id_t grant, size_t size, int UNUSED(flags), cdev_id_t UNUSED(id)) {
    if (size == 0) {
        return 0;
    }

    int ret;
    uint32_t new_len = size + buffer_len;

    if (new_len > buffer_size) {
        realloc_buffer(new_len);
    }

    ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buffer + buffer_len, size);
    if (ret != OK) {
        return ret;
    }

    buffer_len = new_len;

    return size;
}

static int hello_q_ioctl(devminor_t UNUSED(minor), unsigned long request, endpoint_t endpt,
	                cp_grant_id_t grant, int UNUSED(flags), endpoint_t UNUSED(user_endpt), cdev_id_t UNUSED(id)) {
    int ret;
    char exchange[2];

	switch(request) {
	case HQIOCRES:
		reset_queue();
        return OK;
	case HQIOCSET:
		if (MSG_SIZE > buffer_len) {
            if (MSG_SIZE > buffer_size) {
                realloc_buffer(MSG_SIZE);
            }
            ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buffer, MSG_SIZE);
            buffer_len = MSG_SIZE;
        } else {
            ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) buffer + (buffer_len - MSG_SIZE), MSG_SIZE);
        }

		return ret;
	case HQIOCXCH:
		ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) exchange, 2);
		if (ret != OK) {
            return ret;
        }

        for (int i = 0; i < buffer_len; i++) {
            if (buffer[i] == exchange[0]) {
                buffer[i] = exchange[1];
            }
        }

        return OK;
	case HQIOCDEL:
		remove_every_third();
		return OK;
	}

	return ENOTTY;
}


// Copied directly from the hello driver.
static void sef_local_startup() {
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *info) {
    int do_announce_driver = TRUE;

    switch(type) {
        case SEF_INIT_FRESH:
            reset_queue();
            break;

        case SEF_INIT_LU:
            lu_state_restore();
            do_announce_driver = FALSE;
            break;

        case SEF_INIT_RESTART:
            lu_state_restore();
            break;
    }

    if (do_announce_driver) {
        chardriver_announce();
    }

    return OK;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
    if (ds_publish_u32("buffer_size", buffer_size, DSF_OVERWRITE) != OK) {
        exit_fail("Hello queue: save state");
    }

    if (ds_publish_u32("buffer_len", buffer_len, DSF_OVERWRITE) != OK) {
        exit_fail("Hello queue: save state");
    }

    if (ds_publish_mem("buffer", (void *) buffer, sizeof(char) * buffer_len, DSF_OVERWRITE) != OK) {
        exit_fail("Hello queue: save state");
    }

    free(buffer);
    buffer = NULL;

    return OK;
}

static int lu_state_restore(void) {
    size_t len;

    if (ds_retrieve_u32("buffer_size", &buffer_size) != OK) {
        exit_fail("Hello queue: state restore");
    }
    ds_delete_u32("buffer_size");

    if (ds_retrieve_u32("buffer_len", &buffer_len) != OK) {
        exit_fail("Hello queue: state restore");
    }
    ds_delete_u32("buffer_len");

    buffer = (char *) malloc(sizeof(char) * buffer_size);
    if (buffer == NULL) {
        exit_fail("Hello queue: state restore malloc");
    }

    if (ds_retrieve_mem("buffer", buffer, &len) != OK || len != buffer_len) {
        exit_fail("Hello queue: state restore buffer");
    }
    ds_delete_mem("buffer");

    return OK;
}


int main() {
    buffer = NULL;
    if (DEVICE_SIZE <= 0) {
        exit_fail("Hello queue: DEVICE_SIZE is not positive");
    }

    sef_local_startup();
    chardriver_task(&hello_q_tab);
    free(buffer);

    return OK;
}