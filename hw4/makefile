 
all: hw4.o lib_dyn.so lib_st.a lib_dyn_rt.so
		gcc -o hw4 hw4.o -L. -ldl -l_dyn -l_st -Wl,-rpath,.
run: 	
		./hw4
hw4.o: hw4.c 
		gcc -c hw4.c -o hw4.o
lib_st.o: lib_st.c
		gcc -c lib_st.c -o lib_st.o
lib_st.a: lib_st.o
		ar rcs lib_st.a lib_st.o
lib_dyn.o: lib_dyn.c
		gcc -c -fPIC lib_dyn.c
lib_dyn.so: lib_dyn.o
		gcc -shared lib_dyn.o -o lib_dyn.so
lib_dyn_rt.o: lib_dyn_rt.c
		gcc -c -fPIC lib_dyn_rt.c
lib_dyn_rt.so: lib_dyn_rt.o
		gcc -shared lib_dyn_rt.o -o lib_dyn_rt.so
clean:
	-rm -f *.a *.so *.o
