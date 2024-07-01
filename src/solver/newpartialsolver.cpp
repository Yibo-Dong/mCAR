#include "newpartialsolver.h"
#include "utility.h"
#include "DEBUG_PRINTING.h"
#include <algorithm>
using namespace std;
namespace car
{
	PartialSolver::PartialSolver (Model* m, const bool verbose) 
	{
		model_ = m;
		max_partial_id = m->max_id() +1;

	    //constraints
		for (int i = 0; i < m->outputs_start (); i ++)
		{
			add_clause (m->element (i));
		}
		//outputs
		for (int i = m->outputs_start (); i < m->latches_start (); i ++)
		{
			add_clause (m->element (i));
		}
		//latches
		for (int i = m->latches_start (); i < m->size (); i ++)
		{
			add_clause (m->element (i));
		}

	}
	
	void PartialSolver::add_clause_with_flag(Assignment& cls)
	{
		vector<int> cl = cls;
		int flag = max_partial_id;
		cls.push_back(flag);
		add_clause(cl);
	}

	Assignment PartialSolver::get_conflict()
	{
		Assignment conflict = get_uc();
		model_ -> shrink_to_latch_vars(conflict);
		std::sort (conflict.begin (), conflict.end (), car::comp);
		return conflict;
	}

	Assignment PartialSolver::get_conflict_no_bad(int bad)
	{
		Assignment conflict = get_uc_no_bad(bad);
		model_ -> shrink_to_latch_vars(conflict);

		std::sort (conflict.begin (), conflict.end (), car::comp);
		return conflict;
	}
	

	bool PartialSolver::solve_with_assumption(const Assignment& assumption)
	{
		set_assumption(assumption);
		bool res = solve_assumption ();
		return res;
	}
	
	void PartialSolver::set_assumption (const Assignment& assum)
	{
		assumptions.clear ();
		for (const int &var :assum)
		{
			assumptions.push (SAT_lit (var));
		}
	}
}
