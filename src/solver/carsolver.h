
#ifndef CAR_SOLVER_H
#define	CAR_SOLVER_H

#include "statistics.h"
#include <iostream>

#ifdef MINISAT
	#include "Solver.h"
	namespace CARSolverNS {
		using SolverType = Minisat::Solver;
	}
#else
	#include "Solver.h"
	namespace CARSolverNS {
		using SolverType = Glucose::Solver;
	}
#endif // MINISAT
#include <vector>

namespace car
{
		extern Statistics CARStats;
		class CARSolver : public CARSolverNS::SolverType
		{
		public:
			CARSolver() {}

#ifdef MINISAT
			Minisat::Lit SAT_lit(int id);	// create the Lit used in SAT solver for the id.
			int lit_id(Minisat::Lit) const; // return the id of SAT lit
#else
			Glucose::Lit SAT_lit(int id);			// create the Lit used in SAT solver for the id.
			int lit_id(Glucose::Lit) const;			// return the id of SAT lit
#endif // MINISAT
            /**
             * @brief Solve with the assumptions in _assumption. 
             * @note before this, make sure all the assumption lits are put into assumptions.
             */
			inline bool solve_assumption() { auto res = solve_(); return res == l_True; }								// Solve with the assumptions in _assumption.
			inline int size() { return clauses.size(); }			// Solve with the assumptions in _assumption.
			inline void clear_assumption() { assumptions.clear(); } // clear the assumptions
			std::vector<int> get_assumption() const;				// get the assumption
			std::vector<int> get_model() const;						// get the model from SAT solver
			std::vector<int> get_uc() const;						// get UC from SAT solver
			std::vector<int> get_uc_no_bad(int bad) const;			// get UC from SAT solver
            std::vector<int> get_uc_another();			            // get another UC from SAT solver
			void add_cube(const std::vector<int> &);				// add each element in uc as a clause
			void add_cube_negate(const std::vector<int> &cu);	    // add the negate of the cube
			void add_clause_internal(const std::vector<int> &);		// add the or clause

			template <typename... Args>
			void add_clause(Args... args)
			{
				std::vector<int> v = {args...};
				add_clause_internal(v);
			}

			// printers
			void print_last_3_clauses();
			void print_clauses(std::ostream & out_stream);
			void print_assumption(std::ostream & out_stream);
		};
}

#endif
