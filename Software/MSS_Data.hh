// File MSS_Data.hh
#ifndef MSS_DATA_HH
#define MSS_DATA_HH

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

class Student
{
  friend ostream& operator<<(ostream&, const Student&);
  friend istream& operator>>(istream&, Student&);
public:
  Student(){};
  
  
  void SetDisciplineW(unsigned num) {discipline_w=num;}
  int GetDisciplineW() const {return discipline_w;}
  void SetHospitalW(unsigned num) {hospital_w=num;}
  int GetHospitalW() const {return hospital_w;}
  void SetChangeW(int num) {change_w=num;}
  int GetChangeW() const {return change_w;}
  void SetWaitW(int num) {wait_w=num;}
  int GetWaitW() const {return wait_w;}
  bool GetDisciplineList(unsigned i,unsigned j) const {return discipline_list[i][j];}
  void SetDisciplineList(unsigned i,unsigned j,bool k){discipline_list[i][j]=k;}
  void InitAbility(unsigned h,unsigned d) {ability.resize(h,vector<bool>(d,false));}
  void InitDisciplineList(unsigned i,unsigned j) {discipline_list.resize(i,vector<bool>(j,false));}
  void InitCompDiscList(unsigned i) {compact_discipline_list.resize(i,vector<unsigned> ());}
  void SetCompDiscList(unsigned i,unsigned j){compact_discipline_list[i].push_back(j);}
  unsigned GetCompDiscList(unsigned i,unsigned j) const {return compact_discipline_list[i][j];}
  unsigned SizeCompDiscList(unsigned i) const {return compact_discipline_list[i].size();}
  unsigned GetNumDisc() const{return num_disc;}
  void SetNumDisc(unsigned k) {num_disc=k;}
  unsigned GetDiscPr() const {return max_disc_pr;}  
  void SetDiscPr(unsigned k){max_disc_pr=k;}
  unsigned GetHospPr() const {return max_hosp_pr;} 
  void SetHospPr(unsigned k){max_hosp_pr=k;}
  unsigned GetPerfectHospScore() const {return max_hosp_score;}
  void SetPerfectHospScore (unsigned k) {max_hosp_score=k;}
  unsigned GetPerfectDiscScore() const {return max_disc_score;}
  void SetPerfectDiscScore (unsigned k) {max_disc_score=k;}
  bool GetAbility(unsigned h,unsigned d) const {return ability[h][d];}
  void SetAbility(unsigned h,unsigned d,unsigned val) {ability[h][d]=val;}
private:
  unsigned num_disc; 
  unsigned max_disc_pr; 
  unsigned max_hosp_pr; 
  unsigned max_disc_score; 
  unsigned max_hosp_score; 

  int discipline_w;  
  int hospital_w;    
  int change_w;      
  int wait_w;        
  
  
  vector<vector<bool>>discipline_list; 
  vector<vector<unsigned>>compact_discipline_list; 
  vector<vector<bool> >ability; 
};

class Hospital
{
  friend ostream& operator<<(ostream&, const Hospital&);
  friend istream& operator>>(istream&, Hospital&);
public:
  Hospital(){};
  unsigned GetMaxDemand(unsigned i,unsigned j) const {return max_demand[i][j];}
  void SetMaxDemand(unsigned i,unsigned j,unsigned k){max_demand[i][j]=k;}
  void InitMaxDemand(unsigned i,unsigned j) {max_demand.resize(i,vector<unsigned>(j,0));}
  unsigned GetMinDemand(unsigned i,unsigned j) const {return min_demand[i][j];}
  void SetMinDemand(unsigned i,unsigned j,unsigned k){min_demand[i][j]=k;}
  void InitMinDemand(unsigned i,unsigned j) {min_demand.resize(i,vector<unsigned>(j,0));}

private:
  
  vector<vector<unsigned> >max_demand;
  
  vector<vector<unsigned> >min_demand;
};
class Discipline
{
  friend ostream& operator<<(ostream&, const Discipline&);
  friend istream& operator>>(istream&, Discipline&);
public:
  Discipline() {}
  unsigned GetGroupId() const {return group_id;}
  void SetGroupId(unsigned i) {group_id=i;}
  bool GetDiscRequired(unsigned i) const {return discipline_required[i];}
  void SetDiscRequired(unsigned i,bool k){discipline_required[i]=k;}
  void InitDiscRequired(unsigned i) {discipline_required.resize(i);}
  void SetDependentOn(unsigned i) {dependent_on=i;}
  unsigned GetDependentOn() const {return dependent_on;}
  void AddDependency(unsigned i){dependency.push_back(i);}
  unsigned SizeDependency() const {return dependency.size();}
  unsigned GetDependency(unsigned i) const {return dependency[i];}
  unsigned GetRequiredby(unsigned i) const {return is_required_by[i];}
  void AddRequiredby(unsigned i){is_required_by.push_back(i);}
  unsigned SizeRequiredby() const {return is_required_by.size();}
  void AddIsAvailableAt(unsigned j){is_available_at.push_back(j);}
  unsigned GetIsAvailableAt (unsigned i) const {return is_available_at[i];}
  unsigned SizeIsAvailableAt() const {return is_available_at.size();}
private:
  
  unsigned group_id; 
  unsigned dependent_on; 
  vector<bool> discipline_required; 
  vector<unsigned> dependency; 
  vector<unsigned> is_required_by; 
  vector<unsigned> is_available_at; 
};

class Ward
  {
  friend ostream& operator<<(ostream&, const Ward&);
public:
  Ward(){}
  unsigned GetId() const {return id;}
  void SetId(unsigned i){id=i;}
  unsigned GetDiscId() const {return discipline_id;}
  void SetDiscId(unsigned i){discipline_id=i;} 
  unsigned GetHospId () const {return hospital_id;} 
  void SetHospId(unsigned i){hospital_id=i;}
private:
  unsigned id;
  unsigned discipline_id;
  unsigned hospital_id;
  };

class MSS_Input 
{
  friend ostream& operator<<(ostream& os, const MSS_Input& bs);
public:
  MSS_Input(string file_name);
  
  unsigned Students() const {return students;} 
  unsigned Hospitals() const {return hospitals;} 
  unsigned Disciplines() const {return disciplines;} 
  unsigned TimePeriods() const {return time_periods;}
  unsigned Wards() const {return wards;}
  unsigned Groups() const {return groups;}
  unsigned Duration() const {return duration;} 
  unsigned GetMaxDisc() const {return max_disc;}
  
  unsigned TotalWards() const {return total_wards;}
  unsigned Assignments() const {return assignments;}
  
  const Student& StudentsVector(int i) const { return students_vect[i]; } 
  const Hospital& HospitalsVector(int i) const { return hospitals_vect[i]; }
  const Discipline& DisciplinesVector(int i) const {return disciplines_vect[i];}
  
  const Ward& WardsVector(int i) const {return wards_vect[i];}
  
  unsigned AssignmentsIndex(unsigned i) const {return assignments_index[i];}
  unsigned GetStfromAI(unsigned i) const;
  unsigned GetToFullfill(unsigned i,unsigned j) const {return to_fullfill[i][j];}
  bool GetAvailability (unsigned i,unsigned j) const {return availability[i][j];}
  unsigned GetDiscPref (unsigned i,unsigned j) const {return discipline_preferences[i][j];}
  unsigned GetHospPref (unsigned i,unsigned j) const {return hospital_preferences[i][j];}
  unsigned GetMangPref(unsigned i) const {return manager_preferences[i];}
  unsigned GetMaxDesMan() const {return max_desire_man;}
  bool GetDiscPresence(unsigned i,unsigned j) const {return discipline_presence[i][j];}

protected:
  unsigned students,hospitals,disciplines,time_periods,wards,groups,duration,max_disc;
  unsigned total_wards; 
  unsigned max_desire_man;
  unsigned assignments;
  
  vector<Student> students_vect;
  vector<Hospital> hospitals_vect;
  vector<Discipline> disciplines_vect;
  vector<Ward> wards_vect;

  vector<unsigned> manager_preferences;
  vector<vector<unsigned> > to_fullfill;
  vector<vector<unsigned> > discipline_preferences; 
  vector<vector<unsigned> > hospital_preferences; 
  vector<vector<bool> > availability;
  vector<vector<bool> > discipline_presence;
  vector<unsigned>assignments_index;
  float mean_to_fullfill; 
  float occupancy,busyness,density,depth,excess;
};

class MSS_Output 
{
  friend ostream& operator<<(ostream& os, const MSS_Output& out);
  friend istream& operator>>(istream& is, MSS_Output& out);
public:
  MSS_Output(const MSS_Input& i);
  MSS_Output& operator=(const MSS_Output& out);
  int operator()(unsigned i, unsigned j) const { return mat[i][j]; }
  int& operator()(unsigned i, unsigned j) { return mat[i][j]; }
  int GetScore() {return score;}
  int GetWorstScore() {return worst_score;}
  void SetScore(int k) {score=k;}
  void SetWorstScore(int k) {worst_score=k;}
  void Reset(); 
  
protected:
  const MSS_Input& in;
  vector<vector<int>> mat; 
  int score,worst_score;
 };
#endif
