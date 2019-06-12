/* ******************************************************
 * statechart.c
 * @brief
 * @details
 ********************************************************/
#include "statechart.h"

void sc_machine_init( void *p_context,
		const struct sc_state *p_initial )
{
	struct sc_context *p_machine = (struct sc_context*)p_context;

	p_machine->p_current = p_initial;
	p_initial->on_entry(p_context);
}

/**
 * \brief Transitions from source state to target state
 * \param p_context pointer to the machine context storage
 * \param p_this pointer to current state descriptor
 * \param p_target pointer to target state descriptor
 * \details This function is called when the state wants to
 * make a transition.
 * \return none
 */
void sc_machine_trans( void *p_context, const struct sc_state *p_target )
{
	// get machine context
	struct sc_context *p_machine = (struct sc_context*) p_context;

	BUG_ON(p_context == NULL );
	BUG_ON(p_target == NULL);

	// corrupted context
	// make sure machine is initialized
	// make sure first member of context is base context instance
	BUG_ON(p_machine->p_current == NULL);

	/*
	 * Implements a lowest common ancestor (LCA) search algorithm
	 * Time complexity is O(h) where h is the height of the tree.
	 *
	 * The purpose is to find a path that exits the source state
	 * and enters into the target state.
	 *
	 * See an animation of this algorithm here.
	 * https://thimbleby.gitlab.io/algorithm-wiki-site/wiki/lowest_common_ancestor/
	 *
	 * The search starts with writing source state and all its parents
	 * into array exit_chain, then searching for target or its parent
	 * in exit_chain while saving target and its parent into enter_chain.
	 */
	const struct sc_state *exit_chain[SC_MAX_DEPTH];
	const struct sc_state *enter_chain[SC_MAX_DEPTH];

	unsigned exit_index, exit_depth;
	unsigned enter_index, enter_depth;

	// from source state to its topmost state
	exit_index = 0;
	for( const struct sc_state *p_iter = p_machine->p_current;
			p_iter != NULL; p_iter = p_iter->p_parent )
	{
		// source state nested too deep
		// increase SC_MAX_DEPTH
		BUG_ON(exit_index >= SC_MAX_DEPTH);

		exit_chain[exit_index++] = p_iter;
	}

	// from target state to LCA, or target's topmost state
	enter_index = 0;
	for( const struct sc_state *p_iter = p_target; p_iter != NULL;
			p_iter = p_iter->p_parent )
	{
		// target state nested too deep
		// increase SC_MAX_DEPTH
		BUG_ON(enter_index >= SC_MAX_DEPTH);

		enter_chain[enter_index] = p_iter;

		for( unsigned counter = 0; counter < exit_index; counter++ )
		{
			// LCA found
			if( exit_chain[counter] == p_iter ) {
				exit_depth = counter;
				enter_depth = enter_index;
				goto with_ancestor;
			}
		}

		enter_index++;
	}
	// no common ancestor is found when the loop exits this way
	exit_depth = exit_index;
	enter_depth = enter_index;

	const struct sc_state *p_state;

	// if no common ancestor is found, the two states are in distinct
	// hierarchies. Perform the transition at the top level.
with_ancestor:

	// exit source state
	for( unsigned counter = 0; counter < exit_depth; counter++ )
	{
		p_state = exit_chain[counter];

		if( p_state->on_exit != NULL )
			p_state->on_exit(p_context);
	}

	// enter target state
	for( unsigned counter = enter_depth; counter > 0; counter-- )
	{
		p_state = enter_chain[counter-1];

		if( p_state->on_entry != NULL )
			p_state->on_entry(p_context);
	}

	// enter default child state
	const struct sc_state *p_current = p_target;
	for( const struct sc_state *p_iter = p_target->p_default;
			p_iter != NULL; p_iter = p_iter->p_default )
	{
		p_iter->on_entry(p_context);
		p_current = p_iter;
	}

	// update current state
	p_machine->p_current = p_current;
}

/**
 * \brief Dispatches an event to the state machine
 * \param p_context pointer to the state machine context storage
 * \param p_event pointer to a state machine event object
 * \details The state machine first dispatches the event to the
 * leaf state (inner most state) and if not handled, its parent
 * state will attempt to handle it until the event is propagated
 * all the way up to the top, where it will be discarded.
 */
void sc_machine_dispatch( void *p_context, sc_event_t event )
{
	// get machine context
	struct sc_context *p_machine = (struct sc_context*) p_context;

	BUG_ON(p_context == NULL);

	// corrupted context
	// make sure machine is initialized
	// make sure first member of context is base context instance
	BUG_ON(p_machine->p_current == NULL);

	const struct sc_state *p_iter;
	sc_result_t result;

	for( p_iter = p_machine->p_current; p_iter != NULL;
			p_iter = p_iter->p_parent )
	{
		// dispatch the event to the leaf state
		// if unhandled in leaf state, propagate up to its parent state

		// no handlers registered in leaf state
		if( p_iter->on_event == NULL )
			continue; // propagate up

		result = p_iter->on_event(p_context, event);

		// handled by leaf state
		if( result == SC_HANDLED )
			break; // stop the propagation

		// ignored by leaf state because a guard condition evaluated false
		else if( result == SC_IGNORED )
			break; // stop the propagation

		// unhandled by leaf state
		else if (result == SC_UNHANDLED )
			continue; // propagate up

	}
}
