#include "reef.h"

enum mjs_type {
  /* Primitive types */
  MJS_TYPE_UNDEFINED,
  MJS_TYPE_NULL,
  MJS_TYPE_BOOLEAN,
  MJS_TYPE_NUMBER,
  MJS_TYPE_STRING,
  MJS_TYPE_FOREIGN,

  /* Different classes of Object type */
  MJS_TYPE_OBJECT_GENERIC,
  MJS_TYPE_OBJECT_ARRAY,
  /*
   * TODO(dfrank): if we support prototypes, need to add items for them here
   */

  MJS_TYPES_CNT
};

struct mbuf {
  char *buf;   /* Buffer pointer */
  size_t len;  /* Data length. Data is located between offset 0 and len. */
  size_t size; /* Buffer size allocated by realloc(1). Must be >= len */
};

typedef int64_t bf_cell_t;
/* typedef int16_t bf_cell_t; */

typedef int8_t bf_opcode_t;

/* index in IRAM */
typedef int16_t bf_word_ptr_t;

struct bf_vm;
typedef void (*bf_native_t)(struct bf_vm *vm);

struct bf_code {
  const bf_opcode_t *opcodes;      /* all word bodies */
  size_t opcodes_len;              /* max 32768 */
  const bf_word_ptr_t *table;      /* points to opcodes */
  size_t table_len;                /* max 127 */
  const bf_native_t *native_words; /* native words */
  size_t native_words_len;

  const char *const *word_names; /* table_len number of entries, for tracing */
  const char *const *pos_names; /* opcodes_len number of entries, for tracing */
};

/*
 * Called after "entering" each word
 */
typedef void(bf_cb_op_t)(struct bf_vm *vm);

struct bf_callbacks {
  bf_cb_op_t *after_bf_enter;
};

struct bf_vm {
  const struct bf_code *code;
  struct bf_mem *iram;

  bf_word_ptr_t ip;
  struct mbuf dstack; /* data stack */
  struct mbuf rstack; /* return stack */
  bf_cell_t tmp;

  struct bf_callbacks cb;
  void *user_data;
};

static void mjs_op_time_systime(struct bf_vm *vm)
{
    struct mjs *jsm = (struct mjs *) vm->user_data;
    mjs_val_t ret = MJS_UNDEFINED;

    char timestr[25] = {0};
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    strftime(timestr, 25, "%Y-%m-%d %H:%M:%S", tm);
    timestr[24] = '\0';

    ret = mjs_mk_object(jsm);
    mjs_set(jsm, ret, "localtime", ~0, mjs_mk_string(jsm, timestr, ~0, 1));
    mjs_set(jsm, ret, "timestamp", ~0, mjs_mk_number(jsm, mos_timef()));

    mjs_push(jsm, ret);
}

static void mjs_op_os_usleep(struct bf_vm *vm)
{
    struct mjs *jsm = (struct mjs *) vm->user_data;
    int nargs = mjs_get_int(jsm, mjs_pop(jsm));

    mjs_val_t val;

    if (nargs < 1) {
        mjs_prepend_errorf(jsm, MJS_TYPE_ERROR, "missing a value to usleep");
        return;
    }
    val = mjs_pop(jsm);

    int usec = 0;
    if (mjs_is_number(val)) {
        usec = mjs_get_int(jsm, val);
        if (usec > 0) usleep(usec);
    }
}

static void mjs_op_os_itostring(struct bf_vm *vm)
{
    struct mjs *jsm = (struct mjs *) vm->user_data;
    int nargs = mjs_get_int(jsm, mjs_pop(jsm));

    mjs_val_t val;

    if (nargs < 1) {
        mjs_prepend_errorf(jsm, MJS_TYPE_ERROR, "missing a value to usleep");
        return;
    }
    val = mjs_pop(jsm);

    if (mjs_is_number(val)) {
        char tok[32] = {0};
        snprintf(tok, sizeof(tok), "%d", mjs_get_int(jsm, val));

        mjs_push(jsm, mjs_mk_string(jsm, tok, ~0, 1));
    }
}

static void mjs_op_os_ftostring(struct bf_vm *vm)
{
    struct mjs *jsm = (struct mjs *) vm->user_data;
    int nargs = mjs_get_int(jsm, mjs_pop(jsm));

    mjs_val_t val;

    if (nargs < 1) {
        mjs_prepend_errorf(jsm, MJS_TYPE_ERROR, "missing a value to usleep");
        return;
    }
    val = mjs_pop(jsm);

    char tok[64] = {0};
    snprintf(tok, sizeof(tok), "%f", mjs_get_double(jsm, val));

    mjs_push(jsm, mjs_mk_string(jsm, tok, ~0, 1));
}

void mjs_init_local(struct mjs *jsm, mjs_val_t o)
{
    mjs_val_t time = mjs_mk_object(jsm);
    mjs_val_t os = mjs_mk_object(jsm);

    mjs_set(jsm, o, "TIME", ~0, time);
    mjs_set(jsm, time, "systime", ~0, mjs_mk_foreign(jsm, mjs_op_time_systime));

    mjs_set(jsm, o, "OS", ~0, os);
    mjs_set(jsm, os, "usleep", ~0, mjs_mk_foreign(jsm, mjs_op_os_usleep));

    mjs_set(jsm, os, "itostring", ~0, mjs_mk_foreign(jsm, mjs_op_os_itostring));
    mjs_set(jsm, os, "ftostring", ~0, mjs_mk_foreign(jsm, mjs_op_os_ftostring));
}
