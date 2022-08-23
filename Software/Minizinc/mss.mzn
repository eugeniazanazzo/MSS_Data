include "globals.mzn";

%%% constants
int: Students;
int: Horizon;
int: Hospitals;
int: Duration;
int: Disciplines;
int: Groups;
int: MaxDiscPerHosp;

%% matrices (input order)
array [1..Disciplines] of 1..Groups: DiscGroup; 
array [1..Students,1..Groups] of 0..Disciplines: StudDiscGroup; 
array [1..Students,1..Disciplines] of 0..1: AllowedDisc;
array [1..Disciplines,1..Disciplines] of 0..1: Precededby;
array [1..Students,1..Horizon] of 0..1: Availability;
array [1..Students,1..Hospitals,1..Disciplines] of 0..1: Ability;
%%%% min/max number of students per hosp.
array [1..Hospitals, 1..Disciplines, 1..Horizon] of int: MaxPosHosp; 
array [1..Hospitals, 1..Disciplines, 1..Horizon] of int: MinPosHosp; 

%%% preferences weights/values:
array [1..Students,1..4] of int: WeightPref;
array [1..Students,1..Disciplines] of int: PrefStudDisc;
array [1..Students,1..Hospitals] of int: PrefStudHosp;
array [1..Disciplines] of 0..5: ManPref;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%     
%%%%%%%%%%%           DECISION VARIABLE    %%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      
 
array [1..Students,1..Horizon,1..Hospitals,1..Disciplines] of var 0..1: schedule;

%%% AUX VARIABLES
array [1..Students,1..Horizon,1..Disciplines] of var 0..1: sched_disc;
array [1..Students,1..Horizon,1..Hospitals] of var 0..1: sched_hosp;
array [1..Students,1..Horizon] of var 0..1: attending;
array [1..Students,1..Disciplines] of var 0..1: disc_attending;
array [1..Students,1..Horizon] of var 0..Hospitals: hosp_attending;
array [1..Students,1..Horizon] of var 0..Disciplines: disc_schedule;
array [1..Students,1..Horizon] of var 0..1: aux_pause;
array [1..Students,1..Horizon] of var 0..Hospitals: aux_chhosp;

%%% COST VARIABLES

array [1..Students] of var int: SatDisc; % Satisfaction student relative to disciplines in his/her schedule 
array [1..Students] of var int: SatHosp; % Satisfaction student relative to hospitals in his/her schedule 
array [1..Students] of var int: SatWait; % Satisfaction student relative to waiting times in his/her schedule 
array [1..Students] of var int: SatMan;  % Satisfaction manager relative to the disciplines a student attends in his/her schedule 
array [1..Students] of var int: SatChn;  % Satisfaction student relative to changes of hospital in his/her schedule 
array [1..Students] of var int: SatTot;  % Sum of satisfactions above

var int: w_score; % worst score
var int: c; % value maximized

%%% Constraints to assign values to aux variables
constraint   forall(s in 1..Students, t in 1..Horizon, d in 1..Disciplines)
      (sched_disc[s,t,d] = sum(h in 1..Hospitals)(schedule[s,t,h,d]));

constraint   forall(s in 1..Students, t in 1..Horizon, h in 1..Hospitals)
      (sched_hosp[s,t,h] = sum(d in 1..Disciplines)(schedule[s,t,h,d]));

constraint   forall(s in 1..Students, t in 1..Horizon)
      (attending[s,t] = sum(d in 1..Disciplines,h in 1..Hospitals)(schedule[s,t,h,d]));

%%% Disc is attended if it fills n. of slots in the schedule equal to the duration
constraint   forall(s in 1..Students, d in 1..Disciplines)
      (disc_attending[s,d] = (sum(t in 1..Horizon,h in 1..Hospitals) (schedule[s,t,h,d])) div Duration);  
%%% Channeling constraint between hosp_attending and sched_hops                 
constraint   forall(s in 1..Students,t in 1..Horizon,h in 1..Hospitals)
      ((sched_hosp[s,t,h]>0 -> hosp_attending[s,t]=h));
%%% Channeling constraint between schedule and hosp_attending/disc_schedule
constraint   forall(s in 1..Students, t in 1..Horizon, d in 1..Disciplines, h in 1..Hospitals)
      (schedule[s,t,h,d]>0 -> (disc_schedule[s,t]=d /\ hosp_attending[s,t]=h));
%%% If attending ==0 then hosp_attending/disc_schedule ==0 
constraint   forall(s in 1..Students, t in 1..Horizon)
      (attending[s,t]=0 -> (hosp_attending[s,t]=0 /\ disc_schedule[s,t]=0));
%%% Disc. needs to be attended in consecutive periods
constraint   forall(s in 1..Students, t in 2..Horizon-Duration+1)
      ((disc_schedule[s,t]>0 /\ disc_schedule[s,t-1]!=disc_schedule[s,t])->(forall(dur in 1..Duration-1) (disc_schedule[s,t+dur]=disc_schedule[s,t])));
%%% For all duration of disc. the hospital has to stay the same
constraint   forall(s in 1..Students, t in 2..Horizon-Duration+1)
      ((hosp_attending[s,t]>0 /\ disc_schedule[s,t-1]!=disc_schedule[s,t])->(forall(dur in 1..Duration-1) (hosp_attending[s,t+dur]=hosp_attending[s,t])));
%%% As above: takes care of first period in the schedule
constraint   forall(s in 1..Students)
            (disc_schedule[s, 1]>0-> (forall(dur in 1..Duration-1) (disc_schedule[s,1+dur]=disc_schedule[s,1] /\ hosp_attending[s,1+dur]=hosp_attending[s,1]))) ;
%%% As above: takes care of the last period in the schedule
constraint   forall(s in 1..Students)
            (disc_schedule[s, Horizon]>0-> (forall(dur in 1..Duration-1) (disc_schedule[s,Horizon-dur]=disc_schedule[s,Horizon] /\ hosp_attending[s,Horizon-dur]=hosp_attending[s,Horizon])));
           
%%% Discipline can be attended only once (i.e. num of slots with said disc. <= duration) 
constraint   forall(s in 1..Students, d in 1..Disciplines)
    (sum(t in 1..Horizon)(sched_disc[s,t,d]) < Duration+1); 

%%% Max one discipline at the time for each student

constraint   forall(s in 1..Students, t in 1..Horizon)
    (sum(d in 1..Disciplines)(sched_disc[s,t,d]) < 2);
  
%%% Max one hospital at the time for each student

constraint   forall(s in 1..Students, t in 1..Horizon)
    (sum(h in 1..Hospitals)(sched_hosp[s,t,h]) < 2);  
     
%%% Number of disc. attended (per group) by eachs student has to be the one required
constraint   forall(s in 1..Students, g in 1..Groups)
      (StudDiscGroup[s,g]* Duration= 
       sum(t in 1..Horizon, h in 1..Hospitals, d in 1..Disciplines where DiscGroup[d] = g)
          ( schedule[s,t,h,d]));
          
%%% Student can attend disc. only if he/she is allowed to
constraint forall (s in 1..Students,d in 1..Disciplines)
         (disc_attending[s,d]>0->AllowedDisc[s,d]>0);   
       
%%% Precedences between disciplines
constraint   forall(s in 1..Students,d in 1..Disciplines,t in 1..Horizon)(
           ((sched_disc[s,t,d]=1)-> forall(d1 in 1..Disciplines)
                  (Precededby[d,d1]>0 -> sum(i in 1..t)(sched_disc[s,i,d1])>0)));
%%% Number of disc. attended in a single hospital cannot exceed the max required
constraint forall (s in 1..Students,h in 1..Hospitals)
           (sum(t in 1..Horizon) (sched_hosp[s,t,h]) div Duration < MaxDiscPerHosp+1);
                   
%%% Max number of students allowed on a ward in a given period
constraint   forall(h in 1..Hospitals, t in 1..Horizon, d in 1..Disciplines)
               (MaxPosHosp[h,d,t]  >= sum(s in 1..Students)( schedule[s,t,h,d]));
%%% Min number of students required by a ward in a given period
constraint   forall(h in 1..Hospitals, t in 1..Horizon, d in 1..Disciplines)   
                (MinPosHosp[h,d,t]  <= sum(s in 1..Students)( schedule[s,t,h,d]));
%%% Student must possess the ability to work in a given ward 
constraint forall (s in 1..Students,h in 1..Hospitals,d in 1..Disciplines)
         (Ability[s,h,d]<1 -> sum(t in 1..Horizon) (schedule[s,t,h,d]) <1);
            
%%% Student has to be available to work in given period
constraint  forall(s in 1..Students, t in 1..Horizon)
            (attending[s,t] > 0 -> Availability[s,t] > 0);                   
                    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%     
%%%%%%%%%%%           COST FUNCTIONS       %%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        

%%% Desire in terms of disciplines is computed
constraint  
forall(s in 1..Students)
            (SatDisc[s]=sum(d in 1..Disciplines) 
            (WeightPref[s,1]*PrefStudDisc[s,d]*disc_attending[s,d])); 
%%% Desire in terms of hospitals is computed
constraint  forall(s in 1..Students) 
            (SatHosp[s]=(sum(d in 1..Disciplines,t in 1..Horizon,h in 1..Hospitals) 
            (WeightPref[s,2]*PrefStudHosp[s,h]*schedule[s,t,h,d]) div Duration));
%%% Determines which free periods are to be considered waiting times and which don't.
constraint   forall(s in 1..Students)(aux_pause[s,Horizon] = attending[s,Horizon]) /\
             forall(s in 1..Students,t in 1..Horizon-1)(
             %%% Max works as an OR. It's 0 only if all the following slots are empty
               (aux_pause[s,Horizon-t] = 
                         max(attending[s,Horizon-t],aux_pause[s,Horizon-t+1])));
%%% Desire in terms of waiting times in the schedule computed
constraint forall(s in 1..Students)
           (SatWait[s]=sum(t in 1..Horizon-1)
           (WeightPref[s,4]*(1-attending[s,t])*aux_pause[s,t]));
           
%%% Computes the number of times a student changes hospitals in his/her schedule
%%% Following three constraints determine the value of aux_chhosp.
%%% aux_chhosp == if attending ==>hospital attended else == last hospital attended before the t-th period.

%%% Aux_chhosp is 0 if students does not start at time t=1.
constraint forall(s in 1..Students)
           (aux_chhosp[s,1]=hosp_attending[s,1]);
%%% If attending value is simply copied
constraint forall(s in 1..Students,t in 2..Horizon)
           (hosp_attending[s,t]>0-> (aux_chhosp[s,t]=hosp_attending[s,t]));
%%% If not attending: value is copied from the slot immediatly before the one in t
constraint forall(s in 1..Students,t in 2..Horizon)
           (hosp_attending[s,t]=0-> (aux_chhosp[s,t]=aux_chhosp[s,t-1]));
%%% Desire in terms of changes of hospitals in the schedule computed
constraint forall(s in 1..Students)
           (SatChn[s]=sum(t in 1..Horizon-1)(aux_chhosp[s,t]!=0 /\ aux_chhosp[s,t]!=aux_chhosp[s,t+1])*WeightPref[s,3]);
%%% Desire of the medical school in terms of disciplines followed by a student
constraint  forall (s in 1..Students)
            (SatMan[s]=sum(d in 1..Disciplines) 
            (ManPref[d]*disc_attending[s,d])); 
%%% Sum of all desires computed above (per student)
constraint forall(s in 1..Students)
           (SatTot[s]=SatDisc[s]+SatHosp[s]+SatMan[s]+SatChn[s]+SatWait[s]); 
%%% Finds the worst score 
constraint w_score=min(s in 1..Students) (SatTot[s]);
%%% Computes the value to be maximized
constraint  
   c =   sum (s in 1..Students) (SatTot[s]) + w_score;   
                                   
solve maximize c;

output
["schedule=["]++
[show(schedule[s,t,h,d])++","| s in 1..Students,t in 1..Horizon, h in 1..Hospitals, d in 1..Disciplines]++["]\n"]++
["{cost: "++show(c)++"}"]


