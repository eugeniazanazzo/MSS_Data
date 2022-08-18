//file MSS_Helpers.cc
#include "MSS_Helpers.hh"

MSS_StateManager::MSS_StateManager(const MSS_Input & pin) 
  : StateManager<MSS_Input,MSS_State>(pin, "MSSStateManager")
  {} 

void MSS_StateManager::RandomState(MSS_State& st) 
{
  st.ResetState();
  unsigned i,j,k,h,to_fullfill,disciplines_available,wards_available,duration;
  unsigned d_index,d_choice,w_index,w_choice,h_choice;
  int tp_choice;
  for (i = 0; i < in.Students(); i++)
    {
      tp_choice=-1;
      for (j = 0; j < in.Groups(); j++){
        to_fullfill=in.GetToFullfill(i,j);
        for(k=0;k<to_fullfill;k++){
          do 
          { 
            disciplines_available=in.StudentsVector(i).SizeCompDiscList(j)-1; 
            d_index = Random::Uniform<int>(0,disciplines_available);
            d_choice =in.StudentsVector(i).GetCompDiscList(j,d_index);
            duration=in.Duration();
          }
          while(st.Allocated(i,d_choice));
          do
          {
            wards_available=in.DisciplinesVector(d_choice).SizeIsAvailableAt()-1;
            w_index=Random::Uniform<int>(0,wards_available);
            w_choice=in.DisciplinesVector(d_choice).GetIsAvailableAt(w_index);
            h_choice=w_choice/in.Wards(); 
          }
          while(st.IsAble(i,h_choice,d_choice) && st.GetDiscperHosp(i,h_choice)>=in.GetMaxDisc()); 
          do
          {
            tp_choice++;
          } while ( !st.IntoBounds(tp_choice,duration) || st.Occupied(i,tp_choice,duration)
                   ||!st.CanAttend(i,tp_choice,duration));           
          for(h=0;h<duration;h++){
            st(i,tp_choice+h)=w_choice;
          }
          st.Allocate(i,d_choice);
          st.IncDiscperHosp(i,h_choice);
          st.SetFinished(i,d_choice,tp_choice+duration);
          st.UpdateFirstLast(i,tp_choice);
          st.AddAssignment(i,w_choice,tp_choice);
        }
      }
    }
    st.UpdateRedundantStateData();
    st.UpdateScores();
}

bool MSS_StateManager::CheckConsistency(const MSS_State& st) const
{
  int i,j,k;
  int ward,discipline,hospital;
  unsigned group,required,last;
  vector<vector<unsigned>>end;
  vector<unsigned>wait;
  vector<unsigned>changes;
  vector<vector<bool>>attended;
  vector<vector<unsigned>>presences;
  vector<vector<unsigned>>per_group;
  changes.resize(in.Students(),0);
  wait.resize(in.Students(),0);
  end.resize(in.Students(),vector<unsigned>(in.Disciplines(),in.TimePeriods()+1));
  attended.resize(in.Students(),vector<bool>(in.Disciplines(),false));
  presences.resize(in.TotalWards(),vector<unsigned>(in.TimePeriods(),0));
  per_group.resize(in.Students(),vector<unsigned>(in.Groups(),0));
	for(i=0;i<in.Students();i++){
    hospital=-1;
    for(j=0;j<in.TimePeriods();j++){
      ward=st(i,j);
      if(ward!=-1){
      discipline=ward%in.Wards();
      if(ward/in.Wards()!=hospital && hospital!=-1){
        changes[i]++;
      }
      hospital=ward/in.Wards();
      group=in.DisciplinesVector(discipline).GetGroupId();
      last=j;
      if(!attended[i][discipline]){
        presences[ward][j]++;
        attended[i][discipline]=true;
        per_group[i][group]++;
      }
      if(!in.StudentsVector(i).GetDisciplineList(group,discipline))
      {
        cout<<"I'm doing a discipline not on list"<<endl;
        cout<<"Student "<<i<<" discipline "<<discipline<<endl;
        return false;
      }
      if(!in.GetAvailability(i,j))
      {
        cout<<"I'm not available"<<endl;
        return false;
      }
      if(j<in.TimePeriods()-1)
      {
        if(st(i,j)!=st(i,j+1)){
          end[i][discipline]=j+1;
        }
      }
      for(k=0;k<in.DisciplinesVector(discipline).SizeDependency();k++){
          required=in.DisciplinesVector(discipline).GetDependency(k);
            if(end[i][required]>end[i][discipline]){
              cout<<"Student "<<i<<" discipline "<<discipline<<" ends after "<<required<<endl;
              return false;
            }
        }
      }
      else if(j<last) 
        wait[i]++;
      }
    if(changes[i]!=st.Change(i))
      {
        cout<<" wrong changing on student "<<i<<endl;
        return false;
      }
  }
  for(i=0;i<in.Students();i++){
    for(j=0;j<in.Groups();j++){
      if(per_group[i][j]!=in.GetToFullfill(i,j)){
        cout<<" number per group is wrong"<<endl;
        return false;
      }
    }
  }
  return true;
}

int MSS_HospMaxRequirements::ComputeCost(const MSS_State& st) const
{ 
  unsigned i,j;
  int p;
  unsigned cost = 0,hospital,num_ward;
  for (i=0;i<in.TotalWards();i++){
    hospital=i/in.Disciplines();
    num_ward=i%in.Disciplines();
    for (j=0;j<in.TimePeriods();j++)
    {
      p=st.StudentsAvailable(i,j)-in.HospitalsVector(hospital).GetMaxDemand(j,num_ward);
      if (p>0)
        cost+=p;
    }
  }
  return cost;
}
 
void MSS_HospMaxRequirements::PrintViolations(const MSS_State& st, ostream& os) const
{
  unsigned i,j;
  int p;
  unsigned hospital,num_ward;
  for (i=0;i<in.TotalWards();i++){
    hospital=i/in.Wards();
    num_ward=i%in.Wards();
    for (j=0;j<in.TimePeriods();j++)
    {
      p=st.StudentsAvailable(i,j)-in.HospitalsVector(hospital).GetMaxDemand(j,num_ward);
      if (p>0)
        os<<"In ward "<<i<< " on period "<<j<<" there are "<<st.StudentsAvailable(i,j)<< " students when max "<<
        in.HospitalsVector(hospital).GetMaxDemand(j,num_ward)<< " are allowed"<<endl;
    }
  }
}

int MSS_HospMinRequirements::ComputeCost(const MSS_State& st) const
{ 
  unsigned i,j;
  int p;
  unsigned cost = 0,hospital,num_ward;
  for (i=0;i<in.TotalWards();i++){
    hospital=i/in.Wards();
    num_ward=i%in.Wards();
    for (j=0;j<in.TimePeriods();j++)
    {
      if(in.HospitalsVector(hospital).GetMinDemand(j,num_ward)>0){
        p=in.HospitalsVector(hospital).GetMinDemand(j,num_ward)-st.StudentsAvailable(i,j);
      if (p>0)
        cost+=p;
      }
    }
  }
  return cost;
}
 
void MSS_HospMinRequirements::PrintViolations(const MSS_State& st, ostream& os) const
{
  unsigned i,j;
  int p;
  unsigned hospital,num_ward;
  for (i=0;i<in.TotalWards();i++){
    hospital=i/in.Wards();
    num_ward=i%in.Wards();
    for (j=0;j<in.TimePeriods();j++)
    {
      if(in.HospitalsVector(hospital).GetMinDemand(j,num_ward)>0){
      p=in.HospitalsVector(hospital).GetMinDemand(j,num_ward)-st.StudentsAvailable(i,j);
      if (p>0)
        os<<"In ward "<<i<< " on period "<<j<<" there are "<<st.StudentsAvailable(i,j)<< " students when min requested is"<<
        in.HospitalsVector(hospital).GetMinDemand(j,num_ward)<<endl;
      }
    }
  }
}

int MSS_PrecRequirements::ComputeCost(const MSS_State& st) const
{
  unsigned i,j,k,required;
  unsigned cost = 0;
  for (i=0;i<in.Students();i++){
    for (j=0;j<in.Disciplines();j++)
    {
      if(st.Allocated(i,j)){
        for(k=0;k<in.DisciplinesVector(j).SizeDependency();k++){
          required=in.DisciplinesVector(j).GetDependency(k);
            if(st.GetFinished(i,required)>st.GetFinished(i,j)){
              cost++;
            }
        }
      }
    }
  }
  return cost;
}

void MSS_PrecRequirements::PrintViolations(const MSS_State& st, ostream& os) const 
{                                                                                  
  unsigned i,j,k,required; 
  for (i=0;i<in.Students();i++){
    for (j=0;j<in.Disciplines();j++)
    {
      if(st.Allocated(i,j)){
        for(k=0;k<in.DisciplinesVector(j).SizeDependency();k++){
          required=in.DisciplinesVector(j).GetDependency(k);
          if(st.GetFinished(i,required)>st.GetFinished(i,j)){ 
            os<<" Student "<<i<< " finishes disc "<<j<<" at time "<<st.GetFinished(i,j)<<" before disc "<<required<<
            " at time "<<st.GetFinished(i,required)<<endl;
          }
        }
      }
    }
  }
} 

int MSS_DiscPreference::ComputeCost(const MSS_State& st) const
{
  unsigned i,max_attain,cost=0;
  for (i=0;i<in.Students();i++){
    max_attain=in.StudentsVector(i).GetPerfectDiscScore();
    cost+=max_attain-st.DiscScore(i);
  }
  return cost;
}
       
void MSS_DiscPreference::PrintViolations(const MSS_State& st, ostream& os) const 
{                                                                                  
  unsigned i,tot_score=0;
  for (i=0;i<in.Students();i++){  
    tot_score+=st.DiscScore(i);
    os<<"Discipline score for student "<<i<< " is "<<st.DiscScore(i)<<endl;
  }
  os<<" Total Score for Disc Pref="<<tot_score<<endl;
}

int MSS_HospPreference::ComputeCost(const MSS_State& st) const
{
  unsigned i,max_attain,cost=0;
  for (i=0;i<in.Students();i++){
    max_attain=in.StudentsVector(i).GetPerfectHospScore();
    cost+=max_attain-st.HospScore(i);
  }
  return cost;
}

void MSS_HospPreference::PrintViolations(const MSS_State& st, ostream& os) const 
{                                                                                  
  unsigned i,tot_score=0;
  for (i=0;i<in.Students();i++){  
    tot_score+=st.HospScore(i);
    os<<"Hospital score for student "<<i<< " is "<<st.HospScore(i)<<endl;
  }
  os<<" Total Score for Hosp Pref="<<tot_score<<endl;
}

int MSS_ManPreference::ComputeCost(const MSS_State& st) const
{
  unsigned i,cost=0;
  for (i=0;i<in.Students();i++){
    cost+=in.GetMaxDesMan()-st.MngrScore(i);
  }
  return cost;
}

void MSS_ManPreference::PrintViolations(const MSS_State& st, ostream& os) const 
{                                                                                  
  unsigned i;
  for (i=0;i<in.Students();i++){  
    os<<"Manager score for student "<<i<< " is "<<st.MngrScore(i)<<endl;
  }
}

int MSS_WaitingCosts::ComputeCost(const MSS_State& st) const
{
  unsigned cost=0;
  for (int i=0;i<in.Students();i++){
    cost+=-st.GetWaitScore(i);
  }
  return cost;
}
     
void MSS_WaitingCosts::PrintViolations(const MSS_State& st, ostream& os) const 
{                                                                                  
  for (int i=0;i<in.Students();i++)
  {
    if(st.Waiting(i)>0)
      os<< "Student "<<i<< " waits "<<st.Waiting(i)<<" periods of time"<<endl;
  }
} 

int MSS_ChangeCosts::ComputeCost(const MSS_State& st) const
{
  unsigned cost=0;
  for (int i=0;i<in.Students();i++){
      cost+=-st.GetChangeScore(i);
  }
  return cost;
}
         
void MSS_ChangeCosts::PrintViolations(const MSS_State& st, ostream& os) const 
{                                                                                  
  for (int i=0;i<in.Students();i++)
  {
    if(st.Change(i)>0)
      os<< "Student "<<i<< " changes hospital "<<st.Change(i)<<" times "<<endl;
  }
} 

int MSS_WorstScore::ComputeCost(const MSS_State& st) const
{
  unsigned highest_w=3;
  unsigned times=5;
  unsigned cost=(in.Disciplines()*(pow(highest_w,times)))-st.GetWorstScore();
  return cost;
}

void MSS_WorstScore::PrintViolations(const MSS_State& st, ostream& os) const 
{
  unsigned worst_index=st.GetWorstStudent();
  os<<" Student "<<worst_index<<" has the worst score of "<<st.GetWorstScore()
  <<" the disc part= "<<st.DiscScore(worst_index)<<" the hosp part="<<st.HospScore(worst_index)
  <<" the manager part="<<st.MngrScore(worst_index)
  <<" penalty for change="<<st.GetChangeScore(worst_index)
  <<" penalty for wait="<<st.GetWaitScore(worst_index)<<endl;
}


/*****************************************************************************
 * Output Manager Methods
 *****************************************************************************/

void MSS_OutputManager::InputState(MSS_State& st, const MSS_Output& out) const 
{
 unsigned i, j;
  st.ResetState();
  for (i = 0; i < in.Students(); i++){
    for (j = 0; j < in.TimePeriods(); j++)    
      st(i,j) = out(i,j);
  }
  st.UpdateAdditionalRedundantStateData();
  st.UpdateRedundantStateData();
  st.UpdateScores();
}

void MSS_OutputManager::PrettyPrintOutput(const MSS_State& st, const string &file_name) const
{
  unsigned ward,period,discipline,hospital,group;
  char sep='|';
  cout<<"PP "<<sep<<" II "<<sep<<" GG "<<sep<<" DD "<<sep<<" TT "<<sep<<" Du "<<sep<<" HH "<<sep<< 
  " PrD "<<sep<<" PrH "<<sep<<" PrP "<<sep<<" K_G "<<sep<<" C_C "<<sep<<" FH (Schedule)"<<endl;
  unsigned i, j; 
  for (i = 0; i < in.Students(); i++)
  {
    for(j = 0;j <in.StudentsVector(i).GetNumDisc(); j++){
      ward=st.AssignmentWard(i,j);
      period=st.AssignmentTime(i,j);
      discipline=ward%in.Wards();
      hospital=ward/in.Wards();
      group=in.DisciplinesVector(discipline).GetGroupId();
      if(i<10){
        cout<<"00 "<<sep<<" 0"<<i<<" "<<sep<<" 0"<<group<<" "<<sep;
      }
      else
      {
        cout<<"00 "<<sep<<" "<<i<<" "<<sep<<" 0"<<group<<" "<<sep;
      }
      if(discipline<10)
      {
        cout<<" 0"<<discipline<<" "<<sep;
      }
      else
      {
        cout<<" "<<discipline<<" "<<sep;
      }
      if(period<10){
        cout<<" 0"<<period<<" "<<sep<<" 0"<<in.Duration()<<" "<<sep<<" 0"
        <<hospital<<" "<<sep;
      }
      else{
        cout<<" "<<period<<" "<<sep<<" 0"<<in.Duration()<<" "<<sep<<" 0"
        <<hospital<<" "<<sep;
      }
      cout<<" 00"<<in.GetDiscPref(i,discipline)<<" "<<sep
      <<" 00"<<in.GetHospPref(i,hospital)<<" "<<sep<<" 001 "<<sep;

      if(in.GetToFullfill(i,group)<10)
      {
        cout<<" 00"<<in.GetToFullfill(i,group)<<" "<<sep<<" 001 "<<sep<<endl;
      }
      else
      {
        cout<<" 0"<<in.GetToFullfill(i,group)<<" "<<sep<<" 001 "<<sep<<endl;
      }

    }
  }
  unsigned d_score,h_score,d_pref,h_pref;
  cout<<"PP II GG DD HH  (Fulfilled already and exist in the list)"<<endl;
  cout<<"III PrD PrH PrW PrC PrCn PrP W_d W_h W_w W_c W_cn W_p DesI DNi DPi"<<endl;
  for(i=0;i<in.Students();i++){
    if(i<10)
      cout<<"00"<<i;
    else
      cout<<"0"<<i;
    d_score=st.DiscScore(i);
    h_score=st.HospScore(i);
    d_pref=d_score/in.StudentsVector(i).GetDisciplineW();
    h_pref=h_score/in.StudentsVector(i).GetHospitalW();
    if(d_pref<10){
      cout<<" 00"<<d_pref;
    }
    else if(d_pref<100)
    {
      cout<<" 0"<<d_pref;
    }
    else{
      cout<<" "<<d_pref;
    }
    if(h_pref<10){
      cout<<" 00"<<h_pref;
    }
    else if(h_pref<100)
    {
      cout<<" 0"<<h_pref;
    }
    else{
      cout<<" "<<h_pref;
    }
    if(st.Waiting(i)<10)
    {
      cout<<" 00"<<st.Waiting(i);
    }
    else
      cout<<" 0"<<st.Waiting(i);
    if(st.Change(i)<10)
      cout<<" 00"<<st.Change(i)<< " 0000 ";
    else
      cout<<" 0"<<st.Change(i)<< " 0000 ";
    if(in.StudentsVector(i).GetNumDisc()<10)
      cout<<"00"<<in.StudentsVector(i).GetNumDisc()<<" 00"<<in.StudentsVector(i).GetDisciplineW()
      <<" 00"<<in.StudentsVector(i).GetHospitalW();
    else
      cout<<"0"<<in.StudentsVector(i).GetNumDisc()<<" 00"<<in.StudentsVector(i).GetDisciplineW()
      <<" 00"<<in.StudentsVector(i).GetHospitalW();
    cout<<" -0"<<-in.StudentsVector(i).GetWaitW()<<" -0"<<-in.StudentsVector(i).GetChangeW()
    <<" -000 001 "<<st.GetTotalScore(i)<<",000 "<<"00,00 "<<"00,00 "<<endl;
  }

  cout<<"III DesI DesP MaxP "<<endl;
  for(i=0;i<in.Students();i++){
    if(i<10)
      cout<<"00"<<i;
    else
      cout<<"0"<<i;
    if(st.GetTotalScore(i)<10){
      cout<<" 00"<<st.GetTotalScore(i);
    }
    else if(st.GetTotalScore(i)<100)
    {
      cout<<" 0"<<st.GetTotalScore(i);
    }
    else{
      cout<<" "<<st.GetTotalScore(i);
    }
    if(st.GetWorstScore()<10)
      cout<<" 00"<<st.GetWorstScore();
    else
      cout<<" 0"<<st.GetWorstScore();
    unsigned perfect_score=0;
    perfect_score=in.StudentsVector(i).GetPerfectDiscScore();
    perfect_score+=in.StudentsVector(i).GetPerfectHospScore();
    perfect_score+=in.StudentsVector(i).GetNumDisc();
    if(perfect_score<10){
      cout<<" 00"<<perfect_score<<endl;
    }
    else if(perfect_score<100)
    {
      cout<<" 0"<<perfect_score<<endl;
    }
    else{
      cout<<" "<<perfect_score<<endl;
    }
  }
}

void MSS_OutputManager::OutputState(const MSS_State& st, MSS_Output& out) const 
{
  unsigned i,j;
  for (i=0;i<in.Students();i++){
    for (j=0;j<in.TimePeriods();j++)
      out(i,j)=st(i,j);
  }
  out.SetScore(st.GetRealScore());
  out.SetWorstScore(st.GetWorstScore());
}

/*****************************************************************************
 * MSS_Move Neighborhood Explorer Methods
 *****************************************************************************/

void MSS_MoveNeighborhoodExplorer::RandomMove(const MSS_State& st, MSS_Move& mv) const
{
  do{
    unsigned chosen,num_disc,discipline,duration,group,new_group,choices;
    chosen = Random::Uniform<unsigned>(0,in.Assignments()-1);
    mv.student=in.GetStfromAI(chosen);
    num_disc=in.StudentsVector(mv.student).GetNumDisc();
    mv.from_index=num_disc-(in.AssignmentsIndex(mv.student)-chosen)-1;
    mv.old_ward=st.AssignmentWard(mv.student,mv.from_index);
    mv.from_period=st.AssignmentTime(mv.student,mv.from_index);
    discipline =mv.old_ward%in.Wards();
    duration=in.Duration();
    do
    {
      mv.to_period=Random::Uniform<unsigned>(0,in.TimePeriods()-1);
    } while (!st.IntoBounds(mv.to_period,duration) ||
          (st.OtOccupied(mv.student,mv.to_period,duration,mv.old_ward) && mv.to_period!=mv.from_period)||
          !st.CanAttend(mv.student,mv.to_period,duration));
    group=in.DisciplinesVector(discipline).GetGroupId();
    choices=in.StudentsVector(mv.student).SizeCompDiscList(group);
    if(choices==in.GetToFullfill(mv.student,group))
    {
      unsigned hosp_c;
      if(mv.to_period==mv.from_period)
        hosp_c=Random::Uniform<unsigned>(1,in.Hospitals()-1); 
      else
        hosp_c=Random::Uniform<unsigned>(0,in.Hospitals()-1);
      mv.new_ward=(mv.old_ward+(in.Wards()*hosp_c))%in.TotalWards();
    }
  else
    {
      do
      {
      unsigned index_c;
      unsigned size_free=st.FreeWardSize(mv.student);
      if(mv.to_period==mv.from_period){
        index_c=Random::Uniform<unsigned>(1,in.Hospitals()+size_free-1);
      }
      else
        index_c=Random::Uniform<unsigned>(0,in.Hospitals()+size_free-1);
      if(index_c<in.Hospitals())
        mv.new_ward=(mv.old_ward+(in.Wards()*index_c))%in.TotalWards();
      else
      {
        mv.new_ward=st.FreeWard(mv.student,index_c-in.Hospitals());
        mv.free_index=(index_c-in.Hospitals())/in.Hospitals();
      }
      new_group=in.DisciplinesVector(mv.new_ward%in.Wards()).GetGroupId();
      } while(group!=new_group);
    }
  } while (!FeasibleMove(st,mv));
} 

bool MSS_MoveNeighborhoodExplorer::FeasibleMove(const MSS_State& st, const MSS_Move& mv) const
{
  unsigned discipline =mv.new_ward%in.Wards(); unsigned hosp=mv.new_ward/in.Wards();
  return (in.StudentsVector(mv.student).GetAbility(hosp,discipline) && st.GetDiscperHosp(mv.student,hosp)<in.GetMaxDisc());
} 

void MSS_MoveNeighborhoodExplorer::MakeMove(MSS_State& st, const MSS_Move& mv) const
{
  unsigned o_discipline,n_discipline,duration,o_hosp,n_hosp,i,offset=in.Duration()-1; 
  int delta;
  n_discipline =mv.new_ward%in.Wards();
  n_hosp=mv.new_ward/in.Wards();
  o_discipline =mv.old_ward%in.Wards(); 
  o_hosp=mv.old_ward/in.Wards();
  duration=in.Duration();
  if(mv.from_period!=mv.to_period)
  {
    for(i=0;i<in.Duration();i++)
      st(mv.student,mv.from_period+i)=-1;
  }
  for(i=0;i<in.Duration();i++)
    st(mv.student,mv.to_period+i)=mv.new_ward;
  st.ChangeAssignment(mv.student,mv.from_index,mv.new_ward,mv.to_period);
  if(n_discipline!=o_discipline)
  {
    st.Unallocate(mv.student,o_discipline);
    st.Allocate(mv.student,n_discipline);
    st.SetFinished(mv.student,o_discipline,in.TimePeriods()+1); 
    st.SetAtHospital(mv.student,o_discipline,-1);
    st.ChangeFreeDisc(mv.student,mv.free_index,o_discipline);
    for(i=0;i<in.Hospitals();i++)
    {
      st.ChangeFreeWards(mv.student,(mv.free_index*in.Hospitals())+i,o_discipline+(i*in.Wards()));
    }
  }
  st.SetFinished(mv.student,n_discipline,mv.to_period+duration);
  st.SetAtHospital(mv.student,n_discipline,mv.new_ward/in.Wards());
  st.IncDiscperHosp(mv.student,n_hosp);
  st.DecDiscperHosp(mv.student,o_hosp);
  for(i=0;i<duration;i++){
    st.DecStudentsAvailable(mv.old_ward,mv.from_period+i);
    st.IncStudentsAvailable(mv.new_ward,mv.to_period+i);
  }
  unsigned old_first=st.FirstPeriod(mv.student),old_last=st.LastPeriod(mv.student);
  unsigned new_first=old_first,new_last=old_last; 
  if(mv.from_period!=mv.to_period && st.AssignmentsSize(mv.student)>1)
    {
     if(mv.from_period==st.FirstPeriod(mv.student))
     {
      if(mv.from_period>mv.to_period)
        {
          st.SetFirstPeriod(mv.student,mv.to_period);
          new_first=mv.to_period;
        }
      else
        {
          new_first=st.SearchNewFirst(mv.student,mv.from_period);
          st.SetFirstPeriod(mv.student,new_first); 
        }

     }
     else if(mv.to_period<old_first){
      st.SetFirstPeriod(mv.student,mv.to_period);
      new_first=mv.to_period;
     }
     if(mv.from_period+offset==st.LastPeriod(mv.student)){
      if(mv.from_period<mv.to_period)
        {
          st.SetLastPeriod(mv.student,mv.to_period+offset);
          new_last=mv.to_period+duration-1;
        }
      else
       {
          new_last=st.SearchNewLast(mv.student,mv.from_period+offset);
          st.SetLastPeriod(mv.student,new_last); 
       }
     }
     else if(mv.to_period>old_last){
      st.SetLastPeriod(mv.student,mv.to_period+duration-1);
      new_last=mv.to_period+duration-1;
     }
    }
  else if(st.AssignmentsSize(mv.student)<=1)
  {
    new_first=mv.to_period;
    new_last=mv.to_period+duration-1;
  }
  int delta_wait=0,delta_change=0;
  if(mv.from_period!=mv.to_period && old_first+offset!=old_last){
    if(mv.from_period==old_first)
    {
      if(mv.to_period==new_first && mv.to_period<mv.from_period) 
      {}
      else if(mv.to_period==new_first && mv.to_period>mv.from_period) 
      {}
      else if(mv.to_period+offset==new_last)
      {
        delta_wait+=mv.to_period-old_last-1;
        delta_wait+=duration;
      }
    }
    else if(mv.from_period+offset==old_last){
      if(mv.to_period+offset==new_last && mv.to_period>mv.from_period)
      {
        delta_wait+=mv.to_period-mv.from_period; 
      }
      else if(mv.to_period+offset==new_last && mv.to_period<mv.from_period) 
      {
        delta_wait-=mv.from_period-mv.to_period;
      }
      else if(mv.to_period==new_first)
      {
        delta_wait-=mv.from_period-new_last-1;
        delta_wait-=duration;
      }  
      else
        {
        delta_wait-=mv.from_period-new_last-1; 
        delta_wait-=duration;
        }
    }
    else if(mv.to_period==new_first) 
      {}
    else if(mv.to_period+offset==new_last)
      {
        delta_wait+=mv.to_period+offset-old_last;
      }
  }
  if(mv.from_period!=mv.to_period && old_first+offset==old_last)
  {
    if(mv.to_period>mv.from_period)
      delta_wait+=mv.to_period-mv.from_period;
    else
      delta_wait-=mv.from_period-mv.to_period;
  }
  st.AddWaitingTimes(mv.student,delta_wait);
  int w_score=delta_wait*in.StudentsVector(mv.student).GetWaitW();
  st.AddWaitScore(mv.student,w_score);
  delta=w_score;
  unsigned disc_w,disc_osc,disc_nsc;
  int d_score,m_score;
  if(n_discipline!=o_discipline){
    disc_w=in.StudentsVector(mv.student).GetDisciplineW();
    disc_osc=in.GetDiscPref(mv.student,o_discipline);
    d_score=disc_w*disc_osc;
    delta-=d_score;
    st.DecDiscScore(mv.student,d_score);
    disc_nsc=in.GetDiscPref(mv.student,n_discipline);
    d_score=disc_w*disc_nsc;
    delta+=d_score;
    st.AddDiscScore(mv.student,d_score);
    m_score=-(in.GetMangPref(o_discipline)-in.GetMangPref(n_discipline));
    delta+=m_score;
    st.AddMngrScore(mv.student,m_score);
  }
  unsigned hosp_w,hosp_osc,hosp_nsc,h_score;
  if(o_hosp!=n_hosp){
    hosp_w=in.StudentsVector(mv.student).GetHospitalW();
    hosp_osc=in.GetHospPref(mv.student,o_hosp);
    h_score=hosp_w*hosp_osc;
    delta-=h_score;
    st.DecHospScore(mv.student,h_score);
    hosp_nsc=in.GetHospPref(mv.student,n_hosp);
    h_score=hosp_w*hosp_nsc;
    delta+=h_score;
    st.AddHospScore(mv.student,h_score);
  }

  int left_from=-1,right_from=-1,left_to=-1,right_to=-1;
  int left_from_h=-1,right_from_h=-1,left_to_h=-1,right_to_h=-1;
  if((mv.to_period!=mv.from_period || n_hosp!=o_hosp) && old_first+offset!=old_last) {
    if(mv.from_period==old_first) 
      {
        right_from=st.SearchRightof(mv.student,mv.from_period,mv.to_period);
        right_from_h=right_from/in.Wards();
        if(o_hosp!=right_from_h)
          delta_change--;
      }
    else if (mv.from_period+offset==old_last)
    {
      left_from=st.SearchLeftof(mv.student,mv.from_period,mv.to_period);
      left_from_h=left_from/in.Wards();
      if(o_hosp!=left_from_h)
          delta_change--;
    }
    else
    {
      right_from=st.SearchRightof(mv.student,mv.from_period,mv.to_period);
      right_from_h=right_from/in.Wards();
      left_from=st.SearchLeftof(mv.student,mv.from_period,mv.to_period);
      left_from_h=left_from/in.Wards();
      if(o_hosp!=left_from_h && o_hosp !=right_from_h){
        if(left_from_h==right_from_h)
          delta_change-=2;
        else
          delta_change--;
      }
    }  
    if(mv.to_period==new_first)
      {
        right_to=st.SearchRightof(mv.student,mv.to_period,mv.from_period);
        right_to_h=right_to/in.Wards();
        if(n_hosp!=right_to_h)
          delta_change++;
      }
    else if(mv.to_period+offset==new_last)
      {
      left_to=st.SearchLeftof(mv.student,mv.to_period,mv.from_period);
      left_to_h=left_to/in.Wards();
      if(n_hosp!=left_to_h)
          delta_change++;
      }
    else
      {
        right_to=st.SearchRightof(mv.student,mv.to_period,mv.from_period);
        right_to_h=right_to/in.Wards();
        left_to=st.SearchLeftof(mv.student,mv.to_period,mv.from_period);
        left_to_h=left_to/in.Wards();
        if(left_to_h==right_to_h && n_hosp!=left_to_h)
          delta_change+=2;
        else if(left_to_h!=right_to_h && n_hosp!=left_to_h && n_hosp!=right_to_h)
          delta_change++;
      }
  }
  st.AddChanges(mv.student,delta_change);
  int c_score=delta_change*in.StudentsVector(mv.student).GetChangeW();
  st.AddChangeScore(mv.student,c_score);
  delta+=c_score;
  st.AddTotalScore(mv.student,delta);
  st.AddRealScore(delta);
  
  if(mv.student==st.GetWorstStudent()) 
   {
    if(delta>0)
      {
      st.CheckWorst();
      } 
    else if (delta<0)
      st.UpdateAllGaps(delta); 
   }
   else{
    if(st.GetGapToWorst(mv.student)+delta<0)
    {
      st.UpdateWorst(mv.student);
    }
    else
      st.UpdateGapToWorst(mv.student);
   }
}  

void MSS_MoveNeighborhoodExplorer::FirstMove(const MSS_State& st, MSS_Move& mv) const
{
 AnyFirstMove(st,mv);
 while (!FeasibleMove(st,mv))
   AnyNextMove(st,mv);
}

bool MSS_MoveNeighborhoodExplorer::NextMove(const MSS_State& st, MSS_Move& mv) const
{
 do
   if (!AnyNextMove(st,mv))
     return false;
 while (!FeasibleMove(st,mv));
 return true;
}

void MSS_MoveNeighborhoodExplorer::AnyFirstMove(const MSS_State& st, MSS_Move& mv) const
{
  mv.student=0;
  mv.from_index=0;
  mv.old_ward=st.AssignmentWard(mv.student,mv.from_index);
  mv.from_period=st.AssignmentTime(mv.student,mv.from_index);
  mv.new_ward=mv.old_ward;
  FirstPeriodAvail(st,mv);
}

void MSS_MoveNeighborhoodExplorer::FirstPeriodAvail (const MSS_State& st, MSS_Move& mv) const
{
  unsigned duration,i,j;
  duration=in.Duration();
  if(mv.old_ward!=mv.new_ward) 
    {
      mv.to_period=mv.from_period;
      return;
    }
  else{
  for(i=0;i<in.TimePeriods();i++)
    {
      for(j=0;j<duration;j++)
      {
        if((st(mv.student,i+j)!=-1 && st(mv.student,i+j)!=mv.old_ward) || i+j>in.TimePeriods()||!in.GetAvailability(mv.student,i+j))
            break;
        else if(st(mv.student,i+j)==mv.old_ward && i==mv.from_period)
            break;
        if (j==duration-1)
        {
          mv.to_period=i;
          return;
        }
      }
    }
  }
  if(NextToWard(st,mv))
    {
      FirstPeriodAvail(st,mv);
      return;
    }
  throw "I should not be here ever";
}

void MSS_MoveNeighborhoodExplorer::FirstNewWard(const MSS_State& st, MSS_Move& mv) const
{
  mv.new_ward=mv.old_ward;
}

void MSS_MoveNeighborhoodExplorer::FirstFromIndex(const MSS_State& st, MSS_Move& mv) const
{
  mv.from_index=0;
  mv.old_ward=st.AssignmentWard(mv.student,mv.from_index);
  mv.from_period=st.AssignmentTime(mv.student,mv.from_index);
}

void MSS_MoveNeighborhoodExplorer::FirstNewDiscipline(const MSS_State& st, MSS_Move& mv) const
{
  unsigned discipline,new_discipline,group,new_group;
  int index=-1;
  discipline=in.WardsVector(mv.old_ward).GetDiscId();
  group=in.DisciplinesVector(discipline).GetGroupId();
  do{
  index++;
  mv.new_ward=st.FreeDiscDisc(mv.student,index);
  new_discipline=in.WardsVector(mv.new_ward).GetDiscId();
  new_group=in.DisciplinesVector(new_discipline).GetGroupId();
  mv.free_index=index;
  } while(group!=new_group);
}

bool MSS_MoveNeighborhoodExplorer::AnyNextMove(const MSS_State& st, MSS_Move& mv) const
{ 
	if (NextPeriodAvail(st,mv)) 
    {
      return true;
    }
  else if (NextToWard(st,mv)) 
    {
      FirstPeriodAvail(st,mv); 
      return true;
    }

  else if (NextFromIndex(st,mv))
   
    {
      FirstNewWard(st,mv);    
      FirstPeriodAvail(st,mv);
      return true;
    }
  
  else if (mv.student < in.Students() - 1) 
    {
      mv.student++;
      FirstFromIndex(st,mv);
      FirstNewWard(st,mv);
      FirstPeriodAvail(st,mv);
      return true;
    }
  else return false;
}

bool MSS_MoveNeighborhoodExplorer::NextPeriodAvail (const MSS_State& st, MSS_Move& mv) const
{
  unsigned duration,current_period,i,j;
  duration=in.Duration();
  if(mv.from_period==mv.to_period)
    current_period=0;
  else
    current_period=mv.to_period+1;
  for(i=current_period;i<in.TimePeriods();i++)
    {
      for(j=0;j<duration;j++)
      {
        if((st(mv.student,i+j)!=-1 && st(mv.student,i+j)!=mv.old_ward) || i+j>in.TimePeriods()||!in.GetAvailability(mv.student,i+j))
            break;
        else if(st(mv.student,i+j)==mv.old_ward && i==mv.from_period)
            break;
        if (j==duration-1)
          {
            mv.to_period=i;
            return true;
          }
      }
    }
  return false;
}

bool MSS_MoveNeighborhoodExplorer::NextToWard(const MSS_State& st, MSS_Move& mv) const
{
  unsigned discipline,group,choices;
  bool out_of_bounds;
  discipline=in.WardsVector(mv.old_ward).GetDiscId();
  group=in.DisciplinesVector(discipline).GetGroupId();
  choices=in.StudentsVector(mv.student).SizeCompDiscList(group);
  if(choices==in.GetToFullfill(mv.student,group)){
    mv.new_ward=(mv.new_ward+in.Wards())%in.TotalWards();
    if(mv.new_ward==mv.old_ward)
      {
        return false;
      }
    else return true;
  }
  else
    {
    out_of_bounds=(mv.new_ward+in.Wards()>=in.TotalWards())? true:false;
    mv.new_ward=(mv.new_ward+in.Wards())%in.TotalWards();
    if(mv.new_ward==mv.old_ward)
      {
        FirstNewDiscipline(st,mv);
        return true;
      }
    if(DiffDisc(st,mv))  
    {
      
      if(!out_of_bounds)
        return true;
      else if(out_of_bounds && NextDiscipline(st,mv))
        return true;
      else if(out_of_bounds && !NextDiscipline(st,mv))
        return false;
    }    
    
    }
  return true;
}

bool MSS_MoveNeighborhoodExplorer::NextFromIndex(const MSS_State& st, MSS_Move& mv) const
{
  mv.from_index++;
  if(mv.from_index<in.StudentsVector(mv.student).GetNumDisc())
  {
    mv.old_ward=st.AssignmentWard(mv.student,mv.from_index);
    mv.from_period=st.AssignmentTime(mv.student,mv.from_index);
    return true;
  }
  else return false;
}

bool MSS_MoveNeighborhoodExplorer::NextDiscipline(const MSS_State& st, MSS_Move& mv) const
{
  unsigned i,j,discipline,group,size;
  discipline=in.WardsVector(mv.new_ward).GetDiscId();
  group=in.DisciplinesVector(discipline).GetGroupId();
  size=st.FreeDiscSize(mv.student)-1;
  for(i=0;i<size;i++)
    {
      if(discipline==st.FreeDiscDisc(mv.student,i))
      {
        for(j=i+1;j<=size;j++){ 
          if(st.FreeDiscGroup(mv.student,j)==group)
          {
            mv.new_ward=st.FreeDiscDisc(mv.student,j);
            mv.free_index=j;
            return true;
          }
        }
      }
    }
  return false;
}

bool MSS_MoveNeighborhoodExplorer::DiffDisc(const MSS_State& st, MSS_Move& mv) const
{
  unsigned old_discipline,new_discipline;
  old_discipline=in.WardsVector(mv.old_ward).GetDiscId();
  new_discipline=in.WardsVector(mv.new_ward).GetDiscId();
  return old_discipline!=new_discipline;
}



void MSS_SwapNeighborhoodExplorer::RandomMove(const MSS_State& st, MSS_Swap& sw) const
{
  unsigned chosen,chosen_index,num_disc,next_index;
  do{
  chosen = Random::Uniform<unsigned>(0,in.Assignments()-1);
  
  sw.student=in.GetStfromAI(chosen);
  } while(st.AssignmentsSize(sw.student)<=1); 
  num_disc=in.StudentsVector(sw.student).GetNumDisc();
  
  chosen_index=num_disc-(in.AssignmentsIndex(sw.student)-chosen)-1;
  do{
    next_index = Random::Uniform<unsigned>(0,st.AssignmentsSize(sw.student)-1);
  } while(chosen_index==next_index); 
  if(chosen_index<next_index)
  {
    sw.from_index=chosen_index;
    sw.to_index=next_index;
  }
  else
  {
    sw.from_index=next_index;
    sw.to_index=chosen_index;
  }
  sw.old_ward=st.AssignmentWard(sw.student,sw.from_index);
  sw.from_period=st.AssignmentTime(sw.student,sw.from_index);
  sw.to_period=st.AssignmentTime(sw.student,sw.to_index);
  sw.new_ward=st.AssignmentWard(sw.student,sw.to_index);
}

void MSS_SwapNeighborhoodExplorer::MakeMove(MSS_State& st, const MSS_Swap& sw) const
{
  unsigned o_discipline,n_discipline,duration,o_hosp,n_hosp,i;
  unsigned first=st.FirstPeriod(sw.student),last=st.LastPeriod(sw.student),offset=in.Duration()-1;
  n_discipline =sw.new_ward%in.Wards(); 
  n_hosp=sw.new_ward/in.Wards();
  duration=in.Duration();
  o_discipline =sw.old_ward%in.Wards();
  o_hosp=sw.old_ward/in.Wards();
  for(i=0;i<duration;i++){
      st(sw.student,sw.from_period+i)=sw.new_ward;
      st(sw.student,sw.to_period+i)=sw.old_ward;
  }
  st.SwapAssignment(sw.student,sw.from_index,sw.to_index);
  for(i=0;i<duration;i++){
  st.DecStudentsAvailable(sw.old_ward,sw.from_period+i);
  st.IncStudentsAvailable(sw.old_ward,sw.to_period+i);
  st.DecStudentsAvailable(sw.new_ward,sw.to_period+i);
  st.IncStudentsAvailable(sw.new_ward,sw.from_period+i);
  }
  st.SetFinished(sw.student,n_discipline,sw.from_period+duration);
  st.SetFinished(sw.student,o_discipline,sw.to_period+duration);
  if(n_hosp!=o_hosp && st.AssignmentsSize(sw.student)>2){ 
  int delta_change=0;
  int left_from=-1,right_from=-1,left_to=-1,right_to=-1;
  int left_from_h=-1,right_from_h=-1,left_to_h=-1,right_to_h=-1;
    if(sw.from_period==first)  
      {
        right_from=st.SearchRightof(sw.student,sw.from_period);
        if(right_from!=sw.old_ward) 
        {
          right_from_h=right_from/in.Wards();
          if(o_hosp==right_from_h)
            delta_change++;
          else if(n_hosp==right_from_h)
           delta_change--; 
        }
      }
    else if (sw.from_period+offset==last) 
    {
        left_from=st.SearchLeftof(sw.student,sw.from_period);
        if(left_from!=sw.old_ward) 
        {
          left_from_h=left_from/in.Wards();
          if(o_hosp==left_from_h)
            delta_change++;
          else if(n_hosp==left_from_h)
           delta_change--; 
        }
    }
    else 
    {
      right_from=st.SearchRightof(sw.student,sw.from_period);
      right_from_h=right_from/in.Wards();
      left_from=st.SearchLeftof(sw.student,sw.from_period);
      left_from_h=left_from/in.Wards();
        if(left_from_h==right_from_h){
          if(right_from==sw.old_ward || left_from==sw.old_ward)
            delta_change++;
          else if(o_hosp==right_from_h)
            delta_change+=2;
          else if(n_hosp==right_from_h)
            delta_change-=2;
          
        }
        else
        {
          if((left_from==sw.old_ward && n_hosp==right_from_h) ||
             (right_from==sw.old_ward && n_hosp==left_from_h))
          {
            delta_change--;
          }
          else if((left_from_h==o_hosp && n_hosp!=right_from_h && left_from!=sw.old_ward) ||
            (right_from_h==o_hosp && n_hosp!=left_from_h && right_from!=sw.old_ward))
          {
            delta_change++;
          }
          else if((left_from_h==n_hosp && o_hosp!=right_from_h) ||
                  (right_from_h==n_hosp && o_hosp!=left_from_h)) 
          {
            delta_change--;
          }
          
        }
      }
      
      if(sw.to_period==first)  
      {
        right_to=st.SearchRightof(sw.student,sw.to_period);
        if(right_to!=sw.new_ward) 
        {
          right_to_h=right_to/in.Wards();
          if(n_hosp==right_to_h)
            delta_change++;
          else if(o_hosp==right_to_h)
           delta_change--; 
        }
      }
      else if (sw.to_period+offset==last) 
      {
        left_to=st.SearchLeftof(sw.student,sw.to_period);
        if(left_to!=sw.new_ward) 
        {
          left_to_h=left_to/in.Wards();
          if(n_hosp==left_to_h)
            delta_change++;
          else if(o_hosp==left_to_h)
           delta_change--; 
        }
      }
      else{
      right_to=st.SearchRightof(sw.student,sw.to_period);
      right_to_h=right_to/in.Wards();
      left_to=st.SearchLeftof(sw.student,sw.to_period);
      left_to_h=left_to/in.Wards();
        if(left_to_h==right_to_h){
          if(right_to==sw.new_ward || left_to==sw.new_ward)
            delta_change++;
          else if(n_hosp==right_to_h)
            delta_change+=2;
          else if(o_hosp==right_to_h)
            delta_change-=2;
        }
        else
        {
          if((left_to==sw.new_ward && o_hosp==right_to_h) ||
             (right_to==sw.new_ward && o_hosp==left_to_h))
          {
            delta_change--;
          }
          else if((left_to_h==n_hosp && o_hosp!=right_to_h && left_to!=sw.new_ward) ||
            (right_to_h==n_hosp && o_hosp!=left_to_h && right_to!=sw.new_ward))
          {
            delta_change++;
          }
          else if((left_to_h==o_hosp && n_hosp!=right_to_h) ||
                  (right_to_h==o_hosp && n_hosp!=left_to_h)) 
          {
            delta_change--;
          }
        }
      }
    if(delta_change!=0){ 
      st.AddChanges(sw.student,delta_change);
      int c_score=delta_change*in.StudentsVector(sw.student).GetChangeW();
      st.AddChangeScore(sw.student,c_score);
      st.AddTotalScore(sw.student,c_score);
      st.AddRealScore(c_score);
      if(sw.student==st.GetWorstStudent()) 
      {
        if(c_score>0)
          st.CheckWorst();
        
        else if (c_score<0)
          st.UpdateAllGaps(c_score); 
      }
      else
      {
        if(st.GetGapToWorst(sw.student)+c_score<0)
          st.UpdateWorst(sw.student); 
        else
          st.UpdateGapToWorst(sw.student); 
      }
    }
  }
}

void MSS_SwapNeighborhoodExplorer::FirstMove(const MSS_State& st, MSS_Swap& sw) const
{
  
  sw.student=-1;
  do
  {
    sw.student++;
  } while(st.AssignmentsSize(sw.student)<=1);
  sw.from_index=0;
  sw.old_ward=st.AssignmentWard(sw.student,sw.from_index);
  sw.from_period=st.AssignmentTime(sw.student,sw.from_index);
  sw.to_index=sw.from_index+1;
  sw.to_period=st.AssignmentTime(sw.student,sw.to_index);
  sw.new_ward=st.AssignmentWard(sw.student,sw.to_index);
  }

bool MSS_SwapNeighborhoodExplorer::NextMove(const MSS_State& st, MSS_Swap& sw) const
{ 
  if (NextToIndex(st,sw)) 
    {
      return true;
    }
  else if (NextFromIndex(st,sw)) 
    {
      FirstToIndex(st,sw); 
      return true;
    }
  else if (sw.student < in.Students() - 1) 
  
    {
      do
      {
      sw.student++;
      } while(st.AssignmentsSize(sw.student)<=1 && sw.student < in.Students());
      if(sw.student==in.Students())
        return false;
      FirstFromIndex(st,sw);
      FirstToIndex(st,sw);
      return true;
    }
  else return false;
}


void MSS_SwapNeighborhoodExplorer::FirstToIndex(const MSS_State& st, MSS_Swap& sw) const
{
  sw.to_index=sw.from_index+1;
  sw.to_period=st.AssignmentTime(sw.student,sw.to_index);
  sw.new_ward=st.AssignmentWard(sw.student,sw.to_index);
}


void MSS_SwapNeighborhoodExplorer::FirstFromIndex(const MSS_State& st, MSS_Swap& sw) const
{
  sw.from_index=0;
  sw.old_ward=st.AssignmentWard(sw.student,sw.from_index);
  sw.from_period=st.AssignmentTime(sw.student,sw.from_index);
}

bool MSS_SwapNeighborhoodExplorer::NextToIndex(const MSS_State& st, MSS_Swap& sw) const
{
  sw.to_index++;
  if(sw.to_index>=st.AssignmentsSize(sw.student))
    return false;
  sw.to_period=st.AssignmentTime(sw.student,sw.to_index);
  sw.new_ward=st.AssignmentWard(sw.student,sw.to_index);
  return true;
}

bool MSS_SwapNeighborhoodExplorer::NextFromIndex(const MSS_State& st, MSS_Swap& sw) const
{
  sw.from_index++;
  if(sw.from_index>=st.AssignmentsSize(sw.student)-1) 
    return false;
  sw.from_period=st.AssignmentTime(sw.student,sw.from_index);
  sw.old_ward=st.AssignmentWard(sw.student,sw.from_index);
  return true;
}



          
int MSS_MoveDeltaHospMaxRequirements::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta = 0,i,j;
  unsigned o_hosp=mv.old_ward/in.Wards(),n_hosp=mv.new_ward/in.Wards();
  unsigned o_numward=mv.old_ward%in.Wards(),n_numward=mv.new_ward%in.Wards();
  bool overlap;
  if(mv.old_ward!=mv.new_ward){ 
  for(i=0;i<in.Duration();i++){
    if(st.StudentsAvailable(mv.old_ward,mv.from_period+i)>in.HospitalsVector(o_hosp).GetMaxDemand(mv.from_period+i,o_numward))
        { delta--; }
  
  if(st.StudentsAvailable(mv.new_ward,mv.to_period+i)>=in.HospitalsVector(n_hosp).GetMaxDemand(mv.to_period+i,n_numward))
       { delta++; }
  }
  }
  else
  {
    for(i=0;i<in.Duration();i++){
      overlap=false;
      for(j=0;j<in.Duration();j++){
        if(mv.from_period+i==mv.to_period+j)
          {overlap=true;
           break;}
      }
      if(!overlap){
        if(st.StudentsAvailable(mv.old_ward,mv.from_period+i)>in.HospitalsVector(o_hosp).GetMaxDemand(mv.from_period+i,o_numward))
          { delta--; }
      }
    }
    for(j=0;j<in.Duration();j++){
      overlap=false;
        for(i=0;i<in.Duration();i++){
          if(mv.from_period+i==mv.to_period+j)
          {overlap=true;
           break;}
      }
      if(!overlap){
          if(st.StudentsAvailable(mv.new_ward,mv.to_period+j)>=in.HospitalsVector(n_hosp).GetMaxDemand(mv.to_period+j,n_numward))
            { delta++; }
        }
    }
  }
  return delta;
}
int MSS_MoveDeltaHospMinRequirements::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta = 0,i,j;
  unsigned o_hosp=mv.old_ward/in.Wards(),n_hosp=mv.new_ward/in.Wards();
  unsigned o_numward=mv.old_ward%in.Wards(),n_numward=mv.new_ward%in.Wards();
  bool overlap;
  if(mv.old_ward!=mv.new_ward){ 
  for(i=0;i<in.Duration();i++){
    if(in.HospitalsVector(o_hosp).GetMinDemand(mv.from_period+i,o_numward)!=0 && 
       in.HospitalsVector(o_hosp).GetMinDemand(mv.from_period+i,o_numward)>=st.StudentsAvailable(mv.old_ward,mv.from_period+i))
        { delta++; }
    
    if(in.HospitalsVector(n_hosp).GetMinDemand(mv.to_period+i,n_numward)!=0 && 
       in.HospitalsVector(n_hosp).GetMinDemand(mv.to_period+i,n_numward)>st.StudentsAvailable(mv.new_ward,mv.to_period+i))
       { delta--; }
  }
  }
  else
  {
    for(i=0;i<in.Duration();i++){
      overlap=false;
      for(j=0;j<in.Duration();j++){
        if(mv.from_period+i==mv.to_period+j)
          {overlap=true;
           break;}
      }
      if(!overlap){
        if(in.HospitalsVector(o_hosp).GetMinDemand(mv.from_period+i,o_numward)!=0 && 
          in.HospitalsVector(o_hosp).GetMinDemand(mv.from_period+i,o_numward)>=st.StudentsAvailable(mv.old_ward,mv.from_period+i))
            {delta++;}
      }
    }
    for(j=0;j<in.Duration();j++){
      overlap=false;
        for(i=0;i<in.Duration();i++){
          if(mv.from_period+i==mv.to_period+j)
          {overlap=true;
           break;}
      }
      if(!overlap){
          if(in.HospitalsVector(n_hosp).GetMinDemand(mv.to_period+j,n_numward)!=0 && 
             in.HospitalsVector(n_hosp).GetMinDemand(mv.to_period+j,n_numward)>st.StudentsAvailable(mv.new_ward,mv.to_period+j))
            { delta--; }
      }
    }
  }
  return delta;
}

int MSS_MoveDeltaPrecRequirements::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta=0,i;
  unsigned o_disc=mv.old_ward%in.Wards(),n_disc=mv.new_ward%in.Wards();
  unsigned required,required_by;
  if(o_disc==n_disc && mv.from_period!=mv.to_period)
  {
    
      for(i=0;i<in.DisciplinesVector(o_disc).SizeDependency();i++){
        
        required=in.DisciplinesVector(o_disc).GetDependency(i);
            
        if(st.GetFinished(mv.student,required)>st.GetFinished(mv.student,o_disc) &&
          mv.to_period>=st.GetFinished(mv.student,required)) 
          {  delta--; }
            
        else if(st.GetFinished(mv.student,required)<st.GetFinished(mv.student,o_disc) &&
                mv.to_period<st.GetFinished(mv.student,required))
          { delta++;  }
            
        } 
      
      
      for(i=0;i<in.DisciplinesVector(o_disc).SizeRequiredby();i++) 
      {
        required_by=in.DisciplinesVector(o_disc).GetRequiredby(i);
        if(st.GetFinished(mv.student,o_disc)>st.GetFinished(mv.student,required_by) &&
           mv.to_period<st.GetFinished(mv.student,required_by))
          delta--;
        else if(st.GetFinished(mv.student,o_disc)<st.GetFinished(mv.student,required_by) &&
           mv.to_period>=st.GetFinished(mv.student,required_by))
          delta++;
      }
      
    }
    else if(o_disc!=n_disc) 
    {
      
      for(i=0;i<in.DisciplinesVector(o_disc).SizeRequiredby();i++) 
      {
        required_by=in.DisciplinesVector(o_disc).GetRequiredby(i);
        
        if(st.GetFinished(mv.student,o_disc)<st.GetFinished(mv.student,required_by)
          && st.Allocated(mv.student,required_by))
          delta++;
        
      }
      for(i=0;i<in.DisciplinesVector(o_disc).SizeDependency();i++){
        required=in.DisciplinesVector(o_disc).GetDependency(i);
        if(st.GetFinished(mv.student,required)>st.GetFinished(mv.student,o_disc))
          delta--;
        
      }
      
      for(i=0;i<in.DisciplinesVector(n_disc).SizeRequiredby();i++) 
      {
        required_by=in.DisciplinesVector(n_disc).GetRequiredby(i);
        if(mv.to_period<st.GetFinished(mv.student,required_by) && st.Allocated(mv.student,required_by) && required_by!=o_disc)
          delta--;
      }
      for(i=0;i<in.DisciplinesVector(n_disc).SizeDependency();i++){
        required=in.DisciplinesVector(n_disc).GetDependency(i);
        if(st.GetFinished(mv.student,required)>mv.to_period || required==o_disc)
          delta++;
      }
    }
    return delta;
}

int MSS_MoveDeltaDiscPreference::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta=0;
  unsigned o_disc=mv.old_ward%in.Wards();
  unsigned n_disc=mv.new_ward%in.Wards();
  unsigned disc_w,disc_osc,disc_nsc;
  if(o_disc!=n_disc) 
  {
    disc_w=in.StudentsVector(mv.student).GetDisciplineW();
    disc_osc=in.GetDiscPref(mv.student,o_disc);
    disc_nsc=in.GetDiscPref(mv.student,n_disc);
    delta-=disc_w*disc_osc;
    delta+=disc_w*disc_nsc;
  }
  return -delta;
}

int MSS_MoveDeltaMngrPreference::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta=0;
  unsigned o_disc=mv.old_ward%in.Wards();
  unsigned n_disc=mv.new_ward%in.Wards();
  if(o_disc!=n_disc) 
  {
    delta-=in.GetMangPref(o_disc);
    delta+=in.GetMangPref(n_disc);
  }
  return -delta;
}

int MSS_MoveDeltaHospPreference::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta=0;
  unsigned o_hosp=mv.old_ward/in.Wards();
  unsigned n_hosp=mv.new_ward/in.Wards();
  unsigned hosp_w,hosp_osc,hosp_nsc;
  if(o_hosp!=n_hosp) 
  {
    hosp_w=in.StudentsVector(mv.student).GetHospitalW();
    hosp_osc=in.GetHospPref(mv.student,o_hosp);
    hosp_nsc=in.GetHospPref(mv.student,n_hosp);
    delta-=hosp_w*hosp_osc;
    delta+=hosp_w*hosp_nsc;
  }
  return -delta;
}
int MSS_MoveDeltaWaitingCosts::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta=0,delta_wait=0,offset=in.Duration()-1;
  int old_first=st.FirstPeriod(mv.student),old_last=st.LastPeriod(mv.student);
  int new_first,new_last;
  if(mv.from_period!=mv.to_period && old_first+offset!=old_last){
    new_first=st.NewFirst(mv.student,mv.from_period,mv.to_period);
    new_last=st.NewLast(mv.student,mv.from_period,mv.to_period);
    delta_wait=st.DeltaWait(mv.student,mv.from_period,mv.to_period,new_first,new_last);
    delta=delta_wait*in.StudentsVector(mv.student).GetWaitW();
  }
  if(mv.from_period!=mv.to_period && old_first+offset==old_last){
    if(mv.to_period>mv.from_period)
      delta_wait+=mv.to_period-mv.from_period;
    else
      delta_wait-=mv.from_period-mv.to_period;
    delta=delta_wait*in.StudentsVector(mv.student).GetWaitW();
  }
 
 return -delta;
}



int MSS_MoveDeltaChangeCosts::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta=0,delta_change=0,offset=in.Duration()-1;
  int old_first=st.FirstPeriod(mv.student),old_last=st.LastPeriod(mv.student);
  int new_first,new_last;
  int o_hosp=mv.old_ward/in.Wards(),n_hosp=mv.new_ward/in.Wards();

  if((mv.to_period!=mv.from_period || n_hosp!=o_hosp) && old_first+offset!=old_last) { 
    if(mv.to_period!=mv.from_period){
    new_first=st.NewFirst(mv.student,mv.from_period,mv.to_period);
    new_last=st.NewLast(mv.student,mv.from_period,mv.to_period);
    }
    else{
      new_first=old_first;
      new_last=old_last;
    }
    delta_change=st.DeltaChangeMove(mv.student,mv.from_period,mv.to_period,mv.new_ward,new_first,new_last);
    delta=delta_change*in.StudentsVector(mv.student).GetChangeW();
  }
  return -delta;
}


int MSS_MoveDeltaWorstScore::ComputeDeltaCost(const MSS_State& st, const MSS_Move& mv) const
{
  int delta=0,delta_st=0,delta_change=0,delta_wait=0,offset=in.Duration()-1;
  unsigned o_disc=mv.old_ward%in.Wards();
  unsigned n_disc=mv.new_ward%in.Wards();
  unsigned disc_w,disc_osc,disc_nsc;
  if(o_disc!=n_disc) 
  {
    disc_w=in.StudentsVector(mv.student).GetDisciplineW();
    disc_osc=in.GetDiscPref(mv.student,o_disc);
    disc_nsc=in.GetDiscPref(mv.student,n_disc);
    delta_st-=disc_w*disc_osc;
    delta_st+=disc_w*disc_nsc;
    delta_st-=in.GetMangPref(o_disc);
    delta_st+=in.GetMangPref(n_disc);
  }
  unsigned o_hosp=mv.old_ward/in.Wards();
  unsigned n_hosp=mv.new_ward/in.Wards();
  unsigned hosp_w,hosp_osc,hosp_nsc;
  if(o_hosp!=n_hosp) 
  {
    hosp_w=in.StudentsVector(mv.student).GetHospitalW();
    hosp_osc=in.GetHospPref(mv.student,o_hosp);
    hosp_nsc=in.GetHospPref(mv.student,n_hosp);
    delta_st-=hosp_w*hosp_osc;
    delta_st+=hosp_w*hosp_nsc;
  }
  int old_first=st.FirstPeriod(mv.student),old_last=st.LastPeriod(mv.student);
  int new_first,new_last;
  if((mv.to_period!=mv.from_period || n_hosp!=o_hosp) && old_first+offset!=old_last) {
    new_first=st.NewFirst(mv.student,mv.from_period,mv.to_period);
    new_last=st.NewLast(mv.student,mv.from_period,mv.to_period);
    delta_change=st.DeltaChangeMove(mv.student,mv.from_period,mv.to_period,mv.new_ward,new_first,new_last);
    delta_st+=delta_change*in.StudentsVector(mv.student).GetChangeW();
  }
  if(mv.from_period!=mv.to_period && old_first+offset!=old_last){
    new_first=st.NewFirst(mv.student,mv.from_period,mv.to_period);
    new_last=st.NewLast(mv.student,mv.from_period,mv.to_period);

    delta_wait=st.DeltaWait(mv.student,mv.from_period,mv.to_period,new_first,new_last);
    delta_st+=delta_wait*in.StudentsVector(mv.student).GetWaitW();
  }
  if(mv.from_period!=mv.to_period && old_first+offset==old_last){
    if(mv.to_period>mv.from_period)
      delta_wait+=mv.to_period-mv.from_period;
    else
      delta_wait-=mv.from_period-mv.to_period;
    delta_st+=delta_wait*in.StudentsVector(mv.student).GetWaitW();
  }
  if(mv.student==st.GetWorstStudent()) 
   {
    if(delta_st>0) 
    {
      if(st.NewWorst(delta_st)!=mv.student) 
        delta=-(st.GetTotalScore(st.NewWorst(delta_st))-(st.GetWorstScore()));
      else 
        delta=-(delta_st);
    }   
    else { delta=-delta_st; } 
   }
  else if(st.GetGapToWorst(mv.student)+delta_st<0)
    {
    delta=-(st.GetGapToWorst(mv.student)+delta_st); 
    }
  else
    {
      delta=0;
    }
    
  return delta;
}



int MSS_SwapDeltaHospMaxRequirements::ComputeDeltaCost(const MSS_State& st, const MSS_Swap& sw) const
{
  
  int delta = 0;
  unsigned f_hosp=sw.old_ward/in.Wards(),s_hosp=sw.new_ward/in.Wards();
  unsigned f_numward=sw.old_ward%in.Wards(),s_numward=sw.new_ward%in.Wards();
  for(int i=0;i<in.Duration();i++){
  if(st.StudentsAvailable(sw.old_ward,sw.from_period+i)>in.HospitalsVector(f_hosp).GetMaxDemand(sw.from_period+i,f_numward))
    { delta--; }
  if(st.StudentsAvailable(sw.new_ward,sw.to_period+i)>in.HospitalsVector(s_hosp).GetMaxDemand(sw.to_period+i,s_numward))
    { delta--; }
  if(st.StudentsAvailable(sw.old_ward,sw.to_period+i)>=in.HospitalsVector(f_hosp).GetMaxDemand(sw.to_period+i,f_numward))
    { delta++;}
  if(st.StudentsAvailable(sw.new_ward,sw.from_period+i)>=in.HospitalsVector(s_hosp).GetMaxDemand(sw.from_period+i,s_numward))
    { delta++; }
  }
  return delta;
}

int MSS_SwapDeltaHospMinRequirements::ComputeDeltaCost(const MSS_State& st, const MSS_Swap& sw) const
{
  int delta = 0;
  unsigned f_hosp=sw.old_ward/in.Wards(),s_hosp=sw.new_ward/in.Wards();
  unsigned f_numward=sw.old_ward%in.Wards(),s_numward=sw.new_ward%in.Wards();
  for(int i=0;i<in.Duration();i++){
    if(in.HospitalsVector(f_hosp).GetMinDemand(sw.from_period+i,f_numward)>0){
      if(in.HospitalsVector(f_hosp).GetMinDemand(sw.from_period+i,f_numward)>=st.StudentsAvailable(sw.old_ward,sw.from_period+i))
        { delta++; }
    }
    if(in.HospitalsVector(s_hosp).GetMinDemand(sw.to_period+i,s_numward)>0){
      if(in.HospitalsVector(s_hosp).GetMinDemand(sw.to_period+i,s_numward)>=st.StudentsAvailable(sw.new_ward,sw.to_period+i))
        { delta++; }
    }
    if(in.HospitalsVector(f_hosp).GetMinDemand(sw.to_period+i,f_numward)>0){
      if(in.HospitalsVector(f_hosp).GetMinDemand(sw.to_period+i,f_numward)>st.StudentsAvailable(sw.old_ward,sw.to_period+i))
      { delta--;}
    }
    if(in.HospitalsVector(s_hosp).GetMinDemand(sw.from_period+i,s_numward)>0){
      if(in.HospitalsVector(s_hosp).GetMinDemand(sw.from_period+i,s_numward)>st.StudentsAvailable(sw.new_ward,sw.from_period+i))
      { delta--; }
    }
  }
  return delta;
}

int MSS_SwapDeltaPrecRequirements::ComputeDeltaCost(const MSS_State& st, const MSS_Swap& sw) const
{
  int delta=0,i;
  unsigned first_disc=sw.old_ward%in.Wards(),second_disc=sw.new_ward%in.Wards();
  unsigned required,required_by;
      for(i=0;i<in.DisciplinesVector(first_disc).SizeDependency();i++){
        required=in.DisciplinesVector(first_disc).GetDependency(i);
        if(st.GetFinished(sw.student,required)>st.GetFinished(sw.student,first_disc) &&
          sw.to_period>=st.GetFinished(sw.student,required)) 
          {  delta--; }
        else if(st.GetFinished(sw.student,required)<st.GetFinished(sw.student,first_disc) &&
                sw.to_period<st.GetFinished(sw.student,required))
          { delta++;  }
        } 
      for(i=0;i<in.DisciplinesVector(first_disc).SizeRequiredby();i++) 
      {
        required_by=in.DisciplinesVector(first_disc).GetRequiredby(i);
        if(st.GetFinished(sw.student,first_disc)>st.GetFinished(sw.student,required_by) &&
           sw.to_period<st.GetFinished(sw.student,required_by))
          delta--;
        else if(st.GetFinished(sw.student,first_disc)<st.GetFinished(sw.student,required_by) &&
           sw.to_period>=st.GetFinished(sw.student,required_by))
          delta++;
      }
      for(i=0;i<in.DisciplinesVector(second_disc).SizeDependency();i++){
        required=in.DisciplinesVector(second_disc).GetDependency(i);
        if(st.GetFinished(sw.student,required)>st.GetFinished(sw.student,second_disc) &&
          sw.from_period>=st.GetFinished(sw.student,required)) 
          {  delta--;}
        else if(st.GetFinished(sw.student,required)<st.GetFinished(sw.student,second_disc) &&
                sw.from_period<st.GetFinished(sw.student,required))
          { delta++; }
            
        } 
      for(i=0;i<in.DisciplinesVector(second_disc).SizeRequiredby();i++) 
      {
        required_by=in.DisciplinesVector(second_disc).GetRequiredby(i);
        if(st.GetFinished(sw.student,second_disc)>st.GetFinished(sw.student,required_by) &&
           sw.from_period<st.GetFinished(sw.student,required_by))
          delta--;
        else if(st.GetFinished(sw.student,second_disc)<st.GetFinished(sw.student,required_by) &&
           sw.from_period>=st.GetFinished(sw.student,required_by))
          delta++;
      }
    return delta;
}

int MSS_SwapDeltaChangeCosts::ComputeDeltaCost(const MSS_State& st, const MSS_Swap& sw) const
{
  int delta=0;
  int from_hosp=sw.old_ward/in.Wards(),to_hosp=sw.new_ward/in.Wards();
  if(from_hosp!=to_hosp)
  {
    delta=st.DeltaChangeSwap(sw.student,sw.from_index,sw.to_index);
  }
  return -delta;
  }

int MSS_SwapDeltaWorstScore::ComputeDeltaCost(const MSS_State& st, const MSS_Swap& sw) const
{
  int delta_st=0,delta=0;
  int o_hosp=sw.old_ward/in.Wards(),n_hosp=sw.new_ward/in.Wards();
  if(o_hosp!=n_hosp) 
  {
   delta_st=st.DeltaChangeSwap(sw.student,sw.from_index,sw.to_index);
   if(sw.student==st.GetWorstStudent()) 
   {
    if(delta_st>0) 
    {
      if(st.NewWorst(delta_st)!=sw.student)
        delta=-(st.GetTotalScore(st.NewWorst(delta_st))-st.GetWorstScore());
      else 
        delta=-(delta_st);
    }   
    else { delta=-delta_st; } 
   }
    else if(st.GetGapToWorst(sw.student)+delta_st<0)
    {
      delta=-(st.GetGapToWorst(sw.student)+delta_st); 
    }
    else
    {
      delta=0;
    }
   }
  return delta;
}