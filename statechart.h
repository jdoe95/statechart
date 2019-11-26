/*
 * C StateChart Library
 * John Yu created on June 12 2019
 * Dialect C99
 */
#ifndef HC774DDE3_D18C_4B37_B601_74395ADDF8FA
#define HC774DDE3_D18C_4B37_B601_74395ADDF8FA



/*
 * An object of the struct represents a state machine. Members in this object should only be
 * accessed by the StateChart Library.
 */
struct sc_machine
{
	const struct sc_state *current;        /* Private: current active state  */
	const struct sc_tran_info *tran_info;  /* Private: on-going transition info */
};



/*
 * Records transition information
 */
struct sc_tran_info
{
	const struct sc_state *source;       /* Public: transition source state */
	const struct sc_state *target;       /* Public: transition target state */
	const struct sc_state *last_exited;  /* Public: last exited state during transition */
	const struct sc_state *last_entered; /* Public: last entered state during transition */
};



/*
 * Return values of the state event handler. When the Library dispatches an event to the state
 * event handler, the return value is checked to decide whether to forward the event to the parent
 * state or not. Some return values might serve exactly the same purpose but using a different
 * name allows them to self-document the source code.
 */
enum sc_result
{
	SC_UNHANDLED = 0, /* Event is not handled and should be forwarded to parents.     */
	SC_HANDLED,       /* Event is handled and should be forwarded to parents.         */
	SC_DISCARD,       /* Event should not be forwarded to parents.                    */
	SC_FORWARD        /* Event should be forwarded to parents.                        */
};



/*
 * State descriptor. An object of this struct represents a state in a state machine. The objects
 * are usually defined with 'const', but you may define them as 'mutable' to create a state machine
 * that can modify itself but this is strongly discouraged.
 */
struct sc_state
{
	const struct sc_state* parent;                              /* parent state        */
	const struct sc_state* child;                               /* default child state */
	void (*entry)(struct sc_machine *machine, void *m_data);    /* entry action        */
	void (*exit)(struct sc_machine *machine, void *m_data);     /* exit action         */
	enum sc_result (*handler)(struct sc_machine *machine, void *m_data, const void *e_data);
																/* event handler       */
};



#ifdef __cplusplus
extern "C" {
#endif

void sc_init(struct sc_machine *machine);
void sc_dispatch(struct sc_machine *machine, void *m_data, const void *e_data);
void sc_tran(struct sc_machine *machine, void *m_data, const struct sc_state *target,
		void (*action)(struct sc_machine *machine, void *m_data));
const struct sc_tran_info* sc_ongoing_tran_info(struct sc_machine *machine);

#ifdef __cplusplus
}
#endif

#endif /* HC774DDE3_D18C_4B37_B601_74395ADDF8FA */
