#ifndef _MSS_MIPSOLVER_HH_
#define _MSS_MIPSOLVER_HH_
//#include "MSS_Data.hh"
#include "MSS_Helpers.hh"

#define IL_STD
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

void PrintCosts(const IloCplex& solver, const MSS_Input& in, 
                const IloArray<IloArray<IloArray<IloBoolVarArray>>>& y,
                const IloArray<IloArray<IloArray<IloBoolVarArray>>>& v,
                const IloArray<IloNumVarArray>& W,
                const IloArray<IloBoolVarArray>& Ch);
tuple<double,double,IloAlgorithm::Status,double> MSS_MIPSolver(const MSS_Input& in, MSS_Output& out, double timeout, bool init_solution, bool verbose);

#endif
