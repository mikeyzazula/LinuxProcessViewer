
/*Michael Zazula
5/17/16
Assign. 3 
CS352 - Bover */

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#define MAX_LINE 100
#define MAX_SIZE 4096
#define MAX_PIDS 4096
void get_pid(GtkListStore* store);
char* get_proc_user(char* pid);
float get_proc_mem(char* pid);
char* get_proc_name(char* pid);
float get_proc_cput (char* pid);
int num_of_pid = 0;
float calculate_cpu();
GtkWidget *treeview;
int stored_pids[MAX_PIDS];
int number_of_pids = 0;
int new_pids[MAX_PIDS] = {0}; ///used for adding and removing pids from our structure
int new_number_of_pids = 0;

void memory_display (GtkTreeViewColumn* col, 
                       GtkCellRenderer* renderer,
                       GtkTreeModel* model,
                       GtkTreeIter* iter,
                       gpointer data );
void cpu_display (GtkTreeViewColumn* col, 
                       GtkCellRenderer* renderer,
                       GtkTreeModel* model,
                       GtkTreeIter* iter,
                       gpointer data );
float proc_tick_array[50000];
float cpu_tick_array[50000]; // Not sure if this is the correct way to go about handling the pids and indexing
enum {
    PROC_NAME = 0,
    USER,
    CPU,
    PID,
    MEMORY,
    IMAGE,
    COLUMNS
};




void build_treeview (GtkWidget *treeview) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    

    // add the procc to the treeview with image
            /* ~ the image ~ */
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", IMAGE, NULL);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", PROC_NAME,
                NULL); 
    
    gtk_tree_view_column_set_title(column, "Proccess Name");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    gtk_tree_view_column_set_sort_column_id(column, PROC_NAME);
    gtk_tree_view_column_set_sort_indicator(column, TRUE);
  
    // add the user to the treeview with image
            /* ~ the image ~ */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes (
                "Username", renderer, "text", USER,
                NULL); 
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    gtk_tree_view_column_set_sort_column_id(column, USER);
    gtk_tree_view_column_set_sort_indicator(column, TRUE);
   
     // add the cpu to the treeview
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new(); 
    gtk_tree_view_column_set_title (column, "CPU%");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, renderer, cpu_display,
                                             NULL, NULL);
    gtk_tree_view_column_set_sort_column_id(column, CPU);
    gtk_tree_view_column_set_sort_indicator(column, TRUE);    
    
    // add the PID to the treeview
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes (
                "ID", renderer, "text", PID,
                NULL); 
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    gtk_tree_view_column_set_sort_column_id(column, PID);
    gtk_tree_view_column_set_sort_indicator(column, TRUE);

    // add the memory to the treeview
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new(); 
    gtk_tree_view_column_set_title (column, "Memory (MiB)");
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, renderer, memory_display,
                                             NULL, NULL);
    gtk_tree_view_column_set_sort_column_id(column, MEMORY);
    gtk_tree_view_column_set_sort_indicator(column, TRUE);    

  

}


void display (GtkWidget *treeview) {
    
    // create the window
    GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Process Monitor");
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (window, 200, 200);
    g_signal_connect (window, "delete_event", gtk_main_quit, NULL);
    
    // create a scrolled window
    GtkWidget* scroller = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    
    // pack the containers
    gtk_container_add (GTK_CONTAINER (scroller), treeview);
    gtk_container_add (GTK_CONTAINER (window), scroller);
    gtk_widget_show_all (window);
}


void build_list(GtkListStore* store){
    char** pid_array = malloc(MAX_SIZE);
    char* proc = "/proc";
    DIR* dir;
    int i = 0;
    dir = opendir(proc);
    GtkTreeIter iter;
    GError *error = NULL;

    GdkPixbuf* image1 = gdk_pixbuf_new_from_file("metroid.png", &error);
    GdkPixbuf* image2 = gdk_pixbuf_new_from_file("samus.png", &error);
    GdkPixbuf* image;

    while(1){
        struct dirent *file;

        file = readdir(dir);
        if(! file){
            break;
        }
        char* filename = file -> d_name;
        if(isdigit(filename[0])){ //check if the first char in the name is a digit. if true we know thats a pid
            gtk_list_store_append (store, &iter);
            image = (strcmp(get_proc_user(file -> d_name), "root") == 0) ? image1 : image2;
            gtk_list_store_set (store, &iter, PROC_NAME,get_proc_name(file -> d_name),   
                    USER, get_proc_user(file -> d_name), 
                    CPU, 0.0,
                    PID, atoi(file -> d_name),
                    MEMORY,get_proc_mem(file -> d_name),
                    IMAGE, image,-1);  
            proc_tick_array[atoi(file -> d_name)] = get_proc_cput(file -> d_name);
            cpu_tick_array[atoi(file -> d_name)] = calculate_cpu() ; //   ****************
            stored_pids[i] = atoi(file -> d_name);
            i++;
            number_of_pids ++;
        }
    }

    closedir(dir);
}





char* get_proc_name(char* pid){
    char location[256];
    memset(location,0,sizeof(location)); //gurantee a clean array
    strcpy(location,"/proc");
    strcat(location,"/");
    strcat(location,pid);
    strcat(location,"/status");
    //printf("Location process name: %s\n",location );

  
    FILE *fp;
    fp = fopen(location, "r");
    char name[256];
    fgets(name, sizeof(name), fp);
    fclose(fp);
    
    memmove(name,name+6,strlen(name+6)+6); //skipping over the "Name: " part in the proc/status


    char* ret_name = malloc(sizeof(*name)+1);
    strcpy(ret_name,name);
   // printf("Process Name:%s\n",ret_name );
    

    return ret_name;
}

char* get_proc_user(char* pid){
    char location[256];
    memset(location,0,sizeof(location));

    strcpy(location,"/proc");
    strcat(location,"/");
    strcat(location,pid);
    strcat(location,"/status");
   // printf("Location user: %s\n",location );

    FILE *fp;
    int count = 0;
    fp = fopen(location, "r");
    char line[256];
    while(fgets(line, sizeof(line), fp) != NULL){
        if (count == 7){
            break;
        }
        else{
            count++;
        }       
    }
    memmove(line,line+5,strlen(line+5)+5); //skipping over the "Uid: "
    char uid[32];
    int id_count =0 ;
    int i = 0;
    sscanf(line,"%s",uid);
    fclose(fp);

    int uid_int = atoi(uid);
    struct passwd *user;

    user = getpwuid((uid_t)uid_int);
    char* name = (char*)user->pw_name;
    //printf("User: %s \n",name);

    return name; 
}

float get_proc_mem(char* pid){
    char location[512];
    strcpy(location,"/proc");
    strcat(location,"/");
    strcat(location,pid);
    strcat(location,"/statm");
    //printf("Location mem: %s\n",location );

    FILE *fp;
    fp = fopen(location, "r");
    char mem_line[MAX_SIZE];
    fgets(mem_line, sizeof(mem_line), fp);
    fclose(fp);

    unsigned long size = 0;
    unsigned long resident = 0;
    unsigned long share = 0;
    unsigned long text = 0;
    unsigned long lib = 0;
    unsigned long data = 0;
    unsigned long dt = 0;
    sscanf(mem_line, "%ld %ld %ld %ld %ld %ld %ld", &size,&resident,&share,&text,&lib,&data,&dt);

    // multiply pages by 4 to convert to kb
    float t_mem =  (((float)resident - (float)share)*4) /1000  ;


    return t_mem;
}
void memory_display (GtkTreeViewColumn* col, 
                       GtkCellRenderer* renderer,
                       GtkTreeModel* model,
                       GtkTreeIter* iter,
                       gpointer data )  {
    gfloat memory;
    gchar buf[32];
    gtk_tree_model_get (model, iter, MEMORY, &memory, -1);
    g_snprintf (buf, 20, "%.2f", memory);
    g_object_set (renderer, "text", buf, NULL);

}

void cpu_display (GtkTreeViewColumn* col, 
                       GtkCellRenderer* renderer,
                       GtkTreeModel* model,
                       GtkTreeIter* iter,
                       gpointer data )  {
    gfloat cpu;
    gchar buf[32];
    gtk_tree_model_get (model, iter, CPU, &cpu, -1);
    g_snprintf (buf, 20, "%.2f", cpu);
    g_object_set (renderer, "text", buf, NULL);

}

float get_proc_cput (char* pid){
    char location[256];
    memset(location,0,sizeof(location));
    strcpy(location,"/proc");
    strcat(location,"/");
    strcat(location,pid);
    strcat(location,"/stat");
    //printf("Location CPU: %s\n",location );

    FILE *fp;
    fp = fopen(location, "r");
    char mem_line[MAX_SIZE];
    fgets(mem_line, sizeof(mem_line), fp);
    fclose(fp);

    //using sscanf to grab the data we need to calculate CPU usage. It's a little(really) ugly, but works.
    int cu_pid = 0;
    char comm[32];
    char state;
    int ppid = 0;
    int pgrp = 0; 
    int session = 0;
    int tty_nr = 0;
    int tpgid = 0;
    unsigned flags;
    unsigned long minflt = 0;
    unsigned long cminflt = 0;
    unsigned long majflt = 0;
    unsigned long cmajflt = 0;
    unsigned long utime = 0;
    unsigned long stime = 0;
    sscanf(mem_line,"%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu",&cu_pid,comm,&state,&ppid,&pgrp,&session,&tty_nr,&tpgid,&flags,&minflt,&cminflt,&majflt,&cmajflt,&utime,&stime);

    //to calculate the total time, we have to add the user time(time) and the kernal time (stime)
    float total_time = utime + stime;
    //printf("utime:%lu stime:%lu total timel: %lu comm: %s \n",utime,stime,total_time,comm );

    return total_time;
}


float calculate_cpu(){

    //now we have to geth cpu ticks and use that as our denominator with the process ticks
    unsigned long user = 0;
    unsigned long nice = 0;
    unsigned long system = 0;
    unsigned long idle = 0;
    unsigned long iowait = 0;
    unsigned long irq = 0; 
    unsigned long softirq = 0 ;
    float cpu_time = 0.0;
    FILE *fp;
    fp = fopen("/proc/stat", "r");
    char mem_line[MAX_SIZE];
    fgets(mem_line, sizeof(mem_line), fp);
    fclose(fp);
    sscanf(mem_line,"%*s %lu %lu %lu %lu %lu %lu %lu",&user,&nice,&system,&idle,&iowait,&irq,&softirq);
    cpu_time = user + nice +idle+ system + irq + softirq; 


    return cpu_time; 
}




void add_procs(GtkTreeIter* iter, GtkListStore* store){
    GError *error = NULL;
    GdkPixbuf* image1 = gdk_pixbuf_new_from_file("metroid.png", &error);
    GdkPixbuf* image2 = gdk_pixbuf_new_from_file("samus.png", &error);
    GdkPixbuf* image;

    int create = 0; // our flag to know if we have to append a new row
    int j = 0, i = 0;
    if(new_pids[number_of_pids+1] != 0){
        //appending pid at iter
        printf("Appending pid: %d\n",new_pids[number_of_pids+1] );
        gtk_list_store_append (store,iter);
        char char_pid[32] = {0};
        sprintf(char_pid,"%d",new_pids[number_of_pids+1]);
        image = (strcmp(get_proc_user(char_pid), "root") == 0) ? image1 : image2;
        gtk_list_store_set (store, iter, PROC_NAME,get_proc_name(char_pid),   
                USER, get_proc_user(char_pid), 
                CPU, 0.0,
                PID, atoi(char_pid),
                MEMORY,get_proc_mem(char_pid),
                IMAGE, image,-1);  
        proc_tick_array[atoi(char_pid)] = get_proc_cput(char_pid);
        cpu_tick_array[atoi(char_pid)] = calculate_cpu() ; //   ****************
        number_of_pids ++;
        //advance the iter
        gtk_tree_model_iter_next(GTK_TREE_MODEL(store), iter);
    }
    

}

//doesn't actually remove, just checks for pids that are no longer in the directory and flags them for removal
int remove_procs(int pid){
    DIR* dir;
    dir = opendir("/proc");
    int pos = 0;
    new_number_of_pids = 0;
    memset(new_pids,0,sizeof(new_pids));
    while(1){
        struct dirent *file;
        file = readdir(dir);
        if(! file){
            break;
        }
        char* filename;
        filename = file -> d_name;
        if(isdigit(filename[0])){
            new_pids[pos] = atoi(file -> d_name);
            new_number_of_pids ++;
            pos ++;
        }

    }
    closedir(dir);
    int j = 0;
    for(j = 0; j < new_number_of_pids ; j++){
        if(pid == new_pids[j]){ //if true, we know the pid is still active and need to update it, if false, we delete it
            return 1;
        }

    }
    return 0;
}



static gboolean update_list(gpointer store){
    GtkTreeIter iter;
    gboolean valid;

    // start at beginning of the store
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store), &iter);

    while (valid) {

        // get the current CPU and the pid
        gint pid = 0;
        gtk_tree_model_get (store, &iter, PID, &pid, -1);
        
        //check for change in processes
        int flag = 0;

        flag = remove_procs((int)pid);
        if (flag == 0){
            gtk_list_store_remove(store,&iter);
            printf("Removing process: %d\n", pid);
            number_of_pids --;
        }
        else{
            //update everything 
            float memory;
            char user[32] = {0};
            char proc_name[64] = {0};
            unsigned long cpu = 0;
            char char_pid[32] = {0};
            //converting the pid to a char so we can pass it in to our old fuctions
            sprintf(char_pid,"%d",pid);
            gtk_tree_model_get (store, &iter, PROC_NAME, &proc_name, USER, &user, MEMORY,&memory,CPU, &cpu, -1);
            memory = get_proc_mem(char_pid);
                       
            //initialize our "new" times fpr CPU calculating 
            float new_time = 0;
            float new_cpu_time = 0;
            // calculate the CPU
            float old_time =proc_tick_array[pid]; //initial tick value process (reset to new tick value after)
            new_time = get_proc_cput(char_pid);  //grabbing new ticks
            float old_cpu_time = cpu_tick_array[pid]; //initial cpu tick value (reset to new tick value after)
            new_cpu_time = calculate_cpu(); //calculating current cpu ticks
            gfloat total_time = 0;
            //calculation for cpu 
            if(new_time-old_time != 0){
                total_time = ((new_time - old_time)*100)/(new_cpu_time - old_cpu_time);
            }

            //..and updating our "old" times @ pid index
            proc_tick_array[pid] = new_time;
            cpu_tick_array[pid] = new_cpu_time;

            
            // set the new values
            gtk_list_store_set (GTK_LIST_STORE(store), &iter, CPU, total_time,MEMORY,memory, -1);
        }

        
        // advance the iterator
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
    //add the new pids where theres not stuff at iter (where valid is no longer...valid)
    add_procs(&iter,GTK_LIST_STORE(store));
    //*stored_pids = *new_pids;
    memcpy(stored_pids,new_pids,sizeof(new_pids));

    return TRUE;
}






int main(int argc, char *argv[]){

    gtk_init (&argc, &argv);

    // build the list store from the file data 
    GtkListStore *store = gtk_list_store_new (COLUMNS, G_TYPE_STRING,
                                              G_TYPE_STRING, G_TYPE_FLOAT,
                                              G_TYPE_INT, G_TYPE_FLOAT,
                                              GDK_TYPE_PIXBUF);
    build_list(store); 

    // create the tree view of the list
    treeview = gtk_tree_view_new ();
    build_treeview(treeview);
    
    // add the tree model to the tree view
    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (store));
    
    // unreference the model so that it will be destroyed when the tree
    // view is destroyed
    g_object_unref (store);
    
    // display the tree view
    display (treeview);
    g_timeout_add (1000, (GSourceFunc) update_list, (gpointer)store); 


    gtk_main ();


    return 0;

}


