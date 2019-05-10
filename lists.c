#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lists.h"

/* Add a group with name group_name to the group_list referred to by 
* group_list_ptr. The groups are ordered by the time that the group was 
* added to the list with new groups added to the end of the list.
*
* Returns 0 on success and -1 if a group with this name already exists.
*
* (I.e, allocate and initialize a Group struct, and insert it
* into the group_list. Note that the head of the group list might change
* which is why the first argument is a double pointer.) 
*/
int add_group(Group **group_list_ptr, const char *group_name) {

    if (find_group(*group_list_ptr, group_name) != NULL) { // Check that the group is in the group_list
        return -1;  
    }

    Group *new_group = malloc(sizeof(Group));

    if (new_group == NULL){ // i.e. malloc failed
	perror("Error allocating memory for new group. Exiting...");
	exit(1);
    }
   
    // Initialize Fields
    int name_length = (int) strlen(group_name); 
    new_group->name = malloc(name_length + 1);
    if (new_group == NULL){
	perror("Error allocating memory for group name. Exiting...");
	exit(1); 
    }    

    strncpy(new_group->name, group_name, name_length);
    new_group->name[name_length] = '\0';

    new_group->users = NULL; // Next three will be NULL since Group hasn't been initialized
    new_group->xcts = NULL;
    new_group->next = NULL;

    // Now, let's insert into list.
    // First, check that there actually are elements in the list
    
    if (*group_list_ptr == NULL) {
	*group_list_ptr = new_group; // Point to this new group, group added!
        return 0;
    }
    
    Group *current = *group_list_ptr; // To iterate over the list

    while (current->next != NULL) { // Go to the very last element of the list
        current = current->next;
    }

    current->next = new_group; // And add the new list!
    return 0;
}

/* Print to standard output the names of all groups in group_list, one name
*  per line. Output is in the same order as group_list.
*/
void list_groups(Group *group_list) {

    if (group_list == NULL) { // If list is empty,    
	printf("No groups have been added yet \n"); 
	return;
    }    

    Group *current = group_list; // To iterate over the list	    

    while (current != NULL){
	printf("%s \n", current->name);
	current = current->next;
    }
    
}

/* Search the list of groups for a group with matching group_name
* If group_name is not found, return NULL, otherwise return a pointer to the 
* matching group list node.
*/
Group *find_group(Group *group_list, const char *group_name) {
    
    // First, we make sure that the list is not empty
    if (group_list == NULL){
	return NULL;
    }

    Group *current = group_list; // To iterate over the list
    
    while (current != NULL){
	if (strcmp(current->name, group_name) == 0){
	    return current;
	}
	current = current->next;
    }

    return NULL;
}

/* Add a new user with the specified user name to the specified group. Return zero
* on success and -1 if the group already has a user with that name.
* (allocate and initialize a User data structure and insert it into the
* appropriate group list)
*/
int add_user(Group *group, const char *user_name) {
    
    // We assume that the group actually exists
    // Check that user_name is not already in group
    if (find_prev_user(group, user_name) != NULL) { // find_prev_user returns a pointer if found, NULL if not found
	return -1;
    } 

    User * new_user = malloc(sizeof(User)); // Setup the user 
    if (new_user == NULL){ // i.e. malloc failed
        perror("Error allocating memory for new user. Exiting...");
	exit(1);
    }
    
    // Initialize fields
    new_user->balance = 0.00;
    
    // Setup name
    int name_length = (int) strlen(user_name);
    new_user->name = malloc(name_length + 1);
    if (new_user->name == NULL){
        perror("Error allocating memory for user's name. Exiting...");
	exit(1);
    }
    strncpy(new_user->name, user_name, name_length); 
    new_user->name[name_length] = '\0';
    
    // Now, to add the new_user to the front of the group (it has the lowest balance)
    new_user->next = group->users;
    group->users = new_user;
    // User added!
    return 0;
}

/* Remove the user with matching user and group name and
* remove all her transactions from the transaction list. 
* Return 0 on success, and -1 if no matching user exists.
* Remember to free memory no longer needed.
* (Wait on implementing the removal of the user's transactions until you 
* get to Part III below, when you will implement transactions.)
*/
int remove_user(Group *group, const char *user_name) {
    
    // Check whether user_name exists in list
    User * prev_user = find_prev_user(group, user_name);
    if (prev_user == NULL) {
	return -1;
    }

    // Now let us find the user that will be removed
    // Note that to_be_removed is the pointer to user_name, the node that will be removed from the linked list
    User * to_be_removed;
    if (strcmp(prev_user->name, user_name) == 0) { // For when user_name is the first in the list
    	to_be_removed = prev_user;
	group->users = prev_user->next; // We need users to refer to the node that is next, effectively removing user_name from users list
    }

    else {
        to_be_removed = prev_user->next;
    }
    
    User * new_next = to_be_removed->next;

    // Now if new_next is null, then to_be_removed was the last node in the list
    if (new_next == NULL){
        prev_user->next = NULL;
    }
    else { // Otherwise just form the missing link
	prev_user->next = new_next;  
    }

    // Now to free the memory occupied by to_be_removed
    free(to_be_removed->name);
    free(to_be_removed);
    // And remove the appropriate transactions done by user_name
    remove_xct(group, user_name);    
    return 0;
}

/* Print to standard output the names of all the users in group, one
* per line, and in the order that users are stored in the list, namely 
* lowest payer first.
*/
void list_users(Group *group) {
    
    // If list is empty print newline
    if (group->users == NULL){
	printf("There are no users in group %s. \n", group->name);
	return;
    }

    User *user = group->users; // To iterate over the list

    printf("Name \t Balance \n");
    while (user != NULL){ 
	printf("%s \t %.2f \n", user->name, user->balance);
	user = user->next;
    }
}

/* Print to standard output the balance of the specified user. Return 0
* on success, or -1 if the user with the given name is not in the group.
*/
int user_balance(Group *group, const char *user_name) {

    // Assuming group exists...
    // Check whether user_name is absent from the list
    User * prev_user = find_prev_user(group, user_name);
    if (prev_user == NULL) {
	return -1;
    }

    // If we got here, then user_name exists. Print the balance
    // Since find_prev_user can return the actual user_name to prev_user when the list consists of just user_name
    printf("User \t Balance \n");
    if (strcmp(prev_user->name, user_name) == 0){
	printf("%s \t %.2f \n", prev_user->name, prev_user->balance);
    }
    else {
	printf("%s \t %.2f \n", prev_user->next->name, prev_user->next->balance);
    }
    return 0; 
}

/* Print to standard output the name of the user who has paid the least 
* If there are several users with equal least amounts, all names are output. 
* Returns 0 on success, and -1 if the list of users is empty.
* (This should be easy, since your list is sorted by balance). 
*/
int under_paid(Group *group) {

    // First check whether the users list is empty
    if (group->users == NULL){
	return -1;
    }

    User * current_user = group->users; // Special Case: Only one user registered in group
    if (current_user->next == NULL){ 
	printf("%s \n", current_user->name);
	return 0;
    }

    if (current_user->balance != current_user->next->balance) {  // This means the first node has underpaid the most (i.e. no tie)
	printf("%s \n", current_user->name);
	return 0;
    }

    // If we got to this point, then there is a tie between several users 
    printf("%s \n", current_user->name);
    while (current_user->next != NULL && current_user->balance == current_user->next->balance){ // To find all users who tie 
	printf("%s \n", current_user->next->name);
	current_user = current_user->next;
    }
    return 0;
}

/* Return a pointer to the user prior to the one in group with user_name. If 
* the matching user is the first in the list (i.e. there is no prior user in 
* the list), return a pointer to the matching user itself. If no matching user 
* exists, return NULL. 
*
* The reason for returning the prior user is that returning the matching user 
* itself does not allow us to change the user that occurs before the
* matching user, and some of the functions you will implement require that
* we be able to do this.
*/
User *find_prev_user(Group *group, const char *user_name) {
    
    // We assume that group exists i.e. is not NULL 
    // First, we will check if the list is empty
    if (group->users == NULL){
        return NULL;
    }
     
    User * current_user = group->users;

    // Now, check whether the first user in out list corresponds to user_name
    if (strcmp(current_user->name, user_name) == 0){
	return current_user;
    }

    User * prev_user = current_user;
    while (current_user->next != NULL) { // We will traverse the linked list to find user_name
	current_user = current_user->next;
       	if ((strcmp(current_user->name, user_name) == 0)) {
	    return prev_user;
	}
        prev_user = current_user; // If not found, continue traversal
    }
    
    // If we have gotten to this point of the algorithm, then the user_name is not present in the group
    return NULL;
}

/* Rearrange the list of users found in group after current_user's balance
* was changed due to a new transaction. The first user in the list must have
* the lowest balance compared to all other users. Userd as a helper function
* in add_xct.
*/
void rearrange_users(Group *group, User *current_user){
    
    User * prev_user, *next_user; 

    while (current_user->next != NULL && current_user->balance > current_user->next->balance){
	// We will excecute a swap if we entered the loop
        // Swaps must be done in different ways depending on the position in the list
	prev_user = find_prev_user(group, current_user->name);	
        next_user = current_user->next;
       
	if (strcmp(prev_user->name, current_user->name) == 0){ // i.e. if current_user is the first user in the linked list
            current_user->next = next_user->next; // Now current_user refers to the third node in the linked list
            next_user->next = current_user; 
            group->users = next_user;  // Swap complete!
        }
        else {
            current_user->next = next_user->next;
            prev_user->next = next_user; 		
            next_user->next = current_user; // Swap complete!
        }
    }
 
}

/* Add the transaction represented by user_name and amount to the appropriate 
* transaction list, and update the balances of the corresponding user and group. 
* Note that updating a user's balance might require the user to be moved to a
* different position in the list to keep the list in sorted order. Returns 0 on
* success, and -1 if the specified user does not exist.
*/
int add_xct(Group *group, const char *user_name, double amount) {
   
    // Assuming a negative amount is NOT a problem...
    User * prev_user = find_prev_user(group, user_name);
    // First, check that the user_name exists in the group
    if (prev_user == NULL) {
	return -1;
    }

    User * current_user; // Find the desired username to update their balance. Prev_user will be current_user if it is first in list.
    if (strcmp(prev_user->name, user_name) == 0){
	current_user = prev_user;
    } 
    else {
	current_user = prev_user->next;
    }

    // Update current_user's information
    current_user->balance += amount;
    // Rearrange the list to reflect changes in balance
    rearrange_users(group, current_user);
 
    // Now to set up the new transaction in the xct list
    Xct * new_xct = malloc(sizeof(Xct));
    if (new_xct == NULL){
	perror("Error allocating memory for new transaction. Exiting...");
	exit(1);
    }

    new_xct->amount = amount;
    
    // Setup name
    int name_length = (int) strlen(user_name);
    new_xct->name = malloc(name_length + 1);
    if (new_xct->name == NULL){
        perror("Error allocating memory for new transaction name. Exiting...");
	exit(1);
    }
 
    strncpy(new_xct->name, user_name, name_length);
    new_xct->name[name_length] = '\0';
    
    // Now to add to front of xct list (Better for recent_xct)
    new_xct->next = group->xcts;
    group->xcts = new_xct;
    return 0;
}

/* Print to standard output the num_xct most recent transactions for the 
* specified group (or fewer transactions if there are less than num_xct 
* transactions posted for this group). The output should have one line per 
* transaction that prints the name and the amount of the transaction. If 
* there are no transactions, this function will print nothing.
*/
void recent_xct(Group *group, long num_xct) {
   
    // First, check if xct_list is empty
    int desired_number = (int) num_xct;
    if (group->xcts == NULL || num_xct <= 0){ // No negative numbers!
	printf("\n");
	return;
    }
    
    int i = 0;
    Xct * current_xct = group->xcts;
    printf("The last %i transactions were: \n", desired_number);
    while (current_xct->next != NULL && i < desired_number){
	printf("Transaction #%i, User: %s, Transaction amount: %.2f \n", i + 1, current_xct->name, current_xct->amount);
        current_xct = current_xct->next;
        i++;
    }
    
    if (i < desired_number) { // To print the last transaction, if required
	printf("Transaction #%i, User: %s, Transaction amount: %.2f \n", i + 1, current_xct->name, current_xct->amount);
    }
}

/* Return a pointer to the xct prior to the one in group with current_xct
* in it. If the matching xct is the first in the list, return a pointer 
* to the matching xct itself. If no matching xct is found, return NULL.
* Used as a helper function for remove_xct.
*/
Xct * find_prev_xct(Group *group, const char *xct_name) {

    // First check if transactions are empty
    if (group->xcts == NULL){
	return NULL;
    }
    
    Xct * current_xct = group->xcts;
    // This will essentially be a copy paste of find_prev_user, but with xct instead 
    if (strcmp(current_xct->name, xct_name) == 0){
	return current_xct;
    }

    Xct * prev_xct = current_xct;
    while (current_xct->next != NULL){
	current_xct = current_xct->next;
	if (strcmp(current_xct->name, xct_name) == 0){
	    return prev_xct;
        }
        prev_xct = current_xct;
    }
    
    return NULL;
}


/* Remove all transactions that belong to the user_name from the group's 
* transaction list. This helper function should be called by remove_user. 
* If there are no transactions for this user, the function should do nothing.
* Remember to free memory no longer needed.
*/
void remove_xct(Group *group, const char *user_name) {
     
     void rearrange_xct(Group *group, Xct * current_xct);
     // We assume that user_name is in group
     // First, check if there are transactions in the xct_list
     if (group->xcts == NULL){
	return;
     }

     Xct * current_xct = group->xcts;
     while (current_xct != NULL){
     if (strcmp(current_xct->name, user_name) == 0){ // This transaction was done by the user
         rearrange_xct(group, current_xct);
     }	
     current_xct = current_xct->next; 
     }
}

/* Helper function for remove_xct. Take the current transaction, and delete
* it from the xct_list in group. Finally, free the memory initially allocated
* to current_xct.
*/
void rearrange_xct(Group * group, Xct * current_xct) {
	
    Xct *prev_xct, *next_xct;
    prev_xct = find_prev_xct(group, current_xct->name);
    next_xct = current_xct->next;
    if (strcmp(prev_xct->name, current_xct->name) == 0) { // We assume prev_xct is not null as we know current_xct is in the list
         group->xcts = next_xct;
    }
    else {
	 prev_xct->next = next_xct; 
    }
    // Free space allocated for name and xct
    free(current_xct->name);
    free(current_xct);
}



