
#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Create a new user with the given name.  Insert it at the tail of the list
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int create_user(const char *name, User **user_ptr_add) {


    User *new_user = malloc(sizeof(User));
    if (new_user == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_user->name, name, MAX_NAME); // name has max length MAX_NAME - 1

    for (int i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL && strcmp(curr->name, name) != 0) {
        prev = curr;
        curr = curr->next; printf("friends line43\n");
    }

    if (*user_ptr_add == NULL) {       // bug fixed 03/04/2016. Now correct on repeat of 1st name
        *user_ptr_add = new_user; printf("friends line47\n");
        return 0;
    } else if (curr != NULL) {
        free(new_user);
        return 1;
    } else {
        prev->next = new_user; printf("friends line53\n");
        return 0;
    }
}


/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 */
User *find_user(const char *name, const User *head) {
    /*    const User *curr = head;
     while (curr != NULL && strcmp(name, curr->name) != 0) {
     curr = curr->next;
     }

     return (User *)curr;
     */
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next; printf("line 75 friends \n");
    }
    printf("line 77 friends %s\n", print_user(head));
    return (User *)head;
}


/*
 * Return a string of the the usernames of all users in the list starting at curr.
 * Names should be returned, one per line.
 */
char * list_users(const User *curr) {
    const char * curr_name;
    int str_size = 0;
    const User * curr1 = curr;
    if (curr == NULL) {
        return NULL;
    }
    while (curr1 != NULL) {
        curr_name = curr1->name;
        str_size = str_size + strlen(curr_name) + 1; //for the newline character
        curr1 = curr1->next;
    }
    char * str_of_users = malloc(sizeof(char)*(str_size+1)); //for the null terminator
    if (str_of_users == NULL) {
        fprintf(stderr, "fail to make string for list of users\n");
        return NULL;
    }
    //want to use strcpy first to make sure that the memory reserved
    //are freed from any previous values that may be stored there
    strcpy(str_of_users, curr->name);
    curr = curr->next;

    strcat(str_of_users, "\n");
    while (curr != NULL) {
        curr_name = curr->name;
        strcat(str_of_users, curr_name);
        strcat(str_of_users, "\n");
        curr = curr->next;
    }
    strcat(str_of_users, "\0");
    return str_of_users;

}



/*
 * Make two users friends with each other.  This is symmetric - a pointer to
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 *
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) {
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }
    printf("friends line 148\n");
    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }
    printf("friends line 157\n");
    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        }
    }
    printf("friends line 163\n");
    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }
    printf("friends line 167\n");
    user1->friends[i] = user2; printf("friends line 168\n");
    user2->friends[j] = user1; printf("friends line 169\n");
    return 0;
}




/*
 *  Print a post.
 *  Use localtime to print the time and date.
 */
char * print_post(const Post *post) {
    if (post == NULL) {
        return NULL;
    }

    return post->contents;
}


/*
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
char * print_user(const User *user) {
    if (user == NULL) {
        return NULL;
    }
    int name_len = strlen(user->name);
    int len_of_name_section = strlen("Name: ") + name_len + strlen("\n\n") +
    strlen("------------------------------------------\n");
    int len_of_friends_section = strlen("Friends:\n");
    const User * curr_user = user;
    int i;
    for (i=0; i<MAX_FRIENDS;i++) {
        if (curr_user->friends[i] != NULL) {
            len_of_friends_section = len_of_friends_section +
            strlen((const char*) (curr_user->friends[i]))+1;}
        else {break;}

    }
    len_of_friends_section = len_of_friends_section +
    strlen("------------------------------------------\n");
    int len_of_posts_section = strlen("Posts:\n");
    Post * post = curr_user->first_post;
    if (post == NULL) {
        len_of_posts_section = len_of_posts_section +
        strlen("------------------------------------------\n");
    }
    else {
        while (post != NULL) {
            len_of_posts_section = len_of_posts_section + strlen("From: ") +
            strlen(post->author) + strlen("\n") + strlen("Date: ") +
            strlen(ctime(post->date)) + strlen("\n\n") + strlen(post->contents)
            + strlen("\n===\n");
            post = post->next;
        }
        len_of_posts_section = len_of_posts_section - strlen("===\n")
        + strlen("------------------------------------------\n");
    }
    int len_of_profile = len_of_name_section + len_of_friends_section +
    len_of_posts_section;
    char * profile_str = malloc(sizeof(char)*(len_of_profile+1)); //for null terminator
    if (profile_str == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    strcpy(profile_str, "Name: ");
    strcat(profile_str, curr_user->name);
    strcat(profile_str, "\n\n");
    strcat(profile_str, "------------------------------------------\n");
    strcat(profile_str, "Friends:\n");
    for (i=0; i<MAX_FRIENDS;i++) {
        if (curr_user->friends[i] != NULL)
        {strcat(profile_str, (const char*) curr_user->friends[i]);
            strcat(profile_str, "\n");}
        else
        {strcat(profile_str, "------------------------------------------\n");
            break;}
    }
    strcat(profile_str, "Posts:\n");
    post = curr_user->first_post;
    if (post==NULL) {
        strcat(profile_str, "------------------------------------------\n");
        strcat(profile_str, "\0");
    }
    else {
        while(post != NULL) {
            strcat(profile_str, "From: ");
            strcat(profile_str, post->author);
            strcat(profile_str, "\n");
            strcat(profile_str, "Date: ");
            strcat(profile_str, ctime(post->date));
            strcat(profile_str, "\n");
            strcat(profile_str, post->contents);
            strcat(profile_str, "\n");
            if (post->next != NULL) {
                strcat(profile_str, "===\n");}
            post = post->next;
        }

        strcat(profile_str, "------------------------------------------\n");
    }
    strcat(profile_str, "\0");
    return profile_str;
}


/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Use the 'time' function to store the current time.
 *
 *  *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (target == NULL || author == NULL) {
        return 2;
    }

    int friends = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }
    if (friends == 0) {
        return 1;
    }

    // Create post
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;
    new_post->date = malloc(sizeof(time_t));
    if (new_post->date == NULL) {
        perror("malloc");
        exit(1);
    }
    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;

    return 0;
}
