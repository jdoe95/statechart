/*
 * C Statechart Library Header
 * Author: John Buyi Yu jdoe35087@gmail.com
 */
#ifndef HC774DDE3_D18C_4B37_B601_74395ADDF8FA
#define HC774DDE3_D18C_4B37_B601_74395ADDF8FA

/*
 * Context base class
 */
struct sc_context
{
	const struct sc_state *current;   /* current state           */
};

/*
 * Event handler return code
 */
enum sc_handler_result
{
	SC_HANDLER_RESULT_UNHANDLED = 0U, /* event is unhandled      */
	SC_HANDLER_RESULT_HANDLED,        /* event is handled        */
	SC_HANDLER_RESULT_PROPAGATE,      /* event should propogate  */
	SC_HANDLER_RESULT_IGNORE,         /* event should be ignored */
};

/*
 * State handler prototypes
 */
typedef void (*sc_entry_type)(void *context);
typedef void (*sc_exit_type )(void *context);
typedef enum sc_handler_result (*sc_handler_type)(void *context, const void *event);

/*
 * State descriptor type
 */
struct sc_state
{
	const struct sc_state* parent;    /* parent state        */
	const struct sc_state* child;    /* default child state */
	sc_entry_type entry;              /* entry action        */
	sc_exit_type exit;                /* exit action         */
	sc_handler_type handler;          /* event handler       */
};

/*
 * Macros for type casting
 */
#define SC_ENTRY_FUNC(func) ((sc_entry_type)(func))
#define SC_EXIT_FUNC(func)  ((sc_exit_type )(func))
#define SC_EVENT_FUNC(func) ((sc_handler_type)(func))

void sc_init(void *context, const struct sc_state *initial);
void sc_dispatch(void *context, const void *event);
void sc_trans(void *context, const struct sc_state *target );

#endif /* HC774DDE3_D18C_4B37_B601_74395ADDF8FA */
