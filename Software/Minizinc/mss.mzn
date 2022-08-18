include "globals.mzn";

%constants
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

%constraints to assign values to aux variables
constraint   forall(s in 1..Students, t in 1..Horizon, d in 1..Disciplines)
      (sched_disc[s,t,d] = sum(h in 1..Hospitals)(schedule[s,t,h,d]));

constraint   forall(s in 1..Students, t in 1..Horizon, h in 1..Hospitals)
      (sched_hosp[s,t,h] = sum(d in 1..Disciplines)(schedule[s,t,h,d]));

constraint   forall(s in 1..Students, t in 1..Horizon)
      (attending[s,t] = sum(d in 1..Disciplines,h in 1..Hospitals)(schedule[s,t,h,d]));

% frequento disc. solo se la frequento per un numero di periodi== durata
constraint   forall(s in 1..Students, d in 1..Disciplines)
      (disc_attending[s,d] = (sum(t in 1..Horizon,h in 1..Hospitals) (schedule[s,t,h,d])) div Duration);  
% channeling constraint tra hosp_attending e sched_hops                 
constraint   forall(s in 1..Students,t in 1..Horizon,h in 1..Hospitals)
      ((sched_hosp[s,t,h]>0 -> hosp_attending[s,t]=h));
% channeling constraint tra schedule e hosp_attending/disc_schedule
constraint   forall(s in 1..Students, t in 1..Horizon, d in 1..Disciplines, h in 1..Hospitals)
      (schedule[s,t,h,d]>0 -> (disc_schedule[s,t]=d /\ hosp_attending[s,t]=h));
% se attending ==0 allora hosp_attending/disc_schedule ==0 
constraint   forall(s in 1..Students, t in 1..Horizon)
      (attending[s,t]=0 -> (hosp_attending[s,t]=0 /\ disc_schedule[s,t]=0));
% disciplina va frequentata in periodi contigui
constraint   forall(s in 1..Students, t in 2..Horizon-Duration+1)
      ((disc_schedule[s,t]>0 /\ disc_schedule[s,t-1]!=disc_schedule[s,t])->(forall(dur in 1..Duration-1) (disc_schedule[s,t+dur]=disc_schedule[s,t])));
% per tutta la durata della disc. devo frequentare stesso osp.
constraint   forall(s in 1..Students, t in 2..Horizon-Duration+1)
      ((hosp_attending[s,t]>0 /\ disc_schedule[s,t-1]!=disc_schedule[s,t])->(forall(dur in 1..Duration-1) (hosp_attending[s,t+dur]=hosp_attending[s,t])));
% primo periodo orizz.: se frequento disc. la "striscia" disc/osp deve rimanere la stessa per tutta durata disc.
constraint   forall(s in 1..Students)
            (disc_schedule[s, 1]>0-> (forall(dur in 1..Duration-1) (disc_schedule[s,1+dur]=disc_schedule[s,1] /\ hosp_attending[s,1+dur]=hosp_attending[s,1]))) ;
% ultimo periodo orizz.: se frequento disc. allora ci dev'essere una "striscia" disc/osp a partire dall'istante Horizon-dur
constraint   forall(s in 1..Students)
            (disc_schedule[s, Horizon]>0-> (forall(dur in 1..Duration-1) (disc_schedule[s,Horizon-dur]=disc_schedule[s,Horizon] /\ hosp_attending[s,Horizon-dur]=hosp_attending[s,Horizon])));
           
%%% AL MASSIMO UNA VOLTA UNA DISCIPLINA PER STUDENTE
%% numero di celle ==1 per disc. non dev'essere > durata disc. 
constraint   forall(s in 1..Students, d in 1..Disciplines)
    (sum(t in 1..Horizon)(sched_disc[s,t,d]) < Duration+1); 

%%% AL MASSIMO UNA DISC ALLA VOLTA PER STUDENTE

constraint   forall(s in 1..Students, t in 1..Horizon)
    (sum(d in 1..Disciplines)(sched_disc[s,t,d]) < 2);
  
%%% AL MASSIMO UN OSPEDALE ALLA VOLTA PER STUDENTE

constraint   forall(s in 1..Students, t in 1..Horizon)
    (sum(h in 1..Hospitals)(sched_hosp[s,t,h]) < 2);  
     
%%% VINCOLO SULLE DISCIPLINE PER STUDENTE,
%%% ORGANIZZATE A GRUPPI

%Numero di disc. frequentate (per gruppo) da studente dev'essere esattamente quanto richiesto
constraint   forall(s in 1..Students, g in 1..Groups)
      (StudDiscGroup[s,g]* Duration= 
       sum(t in 1..Horizon, h in 1..Hospitals, d in 1..Disciplines where DiscGroup[d] = g)
          ( schedule[s,t,h,d]));
          
%Studente può seguire disc. solo se gli/le è permesso
constraint forall (s in 1..Students,d in 1..Disciplines)
         (disc_attending[s,d]>0->AllowedDisc[s,d]>0);   
       
%constraint: Precedenze
constraint   forall(s in 1..Students,d in 1..Disciplines,t in 1..Horizon)(
           ((sched_disc[s,t,d]=1)-> forall(d1 in 1..Disciplines)
                  (Precededby[d,d1]>0 -> sum(i in 1..t)(sched_disc[s,i,d1])>0)));
%Numero disc. seguite in un singolo ospedale non deve eccedere max richiesto
constraint forall (s in 1..Students,h in 1..Hospitals)
           (sum(t in 1..Horizon) (sched_hosp[s,t,h]) div Duration < MaxDiscPerHosp+1);
                   
%%% VINCOLO SULLE DISCIPLINE PER TEMPO, PER OSPEDALE
%constraint: Numero max di studenti richiesti
constraint   forall(h in 1..Hospitals, t in 1..Horizon, d in 1..Disciplines)
               (MaxPosHosp[h,d,t]  >= sum(s in 1..Students)( schedule[s,t,h,d]));
%constraint: Numero min di studenti richiesti
constraint   forall(h in 1..Hospitals, t in 1..Horizon, d in 1..Disciplines)   
                (MinPosHosp[h,d,t]  <= sum(s in 1..Students)( schedule[s,t,h,d]));
%Studente deve essere in grado di seguire disc. in un determinato osp. per poter frequentare disc. in quell'osp. (abilità)
constraint forall (s in 1..Students,h in 1..Hospitals,d in 1..Disciplines)
         (Ability[s,h,d]<1 -> sum(t in 1..Horizon) (schedule[s,t,h,d]) <1);
            
%constraint: disponibilità
constraint  forall(s in 1..Students, t in 1..Horizon)
            (attending[s,t] > 0 -> Availability[s,t] > 0);                   
                    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%     
%%%%%%%%%%%           FUNZIONI DI COSTO    %%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        

% calcolo sat. disc.
constraint  
forall(s in 1..Students)
            (SatDisc[s]=sum(d in 1..Disciplines) 
            (WeightPref[s,1]*PrefStudDisc[s,d]*disc_attending[s,d])); 
% calcolo sat. osp.
constraint  forall(s in 1..Students) 
            (SatHosp[s]=(sum(d in 1..Disciplines,t in 1..Horizon,h in 1..Hospitals) 
            (WeightPref[s,2]*PrefStudHosp[s,h]*schedule[s,t,h,d]) div Duration));
% determino quali sono i periodi in cui lo studente è libero che sono effettivamente "buchi"
constraint   forall(s in 1..Students)(aux_pause[s,Horizon] = attending[s,Horizon]) /\
             forall(s in 1..Students,t in 1..Horizon-1)(
             %%% Max fa or. E' zero solo se non ci sono riprese DOPO
               (aux_pause[s,Horizon-t] = 
                         max(attending[s,Horizon-t],aux_pause[s,Horizon-t+1])));
% calcolo sat. wait
constraint forall(s in 1..Students)
           (SatWait[s]=sum(t in 1..Horizon-1)
           (WeightPref[s,4]*(1-attending[s,t])*aux_pause[s,t]));
           
% Calcolo num. di volte che studente modifica osp.
% aux_chhosp == ospedale frequentato if in quel periodo frequenta osp. else == all'ultimo ospedale frequentato prima dell'istante t-mo.

% aux_chhosp può essere 0 se studente inizia non al tempo t=1.
constraint forall(s in 1..Students)
           (aux_chhosp[s,1]=hosp_attending[s,1]);
% se frequento osp. basta copiare valore
constraint forall(s in 1..Students,t in 2..Horizon)
           (hosp_attending[s,t]>0-> (aux_chhosp[s,t]=hosp_attending[s,t]));
% se non frequento allungo la "striscia" anche su periodi che sono vuoti
constraint forall(s in 1..Students,t in 2..Horizon)
           (hosp_attending[s,t]=0-> (aux_chhosp[s,t]=aux_chhosp[s,t-1]));
% escludiamo celle in cui aux_chhosp=0, perchè corrispondono a buco iniziale
constraint forall(s in 1..Students)
           (SatChn[s]=sum(t in 1..Horizon-1)(aux_chhosp[s,t]!=0 /\ aux_chhosp[s,t]!=aux_chhosp[s,t+1])*WeightPref[s,3]);
% calcolo sat. manager
constraint  forall (s in 1..Students)
            (SatMan[s]=sum(d in 1..Disciplines) 
            (ManPref[d]*disc_attending[s,d])); 
% somma varie sat.
constraint forall(s in 1..Students)
           (SatTot[s]=SatDisc[s]+SatHosp[s]+SatMan[s]+SatChn[s]+SatWait[s]); 
% calcolo peggiore score studente 
constraint w_score=min(s in 1..Students) (SatTot[s]);
% valore effettivamente massimizzato
constraint  
   c =   sum (s in 1..Students) (SatTot[s]) + w_score;   
                                   
solve maximize c;

output
["schedule=["]++
[show(schedule[s,t,h,d])++","| s in 1..Students,t in 1..Horizon, h in 1..Hospitals, d in 1..Disciplines]++["]\n"]++
["{cost: "++show(c)++"}"]


