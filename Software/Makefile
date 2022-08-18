EASYLOCAL = ../easylocal-3
EL_INCLUDE = -I$(EASYLOCAL)/include
EL_LINK = -lboost_program_options -pthread

FLAGS = -std=c++14 -Wno-sign-compare -Wall -Wfatal-errors -Wno-ignored-attributes -O3 # -g

HAVE_CPLEX = mac#linux#no
# linux
ifeq ($(HAVE_CPLEX), linux)

	CPLEX = /export/local/share/ILOG/cplex
	CPLEX_LIB = $(CPLEX)/lib/x86-64_linux/static_pic
	CONCERT = /export/local/share/ILOG/concert
	CONCERT_LIB = $(CONCERT)/lib/x86-64_linux/static_pic
else ifeq ($(HAVE_CPLEX), mac)
	CPLEX = /Applications/CPLEX_Studio201/cplex
	CPLEX_LIB = $(CPLEX)/lib/x86-64_osx/static_pic
	CONCERT = /Applications/CPLEX_Studio201/concert
	CONCERT_LIB = $(CONCERT)/lib/x86-64_osx/static_pic
endif
ifneq ($(HAVE_CPLEX), no)
	MIP_INCLUDE = -I$(CPLEX)/include -I$(CONCERT)/include -DCPLEX
	MIP_LINK = -L$(CPLEX_LIB) -L$(CONCERT_LIB) -lconcert -lilocplex -lcplex -ldl
endif

SOURCE_FILES = MSS_Data.cc MSS_Basics.cc MSS_Helpers.cc  MSS_MIPSolver.cc MSS_Main.cc 
OBJECT_FILES = MSS_Data.o MSS_Basics.o MSS_Helpers.o MSS_MIPSolver.o MSS_Main.o
HEADER_FILES = MSS_Data.hh MSS_Basics.hh MSS_MIPSolver.hh MSS_Helpers.hh  

mss: $(OBJECT_FILES)
	g++ $(OBJECT_FILES) $(EL_LINK) $(MIP_LINK) -o mss

MSS_Data.o: MSS_Data.cc MSS_Data.hh
	g++ -c $(FLAGS) MSS_Data.cc

MSS_Basics.o: MSS_Basics.cc MSS_Basics.hh MSS_Data.hh
	g++ -c $(FLAGS) $(EL_INCLUDE) MSS_Basics.cc

MSS_Helpers.o: MSS_Helpers.cc MSS_Helpers.hh MSS_Basics.hh MSS_Data.hh
	g++ -c $(FLAGS) $(EL_INCLUDE) MSS_Helpers.cc

MSS_MIPSolver.o: MSS_MIPSolver.cc MSS_MIPSolver.hh MSS_Data.hh MSS_Basics.hh MSS_Helpers.hh
	g++ -c $(FLAGS) $(EL_INCLUDE) $(MIP_INCLUDE) MSS_MIPSolver.cc

MSS_Main.o: MSS_Main.cc MSS_Helpers.hh MSS_Basics.hh MSS_Data.hh
	g++ -c $(FLAGS) $(EL_INCLUDE) $(MIP_INCLUDE) MSS_Main.cc

clean:
	rm -f $(OBJECT_FILES) mss 

