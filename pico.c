#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_DATA 200
#define MAX_ROW  110000


struct Address
{ 
  int id;
  int set;
  char name[MAX_DATA];
  char email[MAX_DATA];
};

struct Database
{
  struct Address rows[MAX_ROW];
};


struct Connection
{
  FILE *file;
  struct Database *db;
};

void die(const char *message){
  if(errno){
    perror(message);
  }
  else {
    printf("ERROR: %s\n",message);
  }
  exit(1);
}

void Address_print(struct Address *addr){
  printf("%d %s %s\n", addr->id, addr->name, addr->email);
}

void Database_load(struct Connection *conn){
  int rc = fread(conn -> db, sizeof(struct Database), 1, conn -> file);
  if (rc != 1) die("failed to load Database");
}

struct Connection *Database_open(const char *filename, char mode){
  struct Connection *conn = malloc(sizeof(struct Connection));
  if(conn == NULL) die("memory error");
  conn -> db = malloc(sizeof(struct Database));
  if(!conn -> db) die("memory error");
  if (mode == 'c'){
    conn -> file  = fopen(filename, "w");
  }
  else {
    conn -> file  = fopen(filename, "r+");
    if (conn -> file ){
      Database_load(conn);
    } 
  }
  if(!conn ->file){
    die("failed to open file");
  }
  return conn;
}

void Database_create(struct Connection *conn){
  int i = 0;
  for(i = 0; i< MAX_ROW; i++){
    struct Address v = {.id = i, .set = 0};
    conn ->db->rows[i] = v;
  }
}

void Database_write(struct Connection *conn){
  rewind(conn->file);
  int rc = fwrite(conn -> db, sizeof(struct Database), 1, conn -> file);
  if(rc  != 1) die("failed to write to Database");
  rc      = fflush(conn ->file);
  if(rc  != 0) die("failed to write to Database"); 
}

void Database_get(struct Connection *conn, int id){
  struct Address *addr = &conn->db->rows[id];
  if (addr == NULL) die("Couldnt get from database");
  if(addr->set){
    Address_print(addr);
  }
  else{
    die("id is not set");
  }
}

void Database_set(struct Connection *conn, int id, const char *name, const char *email){
  struct Address *addr = &(conn->db->rows[id]);
  if (addr->set) die("already set,please delete the row first!");
  addr->set = 1;
  strncpy(addr->name, name, MAX_DATA);
  strncpy(addr->email, email, MAX_DATA);
}

void Database_list(struct Connection *conn){
  int i = 0;
  struct Database *db = conn -> db;
  for(i = 0; i<MAX_ROW ; i++){
    struct Address *dat = &(db -> rows[i]);
    if(dat -> set){
      Address_print(dat);
    }
  }
}

void Database_close(struct Connection *conn){
  if(conn){
    if(conn->file) fclose(conn->file);
    if(conn->db) free(conn->db);
    free(conn);
  }
}

void Database_find(struct Connection *conn,const char *name){
  int i = 0;
  struct Address *addr = &(conn->db->rows);
  for (i = 0; i <= MAX_ROW; ++i){
    printf("Searching for record,looked %d rows\n",i+1);
    if(strcmp((addr+i)->name, name) == 0){
      printf("Found record!\n");
      Address_print(addr + i);
      return;
    }
  }
}

int main(int argc, char const *argv[])
{  
  // File Descriptors to be used

  int listen_fd, comm_fd;
  char str[100];
  
  // Struct to hold IP Address and Port Numbers
  struct sockaddr_in servaddr;
  
  // Each server needs to “listen” for connections. The above function creates a socket with AF_UNIX ( local unix port )
  // and of type SOCK_STREAM. Data from all devices wishing to connect on this socket will be redirected to listen_fd.
  printf("hi\n");
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);

  // Clear servaddr ( Mandatory ).
  bzero(&servaddr, sizeof(servaddr));

  // Set Addressing scheme to – AF_UNIX ( LOCAL )
  servaddr.sin_family = AF_INET;

  // Allow any IP to connect – htons(INADDR_ANY)
  servaddr.sin_addr.s_addr = htons(INADDR_ANY);

  // Listen on port 22000 – htons(11000)
  servaddr.sin_port = htons(22000);

  // Prepare to listen for connections from address/port specified in sockaddr
  bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  // Start Listening for connections , keep at the most 10 connection requests waiting.
  // If there are more than 10 computers wanting to connect at a time, the 11th one fails to.
  
  if(listen(listen_fd, 10) < 0) {
        perror("server: listen"); 
        exit(-1); 
    }  


  // Accept a connection from any device who is willing to connect, 
  // If there is no one who wants to connect , wait. A file descriptor is returned. 
  // This can finally be used to communicate , whatever is sent by the device accepted can be read from comm_fd, 
  // whatever is written to comm_fd is sent to the other device.
  comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);

  while(1){
    read(comm_fd,str,100);
    if(comm_fd > 0){
      if (strcmp(str,"quit\n\0") == 0){
        printf("exiting\n");
        shutdown(comm_fd, SHUT_WR);
        close();
        bzero(str, 100);
        comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
      }
      printf("Echoing back - %s \n",str);
    } 
    printf("%d\n",comm_fd);
    bzero(str, 100);
  }   

  char *filename          = argv[1];
  char action             = argv[2][0];
  struct Connection *conn = Database_open(filename, action);
  int id = 0;
  
  if(id >= MAX_ROW) die("there's not that many rows"); 
  switch(action){
    case 'c':
      Database_create(conn);
      Database_write(conn);
      break;
    case 'g':
      id = atoi(argv[3]);
      if(argc != 4) die("Need and id to get");
      Database_get(conn, id);
      break;
    case 's':
      id = atoi(argv[3]);
      if(argc !=6) die("need id, name, email to set");
      Database_set(conn, id, argv[4], argv[5]);
      Database_write(conn);
      break;
    case 'l':
      Database_list(conn);
      break;
    case 'f':
      Database_find(conn, argv[3]);
      break;
    default:
      die("Invalid action, only: c=create, g=get, s=set, d=del, l=list");

  }  
  Database_close(conn);
  return 0;
}