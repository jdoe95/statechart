/*
 * C StateChart Library
 * John Yu created on June 12 2019
 * Dialect C99
 */
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "statechart.h"



/*
 * The custom assertion macro adds a string to the standard assertion message for unit testing
 * purposes. This works because the result of the expression is the result of the last expression
 * in the expression, but when assertion fails, the whole expression is printed to stderr.
 */
#define SC_ASSERT(COND) \
	assert(" StateChart " && (COND))



/*
 * The size of the array used to record tree traversal paths. Usually it is set to the maximum
 * nesting depth of states. A stack overflow may occur when this value is too large.
 */
#ifndef SC_MAX_LIST
#	define SC_MAX_LIST  ((size_t) 5)
#endif



/*
 * Internal functions
 */
static bool is_forward_event(enum sc_result handler_result);
static size_t path_to_root(const struct sc_state *vertex, const struct sc_state *path[]);
static size_t find_vertex(const struct sc_state *vertex, const struct sc_state *const list[],
		size_t valid_data);


/*
 * Initializes a state machine
 */
void sc_init(struct sc_machine *machine)
{
	SC_ASSERT(machine != NULL);

	machine->current = NULL;
	machine->tran_info = NULL;
}



/*
 * Dispatches an event to a state machine.
 */
void sc_dispatch(struct sc_machine *machine, void *m_data, const void *e_data)
{
	SC_ASSERT(machine != NULL);
	SC_ASSERT(m_data != NULL);
	SC_ASSERT(e_data != NULL);

	/*
	 * Dispatch to current state and forward to its parents if needed.
	 */
	for (const struct sc_state *iter = machine->current; iter != NULL; iter = iter->parent)
	{
		/* default action is to forward when the state does not have a handler */
		if (iter->handler != NULL)
		{
			/* break the loop if handler says no need to forward */
			if (!is_forward_event(iter->handler(machine, m_data, e_data)))
				break;
		}
	}
}



/*
 * Makes a transition to target state with optional transition action.
 * If 'target' is NULL, the state machine will return to its uninitialized state.
 */
void sc_tran(struct sc_machine *machine, void *m_data, const struct sc_state *target,
		void (*action) (struct sc_machine *machine, void *m_data))
{
	SC_ASSERT(machine != NULL);
	SC_ASSERT(m_data != NULL);

	/* if the assertion fails, a transition is likely being made in entry/exit handlers, which is
	 * forbidden. */
	SC_ASSERT(machine->tran_info == NULL);

	/* dynamically allocate on the stack */
	struct sc_tran_info tran_info;
	tran_info.source = machine->current;
	tran_info.target = target;
	tran_info.last_exited = NULL;
	tran_info.last_entered = NULL;

	machine->tran_info = &tran_info;

	/*
	 * Implements a lowest common ancestor (LCA) search algorithm
	 */

	/* record target path to root and target entry needs to be performed in reverse */
	const struct sc_state *target_path[SC_MAX_LIST];
	size_t target_path_size;
	target_path_size = path_to_root(target, target_path);

	/* Search for a vertex in the source path that matches that in the target path. If not found,
	 * the whole source path will need to be exited and the whole target path will need to be
	 * entered. */
	size_t entry_depth = target_path_size;
	for (const struct sc_state *iter = machine->current; iter != NULL; iter = iter->parent)
	{
		entry_depth = find_vertex(iter, target_path, target_path_size);

		/* returned a valid index meaning a match is found in the target path */
		if (entry_depth < target_path_size)
			break;

		/* exit source path vertex */
		if (iter->exit != NULL)
			iter->exit(machine, m_data);

		tran_info.last_exited = iter;
	}

	/* perform transition action */
	if (action != NULL)
		action(machine, m_data);

	/* descend into target state */
	for ( ; entry_depth > (size_t) 0; --entry_depth)
	{
		if (target_path[entry_depth - (size_t) 1]->entry != NULL)
			target_path[entry_depth - (size_t) 1]->entry(machine, m_data);

		tran_info.last_entered = target_path[entry_depth - (size_t) 1];
	}

	/* descend into target state's default child state */
	const struct sc_state *current = target;
	for (const struct sc_state *iter = target->child; iter != NULL; iter = iter->child)
	{
		if (iter->entry != NULL)
			iter->entry(machine, m_data);

		current = iter;
		tran_info.last_entered = iter;
	}

	/* update current state */
	machine->current = current;

	/* Transition complete. "Deallocate" transition info */
	machine->tran_info = NULL;
}



/*
 * Returns pointer to the transition object during a transition, otherwise NULL
 */
const struct sc_tran_info* sc_ongoing_tran_info(struct sc_machine *machine)
{
	SC_ASSERT(machine != NULL);
	return machine->tran_info;
}



/*
 * Returns true if event needs to be forwarded to parent based on the result returned by state
 * handler.
 */
static bool is_forward_event(enum sc_result handler_result)
{
	bool ret;

	if (handler_result == SC_UNHANDLED)
		ret = true;

	else if (handler_result == SC_HANDLED)
		ret = true;

	else if (handler_result == SC_DISCARD)
		ret = false;

	else if (handler_result == SC_FORWARD)
		ret = true;

	else
	{
		/* should not reach here */
		SC_ASSERT(0);
		ret = true;
	}

	return ret;
}



/*
 * Fills 'path' with vertices ascending from 'vertex' to root and return the number of elements in
 * it.
 */
static size_t path_to_root(const struct sc_state *vertex, const struct sc_state *path[])
{
	SC_ASSERT(path != NULL);

	size_t index = (size_t) 0;
	for (const struct sc_state *iter = vertex; iter != NULL; iter = iter->parent)
	{
		SC_ASSERT(index < SC_MAX_LIST);
		path[index++] = iter;
	}

	return index;
}



/*
 * Searches for 'vertex' in the valid data region of 'list' and returns the index of the first
 * encounter if found. If 'vertex' is not found in 'list', returns an invalid index.
 */
static size_t find_vertex(const struct sc_state *vertex, const struct sc_state *const list[],
		size_t valid_data)
{
	size_t index;

	SC_ASSERT(list != NULL);

	for (index = (size_t) 0; index < valid_data; ++index)
	{
		if (vertex == list[index])
			break;
	}

	return index;
}
