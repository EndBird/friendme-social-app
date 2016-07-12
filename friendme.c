/*
 * friendme.c
 *
 *  Created on: Apr 6, 2016
 *      Author: HuynhsComputer
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "friends.h"
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>

#ifndef PORT
#define PORT 52701
#endif


#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 12
#define DELIM " \r\n"


int listenfd;
char * name_list[FD_SETSIZE]; //keep list of names
char * contents;

/*
 * Print a formatted error message to stderr.
 */
void error(char *msg) {
    fprintf(stderr, "Error: %s\r\n", msg);
}

/*
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 */
char * process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr, char * name) {
    User *user_list = *user_list_ptr;
    char * response;
    if (cmd_argc <= 0) {
        return NULL;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return "quit";
    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        char* buf = list_users(user_list);
        
        return buf;
        
    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 2) {
        switch (make_friends(cmd_argv[1], name, user_list)) {
            case 0:
                
                response = "You are now friends.\r\n";
                return response;
            case 1:
                response  = "You are already friends\r\n";
                return response;
            case 2:
                response = "At least one of you entered has the max number of friends\r\n";
                return response;
            case 3:
                response = "You can't friend yourself\r\n";
                return response;
            case 4:
                response = "The user you entered does not exist\r\n";
                return response;
                
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 2; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }
        
        // allocate the space
        contents = malloc(space_needed);
        if (contents == NULL) {
            perror("malloc");
            exit(1);
        }
        
        // copy in the bits to make a single string
        
        strcpy(contents, cmd_argv[2]);
        for (int i = 3; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }
        
        User *author = find_user(name, user_list);
        User *target = find_user(cmd_argv[1], user_list);
        
        switch (make_post(author, target, contents)) {
            case 0:
                response = "";
                
                return response;
            case 1:
                
                response = "You can only post to your friends\r\n";
                return response;
            case 2:
                
                response = "The user you want to post to does not exist\r\n";
                return response;
        }
        
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        response = print_user(user);
        return response;
        
    } else {
        response = "Incorrect syntax\r\n";
        return response;
    }
    return "";
    
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!");
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }
    
    return cmd_argc;
}

void handler(int code) {
    int i;
    printf("freeing up name_list\r\n");
    for (i=0;i<FD_SETSIZE;i++) {
        free(name_list[i]);
    }
    free(contents);
    exit(0);
}


int main() {
    
    
    // Create the heads of the empty data structure
    User *user_list = NULL;
    
    int mysocket;
    char buf[INPUT_BUFFER_SIZE];
    int readval;
    fd_set readfds;
    
    int select_res;
    int arg_check=0;
    int clients_fd[FD_SETSIZE]; //fd of the different clients
    int clients_fd_arg_check[FD_SETSIZE] ; //check if client just entered
    int breakcontrol = 0; //escape while loop
    char * name; //users name
    char response1[FD_SETSIZE];
    // Create socket
    bindandlisten();
    memset(clients_fd, 0, sizeof(int)*FD_SETSIZE);
    memset(clients_fd_arg_check, 0, sizeof(int)*FD_SETSIZE);
    memset(response1, '\0', sizeof(char)*FD_SETSIZE);
    
    //begin setting time and sigaction
    struct sigaction newact;
    newact.sa_handler = handler;
    newact.sa_flags = 0;
    sigemptyset(&newact.sa_mask);
    sigaction(SIGINT, &newact, NULL);
    struct itimerval timer ;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {perror("setitimer");}
    
    //finish setting time and sigaction
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(listenfd, &readfds);
        
        int i;
        for (i=0;i<FD_SETSIZE;i++) {
            if (clients_fd[i]>0) {
                FD_SET(clients_fd[i], &readfds);  //check everytime the while loop restarts elimate the disconnected clients.
            }
            
            
        }
        select_res = select(FD_SETSIZE, &readfds, NULL,NULL,NULL);
        if (select_res<0) {
            perror("select");
            exit(-1);
        }
        if (FD_ISSET(listenfd, &readfds)) { //this means theres a new connection.
            mysocket = accept(listenfd, (struct sockaddr *) 0, 0);
            if (mysocket == -1) {
                perror("accept");
                exit(-1);}
            if (write(mysocket, "What is your user name?\r\n", sizeof("What is your user name?\r\n"))==-1)
            {
                perror("write");
                exit(-1);
            }
            for (i=0;i<FD_SETSIZE;i++) {
                if (clients_fd[i] == 0) {
                    clients_fd[i] = mysocket;
                    clients_fd_arg_check[i] = 0;
                    break;
                }
                
            }
            
        }
        
        
        else { //this means that theres a socket input.
            for (i=0;i<FD_SETSIZE;i++) {
                if (FD_ISSET(clients_fd[i], &readfds)) {
                    arg_check = clients_fd_arg_check[i];
                    mysocket = clients_fd[i];
                    break;
                }
            }
            memset(buf, '\0', sizeof(buf));
            int m = 0;
            int n = 0;
            
            if (arg_check == 0) { //this means that the user has just connected so we ask for his name, the
                //elseif following this, arg_check ==1 will be skipped and after this statement finishes, it restarts with the while
                //loop to ensure that it never stops at a read or accept statement.
                
                //see if this when client puts args for name
                for (i=0;i<FD_SETSIZE;i++) {
                    if (mysocket == clients_fd[i]) {
                        clients_fd_arg_check[i] = 1;
                    }
                }
                
                while ((readval = read(mysocket, buf+m, sizeof(buf)-m)) > 0) {
                    n = strlen(buf+m);
                    if (find_network_newline(buf+m, n) == -1) {
                        m = m + n; //set it to the index that has had a value set by read yet
                    }
                    else if(strcmp(buf, "\r\n") != 0){
                        if (arg_check==0) {
                            
                            
                            buf[strlen(buf)-2] = '\0'; //get rid of newline
                            
                            name = malloc(sizeof(char)*MAX_NAME);
                            strcpy(name, strtok(buf, DELIM));
                            create_user(name, &user_list);
                            for (i=0;i<FD_SETSIZE;i++) {
                                if (clients_fd[i] == mysocket) {
                                    name_list[i] = name;
                                }
                            }
                            
                            if (write(mysocket, "Welcome.\r\n", sizeof("Welcome.\r\n"))==-1)
                    	       {
                                   perror("write");
                                   exit(-1);
                               }
                            if (write(mysocket, "Go ahead and enter user commands>\r\n", sizeof("Go ahead and enter user commands>\r\n"))==-1)
                    	       {
                                   perror("write");
                                   exit(-1);
                               }
                            memset(buf, '\0', sizeof(buf)); //the refresh is so that the buf can be new again for
                            //new input values.
                            
                            m = 0;
                            n = 0;
                            breakcontrol = 1;
                            
                        }
                        
                    }
                    else {breakcontrol = 1;}
                    if (breakcontrol == 1) {breakcontrol = 0;  break;}
                }
                if ((readval) < 0)  {
                    perror("read");
                    exit(-1);
                }
                else if (readval == 0) {
                    printf("connection ended\r\n");
                    for (i=0;i<FD_SETSIZE;i++){
                        if (mysocket == clients_fd[i]) {
                            clients_fd[i] = 0;
                            clients_fd_arg_check[i] = 0;}
                        close(mysocket);
                    }}
                
                
            }
            
            if (arg_check == 1) { //this means that the user has already connected.
                for (i=0;i<FD_SETSIZE;i++) {
                    if (clients_fd[i] == mysocket) {
                        name = name_list[i];
                    }
                }
                while ((readval = read(mysocket, buf+m, sizeof(buf)-m)) > 0) {
                    n = strlen(buf+m);
                    if (find_network_newline(buf+m, n) == -1) {
                        m = m + n;
                    }
                    else if (strcmp(buf, "\r\n")!=0) {char *cmd_argv[INPUT_ARG_MAX_NUM];
                        buf[strlen(buf)-1] = '\0';
                        
                        int cmd_arg = tokenize(buf, cmd_argv);
                        char * response = process_args(cmd_arg, cmd_argv, &user_list, name);
                        if (strcmp(response, "quit") == 0)
                        {readval = 0; break;} //send it straigh to the disconnect
                        //instructions of the program
                        
                        if (write(mysocket, response, sizeof(char)*strlen(response)) ==-1) {
                            perror("write");
                            exit(-1);
                        }
                        if (strcmp(response, "You are now friends.\r\n") == 0)
                        {   int d;
                            for (d=0;d<FD_SETSIZE;d++){
                                if (strcmp(name_list[d],cmd_argv[1]) ==0) {
                                    strcpy(response1, "You have been befriended by ");
                                    strcat(response1, name);
                                    strcat(response1, ".\r\n");
                                    if (write(clients_fd[d], response1, sizeof(char)*strlen(response1)) ==-1) {
                                        perror("write");
                                        exit(-1);
                                    }
                                    break;
                                    memset(response1, '\0', sizeof(char)*FD_SETSIZE);
                                }
                            }
                        }
                        else if (strcmp(cmd_argv[0], "post") == 0 && strcmp(response, "You can only post to your friends\r\n") !=0
                                 && strcmp(response, "The user you want to post to does not exist\r\n")
                                 && already_friends(*find_user(name, user_list), *find_user(cmd_argv[1], user_list)) == 1)
                            //^ to check that it is a post to a friend.
                        {   int d;
                            for (d=0;d<FD_SETSIZE;d++){
                                if (strcmp(name_list[d],cmd_argv[1]) ==0) {
                                    strcpy(response1, "From ");
                                    strcat(response1, name);
                                    strcat(response1, ": ");
                                    int arg_ind;
                                    for (arg_ind=2; arg_ind<cmd_arg;arg_ind++) {
                                        strcat(response1, cmd_argv[arg_ind]);
                                        strcat(response1, " ");
                                    } //^to fully get the str mssg to send
                                    strcat(response1, "\r\n");
                                    if (write(clients_fd[d], response1, sizeof(char)*strlen(response1)) ==-1) {
                                        perror("write");
                                        exit(-1);
                                    }
                                    break;
                                    memset(response1, '\0', sizeof(char)*FD_SETSIZE);
                                }
                            }
                        }
                        
                        memset(buf, '\0', sizeof(buf));
                        m = 0;
                        n = 0;
                        breakcontrol = 1;
                        
                        
                    }
                    else {breakcontrol=1;}
                    
                    if (breakcontrol == 1){breakcontrol = 0; break;} //break so that
                    //the while loop restarts
                    
                }
                
                if ((readval) < 0)  {
                    perror("read");
                    exit(-1);
                }
                else if (readval == 0) {
                    printf("connection ended\r\n");
                    for (i=0;i<FD_SETSIZE;i++){
                        if (mysocket == clients_fd[i]) {
                            clients_fd[i] = 0;
                            clients_fd_arg_check[i] = 0;
                        }
                        close(mysocket);
                    }}
            }
        }}
    
    int on = 1;  //take out listen fd so because the program has been killed.
    int status;
    status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on));
    if(status == -1) {
        perror("setsockopt -- REUSEADDR");
    }
    
    
    
    return 0;
}


int find_network_newline(const char *buf, int inbuf) {
    // Step 1: write this function
    
    int i;
    for (i = 0; i<inbuf; i++){
        if(buf[i]=='\r' && buf[i+1] == '\n'){
            return i;
        }
    }
    return -1; // return the location of '\r' if found
}

void bindandlisten()  /* bind and listen, abort on error */ //taken from sample server
{
    struct sockaddr_in r;
    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);
    
    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
        perror("bind");
        exit(1);
    }
    
    if (listen(listenfd, 5)) {
        perror("listen");
        exit(1);
    }
}

