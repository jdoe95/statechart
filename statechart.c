/*
 * C Statechart Library Header
 * Author: John Buyi Yu jdoe35087@gmail.com
 */
#include <stddef.h>
#include "statechart.h"

#if !defined(SC_MAX_DEPTH)
#	define SC_MAX_DEPTH (10U)   /* maximum depth of parent and child states */
#endif

#if !defined(SC_BUG_ON)
#   define SC_BUG_ON(cond)     /* runtime bug catcher */
#endif

void sc_init(void *context, const struct sc_state *initial)
{
	SC_BUG_ON(context == NULL);
	SC_BUG_ON(initial == NULL);

	struct sc_context *context_typed;

	context_typed = (struct sc_context*)context;
	context_typed->current = initial;
	initial->entry(context);
}

/*
 * Dispatches an event to the state machine.
 * The state machine first dispatches the event to the leaf state (inner most state) and if not
 * handled, its parent state will attempt to handle it until the event is propagated all the way up
 * to the top, where it will be discarded.
 */

void sc_dispatch(void *context, const void* event)
{
	SC_BUG_ON(context == NULL);

	struct sc_context *context_typed;
	context_typed = (struct sc_context*)context;

	/*
	 * Corrupted context:
	 * 1. make sure machine is initialized, and
	 * 2. make sure first member of context is base context instance
	 */
	SC_BUG_ON(context_typed->current == NULL);

	const struct sc_state *iter;
	enum sc_handler_result result;

	for (iter = context_typed->current; iter != NULL; iter = iter->parent)
	{
		/*
		 * Dispatch the event to the leaf state, if unhandled in leaf state, propagate up to its
		 * parent state.
		 */
		if (iter->handler != NULL)
		{
			result = iter->handler(context, event);

			if ((result == SC_HANDLER_RESULT_HANDLED) || (result == SC_HANDLER_RESULT_IGNORE))
				break; /* stop the propagation */
		}
	}
}

/*
 * Transitions from current state to target state
 */
void sc_trans(void *context, const struct sc_state *target )
{
	SC_BUG_ON(context == NULL);
	SC_BUG_ON(target == NULL);

	struct sc_context *context_typed;
	context_typed = (struct sc_context*)context;

	/*
	 * Corrupted context:
	 * 1. Make sure machine is initialized, and
	 * 2. Make sure first member of context is base context instance.
	 */
	SC_BUG_ON(context_typed->current == NULL);

	/*
	 * Implements a lowest common ancestor (LCA) search algorithm
	 *
	 * Time complexity is O(h) where h is the height of the tree.
	 * The purpose is to find a path that exits the source state and enters into the target state.
	 *
	 * See an animation of this algorithm here.
	 * https://thimbleby.gitlab.io/algorithm-wiki-site/wiki/lowest_common_ancestor/
	 *
	 * The search starts with writing source state and all its parents into array exit_chain, then
	 * searching for target or its parent in exit_chain while saving target and its parent into
	 * enter_chain.
	 */
	const struct sc_state *exit_chain[SC_MAX_DEPTH];
	const struct sc_state *enter_chain[SC_MAX_DEPTH];

	unsigned exit_index, exit_depth;
	unsigned enter_index, enter_depth;

	enter_depth = 0U;
	enter_index = 0U;
	exit_depth = 0U;
	exit_index = 0U;

	/* from source state to its topmost state */
	for (const struct sc_state *iter = context_typed->current; iter != NULL; iter = iter->parent)
	{
		/* source state nested too deep. Increase SC_MAX_DEPTH! */
		SC_BUG_ON(exit_index >= SC_MAX_DEPTH);

		exit_chain[exit_index++] = iter;
	}

	/* from target state to LCA, or target's topmost state */
	for (const struct sc_state *iter = target; iter != NULL; iter = iter->parent)
	{
		/* Target state nested too deep. Increase SC_MAX_DEPTH! */
		SC_BUG_ON(enter_index >= SC_MAX_DEPTH);

		enter_chain[enter_index] = iter;

		for (unsigned counter = 0U; counter < exit_index; counter++)
		{
			/* LCA found */
			if (exit_chain[counter] == iter)
			{
				exit_depth = counter;
				enter_depth = enter_index;
				break;
			}
		}

		enter_index++;
	}

	/* LCA found */
	if (enter_depth && exit_depth)
	{
		const struct sc_state *state;

		/* exit source state */
		for (unsigned counter = 0U; counter < exit_depth; counter++)
		{
			state = exit_chain[counter];

			if( state->exit != NULL )
				state->exit(context);
		}

		/* enter target state */
		for (unsigned counter = enter_depth; counter > 0U; counter--)
		{
			state = enter_chain[counter-1];

			if( state->entry != NULL )
				state->entry(context);
		}

		/* enter default child state */
		const struct sc_state *current = target;
		for (const struct sc_state *iter = target->child; iter != NULL; iter = iter->child)
		{
			iter->entry(context);
			current = iter;
		}

		/* update current state */
		context_typed->current = current;
	}

	/* LCA not found. The two states are not in the same state machine */
	else
	{
		SC_BUG_ON(0U);
	}
}


