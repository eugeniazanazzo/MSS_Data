#if defined(CPLEX)
#include "MSS_MIPSolver.hh"
#include <chrono>
#include <algorithm> 

using namespace std;

tuple<double,double,IloAlgorithm::Status,double> MSS_MIPSolver(const MSS_Input& in, MSS_Output& out, double timeout, bool init_solution, bool verbose)
{ 
  unsigned s, d0, d, d1, d2, h, t, g, i, tot_ksg, w, t1, duration;
  int start_time;
  const unsigned BIG_M = 
    in.TimePeriods(); 

  
  stringstream name;

  IloEnv env;
  IloModel model(env);  
  IloCplex solver(model);

  
  IloArray<IloArray<IloArray<IloBoolVarArray>>> v(env,in.Students()); 
  IloArray<IloArray<IloArray<IloBoolVarArray>>> y(env,in.Students()); 
  IloNumVarArray Des(env, in.Students()); 
  IloNumVar Des_min(env, -IloInfinity, IloNumMax, "Des_min"); 
  IloArray<IloNumVarArray> W(env, in.Students()); 
  IloArray<IloBoolVarArray> Ch(env,in.Students()); 

  
  for (s = 0; s < in.Students(); s++)
    {
      v[s] = IloArray<IloArray<IloBoolVarArray>>(env,in.Disciplines()+1); 
      for (d = 0; d <= in.Disciplines(); d++)
        {
          v[s][d] = IloArray<IloBoolVarArray>(env,in.TimePeriods()); 
          for (t = 0; t < in.TimePeriods(); t++)
            {
              v[s][d][t] = IloBoolVarArray(env,in.Hospitals()); 
              for (h = 0; h < in.Hospitals(); h++)
                {         
                  name << "v_" << s << "_" << d << "_" << t << "_" << h;
                  v[s][d][t][h] =  IloBoolVar(env, name.str().c_str());
                  name.str("");  
                }
            }
        }
    }


  for (s = 0; s < in.Students(); s++)
    {
      y[s] = IloArray<IloArray<IloBoolVarArray>>(env,in.Disciplines()+1); 
      for (d1 = 0; d1 <= in.Disciplines(); d1++)
        {
          y[s][d1] = IloArray<IloBoolVarArray>(env,in.Disciplines()+1); 
          for (d2 = 0; d2 <= in.Disciplines(); d2++)
            {
              y[s][d1][d2] = IloBoolVarArray(env,in.Hospitals()); 
              for (h = 0; h < in.Hospitals(); h++)
                {         
                  name << "y_" << s << "_" << d1 << "_" << d2 << "_" << h;
                  y[s][d1][d2][h] =  IloBoolVar(env, name.str().c_str());
                  name.str("");  
                }
            }
        }
    }
   for (s = 0; s < in.Students(); s++)     {
       name << "Des_" << s;
       Des[s] = IloNumVar(env, -IloInfinity, IloNumMax, name.str().c_str()); 
       name.str(""); 
   }
   
   for (s = 0; s < in.Students(); s++)
     {
       W[s] = IloNumVarArray(env, in.Disciplines()+1);
       for (d = 0; d <= in.Disciplines(); d++)
         {
           name << "W_" << s << "_" << d;
           W[s][d] = IloNumVar(env, 0.0, in.TimePeriods(), name.str().c_str());
           name.str(""); 
         }
     }
   
   for (s = 0; s < in.Students(); s++)
     {
       Ch[s] = IloBoolVarArray(env, in.Disciplines()+1);
       for (d = 0; d <= in.Disciplines(); d++)
         {
           name << "Ch_" << s << "_" << d;
           Ch[s][d] = IloBoolVar(env, name.str().c_str());
           name.str(""); 
         }
     }
  
   for (s = 0; s < in.Students(); s++)
     for (g = 0; g < in.Groups(); g++)
       {
         if (in.StudentsVector(s).SizeCompDiscList(g) > 0) 
           {
             IloExpr sum_expr(env);
             for (i = 0; i < in.StudentsVector(s).SizeCompDiscList(g); i++) 
               {
                 d = in.StudentsVector(s).GetCompDiscList(g,i);
                 for (t = 0; t < in.TimePeriods(); t++)
                   for (h = 0; h < in.Hospitals(); h++)
                     {
                       sum_expr += v[s][d][t][h];
                     }
               }
             model.add(sum_expr >= IloInt(in.GetToFullfill(s,g)));
           }
       }
   // 3. Total number of courses a student must attend
   for (s = 0; s < in.Students(); s++)
     {
       IloExpr sum_expr(env);
       tot_ksg = 0;
       
       for (d = 0; d < in.Disciplines(); d++) 
	 for (t = 0; t < in.TimePeriods(); t++)
           for (h = 0; h < in.Hospitals(); h++)
             sum_expr += v[s][d][t][h];
       for (g = 0; g < in.Groups(); g++)
         {
           tot_ksg += in.GetToFullfill(s,g);
         }
       model.add(sum_expr == IloInt(tot_ksg));
     }
   
   // 4. A discipline can be preceded by at most one other discipline
   for (s = 0; s < in.Students(); s++)
     for (d2 = 0; d2 < in.Disciplines(); d2++) 
       {
         IloExpr sum_expr(env);
         for (d1 = 0; d1 <= in.Disciplines(); d1++)
           for (h = 0; h < in.Hospitals(); h++)
             sum_expr += y[s][d1][d2][h];
         model.add(sum_expr <= 1);
       }
   
   // 5. the flow for each student is originated from in the dummy stard node (discipline in.Disciplines())
   for (s = 0; s < in.Students(); s++)
     {
       IloExpr sum_expr(env);
       for (d = 0; d < in.Disciplines(); d++)
         for (h = 0; h < in.Hospitals(); h++)
           sum_expr += y[s][in.Disciplines()][d][h];
       model.add(sum_expr == 1);
     }
   
   // 6. A discipline can not be succeded by the same discipline or the dummy node
   IloExpr sum_expr(env);
   for (s = 0; s < in.Students(); s++)
     for (d = 0; d <= in.Disciplines(); d++)
       for (h = 0; h < in.Hospitals(); h++)
         sum_expr += y[s][d][d][h] + y[s][d][in.Disciplines()][h];
   model.add(sum_expr == 0);
   
   // 7. starting from discipline d1, discipline d2 can only be selected if there is also a preceding discipline d0 for d1
   for (s = 0; s < in.Students(); s++)
     for (d1 = 0; d1 < in.Disciplines(); d1++) 
       {
         IloExpr sum_expr(env), sum_expr2(env);  
         for (d2 = 0; d2 < in.Disciplines(); d2++)
           for (h = 0; h < in.Hospitals(); h++)
             sum_expr += y[s][d1][d2][h];
         
         for (d0 = 0; d0 <= in.Disciplines(); d0++)
           for (h = 0; h < in.Hospitals(); h++)
             sum_expr2 += y[s][d0][d1][h];
         model.add(sum_expr <= sum_expr2);
       }
   // 8. A discipline can only start when it is selected
   for (s = 0; s < in.Students(); s++)
     for (d2 = 0; d2 < in.Disciplines(); d2++) 
       for (h = 0; h < in.Hospitals(); h++)
         {
           IloExpr sum_expr(env), sum_expr2(env);
           for (t = 0; t < in.TimePeriods(); t++)
             sum_expr += v[s][d2][t][h];
           
           for (d1 = 0; d1 <= in.Disciplines(); d1++)
             sum_expr2 += y[s][d1][d2][h];
          model.add(sum_expr == sum_expr2);
         }

    // 9. Precedence between pairs of disciplines (corrected as suggested by Babak)
     for (d2 = 0; d2 < in.Disciplines(); d2++)  
       for (d1 = 0; d1 < in.Disciplines(); d1++) 
         {
	   if(in.DisciplinesVector(d2).GetDiscRequired(d1))
	     {
               for (s = 0; s < in.Students(); s++)
                 {
                   IloExpr sum_expr(env), sum_expr2(env), sum_expr3(env), sum_expr4(env);
                   
                   for (t = 0; t < in.TimePeriods(); t++)
                     for (h = 0; h < in.Hospitals(); h++)
                       {
                         sum_expr += IloInt(t) * v[s][d2][t][h];  
                         sum_expr2 += v[s][d2][t][h];
                         sum_expr3 += IloInt(t) * v[s][d1][t][h];
                         sum_expr4 += v[s][d1][t][h];
                       }
                   model.add(sum_expr - IloInt(BIG_M) * sum_expr2 >= sum_expr3 - IloInt(BIG_M) * sum_expr4);
                 }
	     }
         }
    // 10. Maximum number of disciplines in a hospital for a student
    for (s = 0; s < in.Students(); s++)
     for (h = 0; h < in.Hospitals(); h++)
       {
         IloExpr sum_expr(env);
         for (d1 = 0; d1 <= in.Disciplines(); d1++) 
           for (d2 = 0; d2 < in.Disciplines(); d2++)
             sum_expr += y[s][d1][d2][h]; 
         
         model.add(sum_expr <= IloInt(in.GetMaxDisc()));
       }      
    // 11. Minimum student staffing for each ward (remind: wards and disciplines are in one-to-one relation)
   for (t = 0; t < in.TimePeriods(); t++)
     for (w = 0; w < in.Wards(); w++) 
       for (h = 0; h < in.Hospitals(); h++)
         {
           IloExpr sum_expr(env);
           start_time = max(0,static_cast<int>(t) - static_cast<int>(in.Duration()) + 1);
           for (t1 = start_time; t1 <= t; t1++)
             for (s = 0; s < in.Students(); s++)
               {
                 sum_expr += v[s][w][t1][h]; 
               }
           model.add(sum_expr >= IloInt(in.HospitalsVector(h).GetMinDemand(t, w)));  
         }     
    // 12. Maximum student staffing for each ward
   for (t = 0; t < in.TimePeriods(); t++)
     for (w = 0; w < in.Wards(); w++) 
       for (h = 0; h < in.Hospitals(); h++)
         {
           IloExpr sum_expr(env);
           for (s = 0; s < in.Students(); s++)
             {
               start_time = max(0, static_cast<int>(t) - static_cast<int>(in.Duration())+1);
               for (t1 = start_time; t1 <= t; t1++)
                 sum_expr += v[s][w][t1][h]; 
             }
           model.add(sum_expr <= IloInt(in.HospitalsVector(h).GetMaxDemand(t, w)));
         }
   // 13. Assignments conform to student abilities      
   for (s = 0; s < in.Students(); s++)
     for (d = 0; d < in.Disciplines(); d++)
       for (h = 0; h < in.Hospitals(); h++)
         {
           IloExpr sum_expr(env);
           for (t = 0; t < in.TimePeriods(); t++)
             sum_expr += v[s][d][t][h];
           model.add(sum_expr <=  IloInt(in.StudentsVector(s).GetAbility(h,d)));
         }
   // 14. Student availability
   for (s = 0; s < in.Students(); s++)
     {
       for (t = 0; t < in.TimePeriods(); t++)
         {
           for (d = 0; d < in.Disciplines(); d++)
             {
               IloExpr sum_expr(env);
               unsigned count = 0;
               for (h = 0; h < in.Hospitals(); h++)
                 sum_expr += v[s][d][t][h] * IloInt(in.Duration()); 
               for (t1 = t; t1 < min(t+in.Duration(),in.TimePeriods()); t1++)
                 count += in.GetAvailability(s, t1);
               model.add(sum_expr <= IloInt(count));
             }
         }
     }
     // 15. A student changes hospital
   for (s = 0; s < in.Students(); s++)
     {
       for (d1 = 0; d1 < in.Disciplines(); d1++)
         for (d2 = 0; d2 < in.Disciplines(); d2++)
           for (h = 0; h < in.Hospitals(); h++)
             {
               IloExpr sum_expr(env);
               for (d0 = 0; d0 <= in.Disciplines(); d0++)
                 sum_expr += y[s][d0][d1][h];
               model.add(Ch[s][d2] >= y[s][d1][d2][h] - sum_expr);
             }		      
     }
   
  // 16-17. Waiting times between consecutive disciplines
   for (s = 0; s < in.Students(); s++)
     for (d1 = 0; d1 <= in.Disciplines(); d1++)
       {
	 if(d1 == in.Disciplines()) 
	   duration = 0;
	 else
	   duration = in.Duration();
       for (d2 = 0; d2 < in.Disciplines(); d2++)
           {
             IloExpr sum_expr1(env), sum_expr2(env), sum_expr3(env);
             for (t = 0; t < in.TimePeriods(); t++)
               for (h = 0; h < in.Hospitals(); h++)
                 {
                   sum_expr1 += v[s][d2][t][h] * IloInt(t);  
                   sum_expr2 += v[s][d1][t][h] * IloInt(t);
                 }
             
             for (h = 0; h < in.Hospitals(); h++)
               sum_expr3 += y[s][d1][d2][h];
             
              model.add(sum_expr1 >= W[s][d2] + sum_expr2 + IloInt(duration) - IloInt(BIG_M)*(1-sum_expr3));
           }
       }

   for (s = 0; s < in.Students(); s++)
     for (d1 = 0; d1 <= in.Disciplines(); d1++)  
       {
	 if(d1 == in.Disciplines()) 
	   duration = 0;
	 else
	   duration = in.Duration();
	 for (d2 = 0; d2 < in.Disciplines(); d2++)
	   {
	     IloExpr sum_expr1(env), sum_expr2(env), sum_expr3(env);
	     for (t = 0; t < in.TimePeriods(); t++)
	       for (h = 0; h < in.Hospitals(); h++)
		 {
		   sum_expr1 += v[s][d2][t][h] * IloInt(t);
		   sum_expr2 += v[s][d1][t][h] * IloInt(t);
		 }
	     
	     for (h = 0; h < in.Hospitals(); h++)
	       sum_expr3 += y[s][d1][d2][h];
	     
	     model.add(sum_expr1 <= W[s][d2] + sum_expr2 + IloInt(duration) + IloInt(BIG_M)*(1-sum_expr3));
	   }
       }
   // 18. Desire score for each student
   for (s = 0; s < in.Students(); s++)
       {
	 IloExpr sum_expr1(env), sum_expr2(env), sum_expr3(env),  sum_expr4(env), sum_expr5(env);
	 for (d1 = 0; d1 <= in.Disciplines(); d1++)
	   for (d2 = 0; d2 < in.Disciplines(); d2++)
	     for (h = 0; h < in.Hospitals(); h++)
	       {
		 sum_expr1 += IloInt(in.StudentsVector(s).GetDisciplineW()) * IloInt(in.GetDiscPref(s,d2)) * y[s][d1][d2][h];
		 sum_expr2 += IloInt(in.StudentsVector(s).GetHospitalW()) * IloInt(in.GetHospPref(s,h)) * y[s][d1][d2][h];		 
		 sum_expr5 += IloInt(in.GetMangPref(d2)) * y[s][d1][d2][h]; 
	       }
	 for (d1 = 0; d1 <= in.Disciplines(); d1++)
	   {
	     sum_expr3 += W[s][d1] * IloInt(in.StudentsVector(s).GetWaitW());
	     sum_expr4 += Ch[s][d1] * IloInt(in.StudentsVector(s).GetChangeW());
	   }
  	 model.add(Des[s] <= sum_expr1 + sum_expr2 + sum_expr3 + sum_expr4 + sum_expr5);  
       }
     // 19. Worst desire score across all students
     for (s = 0; s < in.Students(); s++)
       model.add(Des_min <= Des[s]);
     // 20. Additional constraints (not in the original paper)
      for (s = 0; s < in.Students(); s++)
       {
	 IloExpr sum_expr(env);
	 for (h = 0; h < in.Hospitals(); h++)
	   sum_expr += v[s][in.Disciplines()][0][h];
	 model.add(sum_expr == 1);
       }
      
      for (s = 0; s < in.Students(); s++)
	{
	  IloExpr sum_expr(env);
	  for (h = 0; h < in.Hospitals(); h++)
	    for (t = 0; t < in.TimePeriods(); t++)
	      sum_expr += v[s][in.Disciplines()][t][h];
	  model.add(sum_expr == 1);
	}
  // objective  Eq (1)
  IloNumExpr obj_expr(env);
  for (s = 0; s < in.Students(); s++)
    obj_expr += Des[s];
  obj_expr += Des_min;
  model.add(IloMaximize(env, obj_expr));
  if (init_solution)
    {
      IloNumVarArray startVar(env);
      IloNumArray startVal(env);
      for (s = 0; s < in.Students(); s++)
        for (t = 0; t < in.TimePeriods(); t++) 
          {
            if (out(s,t) != -1 && 
                (t == 0 || out(s,t) != out(s,t-1)))
              {
                h = out(s,t) / in.Wards();
                d = out(s,t) % in.Wards();
                startVar.add(v[s][d][t][h]);
                startVal.add(1);
              }
          }                
      solver.addMIPStart(startVar, startVal);
      startVar.end();
      startVal.end();
    }

  if (!verbose)
    solver.setOut(env.getNullStream());


  if (timeout > 0)
    solver.setParam(IloCplex::Param::TimeLimit, timeout);
  
  chrono::time_point<chrono::system_clock> start, end;
  start = chrono::system_clock::now();
  solver.solve();
  end = chrono::system_clock::now();

  tuple<int,double,IloAlgorithm::Status,double> result;
  get<1>(result) = chrono::duration_cast<chrono::seconds>(end-start).count();
  get<1>(result) = solver.getTime();
  get<2>(result) = solver.getStatus();
  get<3>(result) = solver.getBestObjValue(); 

  if (get<2>(result) == IloAlgorithm::Feasible || get<2>(result) == IloAlgorithm::Optimal)
    {
      out.Reset();
      for (s = 0; s < in.Students(); s++)
        for (t = 0; t < in.TimePeriods(); t++) 
          {
            for (d = 0; d < in.Disciplines(); d++)
              for (h = 0; h < in.Hospitals(); h++)
                if (solver.getValue(v[s][d][t][h]) > 0.95) 
                  {
                    for (t1 = t; t1 < t + in.Duration(); t1++) 
                      out(s,t1) = h * in.Wards() + d;
                  }
          }
      get<0>(result) = solver.getObjValue();
    }
  else
    get<0>(result) = 100000000; 
  return result;
}

void PrintCosts(const IloCplex& solver, const MSS_Input& in, 
                const IloArray<IloArray<IloArray<IloBoolVarArray>>>& y,
                const IloArray<IloArray<IloArray<IloBoolVarArray>>>& v,
                const IloArray<IloNumVarArray>& W,
                const IloArray<IloBoolVarArray>& Ch)
{
  vector<int> DiscSc(in.Students(),0), HospSc(in.Students(),0), ManSc(in.Students(),0), WaitSc(in.Students(),0), ChngSc(in.Students(),0);
  unsigned s, d1, d2, h, t;
  cerr << "y d1 d2 h" << endl;
  for (s = 0; s < in.Students(); s++)
    {
      for (d1 = 0; d1 <= in.Disciplines(); d1++)
        {
          for (d2 = 0; d2 < in.Disciplines(); d2++)
            for (h = 0; h < in.Hospitals(); h++)
              {
                if (solver.getValue(y[s][d1][d2][h]) > 0.95)
                  cerr << "y" << " " << s << " " << d1 << " " << d2 << " " << h << " " << endl;
                DiscSc[s] += in.StudentsVector(s).GetDisciplineW() * in.GetDiscPref(s,d2) * static_cast<int>(solver.getValue(y[s][d1][d2][h]) + 0.5);
                HospSc[s] += in.StudentsVector(s).GetHospitalW() * in.GetHospPref(s,h) * static_cast<int>(solver.getValue(y[s][d1][d2][h]) + 0.5);
                ManSc[s] += in.GetMangPref(d2) * static_cast<int>(solver.getValue(y[s][d1][d2][h]) + 0.5); 
              }        
          WaitSc[s] += static_cast<int>(solver.getValue(W[s][d1] + 0.5)) * in.StudentsVector(s).GetWaitW();
          ChngSc[s] += static_cast<int>(solver.getValue(Ch[s][d1] + 0.5)) * in.StudentsVector(s).GetChangeW();
        }
    }
  cerr << "v s d t h " << endl; 
  for (s = 0; s < in.Students(); s++)
    {
      for (d1 = 0; d1 <= in.Disciplines(); d1++)
        {
          for (t = 0; t < in.TimePeriods(); t++)
            for (h = 0; h < in.Hospitals(); h++)
              {
                if (solver.getValue(v[s][d1][t][h]) > 0.95)
                  cerr << "v " << s << " " << d1 << " " << t << " " << h << " " << endl;
              }
        }
    }

  cout << "DiscSc" << endl;
  for (s = 0; s < in.Students(); s++)
    cout << s << " " << DiscSc[s] << endl;
  cout  << "HospSc" << endl;
  for (s = 0; s < in.Students(); s++)
    cout << s << " " << HospSc[s] << endl;
  cout  << "ManSc" << endl;
  for (s = 0; s < in.Students(); s++)
    cout << s << " " << ManSc[s] << endl;
  cout  << "WaitSc" << endl;
  for (s = 0; s < in.Students(); s++)
    cout << s << " " << WaitSc[s] << " (" << WaitSc[s]/in.StudentsVector(s).GetWaitW() << ")" << endl;
  cout  << "ChngSc" << endl;
  for (s = 0; s < in.Students(); s++)
    cout << s << " " << ChngSc[s] << endl;
}
#endif
