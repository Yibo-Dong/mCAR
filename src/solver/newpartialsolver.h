#ifndef NEW_PARTIAL_SOLVER_H
#define NEW_PARTIAL_SOLVER_H

#include "carsolver.h"
#include "data_structure.h"
#include "model.h"
#include "statistics.h"
#include <vector>
#include <assert.h>
#include <iostream>

namespace car
{
	extern Statistics CARStats;

	/**
	 * @brief this is used in forward-car, which means backward search.
	 * When searching backward, such occasion exists:
	 * 	
	 * SAT_ASSUME( Ol /\ T /\ s' ). with s' the assumption.
	 * 
	 * If succeed, get a concrete state t in at level l in O.
	 * However, there may have enormous states {t1,t2, ...} that is SAT.
	 * we get only one t.
	 * Therefore, we try to use another SAT call to get more states.
	 * That is:
	 * 
	 * SAT_ASSUME ( t /\ T /\ ~s' ). with t(latch + input) the assumption.
	 * Obviously, it is UNSAT. because t /\ T is bound to arrive at s.
	 * Therefore, we can get the unsat core uc,  and they are the new partial state.
	 * 
	 * TODO: what is a good order to place assumptionos?
	 */
	class PartialSolver : public CARSolver
	{
	public:
		PartialSolver(Model *, const bool verbose = false);
		~PartialSolver() {}

		// add clause with flag
		void add_clause_with_flag(Assignment& cls);
		
		// generate new flag.
		inline int new_flag() {return ++max_partial_id;}
		inline int get_flag() {return max_partial_id;}

		bool solve_with_assumption(const Assignment& assumption);

		Assignment get_conflict();
		Assignment get_conflict_no_bad(int bad);

		void set_assumption(const Assignment& assum);


	private:
		int max_partial_id;
		Model *model_;
	};

}

#endif
