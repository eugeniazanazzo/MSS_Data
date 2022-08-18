// File MSS_Data.cc
#include "MSS_Data.hh"
#include <fstream>
#include <stdexcept>



ostream& operator<<(ostream& os, const Student& c)
{
  os << c.discipline_w <<" "<<c.hospital_w<<" "<<c.change_w<<" "<<c.wait_w;
  return os;
}

istream& operator>>(istream& is,Student& c)
{
  string junk;
  is >> junk >>junk>>junk>>c.discipline_w>>c.hospital_w
     >> c.change_w>>c.wait_w>>junk;
  return is;
}


ostream& operator<<(ostream& os, const Ward& c)
{
  os << c.id << " " << c.discipline_id<<c.hospital_id;
  return os;
}

MSS_Input::MSS_Input(string file_name)
{
  const unsigned MAX_DIM = 10000;
  int i,j,k,h;
  unsigned jump=3;
  char ch;
  string str;
  ifstream is(file_name);
    if(!is)
    {
    cerr << "Cannot open input file " <<  file_name << endl;
    exit(1);
    }
  is>>ch;
  if(ch=='/'){
  //READ Original format
  is.ignore(MAX_DIM,'\n');
  is>>students>>disciplines>>hospitals>>time_periods>>str>>wards>>groups>>str;
  total_wards=wards*hospitals; 
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  is>>str>>str>>str>>str>>str>>str>>str>>str>>max_disc;
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  manager_preferences.resize(disciplines);
  unsigned max_manager_pr=0;
  for(i=0;i<disciplines;i++){
      is>>manager_preferences[i];
      if(manager_preferences[i]>max_manager_pr)
        max_manager_pr=manager_preferences[i];
  }
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  bool value;
  disciplines_vect.resize(disciplines);
  discipline_presence.resize(groups,vector<bool>(disciplines,false));
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
    for(j=0;j<groups;j++){
      for(k=0;k<disciplines;k++)
      {
      is>>value;
      discipline_presence[j][k]=value;
      if (discipline_presence[j][k])
      disciplines_vect[k].SetGroupId(j);
      }
    }
  hospitals_vect.resize(hospitals);
  wards_vect.resize(hospitals*wards);
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  for (i=0;i<hospitals;i++){
    is.ignore(MAX_DIM,'H');
    is.ignore(MAX_DIM,'\n');
    for (j=0;j<wards;j++){
      wards_vect[(i*wards)+j].SetId((i*wards)+j);
      wards_vect[(i*wards)+j].SetHospId(i);
      for(k=0;k<disciplines;k++){
        is>>value;
        if(value){
        wards_vect[(i*wards)+j].SetDiscId(k);
        disciplines_vect[k].AddIsAvailableAt((i*wards)+j);
        }
      }
    }
  }
  unsigned num;
  is.ignore(MAX_DIM,'\n');
  is.ignore(MAX_DIM,'\n');
  unsigned places_avail=0;
  for (i=0;i<hospitals;i++){
    hospitals_vect[i].InitMaxDemand(time_periods,wards);
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
    for (j=0;j<time_periods;j++){
      for(k=0;k<wards;k++){
        is>>num;
        hospitals_vect[i].SetMaxDemand(j,k,num);
        places_avail+=num;
      }
    }
  }
  unsigned slots_needed=0;
  for (i=0;i<hospitals;i++){
    hospitals_vect[i].InitMinDemand(time_periods,wards);
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
    for (j=0;j<time_periods;j++){
      for(k=0;k<wards;k++){
        is>>num;
        hospitals_vect[i].SetMinDemand(j,k,num);
        slots_needed+=num;
      }
    }
  }
  for (i=0;i<hospitals;i++){ 
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
  }
  for (i=0;i<hospitals;i++){ 
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
  }
  for(i=0;i<jump;i++){ 
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
  }
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  is>>duration;
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');

  unsigned total_count=0,counter;
  for(i=0;i<disciplines;i++){
    counter=0;
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
    disciplines_vect[i].InitDiscRequired(disciplines);
    for(k=0;k<disciplines;k++){
      is>>num;
      disciplines_vect[i].SetDiscRequired(k,num);
      is.ignore(MAX_DIM,'\n');
      if(num>0)
        {
          counter++;
          disciplines_vect[i].AddDependency(k);
          disciplines_vect[k].AddRequiredby(i);
        }
    }
    disciplines_vect[i].SetDependentOn(counter);
  }
  vector<int>top_sort;
  vector<int>height;
  top_sort.resize(disciplines,-1);
  height.resize(disciplines,-1);
  counter=0;
  for(i=0;i<disciplines;i++) {
    if(disciplines_vect[i].GetDependentOn()==0){
      top_sort[counter]=i;
      height[i]=0;
      counter++;
    }
  }
  unsigned current;
  int max_height=0;
  for(i=0;i<disciplines;i++){
    current=top_sort[i];
    for(k=0;k<DisciplinesVector(current).SizeRequiredby();k++){
      unsigned begger=DisciplinesVector(current).GetRequiredby(k);
      disciplines_vect[begger].SetDependentOn(disciplines_vect[begger].GetDependentOn()-1);
      if(disciplines_vect[begger].GetDependentOn()==0){
        top_sort[counter]=begger;
        height[begger]=height[current]+1;
        if(height[begger]>max_height){
          max_height=height[begger];
        }
        counter++;
      }
    }
  }
  depth=max_height;
  for(i=0;i<disciplines;i++){
    for(k=0;k<DisciplinesVector(i).SizeDependency();k++){
      unsigned required=DisciplinesVector(i).GetDependency(k);
      for(j=0;j<DisciplinesVector(required).SizeDependency();j++){
        if(!disciplines_vect[i].GetDiscRequired(DisciplinesVector(required).GetDependency(j))){
          disciplines_vect[i].AddDependency(DisciplinesVector(required).GetDependency(j));
          disciplines_vect[DisciplinesVector(required).GetDependency(j)].AddRequiredby(i);
          disciplines_vect[i].SetDiscRequired(DisciplinesVector(required).GetDependency(j),true);
          disciplines_vect[i].SetDependentOn(disciplines_vect[i].GetDependentOn()+1);
        }
      }
    }
    total_count+=DisciplinesVector(i).SizeDependency();
  }
  density=float(total_count)/float(disciplines*(disciplines-1)/2);
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  students_vect.resize(students);
  for (i=0;i<students;i++){
    is>>students_vect[i];
  }
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  for (i=0;i<students;i++){
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
    students_vect[i].InitDisciplineList(groups,disciplines);
    students_vect[i].InitCompDiscList(groups);
    for(j=0;j<groups;j++){
      for(h=0;h<disciplines;h++){
        is>>num;
        students_vect[i].SetDisciplineList(j,h,num);
        if(num>0)
        {
          students_vect[i].SetCompDiscList(j,h);
        }
      }
    }
  }
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  to_fullfill.resize(students,vector<unsigned>(groups));
  assignments=0;
  assignments_index.resize(students,0);
  unsigned tot;
  float partial=0;
  bool first=true;
  for(i=0;i<students;i++){
    tot=0;
    for(j=0;j<groups;j++){
      is>>to_fullfill[i][j];
      tot+=to_fullfill[i][j];

    }
    assignments+=tot;
    students_vect[i].SetNumDisc(tot);
    partial+=float(tot)/float(disciplines);
    if(first)
    {
      assignments_index[i]=tot-1;
      first=false;
    }
    else
      assignments_index[i]=assignments_index[i-1]+tot;
  }
  occupancy=float(assignments*duration)/float(places_avail);
  excess=float(slots_needed)/float(assignments*duration);
  busyness=float(assignments*duration)/float(time_periods*students);
  mean_to_fullfill=partial/float(students);
  mean_to_fullfill=float(assignments)/float(disciplines*students);
  is.ignore(MAX_DIM,'-');
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  for(i=0;i<students;i++){
    students_vect[i].InitAbility(hospitals,disciplines);
    is.ignore(MAX_DIM,'/');
    is.ignore(MAX_DIM,'\n');
    for(j=0;j<hospitals;j++){
      for(h=0;h<disciplines;h++){
        is>>value;
        students_vect[i].SetAbility(j,h,value);
      }
    }
  }
  is.ignore(MAX_DIM,'-');
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  discipline_preferences.resize(students,vector<unsigned>(disciplines));
  unsigned max_val;
  for(i=0;i<students;i++){
    max_val=0;
    for(j=0;j<disciplines;j++){
      is>>discipline_preferences[i][j];
      if(discipline_preferences[i][j]>max_val)
        max_val=discipline_preferences[i][j];
    }
    students_vect[i].SetDiscPr(max_val);
  }
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  hospital_preferences.resize(students,vector<unsigned>(hospitals));
  for(i=0;i<students;i++){
    max_val=0;
    for(j=0;j<hospitals;j++){
      is>>hospital_preferences[i][j];
      if(hospital_preferences[i][j]>max_val)
        max_val=hospital_preferences[i][j];
    }
    students_vect[i].SetHospPr(max_val);
  }
  is.ignore(MAX_DIM,'/');
  is.ignore(MAX_DIM,'\n');
  availability.resize(students,vector<bool>(time_periods));
  for(i=0;i<students;i++){
    for(j=0;j<time_periods;j++){
      is>>value;
      availability[i][j]=value;
    }
  }
  unsigned score,max_hosp_pr,max_disc_pr,hosp_w,disc_w;
  for(i=0;i<students;i++)
  {
    score=0;
    max_hosp_pr=students_vect[i].GetHospPr();
    max_disc_pr=students_vect[i].GetDiscPr();
    hosp_w=students_vect[i].GetHospitalW();
    disc_w=students_vect[i].GetDisciplineW();
    score=max_hosp_pr*hosp_w*students_vect[i].GetNumDisc();
    students_vect[i].SetPerfectHospScore(score);
    score=max_disc_pr*disc_w*students_vect[i].GetNumDisc(); 
    students_vect[i].SetPerfectDiscScore(score);
  }
  max_desire_man=max_manager_pr*disciplines;
}
//READ Minizinc
else {
  string str;
  is.ignore(MAX_DIM,'=');
  is>>students;
  is.ignore(MAX_DIM,'=');
  is>>disciplines;
  wards=disciplines;
  is.ignore(MAX_DIM,'=');
  is>>hospitals;
  total_wards=disciplines*hospitals;
  is.ignore(MAX_DIM,'=');
  is>>duration;
  is.ignore(MAX_DIM,'=');
  is>>time_periods;
  is.ignore(MAX_DIM,'=');
  is>>groups;
  is.ignore(MAX_DIM,'=');
  is>>max_disc;
  unsigned value;
  disciplines_vect.resize(disciplines);
  discipline_presence.resize(groups,vector<bool>(disciplines,false));
  is.ignore(MAX_DIM,'=');
  is.ignore(MAX_DIM,'[');
  for(i=0;i<disciplines;i++){
    is>>value;
    if(i<disciplines-1)
      is.ignore(MAX_DIM,',');
    else
      is.ignore(MAX_DIM,'|');
    disciplines_vect[i].SetGroupId(value-1);
    for(j=0;j<groups;j++){
      discipline_presence[j][i]=(value-1==j)? true:false;
    }
  }
  wards_vect.resize(total_wards);
  for (i=0;i<hospitals;i++){
    for (j=0;j<disciplines;j++){
      wards_vect[(i*disciplines)+j].SetId((i*disciplines)+j);
      wards_vect[(i*disciplines)+j].SetHospId(i);
      wards_vect[(i*disciplines)+j].SetDiscId(j);
      disciplines_vect[j].AddIsAvailableAt((i*disciplines)+j);
    }
  }
  students_vect.resize(students);
  unsigned g;
  to_fullfill.resize(students,vector<unsigned>(groups));
  assignments=0;
  assignments_index.resize(students,0);
  unsigned tot;
  float partial=0;
  bool first=true;
  for(i=0;i<students;i++){
    tot=0;
    for(j=0;j<groups;j++){
      is>>to_fullfill[i][j];
      tot+=to_fullfill[i][j];
      if(j<groups-1)
        is.ignore(MAX_DIM,',');
      else
        is.ignore(MAX_DIM,'|');
    }
    assignments+=tot;
    students_vect[i].SetNumDisc(tot);
    partial+=float(tot)/float(disciplines);
    if(first)
    {
      assignments_index[i]=tot-1;
      first=false;
    }
    else
      assignments_index[i]=assignments_index[i-1]+tot;
  }
  is.ignore(MAX_DIM,';'); 
  is.ignore(MAX_DIM,'|');
  for(i=0;i<students;i++){
      students_vect[i].InitDisciplineList(groups,disciplines);
      students_vect[i].InitCompDiscList(groups);
      for(h=0;h<disciplines;h++){
          is>>value;
          if(h<disciplines-1)
            is.ignore(MAX_DIM,',');
          else
            is.ignore(MAX_DIM,'|');
          g=disciplines_vect[h].GetGroupId();
          students_vect[i].SetDisciplineList(g,h,value);
          if(value>0){
            students_vect[i].SetCompDiscList(g,h);
          }
      }
  }
  is.ignore(MAX_DIM,'=');
  unsigned total_count=0,counter;
  is.ignore(MAX_DIM,'|');
  for(i=0;i<disciplines;i++){
    counter=0;
    disciplines_vect[i].InitDiscRequired(disciplines);
    for(k=0;k<disciplines;k++){
      is>>value;
      if(k<disciplines-1)
            is.ignore(MAX_DIM,',');
          else
            is.ignore(MAX_DIM,'|');
      disciplines_vect[i].SetDiscRequired(k,value);
      if(value>0)
        {
          counter++;
          disciplines_vect[i].AddDependency(k);
          disciplines_vect[k].AddRequiredby(i);
        }
    }
    disciplines_vect[i].SetDependentOn(counter);
    total_count+=counter;
  }
  vector<int>top_sort;
  vector<int>height;
  top_sort.resize(disciplines,-1);
  height.resize(disciplines,-1);
  counter=0;
  for(i=0;i<disciplines;i++) {
    if(disciplines_vect[i].GetDependentOn()==0){
      top_sort[counter]=i;
      height[i]=0;
      counter++;
    }
  }
  unsigned current;
  int max_height=0;
  for(i=0;i<disciplines;i++){
    current=top_sort[i];
    for(k=0;k<DisciplinesVector(current).SizeRequiredby();k++){
      unsigned begger=DisciplinesVector(current).GetRequiredby(k);
      disciplines_vect[begger].SetDependentOn(disciplines_vect[begger].GetDependentOn()-1);
      if(disciplines_vect[begger].GetDependentOn()==0){
        top_sort[counter]=begger;
        height[begger]=height[current]+1;
        if(height[begger]>max_height){
          max_height=height[begger];
        }
        counter++;
      }
    }
  }
  depth=max_height;
  density=float(total_count)/float(disciplines*(disciplines-1)/2);
  for(i=0;i<disciplines;i++){
    for(k=0;k<DisciplinesVector(i).SizeDependency();k++){
      unsigned required=DisciplinesVector(i).GetDependency(k);
      for(j=0;j<DisciplinesVector(required).SizeDependency();j++){
        if(!disciplines_vect[i].GetDiscRequired(DisciplinesVector(required).GetDependency(j))){
          disciplines_vect[i].AddDependency(DisciplinesVector(required).GetDependency(j));
          disciplines_vect[DisciplinesVector(required).GetDependency(j)].AddRequiredby(i);
          disciplines_vect[i].SetDiscRequired(DisciplinesVector(required).GetDependency(j),true);
        }
      }
    }
  }
  is.ignore(MAX_DIM,';');
  is.ignore(MAX_DIM,'|');
  availability.resize(students,vector<bool>(time_periods));
  for(i=0;i<students;i++){
    for(j=0;j<time_periods;j++){
      is>>value;
      availability[i][j]=value;
      if(j<time_periods-1)
        is.ignore(MAX_DIM,',');
      else
        is.ignore(MAX_DIM,'|');
    }
  }
  is.ignore(MAX_DIM,'(');
  is.ignore(MAX_DIM,'[');
  for(i=0;i<students;i++){
    students_vect[i].InitAbility(hospitals,disciplines);
    for(h=0;h<hospitals;h++){
      for(j=0;j<disciplines;j++){
        is>>value;
        if(i< students-1 || h< hospitals-1 || j<disciplines-1)
          is.ignore(MAX_DIM,',');
        else
          is.ignore(MAX_DIM,';');
        students_vect[i].SetAbility(h,j,value);
      }
    }
  }
  hospitals_vect.resize(hospitals);
  is.ignore(MAX_DIM,'[');
  
  unsigned places_avail=0;
  for(i=0;i<hospitals;i++){
    hospitals_vect[i].InitMaxDemand(time_periods,disciplines);
    for(j=0;j<disciplines;j++){
      for(h=0;h<time_periods;h++){
        is>>value;
        if(i< hospitals-1 || j< disciplines-1 || h<time_periods-1)
          is.ignore(MAX_DIM,',');
        else
          is.ignore(MAX_DIM,'[');
        hospitals_vect[i].SetMaxDemand(h,j,value);
        places_avail+=value;
      }
    }
  }
  unsigned slots_needed=0;
  for(i=0;i<hospitals;i++){
    hospitals_vect[i].InitMinDemand(time_periods,disciplines);
    for(j=0;j<disciplines;j++){
      for(h=0;h<time_periods;h++){
        is>>value;
        if(i< hospitals-1 || j< disciplines-1 || h<time_periods-1)
          is.ignore(MAX_DIM,',');
        else
          is.ignore(MAX_DIM,'|');
        hospitals_vect[i].SetMinDemand(h,j,value); 
        slots_needed+=value;
      }
    }
  }
  unsigned value1;
  int value2,value3;
  for(i=0;i<students;i++){
    is>>value;
    is.ignore(MAX_DIM,',');
    is>>value1;
    is.ignore(MAX_DIM,',');
    is>>value2;
    is.ignore(MAX_DIM,',');
    is>>value3;
    students_vect[i].SetDisciplineW(value);
    students_vect[i].SetHospitalW(value1);
    students_vect[i].SetChangeW(value2);
    students_vect[i].SetWaitW(value3);
    is.ignore(MAX_DIM,'|');
  }
  is.ignore(MAX_DIM,'|');
  discipline_preferences.resize(students,vector<unsigned>(disciplines));
  unsigned max_val;
  for(i=0;i<students;i++){
    max_val=0;
    for(j=0;j<disciplines;j++){
      is>>discipline_preferences[i][j];
      if(j<disciplines-1)
        is.ignore(MAX_DIM,',');
      else
        is.ignore(MAX_DIM,'|');
      if(discipline_preferences[i][j]>max_val)
        max_val=discipline_preferences[i][j];
    }
    students_vect[i].SetDiscPr(max_val);
  }
  is.ignore(MAX_DIM,'|');
  hospital_preferences.resize(students,vector<unsigned>(hospitals));
  for(i=0;i<students;i++){
    max_val=0;
    for(j=0;j<hospitals;j++){
      is>>hospital_preferences[i][j];
      if(j<hospitals-1)
        is.ignore(MAX_DIM,',');
      else
        is.ignore(MAX_DIM,'|');
      if(hospital_preferences[i][j]>max_val)
        max_val=hospital_preferences[i][j];
    }
    students_vect[i].SetHospPr(max_val);
  }
  unsigned max_manager_pr=0;
  manager_preferences.resize(disciplines);
  is.ignore(MAX_DIM,'=');
  is.ignore(MAX_DIM,'[');
  for(i=0;i<disciplines;i++){
    is>>manager_preferences[i];
    if(manager_preferences[i]>max_manager_pr)
        max_manager_pr=manager_preferences[i];
    is.ignore(MAX_DIM,','); 
  }
  unsigned score,max_hosp_pr,max_disc_pr,hosp_w,disc_w;
  for(i=0;i<students;i++)
  {
    score=0;
    max_hosp_pr=students_vect[i].GetHospPr();
    max_disc_pr=students_vect[i].GetDiscPr();
    hosp_w=students_vect[i].GetHospitalW();
    disc_w=students_vect[i].GetDisciplineW();
    score=max_hosp_pr*hosp_w*students_vect[i].GetNumDisc();
    students_vect[i].SetPerfectHospScore(score);
    score=max_disc_pr*disc_w*students_vect[i].GetNumDisc();
    students_vect[i].SetPerfectDiscScore(score);
  }
  max_desire_man=max_manager_pr*disciplines;

  occupancy=float(assignments*duration)/float(places_avail);
  excess=float(slots_needed)/float(assignments*duration);
  busyness=float(assignments*duration)/float(time_periods*students);
  mean_to_fullfill=partial/float(students);
  mean_to_fullfill=float(assignments)/float(disciplines*students);
  }
}

unsigned MSS_Input::GetStfromAI(unsigned index) const
{
  unsigned i;
  for(i=0;i<students;i++){
    if(index<=assignments_index[i])
      return i;
  }
  throw std::invalid_argument("I have the wrong index");
}

ostream& operator<<(ostream& os, const MSS_Input& pa)
{
  unsigned i,j,k;
  string separator;
  os<<"%Scalars"<<endl;
  os<<"Students="<<pa.students<<";"<<endl;
  os<<"Disciplines="<<pa.disciplines<<";"<<endl;
  os<<"Hospitals="<<pa.hospitals<<";"<<endl;
  os<<"Duration="<<pa.duration<<";"<<endl;
  os<<"Horizon="<<pa.time_periods<<";"<<endl;
  os<<"Groups="<<pa.groups<<";"<<endl;
  os<<"MaxDiscPerHosp="<<pa.max_disc<<";"<<endl;
  os<<"%Data structures"<<endl;
  os<<"DiscGroup = [";
  for(i=0;i<pa.disciplines;i++){
     separator=(i<(pa.disciplines-1))? ",":"];\n";
     os<<pa.disciplines_vect[i].GetGroupId()+1<<separator;
  }
  os<<endl;
  os<<"StudDiscGroup =[|\n";
  for(i=0;i<pa.students;i++){
    for(j=0;j<pa.groups;j++){
      separator=(j!=(pa.groups-1))?", ":(i==pa.students-1)?" |];\n":" |\n";
      os<<pa.to_fullfill[i][j]<<separator;
    }
  }
  os<<endl;
  unsigned group;
  os<<"AllowedDisc =[|\n";
  for (i=0;i<pa.students;i++){
    for(j=0;j<pa.disciplines;j++){
      group=pa.disciplines_vect[j].GetGroupId();
      separator=(j!=pa.disciplines-1)?", ":(i==pa.students-1)?" |];\n":" |\n";
      os<<pa.students_vect[i].GetDisciplineList(group,j)<<separator;
    }
  }
  os<<endl;
  os<<"Precededby =[|\n";
  for(i=0;i<pa.disciplines;i++){
    for(j=0;j<pa.disciplines;j++){
      separator=(j!=pa.disciplines-1)?", ":(i==pa.disciplines-1)?" |];\n":" |\n";
      os<<pa.disciplines_vect[i].GetDiscRequired(j)<<separator;
    }
  }
  os<<endl;
  os<<"Availability =[|\n";
  for(i=0;i<pa.students;i++){ 
    for(j=0;j<pa.time_periods;j++){
        {
        separator=(j!=pa.time_periods-1)?", ":(i==pa.students-1)?" |];\n":" |\n";
        os<<pa.availability[i][j]<<separator;
        }
    }
  }
  os<<endl;
  os<<"Ability =\n array3d(1..Students,1..Hospitals,1..Disciplines,[\n";
  for(i=0;i<pa.students;i++){  
    for(j=0;j<pa.hospitals;j++){
      for(k=0;k<pa.disciplines;k++){
        separator=(k!=pa.disciplines-1)?", ":(i==pa.students-1 && j==pa.hospitals-1)? "]);\n":" ,\n";
        os<<pa.students_vect[i].GetAbility(j,k)<<separator;
      }
     }
   }
  os<<endl;
  os<<"MaxPosHosp =\n array3d(1..Hospitals, 1..Disciplines, 1..Horizon,[\n";
   for(i=0;i<pa.hospitals;i++){  
      for(k=0;k<pa.disciplines;k++){
        for(j=0;j<pa.time_periods;j++){
        separator=(j!=pa.time_periods-1)?", ":(i==pa.hospitals-1 && k==pa.disciplines-1)? "]);\n":" ,\n";
        os<<pa.hospitals_vect[i].GetMaxDemand(j,k)<<separator;
      }
     }
    }
  os<<endl;
  os<<"MinPosHosp =\n array3d(1..Hospitals, 1..Disciplines, 1..Horizon,[\n";
  for(i=0;i<pa.hospitals;i++){  
    for(k=0;k<pa.disciplines;k++){
      for(j=0;j<pa.time_periods;j++){
        separator=(j!=pa.time_periods-1)?", ":(i==pa.hospitals-1 && k==pa.disciplines-1)? "]);\n":" ,\n";
        os<<pa.hospitals_vect[i].GetMinDemand(j,k)<<separator;
      }
     }
  }
  os<<endl;
  os<<"%W_disc W_hosp W_change W_wait"<<endl;
  os<<"WeightPref="<<"[|\n";
  for(i=0;i<pa.students;i++){
    separator=(i<pa.students-1)?" |\n":" |];\n";
    os<<pa.students_vect[i].GetDisciplineW()<<","<<pa.students_vect[i].GetHospitalW()<<","<<pa.students_vect[i].GetChangeW()<<","<<pa.students_vect[i].GetWaitW()<<separator;
  }
  os<<endl;
  os<<"PrefStudDisc =[|\n";
  for(i=0;i<pa.students;i++){
    for(j=0;j<pa.disciplines;j++){
      separator=(j!=pa.disciplines-1)?", ":(i==pa.students-1)?" |];\n":" |\n";
      os<<pa.discipline_preferences[i][j]<<separator;
    }
  }
  os<<endl;
  os<<"PrefStudHosp =[|\n";
  for(i=0;i<pa.students;i++){
    for(j=0;j<pa.hospitals;j++){
      separator=(j!=pa.hospitals-1)?", ":(i==pa.students-1)?" |];\n":" |\n";
      os<<pa.hospital_preferences[i][j]<<separator;
    }
  }
  os<<endl;
  os<<"ManPref=[";
  for(i=0;i<pa.disciplines;i++)
  {
    separator=(i<(pa.disciplines-1))? ",":"];\n";
    os<<pa.manager_preferences[i]<<separator;
  } 
  os<<endl;
  os<<"%% Instance features:"<<endl;
  os<<"%Scalars :"<<pa.students <<" , "<<pa.disciplines <<" , "<<pa.duration <<" , "<<pa.time_periods <<" , "<<pa.hospitals <<" , "<<float(pa.max_disc)/float(pa.disciplines)<<endl;
  os<<"%Occupancy: "<<pa.occupancy<<endl;
  os<<"%Excess: "<<pa.excess<<endl;
  os<<"%Busyness: "<<pa.busyness<<endl;
  os<<"%Density: "<<pa.density<<endl;
  os<<"%Depth: "<<pa.depth<<endl;
  return os;
}

MSS_Output::MSS_Output(const MSS_Input& my_in)
  : in(my_in),mat(in.Students(), vector<int>(in.TimePeriods(),-1))
{

}

void MSS_Output::Reset()
{
  unsigned s, t;
  for (s = 0; s < in.Students(); s++)
    for (t = 0; t < in.TimePeriods(); t++) 
      mat[s][t] = -1;
  score = -1;
  worst_score = -1;
}

MSS_Output& MSS_Output::operator=(const MSS_Output& out)	
{
  mat=out.mat;
  score=out.score;
  worst_score=out.worst_score;	
  return *this;
}

ostream& operator<<(ostream& os, const MSS_Output& out)
{ 
 unsigned i, j, r, d;
 os<<"% \n schedule= array4d(1..Students,1..Horizon, 1..Hospitals,1..Disciplines,[";
 for(i=0;i<out.in.Students();i++){
  for(j=0;j<out.in.TimePeriods();j++){
    for(r=0;r<out.in.Hospitals();r++){
      for (d=0;d<out.in.Disciplines();d++){
        if(out.mat[i][j]==(r*out.in.Disciplines())+d)
          os<<"1";
        else os<<"0";
        if(i!=out.in.Students()-1 ||d!=out.in.Disciplines()-1 || j!=out.in.TimePeriods()-1 || r!=out.in.Hospitals()-1)
          os<<",";
        else
          os<<"]);"<<endl;
      }
    }
  }
 }

  os<<" %The real score reached is "<<out.score<<endl;
  os<<" %The worst score reached is "<<out.worst_score<<endl;
  os<<" %The score maximizes is "<<out.score+out.worst_score<<endl;
  return os; 
}

istream& operator>>(istream& is, MSS_Output& out)
{
  unsigned i, j;
  int student,period,discipline,hospital,duration,ward; 
  const unsigned MAX_DIM = 1000;
  string trash;
  char ch;
  for (i = 0; i < out.mat.size(); i++)
  {
    for (j = 0; j < out.mat[i].size(); j++)
      out.mat[i][j] = -1;
  }
  is>>ch;
  if(ch=='S')
  {
    for(i=0;i<out.in.Assignments()*out.in.Duration();i++){
      is>>trash>>student>>trash>>trash>>trash>>ward;
      is.ignore(MAX_DIM,'d');
      is.ignore(MAX_DIM,'d');
      is>>period;
      out.mat[student][period]=ward;
    }
  }
  else if(ch=='%')
  {
    bool value=false;
    is.ignore(MAX_DIM,'[');
    for (i=0;i<out.in.Students();i++){
      for(j=0;j<out.in.TimePeriods();j++){
        for(unsigned k=0;k<out.in.Hospitals();k++){
          for(unsigned l=0;l<out.in.Disciplines();l++){
            is>>value;
            if(value>0)
              out.mat[i][j]=(k*out.in.Disciplines())+l;
            is.ignore(MAX_DIM,',');
          }
        }
      }
    }
  }
  else{
    is.ignore(MAX_DIM,'\n');
    for(i=0;i<out.in.Assignments();i++){
        is>>trash>>ch>>student>>ch>>trash>>ch>>discipline>>ch>>period>>ch>>duration>>ch>>hospital;
        ward=(hospital*out.in.Wards()+discipline);
        for(j=0;j<duration;j++)
        {
          out.mat[student][period+j]=ward;
        }
        is.ignore(MAX_DIM,'\n');
      }
  }
  return is;
}
