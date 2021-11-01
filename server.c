/* PRANJAL GARG
   MT2021099 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>


  /* ...........Structures used in train reservation system......................*/

struct train{                       
	int train_number;       //to store train number
	char train_name[50];    //to store train name
	int total_seats;        //to store total seats
	int available_seats;    //to store available seats
    int deleted;            //to store wheter train deleted or not
};  
struct user{                   
	int login_id;          //to store login id
	char password[50];     //to store password
	char name[50];         //to store user name
	int type;              //to identify the type of user (0: admin, 1: agent, 2: customer)
    int deleted;           // to store whether user deleted or not
};

struct booking{
	int booking_id;      //to store bookin id
	int type;            //to store type of user  
	int uid;             //to store user id
	int tid;             // to store train number
	int seats;           //seats booked
};


/*..............Fucntions....................*/

void service_client(int client_sock);  // function called on each client coonection
void login(int client_sock);           // function for login service
void signup(int client_sock);          // function for signup service
int menu(int client_sock,int type,int user_id);     //function to handle different type of users 
void crud_train(int client_sock);      // function to handle crud operations on train details
void crud_user(int client_sock);       // function to handle crud operations on user details
int user_function(int client_sock,int choice,int type,int user_id);     //function for different operations done by user

void service_client(int client_sock)
{
    int choice;
    printf("Client Connected\n");
    while(1) {
        read(client_sock, &choice, sizeof(choice));
        // printf("%d\n", choice);
        if(choice == 1)
            login(client_sock);
        else if(choice == 2)
            signup(client_sock);
        else if(choice == 3)
            break;
    }
    close(client_sock);
    printf("Client Closed Successfully\n");
}

void login(int client_sock)
{
    int fd = open("user", O_RDWR);
    if(fd == -1)
    {
        printf("error in login opening file\n");
        return service_client(client_sock);
    }
    int user_id, user_response=0, response=0, type;
    char password[50];
    struct user us;
    read(client_sock, &user_id, sizeof(int));
    read(client_sock, &password, sizeof(password));

    // printf("%d\n", user_id);
    // printf("%s\n", password);


    struct flock fl;
  
    fl.l_start = (user_id-1)*(sizeof(struct user));
   
    fl.l_len = sizeof(struct user);
    fl.l_whence = SEEK_SET;
    fl.l_pid = getpid();
    fl.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &fl);

    // printf("Lock accquried\n");
    // int r = read(fd, &us, sizeof(us));
    // printf("r = %d\n", r);
    while(read(fd,&us,sizeof(us))){
        // printf("user id = %d\n", user_id);
        // printf("user name = %s\n", us.name);
        // printf("user password = %s\n", us.password);
        // printf("user type = %d\n", us.type);
        // printf("us.login id = %d\n", us.login_id);
        if(us.deleted == 1) 
        {
            response = 0;
            break;
        }
		if(us.login_id == user_id){
			user_response=1;
            // printf("user_response = %d\n", user_response);
            // printf("%s\n", password);
            // printf("%s\n", us.name);
			if(!strcmp(us.password,password)){
				response = 1;
				type = us.type;
				break;
			}
			else
            {
				response = 0;
				break;
			}	
		}		
		else
        {
			user_response = 0;
			response=0;
		}
	}
    //unlocking for agent
    if(type!=2){
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);
	}
	
	// if response user, show the menu
	if(user_response)
	{
		write(client_sock,&response,sizeof(response));
		if(response){
			write(client_sock,&type,sizeof(type));
			while(menu(client_sock,type,user_id)!=-1);
            // printf("Will get some menu\n");
		}
	}
	else
		write(client_sock,&response,sizeof(response));
	
	// unlocking normal user
	if(type==2){
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);
	}
    close(fd);
}


void signup(int client_sock) 
{
    int fd = open("user", O_RDWR);
    if(fd == -1)
    {
        printf("Error in opening th file\n");
        return;
    }
    char name[50], password[50];
    int type;
    read(client_sock, &type, sizeof(type));
	read(client_sock, &name, sizeof(name));
	read(client_sock, &password, sizeof(password));

    // printf("name = %s\n", name);
    // printf("pass = %s\n", password);
    // printf("type = %d\n", type);

    int fp = lseek(fd, 0, SEEK_END);

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = fp;
    fl.l_len = 0;
    fl.l_whence = SEEK_SET;
    fl.l_pid = getpid();

    fcntl(fd, F_SETLKW, &fl);
    
    
    struct user us;
    if(fp == 0)    // first user
        us.login_id = 1;
    else {
        fp = lseek(fd, -1*sizeof(struct user), SEEK_END);
        read(fd, &us, sizeof(us));
        us.login_id = us.login_id + 1;
    }    
    us.type = type;
    us.deleted = 0;
    strcpy(us.name, name);
    strcpy(us.password, password);
    // printf("user name = %s\n", us.name);
    // printf("user password = %s\n", us.password);
    // printf("user type = %d\n", us.type);
    // printf("type = %d\n", type);
    // printf("us.login id = %d\n", us.login_id);
    write(fd, &us, sizeof(us));
    write(client_sock, &us.login_id, sizeof(us.login_id));

    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);
    close(fd);
    return;
}


int menu(int client_sock, int type, int user_id) 
{
    int choice, res;
    read(client_sock, &choice, sizeof(choice));
    if(type == 0)   //if user is admin
    {
        if(choice == 1)         //calling for crud on train
        {
            crud_train(client_sock);
            return menu(client_sock, type, user_id);
        }
        else if(choice == 2)    //calling for crud on user
        {
            crud_user(client_sock);
            return menu(client_sock, type, user_id);
        }
        else if(choice == 3)    //logout
            return -1;
    }
    else if(type == 1 || type == 2)     //if user is agent or customer
    {
        res = user_function(client_sock, choice,type, user_id);
        if(res != 5)
            return menu(client_sock, type, user_id);
        else if(res == 5) {
            return -1;
        }
            
    }
}


void crud_train(int client_sock){
	int response=0;	
	int choice;
	read(client_sock,&choice,sizeof(choice));
	if(choice==1){  					// Add train  	
		char tname[50];
		int tid = 0, seats;
		read(client_sock,&tname,sizeof(tname));
        read(client_sock,&seats,sizeof(int));
		struct train tdb,temp;
		struct flock fl;
		int fd_train = open("train", O_RDWR);
        if(fd_train == -1)
        {
            printf("Error in opening th file\n");
            return;
        }
		
		strcpy(tdb.train_name,tname);
		tdb.total_seats = seats;
		tdb.available_seats = seats;

		int fp = lseek(fd_train, 0, SEEK_END); 

		fl.l_type = F_WRLCK;
		fl.l_start = fp;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &fl);

		if(fp == 0){
            tdb.train_number = 0;
		}
		else{
			lseek(fd_train, -1 * sizeof(struct train), SEEK_END);
			read(fd_train, &temp, sizeof(temp));
			tdb.train_number = temp.train_number + 1;	
		}
        response = 1;
        tdb.deleted = 0;
        write(fd_train, &tdb, sizeof(tdb));
        write(client_sock, &response, sizeof(response));
		fl.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fl);
		close(fd_train);
		
	}

	else if(choice==2){					// View train list
        struct flock fl;
		struct train tdb;
		int fd_train = open("train", O_RDONLY);
		if(fd_train == -1)
        {
            printf("Error in opening th file\n");
            return;
        }
		fl.l_type = F_RDLCK;
		fl.l_start = 0;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &fl);
		int fp = lseek(fd_train, 0, SEEK_END);
        int no_of_trains = fp / sizeof(struct train);
        struct train t;
        // printf("no of trains %d\n", no_of_trains);
		write(client_sock, &no_of_trains, sizeof(int));

		lseek(fd_train,0,SEEK_SET);
        while(no_of_trains--){
			read(fd_train,&tdb,sizeof(tdb));
			write(client_sock,&tdb.train_number,sizeof(int));
			write(client_sock,&tdb.train_name,sizeof(tdb.train_name));
			write(client_sock,&tdb.total_seats,sizeof(int));
			write(client_sock,&tdb.available_seats,sizeof(int));
            write(client_sock,&tdb.deleted,sizeof(int));
		}
		response = 1;
		fl.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fl);
		close(fd_train);
	}

	else if(choice==3){					// Update a train
		crud_train(client_sock);
		int choice,response=0,tid;
		struct flock fl;
		struct train tdb;
		int fd_train = open("train", O_RDWR);
        if(fd_train == -1)
        {
            printf("Error in opening th file\n");
            return;
        }
		read(client_sock,&tid,sizeof(tid));

		fl.l_type = F_WRLCK;
		fl.l_start = (tid)*sizeof(struct train);
		fl.l_len = sizeof(struct train);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &fl);

		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));
		
		read(client_sock,&choice,sizeof(int));
		if(choice==1){							// update train name
			write(client_sock,&tdb.train_name,sizeof(tdb.train_name));
			read(client_sock,&tdb.train_name,sizeof(tdb.train_name));
			
		}
		else if(choice==2){						// update total number of seats
			write(client_sock,&tdb.total_seats,sizeof(tdb.total_seats));
            int seats = tdb.total_seats - tdb.available_seats;
			read(client_sock,&tdb.total_seats,sizeof(tdb.total_seats));
            tdb.available_seats = tdb.total_seats - seats;
		}
	
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		response=1;
		write(client_sock,&response,sizeof(response));
		fl.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fl);
		close(fd_train);	
	}

	else if(choice==4){					// Delete a train
		crud_train(client_sock);
		struct flock fl;
		struct train tdb;
		int fd_train = open("train", O_RDWR);
        if(fd_train == -1)
        {
            printf("Error in opening th file\n");
            return;
        }
		int tid,response=0;

		read(client_sock,&tid,sizeof(tid));

		fl.l_type = F_WRLCK;
		fl.l_start = (tid)*sizeof(struct train);
		fl.l_len = sizeof(struct train);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &fl);
		
		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));
        tdb.deleted = 1;
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		response=1;
		write(client_sock,&response,sizeof(response));
		fl.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fl);
		close(fd_train);	
	}	
}


void crud_user(int client_sock){
	int response=0;	
	int choice;
	read(client_sock,&choice,sizeof(choice));
	if(choice==1){    					// Add a user
		char name[50],password[50];
		int type;
		read(client_sock, &type, sizeof(type));
		read(client_sock, &name, sizeof(name));
		read(client_sock, &password, sizeof(password));
		
		struct user udb;
		struct flock lock;
		int fd_user = open("user", O_RDWR);
        if(fd_user == -1)
        {
            printf("Error in opening th file\n");
            return;
        }
		int fp = lseek(fd_user, 0, SEEK_END);
		
		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_user, F_SETLKW, &lock);

		if(fp==0)   //size of the user file is 0.
        { 
			udb.login_id = 1;
		}
		else        //else take the last user
        {
			fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
			read(fd_user, &udb, sizeof(udb));
			udb.login_id++;
		}
        strcpy(udb.name, name);
		strcpy(udb.password, password);
        udb.type=type;
        udb.deleted = 0;
        write(fd_user, &udb, sizeof(udb));
        response = 1;
        write(client_sock,&response,sizeof(int));
		write(client_sock, &udb.login_id, sizeof(udb.login_id));
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
		
	}

	else if(choice==2){					// View user list
		struct flock lock;
		struct user udb;
		int fd_user = open("user", O_RDONLY);
        if(fd_user == -1)
        {
            printf("Error in opening th file\n");
            return;
        }
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &lock);
		int fp = lseek(fd_user, 0, SEEK_END);
		int no_of_users = fp / sizeof(struct user);
		no_of_users--;
		write(client_sock, &no_of_users, sizeof(int));

		lseek(fd_user,0,SEEK_SET);
        no_of_users++;
		while(no_of_users--){
			read(fd_user,&udb,sizeof(udb));
			if(udb.type!=0){
				write(client_sock,&udb.login_id,sizeof(int));
				write(client_sock,&udb.name,sizeof(udb.name));
				write(client_sock,&udb.type,sizeof(int));
                write(client_sock,&udb.deleted,sizeof(int));
			}
		}
		response = 1;
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}

	else if(choice==3){					// Update a user
		crud_user(client_sock);
		int choice,response=0,user_id;
		char password[50];
		struct flock fl;
		struct user udb;
		int fd_user = open("user", O_RDWR);
        if(fd_user == -1)
        {
            printf("Error in opening th file\n");
            return;
        }

		read(client_sock,&user_id,sizeof(user_id));

		fl.l_type = F_WRLCK;
		fl.l_start =  (user_id-1)*sizeof(struct user);
		fl.l_len = sizeof(struct user);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &fl);

		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (user_id-1)*sizeof(struct user), SEEK_CUR);
		read(fd_user, &udb, sizeof(struct user));
		
		read(client_sock,&choice,sizeof(int));
		if(choice==1){					// update user name
			write(client_sock,&udb.name,sizeof(udb.name));
			read(client_sock,&udb.name,sizeof(udb.name));
			response=1;
			write(client_sock,&response,sizeof(response));		
		}
		else if(choice==2){				// update user password
			read(client_sock,&password,sizeof(password));
			if(!strcmp(udb.password,password))
				response = 1;
			write(client_sock,&response,sizeof(response));
			read(client_sock,&udb.password,sizeof(udb.password));
		}
	
		lseek(fd_user, -1*sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		if(response)
			write(client_sock,&response,sizeof(response));
		fl.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &fl);
		close(fd_user);	
	}

	else if(choice==4){				// Delete a user
		crud_user(client_sock);
		struct flock fl;
		struct user user;
		int fd = open("user", O_RDWR);
        if(fd == -1)
        {
            printf("Error in opening th file\n");
            return;
        }
		int user_id,response=0;

		read(client_sock,&user_id,sizeof(user_id));

		fl.l_type = F_WRLCK;
		fl.l_start =  (user_id-1)*sizeof(struct user);
		fl.l_len = sizeof(struct user);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &fl);
		
		lseek(fd, 0, SEEK_SET);
		lseek(fd, (user_id-1)*sizeof(struct user), SEEK_CUR);
		read(fd, &user, sizeof(struct user));
        user.deleted = 1;
		lseek(fd, -1*sizeof(struct user), SEEK_CUR);
		write(fd, &user, sizeof(struct user));
		response=1;
		write(client_sock,&response,sizeof(response));
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);	
	}
}

int user_function(int client_sock, int choice, int type, int user_id){
	int response=0;                     
	if(choice==1){						// to book a ticket
		crud_train(client_sock);           
		struct flock fltrain;
		struct flock flbook;
		struct train tdb;
		struct booking bdb;
		int fd_train = open("train", O_RDWR);
        if(fd_train == -1)
        {
            printf("Error in opening th file\n");
            return response;
        }
		int fd_book = open("booking", O_RDWR);
		if(fd_book == -1)
        {
            printf("Error in opening th file\n");
            return response;
        }
        int tid,seats;
		read(client_sock,&tid,sizeof(tid));		
				
		fltrain.l_type = F_WRLCK;
		fltrain.l_start = tid*sizeof(struct train);
		fltrain.l_len = sizeof(struct train);
		fltrain.l_whence = SEEK_SET;
		fltrain.l_pid = getpid();
		
		flbook.l_type = F_WRLCK;
		flbook.l_start = 0;
		flbook.l_len = 0;
		flbook.l_whence = SEEK_END;
		flbook.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &fltrain);
		lseek(fd_train,tid*sizeof(struct train),SEEK_SET);
		
		read(fd_train,&tdb,sizeof(tdb));
		read(client_sock,&seats,sizeof(seats));
        // printf("%d %d", tdb.train_number, tid);
		if(tdb.train_number==tid)
		{		
			if(tdb.available_seats>=seats){
				response = 1;
				tdb.available_seats -= seats;
				fcntl(fd_book, F_SETLKW, &flbook);
				int fp = lseek(fd_book, 0, SEEK_END);
				// printf("fp=%d\n", fp);
				if(fp == 0) {
                    bdb.booking_id = 0;
                }
                    
				else 
                {
                    lseek(fd_book, -1*sizeof(struct booking), SEEK_CUR);
					read(fd_book, &bdb, sizeof(struct booking));
					bdb.booking_id++;
                }
					

				bdb.type = type;
				bdb.uid = user_id;
				bdb.tid = tid;
				bdb.seats = seats;
				write(fd_book, &bdb, sizeof(struct booking));
			    flbook.l_type = F_UNLCK;
				fcntl(fd_book, F_SETLK, &flbook);
			 	close(fd_book);
			}
		
		    lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		    write(fd_train, &tdb, sizeof(tdb));
		}
        else if(tdb.train_number != tid)
            response = 2;

		fltrain.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fltrain);
		close(fd_train);
		write(client_sock,&response,sizeof(response));
		return response;		
	}
	
	else if(choice==2){				// View bookings
		struct flock fl;
		struct booking bdb;
		int fd_book = open("booking", O_RDONLY);
        if(fd_book == -1)
        {
            printf("Error in opening th file\n");
            return response;
        }
		int no_of_bookings=0;
	
		fl.l_type = F_RDLCK;
		fl.l_start = 0;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &fl);
	
		while(read(fd_book,&bdb,sizeof(bdb))){
			if (bdb.uid==user_id)
				no_of_bookings++;
		}
        // printf("no of books %d\n", no_of_bookings);
		write(client_sock, &no_of_bookings, sizeof(int));
		lseek(fd_book,0,SEEK_SET);

		while(read(fd_book,&bdb,sizeof(bdb))){
			if(bdb.uid==user_id){
				write(client_sock,&bdb.booking_id,sizeof(int));
				write(client_sock,&bdb.tid,sizeof(int));
				write(client_sock,&bdb.seats,sizeof(int));
			}
		}
		fl.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &fl);
		close(fd_book);
		return response;
	}

	else if (choice==3){			// update a booking
		int view = 2,bid,val;
		user_function(client_sock,view,type,user_id);
		struct booking bdb;
		struct train tdb;
		struct flock flbook, fltrain;
		int fd_book = open("booking", O_RDWR);
        if(fd_book == -1)
        {
            printf("Error in opening th file\n");
            return response;
        }
		int fd_train = open("train", O_RDWR);
		if(fd_train == -1)
        {
            printf("Error in opening th file\n");
            return response;
        }
        read(client_sock,&bid,sizeof(bid));

		flbook.l_type = F_WRLCK;
		flbook.l_start = bid*sizeof(struct booking);
		flbook.l_len = sizeof(struct booking);
		flbook.l_whence = SEEK_SET;
		flbook.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &flbook);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&bdb,sizeof(bdb));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		fltrain.l_type = F_WRLCK;
		fltrain.l_start = (bdb.tid)*sizeof(struct train);
		fltrain.l_len = sizeof(struct train);
		fltrain.l_whence = SEEK_SET;
		fltrain.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &fltrain);
		lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&tdb,sizeof(tdb));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		read(client_sock,&choice,sizeof(choice));
	
		if(choice==1){							// increase number of seats required of booking id
			read(client_sock,&val,sizeof(val));
			if(tdb.available_seats>=val){
				response=1;
				tdb.available_seats -= val;
				bdb.seats += val;
			}
		}
		else if(choice==2){						// decrease number of seats required of booking id
			read(client_sock,&val,sizeof(val));
            if(val <= bdb.seats) {
                response = 1;
                tdb.available_seats += val;
                bdb.seats -= val;
            }
		}
		
		write(fd_train,&tdb,sizeof(tdb));
		fltrain.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fltrain);
		close(fd_train);
		
		write(fd_book,&bdb,sizeof(bdb));
		flbook.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &flbook);
		close(fd_book);
		
		write(client_sock,&response,sizeof(response));
		return response;
	}
	else if(choice==4){							// Cancel an entire booking
		int view = 2,bid;
		user_function(client_sock,view,type,user_id);
		struct booking bdb;
		struct train tdb;
		struct flock flbook, fltrain;
		int fd_book = open("booking", O_RDWR);
        if(fd_book== -1)
        {
            printf("Error in opening th file\n");
            return response;
        }
		int fd_train = open("train", O_RDWR);
        if(fd_train == -1)
        {
            printf("Error in opening th file\n");
            return response;
        }
		read(client_sock,&bid,sizeof(bid));

		flbook.l_type = F_WRLCK;
		flbook.l_start = bid*sizeof(struct booking);
		flbook.l_len = sizeof(struct booking);
		flbook.l_whence = SEEK_SET;
		flbook.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &flbook);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&bdb,sizeof(bdb));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		fltrain.l_type = F_WRLCK;
		fltrain.l_start = (bdb.tid)*sizeof(struct train);
		fltrain.l_len = sizeof(struct train);
		fltrain.l_whence = SEEK_SET;
		fltrain.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &fltrain);
		lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&tdb,sizeof(tdb));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		tdb.available_seats += bdb.seats;
		bdb.seats = 0;
		response = 1;

		write(fd_train,&tdb,sizeof(tdb));
		fltrain.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fltrain);
		close(fd_train);
		
		write(fd_book,&bdb,sizeof(bdb));
		flbook.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &flbook);
		close(fd_book);
		
		write(client_sock,&response,sizeof(response));
		return response;
		
	}
	else if(choice==5)	// Logout
		return 5;

}


int main()
{
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1) {
        printf("Could not create socket\n");
        return 0;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8081);

    printf("Binding socket .....");
    int bd = bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    if(bd == -1)
    {
        printf("Could not bind\n");
        return 0;
    }
    listen(server_sock, 100);
    printf("listening ....\n");
    int c = sizeof(struct sockaddr_in); 
    while(1) {
        int nsd;
        nsd = accept(server_sock, (struct sockaddr* )&client, (socklen_t*)&c);
        if(nsd == -1)
        {
            printf("connection error\n");
            return 0;
        }
        if(fork() == 0)
        {
            service_client(nsd);
            exit(1);
        }
    }
    close(server_sock);
    printf("Client connection closed\n");
    return 0;
}