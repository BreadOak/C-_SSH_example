#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
 
int scp_read(ssh_session session);
int scp_receive(ssh_session session, ssh_scp scp);

int main()
{
  ssh_session my_ssh_session;
  ssh_channel channel;
  ssh_scp scp;
  int rc;
  int nbytes;
  int size, mode;
  char *filename, *buffer;
  char *password;
  // char buffer[256];
  
  ///// Open session and set options /////
  my_ssh_session = ssh_new();
  if (my_ssh_session == NULL)
    exit(-1);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "192.168.0.39");
  ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "user");
  
  // Connect to server
  rc = ssh_connect(my_ssh_session);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error connecting to localhost: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_free(my_ssh_session);
    exit(-1);
  }
 
  // Verify the server's identity
  // For the source code of verify_knownhost(), check previous example
  // if (verify_knownhost(my_ssh_session) < 0)
  // {
  //   ssh_disconnect(my_ssh_session);
  //   ssh_free(my_ssh_session);
  //   exit(-1);
  // }
 
  ///// Authenticate ourselves /////
  // password = getpass("Password: ");
  password = (char *)"nrmk2013";
  rc = ssh_userauth_password(my_ssh_session, NULL, password);
  if (rc != SSH_AUTH_SUCCESS)
  {
    fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    exit(-1);
  }

  // ///// Make Channel /////
  // channel = ssh_channel_new(my_ssh_session);
  // if (channel == NULL) return SSH_ERROR;
 
  // rc = ssh_channel_open_session(channel);
  // if (rc != SSH_OK)
  // {
  //   ssh_channel_free(channel);
  //   return rc;
  // }

  // ///// Request_exec /////
  // rc = ssh_channel_request_exec(channel, "cd release; sudo ./TCP_test_GUI 0 0 3 45");
  // if (rc != SSH_OK)
  // {
  //   ssh_channel_close(channel);
  //   ssh_channel_free(channel);
  //   return rc;
  // }

  // ssh_channel_send_eof(channel);
  // ssh_channel_close(channel);
  // ssh_channel_free(channel);

  scp = ssh_scp_new
    (my_ssh_session, SSH_SCP_READ, "release/CORE200_LoadTest_Data.csv");
  if (scp == NULL)
  {
    fprintf(stderr, "Error allocating scp session: %s\n",
            ssh_get_error(my_ssh_session));
    return SSH_ERROR;
  }
 
  rc = ssh_scp_init(scp);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error initializing scp session: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_scp_free(scp);
    return rc;
  }

  rc = ssh_scp_pull_request(scp);
  if (rc != SSH_SCP_REQUEST_NEWFILE)
  {
    fprintf(stderr, "Error receiving information about file: %s\n",
            ssh_get_error(my_ssh_session));
    return SSH_ERROR;
  }
 
  size = ssh_scp_request_get_size(scp);
  filename = strdup(ssh_scp_request_get_filename(scp));
  mode = ssh_scp_request_get_permissions(scp);
  printf("Receiving file %s, size %d, permissions 0%o\n",
          filename, size, mode);
  free(filename);
 
  buffer = (char *)malloc(size);
  if (buffer == NULL)
  {
    fprintf(stderr, "Memory allocation error\n");
    return SSH_ERROR;
  }
 
  ssh_scp_accept_request(scp);

  int r = 0;

  while (r < size) 
  {
      int st = ssh_scp_read(scp, buffer+r, size-r);
      r += st;
  }

  printf("Done\n");

  FILE*fp = fopen("CORE200_LoadTest_Data.csv","w");
 
  // write(fp, buffer, size);
  fwrite(buffer, size, 1, fp);
  free(buffer);
 
  rc = ssh_scp_pull_request(scp);
  if (rc != SSH_SCP_REQUEST_EOF)
  {
    fprintf(stderr, "Unexpected request: %s\n",
            ssh_get_error(my_ssh_session));
    return SSH_ERROR;
  }

  fclose(fp);

  ssh_scp_close(scp);
  ssh_scp_free(scp);

  ssh_disconnect(my_ssh_session);
  ssh_free(my_ssh_session);
}





