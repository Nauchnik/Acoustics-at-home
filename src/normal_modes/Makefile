CPP = g++
ALGLIBDIR = ../../../alglib
SPECTRADIR = ../../../Eigenvalues
CPPFLAGS = -O3 -std=c++17 -static

normal_modes: main.o libnormal_modes.a
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} main.o libnormal_modes.a -o normal_modes

main.o: main.cpp
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} main.cpp -c
	
normal_modes.o: normal_modes.cpp
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} -I${SPECTRADIR} normal_modes.cpp -c
	
specialfunctions.o: ${ALGLIBDIR}/specialfunctions.cpp 
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} ${ALGLIBDIR}/specialfunctions.cpp -c

linalg.o: ${ALGLIBDIR}/linalg.cpp 
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} ${ALGLIBDIR}/linalg.cpp -c

ap.o: ${ALGLIBDIR}/ap.cpp 
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} ${ALGLIBDIR}/ap.cpp -c

alglibmisc.o: ${ALGLIBDIR}/alglibmisc.cpp 
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} ${ALGLIBDIR}/alglibmisc.cpp -c

alglibinternal.o: ${ALGLIBDIR}/alglibinternal.cpp 
	${CPP} ${CPPFLAGS} -I${ALGLIBDIR} ${ALGLIBDIR}/alglibinternal.cpp -c

libnormal_modes.a: alglibinternal.o alglibmisc.o ap.o linalg.o specialfunctions.o normal_modes.o
	ar rcs libnormal_modes.a alglibinternal.o alglibmisc.o ap.o linalg.o specialfunctions.o normal_modes.o

clean:
	rm -rf *.o
	rm normal_modes
	rm libnormal_modes.a
	clear
