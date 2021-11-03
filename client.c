/* PRANJAL GARG
   MT2021099 */


#include <string.h>
#include <sys/socket.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <stdio.h>
#include <stdlib.h> 
	 

int client(int sock);                       //function to handle a new client connection
int user_menu(int sock,int type);           //function to dispaly and handle differet user 
int user_function(int sock,int choice);     //function to handle different functionalities of user
int crud_train(int sock,int choice);        //function to handle crud operations on train details
int crud_user(int sock,int choice);         //function to handle crud operations on user details


/*..............Fucntions....................*/
int client(int sock)
{
    int choice, response;
    printf("\n\tOnline Railway Ticket Booking Systemn\n\n");
	printf("1. Sign In\n");
	printf("2. Sign Up\n");
	printf("3. Exit\n");
	printf("\nEnter Your Choice: ");
    scanf("%d", &choice);
    write(sock, &choice, sizeof(int));
    if(choice == 1) {
        int user_id, type;
        char password[50];
        printf("\nLogin id:");
        scanf("%d", &user_id);
        strcpy(password,getpass("Enter password: "));
        write(sock, &user_id, sizeof(user_id));
		write(sock, &password, sizeof(password));
        read(sock, &response, sizeof(response));
        if(response) {
            printf("\nLogged in successfully\n");
            read(sock, &type, sizeof(type));
            while(user_menu(sock, type) != -1); 
            return 1;
        }
        else {
            printf("Unsuccessfull Login \n");
            return 1;
        }
    }
    else if(choice == 2)
    {
        int type, user_id;
        char name[50],password[50],secret_pin[6];
        system("clear");
        printf("\nEnter The Type Of Account: \n");
		printf("0. Admin\n1. Agent\n2. Customer\n");
		printf("\nYour type of account: ");
        scanf("%d", &type);
		printf("\nEnter Your Name: ");
		scanf("%s", name);
		strcpy(password,getpass("Enter Your Password: "));
        // printf("name = %s\n", name);
        // printf("pass = %s\n", password);
        // printf("type = %d\n", type);
        if(!type) {
            while(1){
				strcpy(secret_pin, getpass("\nEnter secret PIN to create ADMIN account: "));
				if(strcmp(secret_pin, "iiitb")!=0) 					
					printf("\nTry again!!!! incorrects pin.\n");
				else
					break;
			}
        }
        // printf("%d\n", type);
        write(sock, &type, sizeof(type));
		write(sock, &name, sizeof(name));
		write(sock, &password, sizeof(password));
		
		read(sock, &user_id, sizeof(user_id));
		printf("\nYour login id for further logins as: %d\n", user_id);
		return 2;
    }
    return 3;
}

int user_menu(int sock, int type) 
{
    int choice;
    if(type)                // if non admin account
    {
        printf("\nThe operations you can perform.\n");
        printf("1. Book Ticket\n");
		printf("2. View Bookings\n");
		printf("3. Update Booking\n");
		printf("4. Cancel booking\n");
		printf("5. Logout\n\n");
		printf("Enter the operation you want to perform: ");
        scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
        if(choice == 5)
            return -1;
		return user_function(sock,choice);
    }
    else                   // if admin
    {
        printf("\nThe operations you can perform.\n");
        printf("1. Operations on train\n");
		printf("2. Operations on user\n");
		printf("3. Logout\n");
		printf("\nEnter the operation you want to perform: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
        if(choice==1){  // crud for train
            int operation;
			printf("1. Add train\n");
			printf("2. View train\n");
			printf("3. Modify train details\n");
			printf("4. Delete train\n");
			printf("\nEnter your choice: ");
			scanf("%d",&operation);	
			write(sock,&operation,sizeof(operation));
			return crud_train(sock,operation);
		}
		else if(choice==2){  //crud for user
            int operation;
			printf("\n1. Add User\n");
			printf("2. View all users\n");
			printf("3. Modify user\n");
			printf("4. Delete user\n");
			printf("\nEnter the operation you want to perform: ");
			scanf("%d",&operation);
			write(sock,&operation,sizeof(operation));
			return crud_user(sock,operation);			
		}
		else if(choice==3) // logout
			return -1;
    }
}


int user_function(int sock, int choice) 
{
    int response;
    if(choice == 1) // book ticket
    {
        int view = 2, tid, seats;
        write(sock, &view, sizeof(int));
        crud_train(sock, view);
        printf("\nEnter the train number you want to book\n");
        scanf("%d", &tid);
        write(sock, &tid, sizeof(tid));

        printf("\nEnter the number of seats you want to book\n");
        scanf("%d", &seats);
        write(sock, &seats, sizeof(seats));

        read(sock, &response, sizeof(response));
        if(response == 1) 
            printf("\nTicket booked successfully\n");
        else if(response == 2) printf("\nNo train with this train number.\n");
        else if(response == 0) printf("\nSeats are not available.\n");
        return response;
    }
    else if(choice == 2) //view booking
    {
        int no_of_bookings;
		int id,tid,seats;
		read(sock,&no_of_bookings,sizeof(no_of_bookings));
        printf("\nBooking ID \t Train Number \t Seats Booked\n\n");
		while(no_of_bookings--){
			read(sock,&id,sizeof(id));
			read(sock,&tid,sizeof(tid));
			read(sock,&seats,sizeof(seats));
			
			if(seats!=0)
				printf("%d\t\t%d\t\t%d\n",id,tid,seats);
		}
		return response;
    }
    else if(choice == 3) // update booking
    {
        int c = 2, bid, val, response;
        user_function(sock, c);
        printf("\nEnter the booking id you want to update\n");
        scanf("%d",&bid);
		write(sock,&bid,sizeof(bid));

		printf("\n1. Increase number of seats\n2. Decrease number of seats\n");
		printf("\nEnter your choice: ");
        int type;
		scanf("%d",&type);
		write(sock,&type,sizeof(type));

        if(type == 1) 
        {
            printf("\nNo. of tickets to increase");
			scanf("%d",&val);
			write(sock,&val,sizeof(val));
        }
        else if(type == 2)
        {
            printf("\nNo. of tickets to decrease");
			scanf("%d",&val);
			write(sock,&val,sizeof(val));
        }
        read(sock,&response,sizeof(response));
		if(response)
			printf("\nBooking updated successfully.\n");
		else
			printf("\nUpdation failed. No more seats available.\n");
		return response;
    }
    else if(choice == 4) // cancel booking 
    {
        int type = 2,bid,response;
        // printf("In cancel booking\n");
		user_function(sock,type);
		printf("\nEnter the booking id you want to cancel: ");
		scanf("%d",&bid);
		write(sock,&bid,sizeof(bid));
		read(sock,&response,sizeof(response));
		if(response)
			printf("\nBooking cancelled successfully.\n");
		else
			printf("\nCancellation of ticket failed.\n");
		return response;
    }
    else if(choice == 5)
        return -1;
}






int crud_train(int sock,int choice){
	int response = 0;
	if(choice==1){				// Add train
		char tname[50];
        int seats;
		printf("\nEnter the train name: ");
		scanf("%s",tname);
		write(sock, &tname, sizeof(tname));
        printf("Enter the number of seats: ");
        scanf("%d", &seats);
        write(sock,&seats,sizeof(seats));
		read(sock,&response,sizeof(response));	
        // printf("response=%d\n", response);
		if(response)
			printf("\nTrain added successfully\n");

		return response;	
	}
	
	else if(choice==2){			// View train list
		int no_of_trains;
		int tno;
		char tname[50];
		int tseats;
		int aseats, deleted;
		read(sock,&no_of_trains,sizeof(no_of_trains));

        printf("\nTrain Number \t Train Name \t Total Seats \t Available Seats\n\n");
		while(no_of_trains--){
			read(sock,&tno,sizeof(tno));
			read(sock,&tname,sizeof(tname));
			read(sock,&tseats,sizeof(tseats));
			read(sock,&aseats,sizeof(aseats));
            read(sock,&deleted,sizeof(deleted));

            if(!deleted)
				printf("%d\t\t%s\t\t%d\t\t%d\n",tno,tname,tseats,aseats);

        }

		return response;	
	}
	
	else if (choice==3){			// Update train list
		int tseats,choice=2,response=0,tid;
		char tname[50];
		write(sock,&choice,sizeof(int));
		crud_train(sock,choice);
		printf("\nEnter the train number you want to modify: ");
		scanf("%d",&tid);
		write(sock,&tid,sizeof(tid));
		
		printf("\n1. Train Name\n2. Total Seats\n");
		printf("Enter your Choice: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
		
		if(choice==1){ //to update train name
			read(sock,&tname,sizeof(tname));
			printf("\n Current train name: %s",tname);
			printf("\n Updated train name:");
			scanf("%s",tname);
			write(sock,&tname,sizeof(tname));
		}
		else if(choice==2){  // to update seat
			read(sock,&tseats,sizeof(tseats));
			printf("\n Current number of seats: %d\n",tseats);
			printf("\n Updated number of seats:");
			scanf("%d",&tseats);
			write(sock,&tseats,sizeof(tseats));
		}
		read(sock,&response,sizeof(response));
		if(response)
			printf("\nTrain data updated successfully\n");
		return response;
	}

	else if(choice==4){				// Delete a train
		int choice=2,tid,response=0;
		write(sock,&choice,sizeof(int));
		crud_train(sock,choice);
		
		printf("\nEnter the train number you want to delete: ");
		scanf("%d",&tid);
		write(sock,&tid,sizeof(tid));
		read(sock,&response,sizeof(response));
		if(response)
			printf("\nTrain deleted successfully\n");
		return response;
	}
	
}

int crud_user(int sock, int choice)
{
    int response = 0;
    if(choice == 1) //to create account by admin
    {
        int type, user_id;
        char name[50], password[50];
        printf("\n Enter the type of account: \n");
        printf("\n1. Agent\n2. Customer\n");
        printf("\nEnter your type of the account: ");
        scanf("%d", &type);
		printf("\nEnter the user name: ");
        scanf("%s", name);
        strcpy(password, getpass("Enter your password: "));
        write(sock, &type, sizeof(type));
        write(sock, &name, sizeof(name));
        write(sock, &password, sizeof(password));
        read(sock, &response, sizeof(response));
        if(response) {
            read(sock, &user_id, sizeof(user_id));
            printf("\nYour login id for further logins as: %d\n", user_id);
        }
        return response;
    }
    else if(choice == 2) //to view user list by admin
    {
        int no_of_users;
        int user_id, type,deleted;
        char user_name[50];
        read(sock, &no_of_users, sizeof(no_of_users));
		// printf("No of user = %d\n", no_of_users);
        printf("User Id \t User Name \t User Type\n");
		while(no_of_users--){
			read(sock,&user_id,sizeof(user_id));
			read(sock,&user_name,sizeof(user_name));
			read(sock,&type,sizeof(type));
            read(sock,&deleted,sizeof(deleted));
			
            if(!deleted)
				printf("%d\t\t%s\t\t%d\n",user_id,user_name,type);
		}

		return response;	
    }
    else if (choice==3){			// Update a user by admin
		int choice=2,response=0,uid;
		char name[50],pass[50];
		write(sock,&choice,sizeof(int));
		crud_user(sock,choice);
		printf("\nEnter the U_id you want to modify: ");
		scanf("%d",&uid);
		write(sock,&uid,sizeof(uid));
		
		printf("\n1. User Name\n2. Password\n");
		printf("\nEnter your choice: ");
		scanf("%d",&choice);
		write(sock,&choice,sizeof(choice));
		
		if(choice==1){  // to change user name
			read(sock,&name,sizeof(name));
			printf("\n Current user name: %s",name);
			printf("\n Updated user name:");
			scanf("%s",name);
			write(sock,&name,sizeof(name));
			read(sock,&response,sizeof(response));
		}
		else if(choice==2){    // to change user password
			printf("\nEnter current password: ");
			scanf("%s",pass);
			write(sock,&pass,sizeof(pass));
			read(sock,&response,sizeof(response));
			if(response){
				printf("\n Enter new password:");
				scanf("%s",pass);
			}
			else
				printf("\nIncorrect password\n");
			
			write(sock,&pass,sizeof(pass));
		}
		if(response){
			read(sock,&response,sizeof(response));
			if(response)
				printf("\nUser data updated successfully\n");
		}
		return response;
	}

	else if(choice==4){					// Delete a user by admin
		int choice=2,uid,response=0;
		write(sock,&choice,sizeof(int));
		crud_user(sock,choice);
		
		printf("\nEnter the id you want to delete: ");
		scanf("%d",&uid);
		write(sock,&uid,sizeof(uid));
		read(sock,&response,sizeof(response));
		if(response)
			printf("\nUser deleted successfully\n");
		return response;
	}
}


int main()
{
    int client_sock; 
    struct sockaddr_in server; 
     
    client_sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (client_sock == -1) { 
        printf("Could not create socket"); 
    } 
    
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_family = AF_INET; 
    server.sin_port = htons(8080); 
   
    if (connect(client_sock, (struct sockaddr*)&server, sizeof(server)) < 0)
        perror("connect failed. Error"); 
    
	while(client(client_sock)!=3);
    close(client_sock); 
	return 0; 
}