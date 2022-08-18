// File MSS_Basics.hh
#ifndef MSS_BASICS_HH
#define MSS_BASICS_HH

#include "MSS_Data.hh"

class MSS_State
{  
  friend ostream& operator<<(ostream& os, const MSS_State& st);
  friend bool operator==(const MSS_State& st1, const MSS_State& st2);
public:
  MSS_State(const MSS_Input &in);
  MSS_State& operator=(const MSS_State& s);
  int operator()(unsigned i, unsigned j) const { return schedule[i][j]; }
  int& operator()(unsigned i, unsigned j) { return schedule[i][j]; }
  

  bool Allocated(unsigned s,unsigned d) const {return allocated[s][d]; }
  void Allocate(unsigned s,unsigned d) {allocated[s][d]=true;}
  
  void Unallocate(unsigned s,unsigned d){allocated[s][d]=false;}
  
  unsigned StudentsAvailable(unsigned i, unsigned j) const 
    { return students_available[i][j]; }
  void ResetStudentsAvailable(unsigned i, unsigned j) 
    { students_available[i][j] = 0; }
  void IncStudentsAvailable(int i, unsigned j) 
    { students_available[i][j]++; }
  void DecStudentsAvailable(unsigned i, unsigned j) 
    { students_available[i][j]--; }
  
  void SetFinished(unsigned i,unsigned j,unsigned k) 
    { finished[i][j]=k; }
  unsigned GetFinished (unsigned i,unsigned j) const
    { return finished[i][j];}
  
  void IncDiscperHosp(unsigned s,unsigned h){disc_per_hosp[s][h]++;}
  void DecDiscperHosp(unsigned s,unsigned h){disc_per_hosp[s][h]--;}
  unsigned GetDiscperHosp(unsigned s, unsigned h) const {return disc_per_hosp[s][h];}
  
  int FirstPeriod(unsigned s) const {return first_period[s];}
  int LastPeriod(unsigned s) const {return last_period[s];}
  void SetFirstPeriod(unsigned s,unsigned k) {first_period[s]=k;}
  void SetLastPeriod(unsigned s,unsigned k) {last_period[s]=k;}
  
  void UpdateFirstLast(unsigned s,unsigned t); 
  
  int GetAtHospital(unsigned i,unsigned j) const  {return at_hospital[i][j];}
  void SetAtHospital(unsigned i,unsigned j,unsigned k) {at_hospital[i][j]=k;}

  
  int Waiting(unsigned i) const {return waiting_times[i];}
  int Change (unsigned i) const {return changes[i];}
  void AddWaitingTimes(unsigned s,int k){waiting_times[s]+=k;}
  
  void AddChanges(unsigned s,int k) {changes[s]+=k;}
  
  unsigned HospScore(unsigned s) const {return hospital_pr_score[s];}
  unsigned DiscScore(unsigned s) const {return discipline_pr_score[s];}
  unsigned MngrScore(unsigned s) const {return manager_pr_score[s];}
  void AddDiscScore(unsigned s,int k) {discipline_pr_score[s]+=k;}
  void DecDiscScore(unsigned s,unsigned k) {discipline_pr_score[s]-=k;}
  void AddHospScore(unsigned s,unsigned k) {hospital_pr_score[s]+=k;}
  void DecHospScore(unsigned s,unsigned k) {hospital_pr_score[s]-=k;}
  void AddMngrScore(unsigned s,int k) {manager_pr_score[s]+=k;}
  int GetWaitScore(unsigned i) const {return wait_score[i];}
  void AddWaitScore(unsigned i,int k) {wait_score[i]+=k;}
  void SetWaitScore(unsigned i,int k) {wait_score[i]=k;} 
  int GetChangeScore(unsigned i) const {return change_score[i];}
  void AddChangeScore(unsigned i,int k){change_score[i]+=k;}
  void SetChangeScore(unsigned i,int k){change_score[i]=k;} 
  int GetTotalScore(unsigned i) const {return total_score[i];}
  void AddTotalScore(unsigned i,int k) {total_score[i]+=k;}
  unsigned GetRealScore() const {return real_score;}
  void AddRealScore(int k){real_score+=k;}
  int GetWorstScore() const {return worst_score;}
  void AddWorstScore(int k){worst_score+=k;}
  
  int GetWorstStudent() const {return worst_student;}
  int GetGapToWorst(unsigned i) const {return gap_to_worst[i];}


  
  unsigned AssignmentsSize(unsigned s) const {return assignments[s].size();}
  void AddAssignment(unsigned s,unsigned w,unsigned tp) {assignments[s].push_back(make_pair(w,tp));}
  unsigned AssignmentWard(unsigned s, unsigned i) const {return assignments[s][i].first;}
  unsigned AssignmentTime(unsigned s, unsigned i) const {return assignments[s][i].second;}
  void ChangeAssignment(unsigned s,unsigned i,unsigned nw,unsigned nt); 
  void SwapAssignment(unsigned s,unsigned i,unsigned i2);  
  void ResetAssignments(unsigned s){assignments[s].clear();}
  
  unsigned FreeDiscSize(unsigned s) const {return free_disciplines[s].size();}
  void AddFreeDisc(unsigned s,unsigned g,unsigned d) {free_disciplines[s].push_back(make_pair(g,d));}
  unsigned FreeDiscGroup(unsigned s, unsigned i) const {return free_disciplines[s][i].first;}
  unsigned FreeDiscDisc(unsigned s, unsigned i)  const {return free_disciplines[s][i].second;}
  void ChangeFreeDisc(unsigned s,unsigned i,unsigned od) {free_disciplines[s][i].second=od;} 
  void ResetFreeDisc(unsigned s) {free_disciplines[s].clear();}
  
  unsigned FreeWardSize(unsigned s) const {return free_wards[s].size();}
  void AddFreeWard(unsigned s,unsigned w) {free_wards[s].push_back(w);}
  unsigned FreeWard(unsigned s, unsigned i)  const {return free_wards[s][i];}
  void ChangeFreeWards(unsigned s,unsigned i,unsigned ow) {free_wards[s][i]=ow;}
  void ResetFreeWards(unsigned s){free_wards[s].clear();}
  
  int SearchNewFirst(unsigned s,unsigned index) const; 
  int SearchNewLast(unsigned s,unsigned index) const; 
  int SearchNewFirst(unsigned s,unsigned index1,unsigned index2) const; 
  int SearchNewLast(unsigned s,unsigned index1,unsigned index2) const;  
  int NewFirst(unsigned s,int from,int to) const; 
  int NewLast(unsigned s,int from,int to) const;  
  int SearchRightof(unsigned s,unsigned otp,unsigned ntp) const; 
  int SearchLeftof(unsigned s,unsigned otp,unsigned ntp) const;  
  int SearchRightof(unsigned s,unsigned tp) const;  
  int SearchLeftof(unsigned s,unsigned tp) const;   


  
  int DeltaChangeMove(unsigned s,int from,int to,int new_ward,int new_first,int new_last) const;
  int DeltaWait(unsigned s,int from,int to,int new_first,int new_last) const;
  int DeltaChangeSwap(unsigned s,int from,int to) const;


  
  void CheckWorst();
  void UpdateAllGaps(int delta);
  void UpdateWorst(unsigned s);
  void UpdateGapToWorst(unsigned s);
  int  NewWorst(int delta) const;

  
  bool IntoBounds(int t,unsigned d) const; 
  bool Occupied (unsigned s,int t,unsigned d) const;
  bool OtOccupied(unsigned s,int t,unsigned d,int nw) const;
  bool CanAttend (unsigned s,int t,unsigned d) const;
  bool Ready(unsigned s,unsigned d) const;
  bool IsAble(unsigned s,unsigned h,unsigned d) const;

  void UpdateRedundantStateData();
  void UpdateAdditionalRedundantStateData(); 
  void UpdateScores();   
  void ResetState();

protected:
  const MSS_Input & in;
  
  vector<vector<int>> schedule;
  
  unsigned real_score;
  int worst_score;
  int worst_student;
  vector<unsigned> hospital_pr_score; 
  vector<unsigned> discipline_pr_score;
  vector<unsigned> manager_pr_score;
  vector<int> wait_score;
  vector<int> change_score;
  vector<int> total_score;
  vector<int>gap_to_worst;
  
  
  vector<vector<pair<unsigned,unsigned>>> assignments;
  vector<vector<unsigned>>finished;
  vector<vector<bool>> allocated;
  vector<vector<int>>at_hospital;
  vector<vector<unsigned>>periods_at_hospital;
  vector<vector<unsigned>> students_available;
  vector<int> waiting_times; 
  vector<int> changes; 
  vector<int> first_period; 
  vector<int> last_period;   
  vector<vector<pair<unsigned,unsigned>>>free_disciplines;
  vector<vector<unsigned>>free_wards;
  vector<vector<unsigned>>disc_per_hosp;
};

class MSS_Move
{
  friend bool operator==(const MSS_Move& m1, const MSS_Move& m2);
  friend bool operator!=(const MSS_Move& m1, const MSS_Move& m2);
  friend bool operator<(const MSS_Move& m1, const MSS_Move& m2);
  friend ostream& operator<<(ostream& os, const MSS_Move& c);
  friend istream& operator>>(istream& is, MSS_Move& c);
 public:
  MSS_Move();
  unsigned student;
  int from_index,old_ward,from_period,new_ward,to_period,free_index;
};
class MSS_Swap
{
  friend bool operator==(const MSS_Swap& m1, const MSS_Swap& m2);
  friend bool operator!=(const MSS_Swap& m1, const MSS_Swap& m2);
  friend bool operator<(const MSS_Swap& m1, const MSS_Swap& m2);
  friend ostream& operator<<(ostream& os, const MSS_Swap& c);
  friend istream& operator>>(istream& is, MSS_Swap& c);
 public:
  MSS_Swap();
  unsigned student;
  int from_index,old_ward,from_period,to_index,new_ward,to_period;
};
#endif

