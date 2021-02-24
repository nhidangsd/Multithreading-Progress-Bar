
wordcount: wordcount.c
	gcc -o wordcount wordcount.c -lpthread

clean: 
	rm -f *.o ${PROGRAM}
