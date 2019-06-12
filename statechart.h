/*
 * C Statechart Library Header
 *
 * Author John Buyi Yu jdoe35087@gmail.com
 */
#ifndef HC774DDE3_D18C_4B37_B601_74395ADDF8FA
#define HC774DDE3_D18C_4B37_B601_74395ADDF8FA

#include <stdint.h>
#include <stdio.h>

// TODO these definitions are platform dependent
// move these to somewhere else
typedef uint32_t sc_event_t;
typedef uint32_t sc_result_t;
#define SC_MAX_DEPTH  (10)
#define BUG_ON(cond) \
		if( cond ) { \
			printf("Bug in file %s at line %d on expression \"%s\"\n",\
					__FILE__, __LINE__, #cond ); \
		}

/*
 * Opaque declarations
 */
struct sc_state;
struct sc_context;

/*
 * Events
 */
enum
{
	SC_EVENT_USER
};

/*
 * State handler prototypes
 */
typedef void (*sc_entry_func_t)(void *p_context);
typedef void (*sc_exit_func_t )(void *p_context);
typedef sc_result_t (*sc_event_func_t)(void *p_context, sc_event_t event);

/*
 * Macros for type casting
 */
#define SC_ENTRY_FUNC(func) ((sc_entry_func_t)(func))
#define SC_EXIT_FUNC(func)  ((sc_exit_func_t )(func))
#define SC_EVENT_FUNC(func) ((sc_event_func_t)(func))

/*
 * Result of event handling
 * Type sc_result_t
 */
enum {
	SC_UNHANDLED,
	SC_PROPAGATE,
	SC_IGNORED,
	SC_HANDLED
};

/*
 * State chart context base class
 */
struct sc_context {
	const struct sc_state *p_current;
};

/*
 * State descriptor type
 */
struct sc_state {
	const struct sc_state* p_parent; // parent state
	const struct sc_state* p_default;// default child state
	sc_entry_func_t on_entry;        // entry action
	sc_exit_func_t on_exit;          // exit action
	sc_event_func_t on_event;        // event action
};

// Initializes the state machine
void sc_machine_init( void *p_context, const struct sc_state *p_initial );

// Dispatches an event to the state machine
void sc_machine_dispatch( void *p_context, sc_event_t event );

// Transitions from source state to target state
void sc_machine_trans( void *p_context, const struct sc_state *p_target );

#endif /* HC774DDE3_D18C_4B37_B601_74395ADDF8FA */
