// File MSS_Basics.cc
#include "MSS_Basics.hh"
#include <iomanip>

MSS_State::MSS_State(const MSS_Input &my_in) 
  : in(my_in),
    schedule(in.Students(),vector<int>(in.TimePeriods(),-1)), 
    hospital_pr_score(in.Students(),0), 
    discipline_pr_score(in.Students(),0), 
    manager_pr_score(in.Students(),0), 
    wait_score(in.Students(),0), 
    change_score(in.Students(),0),
    total_score(in.Students(),0), 
    gap_to_worst(in.Students(),0),
    assignments(in.Students(),vector<pair<unsigned,unsigned>>(0)),
    finished(in.Students(),vector<unsigned>(in.Disciplines(),in.TimePeriods()+1)), 
    allocated(in.Students(),vector<bool>(in.Disciplines(),false)),
    at_hospital(in.Students(),vector<int>(in.Disciplines(),-1)), 
    students_available(in.TotalWards(),vector<unsigned> (in.TimePeriods(),0)),
    waiting_times(in.Students(),0),
    changes(in.Students(),0),
    first_period(in.Students(),-1), 
    last_period(in.Students(),-1), 
    free_disciplines(in.Students(),vector<pair<unsigned,unsigned>>(0)),
    free_wards(in.Students(),vector<unsigned>(0)),
    disc_per_hosp(in.Students(),vector<unsigned>(in.Hospitals(),0))
  { real_score=0;
    worst_score=0;
    worst_student=-1;
  }


MSS_State& MSS_State::operator=(const MSS_State& st)
{
  schedule=st.schedule;
  allocated=st.allocated;
  students_available=st.students_available;
  finished=st.finished;
  assignments=st.assignments;
  at_hospital=st.at_hospital;
  waiting_times=st.waiting_times;
  changes=st.changes;
  first_period=st.first_period;
  last_period=st.last_period;
  free_disciplines=st.free_disciplines;
  free_wards=st.free_wards;
  real_score=st.real_score;
  hospital_pr_score=st.hospital_pr_score;
  discipline_pr_score=st.discipline_pr_score;
  manager_pr_score=st.manager_pr_score;
  wait_score=st.wait_score;
  change_score=st.change_score;
  total_score=st.total_score;
  worst_score=st.worst_score;
  worst_student=st.worst_student;
  disc_per_hosp=st.disc_per_hosp;
  gap_to_worst=st.gap_to_worst;
  return *this;
}
  
bool MSS_State::Occupied (unsigned s,int t,unsigned d) const 
{
  int i;
  for(i=0;i<d;i++){
    if(schedule[s][t+i]!=-1)
      return true;
  }
  return false;
}

bool MSS_State::OtOccupied (unsigned s,int t,unsigned d,int ow) const 
{
  int i;
  for(i=0;i<d;i++){
    if(schedule[s][t+i]!=-1 && schedule[s][t+i]!=ow)
      return true;
  }
  return false;
}
bool MSS_State::IsAble(unsigned s,unsigned h,unsigned d) const
{
  return in.StudentsVector(s).GetAbility(h,d);
}

bool MSS_State::CanAttend (unsigned s,int t,unsigned d) const 
{
  int i;
  for(i=0;i<d;i++){
    if(!in.GetAvailability(s,t+i))
      return false;
  }
  return true;
}

bool MSS_State::Ready (unsigned s,unsigned d)const 
{
  unsigned i,size,current;
  size=in.DisciplinesVector(d).SizeDependency();
  for(i=0;i<size;i++){
    current=in.DisciplinesVector(d).GetDependency(i);
    if(!allocated[s][current])
      {
        return false;
      }
  }
  return true;
}

bool MSS_State::IntoBounds (int t,unsigned d)const 
{
  return t+d<=in.TimePeriods();
}
void MSS_State::ResetState()
{
  real_score=0,worst_score=0,worst_student=-1;
  unsigned i,j;
  for (i=0;i<in.Students();i++){
    for(j=0;j<in.TimePeriods();j++){
      schedule[i][j]=-1;
    }
  }
  for (i=0;i<in.Students();i++){
    for(j=0;j<in.Disciplines();j++){
        allocated[i][j]=false;
    }
  }
  for (i=0;i<in.TotalWards();i++){
    for(j=0;j<in.TimePeriods();j++){
        ResetStudentsAvailable(i,j);
    }
  }
  for (i=0;i<in.Students();i++){
    for(j=0;j<in.Disciplines();j++){
        finished[i][j]=in.TimePeriods()+1;
        at_hospital[i][j]=-1;
    }
  }
  for (i=0;i<in.Students();i++)
  {
    waiting_times[i]=0;
    changes[i]=0;
    first_period[i]=-1;
    last_period[i]=-1;
    ResetAssignments(i);
    ResetFreeDisc(i);
    ResetFreeWards(i);
    hospital_pr_score[i]=0;
    discipline_pr_score[i]=0;
    manager_pr_score[i]=0;
    change_score[i]=0;
    wait_score[i]=0;
    total_score[i]=0;
    gap_to_worst[i]=0;
  }
  for (i=0;i<in.Students();i++){
    for(j=0;j<in.Hospitals();j++){
      disc_per_hosp[i][j]=0;
    }
  }
}

void MSS_State::ChangeAssignment(unsigned s,unsigned i,unsigned nw,unsigned nt)
{
  assignments[s][i].first=nw;
  assignments[s][i].second=nt;
}

void MSS_State::SwapAssignment(unsigned s,unsigned i1,unsigned i2)
{
  unsigned temp=assignments[s][i2].second;
  assignments[s][i2].second=assignments[s][i1].second;
  assignments[s][i1].second=temp;
}

int MSS_State::SearchNewFirst(unsigned s,unsigned index) const
{
  unsigned i;
  for(i=index;i<in.TimePeriods();i++)
  {
    if(schedule[s][i]!=-1)
      return i;
  }
  return -1; 
}

int MSS_State::SearchNewFirst(unsigned s,unsigned index1,unsigned index2) const
{
  unsigned i;
  for(i=index1+in.Duration();i<in.TimePeriods();i++)
  {
    if(schedule[s][i]!=-1 || i==index2)
      return i;
  }
  return -1; 
}

int MSS_State::SearchNewLast(unsigned s,unsigned index) const
{
  unsigned i;
  for(i=0;i<index;i++)
  {
    if(schedule[s][index-i]!=-1)
    {
      return index-i; 
    }          
  }
  return -1; 
}

int MSS_State::SearchNewLast(unsigned s,unsigned index1,unsigned index2) const
{
  unsigned i,offset=in.Duration()-1;
  for(i=1;i<index1;i++)
  {
    if(schedule[s][index1-i]!=-1||(index1-i)==index2+offset)
      return index1-i;           
  }
  return -1; 
}

int MSS_State::SearchRightof(unsigned s,unsigned tp1,unsigned tp2)  const 
{
  for(int i=tp1+in.Duration();i<in.TimePeriods();i++){
    if(schedule[s][i]!=-1 && i!=tp2 && schedule[s][i]!=schedule[s][tp2])
      return schedule[s][i];
  }
  return -1;
}
int MSS_State::SearchLeftof(unsigned s,unsigned tp1,unsigned tp2) const 
{
  for(int i=1;i<=tp1;i++){
    if(schedule[s][tp1-i]!=-1 && (tp1-i)!=tp2 && schedule[s][tp1-i]!=schedule[s][tp2])
      {
        return schedule[s][tp1-i];
      }
  }
  return -1;
}

int MSS_State::SearchRightof(unsigned s,unsigned tp)  const 
{
  for(int i=tp+in.Duration();i<in.TimePeriods();i++){
    if(schedule[s][i]!=-1)
      return schedule[s][i];
  }
  return -1;
}
int MSS_State::SearchLeftof(unsigned s,unsigned tp) const 
{
  for(int i=1;i<=tp;i++){
    if(schedule[s][tp-i]!=-1)
      return schedule[s][tp-i];
  }
  return -1;
}

int MSS_State::NewFirst(unsigned s,int from,int to) const
{
int new_first;
int x=first_period[s];
if(from==x)
     {
      if(from>to) 
          new_first=to;
      else
          new_first=SearchNewFirst(s,from,to); 
     }
else if(to<x){  new_first=to; }
else new_first=x;
return new_first;
}

int MSS_State::NewLast(unsigned s,int from,int to) const
{
unsigned offset=in.Duration()-1;
int new_last;
int x=last_period[s];
if(from+offset==x){
    if(from<to) 
      new_last=to+in.Duration()-1;
    else if(to+offset>=from)
      {
        new_last=to+in.Duration()-1;
      }
    else
      new_last=SearchNewLast(s,from,to);
    }
else if(to>x){ new_last=to+in.Duration()-1; }
else new_last=x;
return new_last;
}
int MSS_State::DeltaWait(unsigned s,int from,int to,int nf,int nl) const
{
  unsigned duration=in.Duration(),offset=in.Duration()-1;
  int delta_wait=0;
  if(from==first_period[s])
    {
      if(to==nf && to<from)
      {}
      else if(to==nf && to>from) 
      {}
      else if(to+offset==nl) 
      {
        delta_wait+=duration;
        delta_wait+=to-last_period[s]-1;
      }
      else   
      {}
    }
    else if(from+offset==last_period[s]){
      if(to+offset==nl && to>from)
      {
        delta_wait+=to-from; 
      }
      else if(to+offset==nl && to<from)
      {
        delta_wait-=from-to;
      }
      else if(to==nf)
      {
        delta_wait-=from-nl-1;
        delta_wait-=duration;
      }  
      else
        {
          delta_wait-=from-nl-1; 
          delta_wait-=duration;
        }
    }
    else if(to==nf) 
    {
      delta_wait+=0;
    }
    else if(to+offset==nl)
    {
      delta_wait+=to+offset-last_period[s];
    }
  return delta_wait;
}

int MSS_State::NewWorst(int delta) const
{ 
  int new_worst=-1;
  int new_worst_score=worst_score+delta;
  for(int i=0;i<in.Students();i++)
  {
    if(i!=worst_student && total_score[i]<new_worst_score)
      {
        new_worst=i;
        new_worst_score=total_score[i];
      }
  }
  if(new_worst!=-1)
    return new_worst;
  return worst_student;
}

int MSS_State::DeltaChangeMove(unsigned s,int from,int to,int new_ward,int nf,int nl) const
{
int delta_change=0;
unsigned offset=in.Duration()-1;
int o_hosp=schedule[s][from]/in.Wards(),n_hosp=new_ward/in.Wards();
int left_from=-1,right_from=-1,left_to=-1,right_to=-1;
int left_from_h=-1,right_from_h=-1,left_to_h=-1,right_to_h=-1;
    if(from==first_period[s]) 
      {
        right_from=SearchRightof(s,from,to);
        right_from_h=right_from/in.Wards();
        if(o_hosp!=right_from_h)
          delta_change--;
      }
    else if (from+offset==last_period[s])
    { 
      left_from=SearchLeftof(s,from,to);
      left_from_h=left_from/in.Wards();
      if(o_hosp!=left_from_h)
          delta_change--;
    }
    else
    {
      right_from=SearchRightof(s,from,to);
      right_from_h=right_from/in.Wards();
      left_from=SearchLeftof(s,from,to);
      left_from_h=left_from/in.Wards();
      if(o_hosp!=left_from_h && o_hosp !=right_from_h){
        if(left_from_h==right_from_h)
          delta_change-=2;
        else
          delta_change--;
      }
    }
    if(to==nf)
      {
        right_to=SearchRightof(s,to,from);
        right_to_h=right_to/in.Wards();
        if(n_hosp!=right_to_h)
          delta_change++;
      }
    else if(to+offset==nl)
      {
      left_to=SearchLeftof(s,to,from);
      left_to_h=left_to/in.Wards();
      if(n_hosp!=left_to_h)
          delta_change++;
      }
    else
      {
        right_to=SearchRightof(s,to,from);
        right_to_h=right_to/in.Wards();
        left_to=SearchLeftof(s,to,from);
        left_to_h=left_to/in.Wards();
        if(left_to_h==right_to_h && n_hosp!=left_to_h)
          delta_change+=2;
        else if(left_to_h!=right_to_h && n_hosp!=left_to_h && n_hosp!=right_to_h)
          delta_change++;
      }
  return delta_change;
}

int MSS_State::DeltaChangeSwap(unsigned s,int from,int to) const 
{
    int old_ward=AssignmentWard(s,from);
    int new_ward=AssignmentWard(s,to);
    int from_period=AssignmentTime(s,from);
    int to_period=AssignmentTime(s,to);
    int from_hosp=old_ward/in.Wards(),to_hosp=new_ward/in.Wards();
    int delta,delta_change=0;
    int first=first_period[s],last=last_period[s],offset=in.Duration()-1;
    int left_from=-1,right_from=-1,left_to=-1,right_to=-1;
    int left_from_h=-1,right_from_h=-1,left_to_h=-1,right_to_h=-1;
    if(from_period==first) 
      {
        right_from=SearchRightof(s,from_period);
        if(right_from!=new_ward) 
        {
          right_from_h=right_from/in.Wards();
          if(from_hosp==right_from_h)
            delta_change++;
          else if(to_hosp==right_from_h)
           {
            delta_change--;
           }  
        }
      }
    else if (from_period+offset==last) 
    {
        left_from=SearchLeftof(s,from_period);
        if(left_from!=new_ward) 
        {
          left_from_h=left_from/in.Wards();
          if(from_hosp==left_from_h)
            delta_change++;
          else if(to_hosp==left_from_h)
           delta_change--; 
        }
    }
    else 
    {
      right_from=SearchRightof(s,from_period);
      right_from_h=right_from/in.Wards();
      left_from=SearchLeftof(s,from_period);
      left_from_h=left_from/in.Wards();
        if(left_from_h==right_from_h){
          if(right_from==new_ward || left_from==new_ward)
            delta_change--;
          else if(from_hosp==right_from_h)
            delta_change+=2;
          else if(to_hosp==right_from_h)
            delta_change-=2;
        }
        else
        {
          if((left_from==new_ward && to_hosp==right_from_h) ||
             (right_from==new_ward && to_hosp==left_from_h))
          {
            delta_change--;
          }
          else if((left_from==new_ward && from_hosp==right_from_h) ||
                 (right_from==new_ward && from_hosp==left_from_h))
          {
            delta_change++;
          }
          else if((left_from_h==from_hosp && to_hosp!=right_from_h) ||
            (right_from_h==from_hosp && to_hosp!=left_from_h))
          {
            delta_change++;
          }
          else if((left_from_h==to_hosp && from_hosp!=right_from_h && left_from!=new_ward) ||
                  (right_from_h==to_hosp && from_hosp!=left_from_h && right_from!=new_ward)) 
          {
            delta_change--;
          }
        }
    }
    if(to_period==first) 
      {
        right_to=SearchRightof(s,to_period);
        if(right_to!=old_ward) 
        {
          right_to_h=right_to/in.Wards();
          if(to_hosp==right_to_h)
            { 
              delta_change++;
            }
          else if(from_hosp==right_to_h)
            {
              delta_change--;
            }
        }
      }
      else if(to_period+offset==last)
      {
        left_to=SearchLeftof(s,to_period);
        if(left_to!=old_ward) 
        {
          left_to_h=left_to/in.Wards();
          if(to_hosp==left_to_h)
            delta_change++;
          else if(from_hosp==left_to_h)
           delta_change--; 
        }
      }
    else 
    {
      right_to=SearchRightof(s,to_period);
      right_to_h=right_to/in.Wards();
      left_to=SearchLeftof(s,to_period);
      left_to_h=left_to/in.Wards();
        if(left_to_h==right_to_h){
          if(right_to==old_ward || left_to==old_ward)
            { 
              delta_change--;
            }
          else if(to_hosp==right_to_h)
            delta_change+=2;
          else if(from_hosp==right_to_h)
            delta_change-=2;
        }
        else
        {
          if((left_to==old_ward && from_hosp==right_to_h) ||
             (right_to==old_ward && from_hosp==left_to_h))
          {
            delta_change--;
          }
          else if((left_to==old_ward && to_hosp==right_to_h) ||
                 (right_to==old_ward && to_hosp==left_to_h))
          {
            delta_change++;
          }
          else if((left_to_h==to_hosp && from_hosp!=right_to_h) ||
            (right_to_h==to_hosp && from_hosp!=left_to_h))
          {
            delta_change++;
          }
          else if((left_to_h==from_hosp && to_hosp!=right_to_h && left_to!=old_ward) ||
                  (right_to_h==from_hosp && to_hosp!=left_to_h && right_to!=old_ward)) 
          {
            delta_change--;
          }
        }
    }
    delta=delta_change*in.StudentsVector(s).GetChangeW();
    return delta;
}

void MSS_State::CheckWorst()
{
  unsigned i;
  worst_score=total_score[worst_student];
  for (i=0;i<in.Students();i++){
    if(i!=worst_student && total_score[i]<total_score[worst_student]) 
    {
      worst_score=total_score[i];
      worst_student=i;
    }
  }
  for (i=0;i<in.Students();i++)
      gap_to_worst[i]=total_score[i]-total_score[worst_student];
  }
void MSS_State::UpdateAllGaps(int delta) 
{
  for (int i=0;i<in.Students();i++){
    if(i!=worst_student) 
      gap_to_worst[i]=gap_to_worst[i]-delta; 
  }
  worst_score=total_score[worst_student]; 
}

void MSS_State::UpdateWorst(unsigned s) 
{
  worst_student=s;
  worst_score=total_score[s];
  for (int i=0;i<in.Students();i++)
      gap_to_worst[i]=total_score[i]-worst_score;
}
void MSS_State::UpdateGapToWorst(unsigned s)
{
  gap_to_worst[s]=total_score[s]-worst_score;
}

void MSS_State::UpdateRedundantStateData()
{
  unsigned i,j,k,ward;
  for (i=0;i<in.Students();i++){
    for(j=0;j<in.TimePeriods();j++){
      if(schedule[i][j]!=-1){
        ward=schedule[i][j];
        IncStudentsAvailable(ward,j);
      }
    }
  }
  unsigned finish;  
  for (i=0;i<in.Students();i++){
    finish=last_period[i];
    for(j=0;j<finish;j++){
      if(schedule[i][j]==-1){
        waiting_times[i]++;
      }
    }
  }
  int current,next,current_hospital,next_hospital,duration=in.Duration();
  for (i=0;i<in.Students();i++){
    current=first_period[i];
    finish=last_period[i];
    next=current+duration;
    current_hospital=in.WardsVector(schedule[i][current]).GetHospId();
      do 
      {
        while(next<=finish) 
        {
          if(schedule[i][next]!=-1) 
          {
            next_hospital=in.WardsVector(schedule[i][next]).GetHospId();
            if(current_hospital!=next_hospital)
            {
              changes[i]++;
            }
            current=next;
            current_hospital=next_hospital;
            next=current+duration;
          }
          else 
            next++;
        }
        current=next;
      } while (current<finish);
  }         
  unsigned period;
  for (i=0;i<in.Students();i++){
    for(j=0;j<in.Disciplines();j++){
      if(allocated[i][j]){
        period=finished[i][j]-1;
        ward=schedule[i][period];
        at_hospital[i][j]=ward/in.Wards();
      }
    }
  }
  unsigned size;
  for (i=0;i<in.Students();i++){
    for(j=0;j<in.Groups();j++){
      if(in.GetToFullfill(i,j)<in.StudentsVector(i).SizeCompDiscList(j))
      {
        size=in.StudentsVector(i).SizeCompDiscList(j);
        for(k=0;k<size;k++){
          current=in.StudentsVector(i).GetCompDiscList(j,k);
          if(!allocated[i][current])
            {
              AddFreeDisc(i,j,current);
              for(int h=0;h<in.Hospitals();h++){
                AddFreeWard(i,current+(h*in.Wards()));
              }
            }
        }
      }
    }
  }
}

void MSS_State::UpdateScores()
{
  int i,j,disc_w,disc_sc,hosp,hosp_w,hosp_sc,d_score,h_score,m_score;
  bool first=true;
  for (i=0;i<in.Students();i++){
    d_score=0;
    h_score=0;
    m_score=0;
    for(j=0;j<in.Disciplines();j++){
      if(allocated[i][j])
      {
        hosp=at_hospital[i][j];
        hosp_w=in.StudentsVector(i).GetHospitalW();
        hosp_sc=in.GetHospPref(i,hosp);
        h_score+=hosp_w*hosp_sc;
        disc_w=in.StudentsVector(i).GetDisciplineW();
        disc_sc=in.GetDiscPref(i,j);
        d_score+=disc_w*disc_sc;
        m_score+=in.GetMangPref(j);
      }
    }
    discipline_pr_score[i]=d_score;
    hospital_pr_score[i]=h_score;
    manager_pr_score[i]=m_score;
    wait_score[i]=waiting_times[i]*in.StudentsVector(i).GetWaitW();
    change_score[i]=changes[i]*in.StudentsVector(i).GetChangeW();
    total_score[i]=d_score+h_score+m_score+wait_score[i]+change_score[i];
    if(first)
      {
        worst_score=total_score[i];
        worst_student=i;
      }
    else if (total_score[i]<worst_score)
      {
        worst_score=total_score[i];
        worst_student=i;
      }
    real_score+=total_score[i];
    first=false;
  }
  for (i=0;i<in.Students();i++)
  {
    gap_to_worst[i]=total_score[i]-worst_score;
  }
}

void MSS_State::UpdateFirstLast(unsigned s,unsigned t)
{
  if(first_period[s]==-1 || first_period[s]>t)
    first_period[s]=t;
  if(last_period[s]==-1 || last_period[s]<t+in.Duration()-1)
    last_period[s]=t+in.Duration()-1;
}


void MSS_State::UpdateAdditionalRedundantStateData()
{
  unsigned discipline,last,hosp;
  for(int i=0;i<in.Students();i++){
    bool found=false;
    for(int j=0;j<in.TimePeriods();j++){
      if(schedule[i][j]!=-1){
        discipline=schedule[i][j]%in.Wards();
          if(!allocated[i][discipline]){
            allocated[i][discipline]=true;
            finished[i][discipline]=j+in.Duration();
            hosp=schedule[i][j]/in.Wards();
            disc_per_hosp[i][hosp]++;
            AddAssignment(i,schedule[i][j],j);
            if(!found)
            {
              first_period[i]=j;
              found=true;
            }
          }
        last=j;
      }
    }
    last_period[i]=last;
  }
}

bool operator==(const MSS_State& st1, const MSS_State& st2)
{
  return st1.schedule==st2.schedule;
}

ostream& operator<<(ostream& os, const MSS_State& st)
{
unsigned i, j, discipline,hospital;
  os << "Raw State: " << endl;
  for (i = 0; i < st.schedule.size(); i++)
    {
      for (j = 0; j < st.schedule[i].size(); j++)   
        {     
        if (st.schedule[i][j]==-1)
          os<<setw(4) << "-1";
        else
          os << setw(4) << st.schedule[i][j];
        }
      os << endl;
    }
  os << endl;
  os << "Student x pairs: " << endl;
  for (i = 0; i < st.schedule.size(); i++)
    {
      os<<"St n."<<setw(2)<<i;
      for (j = 0; j < st.schedule[i].size(); j++)   
        {     
        if (st.schedule[i][j]==-1)
          os<<setw(3) << "X";
        else
          {
            discipline=st.in.WardsVector(st.schedule[i][j]).GetDiscId();
            hospital=st.schedule[i][j]/st.in.Wards();
            os << setw(3) << "("<<discipline<<", "<<hospital<<")";
          }
        }
      os << endl;
    }
  os << endl;
  os << "Student x hospital: " << endl;
  for (i = 0; i < st.schedule.size(); i++)
    {
      os<<"St n."<<setw(2)<<i;
      for (j = 0; j < st.schedule[i].size(); j++)   
        {     
        if (st.schedule[i][j]==-1)
          os<<setw(3) << "X";
        else
          {
            hospital=st.schedule[i][j]/st.in.Wards();
            os << setw(3) <<hospital;
          }
        }
      os << endl;
    }
  os << endl;
  os<< "Num disc student (per hospital)"<<endl;
  for(i=0;i<st.in.Students();i++){
    os<<"St n."<<i<<" ";
    for(j=0;j<st.in.Hospitals();j++){
      os<<st.GetDiscperHosp(i,j)<<" ";
    }
    os<<endl;
  }
  for(i=0;i<st.in.Students();i++){
    if(st.FreeWardSize(i)>0)
    os<<"Student "<<i<< " free_wards: ";
    for(j=0;j<st.FreeWardSize(i);j++){
        os<<st.FreeWard(i,j)<<" ";
    }
    os<<endl;
  }
  os<<endl;
  for(i=0;i<st.in.Students();i++){
    os<<"Student "<<i<<" first period "<<st.FirstPeriod(i)<<" last period "<<st.LastPeriod(i)<<endl;
  }

  for(i=0;i<st.in.Students();i++)
    {
      os<<"Assignments of "<<i;
      for(j=0;j<st.assignments[i].size();j++)
      {
        os<<" ("<<st.assignments[i][j].first<<","<<st.assignments[i][j].second<<")" <<"  ";
      }
      os<<endl;
    }
  
  os<<"Scores"<<endl;
  os<<"Student"<<setw(2)<<setw(8)<<"DiscSc"<<setw(8)<<"HospSc"
    <<setw(8)<<"ManSc"<<setw(8)<<"WaitSc"<<setw(8)<<"ChngSc"<<endl;
  for(i=0;i<st.in.Students();i++){
    os<<"St"<<setw(2)<<i<<setw(8)<<st.DiscScore(i)<<setw(8)<<st.HospScore(i)
    <<setw(8)<<st.MngrScore(i)<<setw(8)<<st.GetWaitScore(i)<<setw(8)<<st.GetChangeScore(i)<<endl;
  }
  os<<"Worst objective score is "<<st.GetWorstScore()<< " for student "<<st.GetWorstStudent()<<endl;
  os<<"Total Objective score "<<st.GetRealScore()+st.GetWorstScore()<<endl;
  
return os;

}

MSS_Move::MSS_Move()
{
  student = 0;
  from_index = 0;
  new_ward = 0;
  to_period= 0; 
}

bool operator==(const MSS_Move& mv1, const MSS_Move& mv2)
{
  return mv1.student == mv2.student 
    && mv1.from_index==mv2.from_index
    && mv1.new_ward == mv2.new_ward
    && mv1.to_period == mv2.to_period;
}

bool operator!=(const MSS_Move& mv1, const MSS_Move& mv2)
{
  return mv1.student != mv2.student 
    || mv1.from_index != mv2.from_index
    || mv1.new_ward != mv2.new_ward
    || mv1.to_period != mv2.to_period;
}

bool operator<(const MSS_Move& mv1, const MSS_Move& mv2)
{
  return (mv1.student < mv2.student)
    || (mv1.student == mv2.student && mv1.from_index < mv2.from_index)
    || (mv1.student == mv2.student && mv1.from_index == mv2.from_index && mv1.new_ward < mv2.new_ward)
    || (mv1.student == mv2.student && mv1.from_index == mv2.from_index && mv1.new_ward == mv2.new_ward && mv1.to_period < mv2.to_period);
}

istream& operator>>(istream& is, MSS_Move& mv)
{
  char ch; 
  is >> mv.student >> ch >>mv.from_index>>ch>>mv.old_ward>>ch>>mv.from_period>>ch>>mv.new_ward >> ch >> mv.to_period;	
  return is;
}

ostream& operator<<(ostream& os, const MSS_Move& mv)
{
  os <<"< "<< mv.student << "( " <<mv.old_ward<< ", " <<mv.from_period  << ") "<<"("<< mv.new_ward<<" , "<<mv.to_period<< ") >";
  return os;
}

MSS_Swap::MSS_Swap()
{
  student = 0;
  from_index = 0;
  to_index= 1;
}

bool operator==(const MSS_Swap& mv1, const MSS_Swap& mv2)
{
  return mv1.student == mv2.student 
    && mv1.from_index==mv2.from_index
    && mv1.to_index == mv2.to_index;
}

bool operator!=(const MSS_Swap& mv1, const MSS_Swap& mv2)
{
  return mv1.student != mv2.student 
    || mv1.from_index != mv2.from_index
    || mv1.to_index != mv2.to_index;
}

bool operator<(const MSS_Swap& mv1, const MSS_Swap& mv2)
{
  return (mv1.student < mv2.student)
    || (mv1.student == mv2.student && mv1.from_index < mv2.from_index)
    || (mv1.student == mv2.student && mv1.from_index == mv2.from_index && mv1.to_index < mv2.to_index);
}

istream& operator>>(istream& is, MSS_Swap& mv)
{
  char ch; 
  is >> mv.student >> ch >>mv.from_index>>ch>>mv.old_ward>>ch>>mv.from_period>>ch>>mv.to_index>>ch>>mv.new_ward >> ch >> mv.to_period; 
  return is;
}

ostream& operator<<(ostream& os, const MSS_Swap& mv)
{
  os <<"< "<< mv.student << "( " <<mv.from_index<< ", " <<mv.to_index  <<") >";
  return os;
}
