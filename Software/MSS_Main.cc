#include "MSS_Helpers.hh"
#ifdef CPLEX
#include "MSS_MIPSolver.hh"
#endif

using namespace EasyLocal::Debug;

int main(int argc, const char* argv[])
{
  ParameterBox main_parameters("main", "Main Program options");
  
  Parameter<string> instance("instance", "Input instance", main_parameters); 
  Parameter<unsigned> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);   
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);
  Parameter<double> swap_rate("swap_rate", "Swap rate", main_parameters);

  ParameterBox mip_parameters("mip", "MIP Program options");

  Parameter<bool> mip_verbose("verbose", "CPLEX runs verbose (for debugging)", mip_parameters);
  Parameter<int> mip_timeout("timeout", "CPLEX timeout", mip_parameters);

  swap_rate = 0.5;
  mip_verbose = false;
  mip_timeout = 0; 

  
  
  CommandLineParameters::Parse(argc, argv, false, true);  

  if (!instance.IsSet())
    {
      cout << "Error: --main::instance filename option must always be set" << endl;
      return 1;
    }
  MSS_Input in(instance);

  if (seed.IsSet())
    Random::SetSeed(seed);

  
  //Hard constraints 
  MSS_HospMaxRequirements cc1(in, 1, true);
  MSS_PrecRequirements    cc2(in, 1, true);
  MSS_HospMinRequirements cc8(in, 1, true); 
  //Soft constraints
  MSS_DiscPreference      cc3(in, 1, false); 
  MSS_HospPreference      cc4(in, 1, false);
  MSS_WaitingCosts        cc5(in, 1, false);
  MSS_ChangeCosts         cc6(in, 1, false);
  MSS_WorstScore          cc7(in, 1, false);
  MSS_ManPreference       cc9(in, 1, false);

  MSS_MoveDeltaHospMaxRequirements dcc1(in, cc1);
  MSS_MoveDeltaPrecRequirements dcc2(in, cc2);
  MSS_MoveDeltaHospMinRequirements dcc12(in, cc8);
  MSS_MoveDeltaDiscPreference dcc3(in, cc3);
  MSS_MoveDeltaHospPreference dcc4(in, cc4);
  MSS_MoveDeltaWaitingCosts dcc5(in, cc5);
  MSS_MoveDeltaChangeCosts dcc6(in, cc6);
  MSS_MoveDeltaWorstScore dcc7(in,cc7);
  MSS_MoveDeltaMngrPreference dcc14(in,cc9);
  MSS_SwapDeltaHospMaxRequirements dcc8(in, cc1);
  MSS_SwapDeltaHospMinRequirements dcc13(in, cc8);
  MSS_SwapDeltaPrecRequirements dcc9(in, cc2);
  MSS_SwapDeltaChangeCosts dcc10(in, cc6);
  MSS_SwapDeltaWorstScore dcc11(in, cc7);


  
  MSS_StateManager MSS_sm(in);
  MSS_MoveNeighborhoodExplorer MSS_nhe(in, MSS_sm);
  MSS_SwapNeighborhoodExplorer MSS_nhe2(in, MSS_sm);

  MSS_OutputManager MSS_om(in);
  
  
  MSS_sm.AddCostComponent(cc1); 
  MSS_sm.AddCostComponent(cc2); 
  MSS_sm.AddCostComponent(cc3); 
  MSS_sm.AddCostComponent(cc4); 
  MSS_sm.AddCostComponent(cc5); 
  MSS_sm.AddCostComponent(cc6); 
  MSS_sm.AddCostComponent(cc7); 
  MSS_sm.AddCostComponent(cc8); 
  MSS_sm.AddCostComponent(cc9); 
  
  
  
  
  MSS_nhe.AddDeltaCostComponent(dcc1);
  MSS_nhe.AddDeltaCostComponent(dcc2);
  MSS_nhe.AddDeltaCostComponent(dcc3);
  MSS_nhe.AddDeltaCostComponent(dcc4);
  MSS_nhe.AddDeltaCostComponent(dcc5);
  MSS_nhe.AddDeltaCostComponent(dcc6);
  MSS_nhe.AddDeltaCostComponent(dcc7);
  MSS_nhe.AddDeltaCostComponent(dcc12);
  MSS_nhe.AddDeltaCostComponent(dcc14);

  
  MSS_nhe2.AddDeltaCostComponent(dcc8);
  MSS_nhe2.AddDeltaCostComponent(dcc9);
  MSS_nhe2.AddDeltaCostComponent(dcc10);
  MSS_nhe2.AddDeltaCostComponent(dcc11);
  MSS_nhe2.AddDeltaCostComponent(dcc13);
  
  
  SetUnionNeighborhoodExplorer<MSS_Input, MSS_State, DefaultCostStructure<int>, decltype(MSS_nhe), decltype(MSS_nhe2)> 
  MSS_bnhe(in, MSS_sm, "Bimodal Move/Swap", MSS_nhe, MSS_nhe2, {1.0 - swap_rate, swap_rate});

  
  HillClimbing<MSS_Input, MSS_State, MSS_Move> MSS_hc(in, MSS_sm, MSS_nhe, "MSS_MoveHillClimbing");
  SteepestDescent<MSS_Input, MSS_State, MSS_Move> MSS_sd(in, MSS_sm, MSS_nhe, "MSS_MoveSteepestDescent");
  SimulatedAnnealing<MSS_Input, MSS_State, MSS_Move> MSS_sa(in, MSS_sm, MSS_nhe, "MSS_MoveSimulatedAnnealing");
  HillClimbing<MSS_Input, MSS_State, decltype(MSS_bnhe)::MoveType> MSS_bhc(in, MSS_sm, MSS_bnhe, "MSS_BHC");
  SteepestDescent<MSS_Input, MSS_State, decltype(MSS_bnhe)::MoveType> MSS_bsd(in, MSS_sm, MSS_bnhe, "MSS_BimodalSteepestDescent");
  SimulatedAnnealingEvaluationBased<MSS_Input, MSS_State, decltype(MSS_bnhe)::MoveType> MSS_bsa(in, MSS_sm, MSS_bnhe, "MSS_BSA");

  
  Tester<MSS_Input, MSS_Output, MSS_State> tester(in,MSS_sm,MSS_om);
  MoveTester<MSS_Input, MSS_Output, MSS_State, MSS_Move> move_test(in,MSS_sm,MSS_om,MSS_nhe, "MSS_Move move", tester);
  MoveTester<MSS_Input, MSS_Output, MSS_State, MSS_Swap> swap_move_test(in,MSS_sm,MSS_om,MSS_nhe2, "MSS_Swap move", tester); 
  MoveTester<MSS_Input, MSS_Output, MSS_State, decltype(MSS_bnhe)::MoveType> bimodal_test(in,MSS_sm,MSS_om,MSS_bnhe, "MSS_Bimodal move", tester); 

  SimpleLocalSearch<MSS_Input, MSS_Output, MSS_State> MSS_solver(in, MSS_sm, MSS_om, "MSS solver");
  if (!CommandLineParameters::Parse(argc, argv, true, false))
    return 1;

  string method_name;
  if (!method.IsSet())
    { 
      if (init_state.IsSet())
        tester.RunMainMenu(init_state);
      else
        tester.RunMainMenu();
    }
  else
    {
      method_name = method; 
      if (method_name == "SA")
        MSS_solver.SetRunner(MSS_sa);
      else if (method_name == "HC")
        MSS_solver.SetRunner(MSS_hc);
      else if (method_name == "SD")
        MSS_solver.SetRunner(MSS_sd);
      if (method_name == "BSA" || method_name == "BSA+MIP")
        MSS_solver.SetRunner(MSS_bsa);
      else if (method_name == "BHC")
        MSS_solver.SetRunner(MSS_bhc);
      else if (method_name == "BSD")
        MSS_solver.SetRunner(MSS_bsd);
      else if (method_name == "MIP")
        ;  
      else{
        cerr << "unrecognized method " << method_name << endl;
        exit(1);
      }

#ifdef CPLEX
    tuple<double,double,IloAlgorithm::Status,double> mip_result;    
#endif
	  
    SolverResult<MSS_Input,MSS_Output,DefaultCostStructure<int>> sa_result(in);    
    MSS_Output out(in);
    bool mip_init_sol = false; 
    int violations = 0, cost = 0, initial_cost = 0;
    double time = 0.0, initial_time = 0.0;
    bool two_stage = method_name.find("+") != string::npos; 
    double mip_time = mip_timeout;
    int lower_bound = 0;

    if (two_stage || init_state.IsSet()) 
      mip_init_sol = true;  
    if (init_state.IsSet())
      {
        ifstream is(init_state);
        is >> out;
      }

    if (method_name != "MIP")  
      { 
        if (!init_state.IsSet())
          sa_result = MSS_solver.Solve();
        else 
          sa_result = MSS_solver.Resolve(out);
        out = sa_result.output;
        cost = sa_result.cost.total;
        violations = sa_result.cost.violations;
        time = sa_result.running_time;
        
        if (two_stage)
          {
            initial_time = time;
            initial_cost = cost;
	    mip_time -= initial_time;	
          }
      }

    if (method_name.find("MIP") != string::npos)
      {        
        if (violations == 0 && mip_time > 0) 
          
          {
#ifdef CPLEX
            mip_result = MSS_MIPSolver(in, out, mip_time, mip_init_sol, mip_verbose);
            
            MSS_State st(in);      
            if (get<2>(mip_result) == IloAlgorithm::Unknown)
              {
                cost = -1;
                lower_bound = -1;
              }
            else
              {
                MSS_om.InputState(st,out);
                MSS_om.OutputState(st,out);
                
                cost = cc3.ComputeCost(st) + cc4.ComputeCost(st) + 
                  cc5.ComputeCost(st) + cc6.ComputeCost(st) + 
                  cc7.ComputeCost(st) + cc8.ComputeCost(st) + 
                  cc9.ComputeCost(st);
                lower_bound = get<3>(mip_result);
              }
            time = get<1>(mip_result);
#endif
          }
      }
    if (output_file.IsSet())
      {
        ofstream os(output_file);
        os << out << endl;
        os << "Cost: " << cost;
        if (violations > 0)
          os << "Violations: " << violations;
#ifdef CPLEX       
        if (get<2>(mip_result) == IloAlgorithm::Optimal) os << "(optimal)" << endl;
        else os << "(non optimal)" << endl;
#endif
        os << "Time: " << time << endl;        
        os.close();
      }
    else
      {
        if (two_stage)
          cout << "{\"cost\": " <<  cost <<  ", "
               << "\"initial_cost\": " << initial_cost <<  ", "
               << "\"time\": " << initial_time + time <<  ", "
               << "\"initial_time\": " << initial_time <<  ", "
               << "\"initial_violations\": " << violations <<  ", "
               << "\"lower_bound\": " << lower_bound <<  ", ";
        else          
          cout << "{\"cost\": " <<  cost <<  ", "
               << "\"violations\": " << violations <<  ", "
               << "\"score\": "<< out.GetScore()+out.GetWorstScore()<<","
               << "\"time\": " << time << ", ";
        cout << "\"seed\": " << Random::GetSeed();
#ifdef CPLEX       
        if (method_name.find("MIP") != string::npos)
          {
            cout << ", \"mip_status\": \"" << get<2>(mip_result) << "\"";
          }
#endif
        cout  << "} " << endl;	
      }
    }
  return 0;
}
