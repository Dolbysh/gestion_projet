all : tests_mapping 

free_sectors.o : free_sectors.c
	gcc -c free_sectors.c

tests_mapping.o : tests_mapping.c 
	gcc -c tests_mapping.c 

tests_mapping : tests_mapping.o free_sectors.o
	gcc -o tests_mapping tests_mapping.o free_sectors.o
	
clean : 
	rm *.o
