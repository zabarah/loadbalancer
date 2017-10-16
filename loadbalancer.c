#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

FILE *sourcefile;
FILE *destinationfile;
int volatile turn_read[3]= {1,1,1};
int volatile turn_write[3]= {1,1,1};
int no_more_incoming = 0;

int filewrite(int *num){
	FILE *destination;
	int len_of_cust_nam = 15;
	char customer[len_of_cust_nam];
	int read_character;
	char filename[22];
	char message[50];
	int number = *num;
	
	sprintf(filename, "server_%d_sessions.txt", number);
	puts(filename); 
	if(!(destination = fopen( filename , "w+"))) printf("didnt open file");
	
	while ( read_character != EOF ) {

		// if the thread already had a chance and others havent it has to wait
		while(!turn_read[number] && (turn_read[(number +1)%3] || turn_read[(number +2)%3])  );

		
		// every one had a chance free the locks	
		if (!turn_read[number] && !turn_read[ (number+1) % 3] && !turn_read [ (number + 2) % 3 ] ) {
			turn_read[number] = 1;
	 		turn_read[ (number + 1) % 3] = 1;
			turn_read[ (number + 2) % 3 ] =1;	
		} 
		turn_read[number] = 0;
			
		if (no_more_incoming){ 
			return 0;}
			

		flockfile(sourcefile); 
		fgets(customer, len_of_cust_nam, sourcefile);
	
		
		printf ("Server %d is reading a session from: %s", number, customer);
		fputs(customer, destination);
		
		do{
	
			read_character=fgetc(sourcefile);
			fputc(read_character, destination);
		}
		while(read_character != '\n' && read_character != EOF);
		
		if(read_character == EOF) no_more_incoming = 1;
		
		funlockfile(sourcefile);
	
	// processing simulation	
		printf ("Server %d is processing request from  %s", number, customer);
		fflush(stdout);	
		sleep(rand()%10);
		

	// writing back to the output buffer
		while(!turn_write[number] && (turn_write[(number +1)%3] || turn_write[(number +2)%3])  );

		turn_write[number] = 0;
	
		if (!turn_write[number] && !turn_write[ (number+1) % 3] && !turn_write [ (number + 2) % 3 ] ) {
			turn_write[number] = 1;
	 		turn_write[ (number+1) % 3] = 1;
			turn_write[ (number + 2) % 3 ] =1;	
		} 
			

		flockfile(destinationfile); 

	
		printf ("Server %d is responding to %s", number, customer);
		sprintf(message,"Server %d response to %s", number, customer);
		fputs(message, destinationfile);  
		int response_time = rand()%10;	
		for (int i; i <= response_time; i++){
	
			fputc('-', destinationfile);
		}
		fputc('\n', destinationfile);
	
		funlockfile(destinationfile);
	
	
	}
	
	fclose(destination);
	return 0;

}








int main(void){
	printf("Hello: Starting Load Balancer...\n");
	char customer_id[14];

	//Creating incoming buffer
	sourcefile = fopen( "incoming_buffer.txt", "w+");
	int number_of_sessions = 10;
	int num_threads = 3;
	for (int j=0; j<number_of_sessions; j++){
		
		sprintf(customer_id,"Customer: %d\n", j);
		fputs(customer_id, sourcefile);
		
		int length_of_session = rand() % 10;
		
		for (int i=0; i<=length_of_session; i++){
			
			fputc('-', sourcefile);

		}
	if (j < (number_of_sessions - 1)){ 
		fputc('\n', sourcefile);}
	}
	rewind(sourcefile);
	
	//Creating outgoing buffer
	destinationfile = fopen("outgoing_buffer.txt","w+");
	
	pthread_t threads[3];
	int id[3]= {0,1,2};
	  
	//Starting threads	
	for ( int i=0; i<num_threads; i++){

			pthread_create(&threads[i], NULL,(void *) &filewrite, &id[i]); 
	}
	//Waiting for threads to end
	for ( int i=0; i<num_threads; i++){

                pthread_join(threads[i], NULL);      
        }
	
	fclose(sourcefile);
	fclose(destinationfile);


	return 0;
}
