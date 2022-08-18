// File MSS_Helpers.hh
#ifndef MSS_HELPERS_HH
#define MSS_HELPERS_HH

#include "MSS_Basics.hh"
#include <easylocal.hh>

using namespace EasyLocal::Core;

/***************************************************************************
 * State Manager 
 ***************************************************************************/

class MSS_StateManager : public StateManager<MSS_Input,MSS_State> 
{
public:
  MSS_StateManager(const MSS_Input &);
  void RandomState(MSS_State&);   
  bool CheckConsistency(const MSS_State& st) const;
protected:
}; 

class MSS_HospMaxRequirements : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_HospMaxRequirements(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_HospMaxRequirements") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
};

class MSS_HospMinRequirements : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_HospMinRequirements(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_HospMinRequirements") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
};

class MSS_PrecRequirements : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_PrecRequirements(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_PrecRequirements") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
};

class MSS_DiscPreference : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_DiscPreference(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_DiscPreference") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
protected:
};

class MSS_HospPreference : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_HospPreference(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_HospitalPref") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
protected:
};

class MSS_ManPreference : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_ManPreference(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_ManPreference") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
protected:
};

class MSS_WorstScore : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_WorstScore(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_WorstScore") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
protected:
};

class MSS_WaitingCosts : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_WaitingCosts(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_WaitingCosts") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
};

class MSS_ChangeCosts : public CostComponent<MSS_Input,MSS_State> 
{
public:
  MSS_ChangeCosts(const MSS_Input & in, int w, bool hard) :    CostComponent<MSS_Input,MSS_State>(in,w,hard,"MSS_ChangeCosts") 
  {}
  int ComputeCost(const MSS_State& st) const;
  void PrintViolations(const MSS_State& st, ostream& os = cout) const;
};

/***************************************************************************
 * MSS_Move Neighborhood Explorer:
 ***************************************************************************/


class MSS_MoveDeltaHospMaxRequirements
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaHospMaxRequirements(const MSS_Input & in, MSS_HospMaxRequirements& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaHospMaxRequirements") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaHospMinRequirements
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaHospMinRequirements(const MSS_Input & in, MSS_HospMinRequirements& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaHospMinRequirements") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaPrecRequirements
 : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaPrecRequirements(const MSS_Input & in, MSS_PrecRequirements& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaPrecRequirements") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaDiscPreference
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaDiscPreference(const MSS_Input & in, MSS_DiscPreference& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaDiscPreference") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaHospPreference
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaHospPreference(const MSS_Input & in, MSS_HospPreference& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaHospPreference") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaMngrPreference
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaMngrPreference(const MSS_Input & in, MSS_ManPreference& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaMngrPreference") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaWorstScore
    : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaWorstScore(const MSS_Input & in, MSS_WorstScore& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaWorstScore") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaWaitingCosts
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaWaitingCosts(const MSS_Input & in, MSS_WaitingCosts& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaWaitingCosts") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_MoveDeltaChangeCosts
    : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>
{
public:
  MSS_MoveDeltaChangeCosts(const MSS_Input & in, MSS_ChangeCosts& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Move>(in,cc,"MSS_MoveDeltaChangeCosts") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const;
};

class MSS_SwapDeltaHospMaxRequirements
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>
{
public:
  MSS_SwapDeltaHospMaxRequirements(const MSS_Input & in, MSS_HospMaxRequirements& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>(in,cc,"MSS_SwapDeltaHospMaxRequirements") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Swap& mv) const;
};

class MSS_SwapDeltaHospMinRequirements
  : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>
{
public:
  MSS_SwapDeltaHospMinRequirements(const MSS_Input & in, MSS_HospMinRequirements& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>(in,cc,"MSS_SwapDeltaHospMinRequirements") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Swap& mv) const;
};


class MSS_SwapDeltaPrecRequirements
 : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>
{
public:
  MSS_SwapDeltaPrecRequirements(const MSS_Input & in, MSS_PrecRequirements& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>(in,cc,"MSS_SwapDeltaPrecRequirements") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Swap& mv) const;
};

class MSS_SwapDeltaChangeCosts
    : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>
{
public:
  MSS_SwapDeltaChangeCosts(const MSS_Input & in, MSS_ChangeCosts& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>(in,cc,"MSS_SwapDeltaChangeCosts") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Swap& mv) const;
};

class MSS_SwapDeltaWorstScore
    : public DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>
{
public:
  MSS_SwapDeltaWorstScore(const MSS_Input & in, MSS_WorstScore& cc) 
    : DeltaCostComponent<MSS_Input,MSS_State,MSS_Swap>(in,cc,"MSS_SwapDeltaWorstScore") 
  {}
  int ComputeDeltaCost(const MSS_State& st, const MSS_Swap& mv) const;
};

class MSS_MoveNeighborhoodExplorer
  : public NeighborhoodExplorer<MSS_Input,MSS_State,MSS_Move> 
{
public:
  MSS_MoveNeighborhoodExplorer(const MSS_Input & pin, StateManager<MSS_Input,MSS_State>& psm)  
    : NeighborhoodExplorer<MSS_Input,MSS_State,MSS_Move>(pin, psm, "MSS_MoveNeighborhoodExplorer") {} 
  void RandomMove(const MSS_State&, MSS_Move&) const;
  bool FeasibleMove(const MSS_State& st, const MSS_Move& mv) const;           
  void MakeMove(MSS_State&, const MSS_Move&) const;             
  void FirstMove(const MSS_State&, MSS_Move&) const;  
  bool NextMove(const MSS_State&, MSS_Move&) const; 
  void AnyFirstMove(const MSS_State&, MSS_Move&) const;  
  bool AnyNextMove(const MSS_State&, MSS_Move&) const; 
  void FirstPeriodAvail(const MSS_State&, MSS_Move&) const; 
  void FirstNewWard(const MSS_State&, MSS_Move&) const;
  void FirstFromIndex (const MSS_State&, MSS_Move&) const;
  void FirstNewDiscipline(const MSS_State&, MSS_Move&) const;
  bool DiffDisc(const MSS_State&, MSS_Move&) const;
  bool NextPeriodAvail(const MSS_State&, MSS_Move&) const;
  bool NextToWard(const MSS_State&, MSS_Move&) const;
  bool NextFromIndex(const MSS_State&, MSS_Move&) const;
  bool NextDiscipline(const MSS_State&, MSS_Move&) const;
protected:
};

class MSS_SwapNeighborhoodExplorer
  : public NeighborhoodExplorer<MSS_Input,MSS_State,MSS_Swap> 
{
public:
  MSS_SwapNeighborhoodExplorer(const MSS_Input & pin, StateManager<MSS_Input,MSS_State>& psm)  
    : NeighborhoodExplorer<MSS_Input,MSS_State,MSS_Swap>(pin, psm, "MSS_SwapNeighborhoodExplorer") {} 
  void RandomMove(const MSS_State&, MSS_Swap&) const;       
  void MakeMove(MSS_State&, const MSS_Swap&) const;             
  void FirstMove(const MSS_State&, MSS_Swap&) const;  
  bool NextMove(const MSS_State&, MSS_Swap&) const; 
  void FirstFromIndex (const MSS_State&, MSS_Swap&) const;
  void FirstToIndex(const MSS_State&, MSS_Swap&) const;
  bool NextToIndex(const MSS_State&, MSS_Swap&) const;
  bool NextFromIndex(const MSS_State&, MSS_Swap&) const;
protected:
};

/***************************************************************************
 * Output Manager:
 ***************************************************************************/
class MSS_OutputManager
  : public OutputManager<MSS_Input,MSS_Output,MSS_State> 
{
public:
  MSS_OutputManager(const MSS_Input & pin)
    : OutputManager<MSS_Input,MSS_Output,MSS_State>(pin,"MSSOutputManager") {}
  void PrettyPrintOutput(const MSS_State &st, const string &file_name) const;  
  void InputState(MSS_State&, const MSS_Output&) const;  
  void OutputState(const MSS_State&, MSS_Output&) const; 
}; 
#endif
