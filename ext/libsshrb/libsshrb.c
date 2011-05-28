#include <libsshrb.h>

VALUE rb_mlibssh;
VALUE rb_clibssh_connection;

VALUE rb_elibssh_error;
VALUE rb_elibssh_connection_error;

int set_ssh_options_iterator(VALUE, VALUE, VALUE);

void Init_libsshrb()
{
  rb_mlibssh = rb_define_module("LibSSH");
  Init_rb_libssh_constants();
  Init_rb_clibssh_connection();
}

void Init_rb_libssh_constants()
{
  rb_elibssh_error = rb_define_class_under(rb_mlibssh, "Error", rb_eStandardError);
  rb_elibssh_connection_error = rb_define_class_under(rb_mlibssh, "ConnectionError", rb_elibssh_error);
}

void Init_rb_clibssh_connection()
{
  rb_clibssh_connection = rb_define_class_under(rb_mlibssh, "Connection", rb_cObject);
  rb_define_alloc_func(rb_clibssh_connection, alloc_rb_libssh_connection);
  rb_define_method(rb_clibssh_connection, "initialize", initialize_rb_clibssh_connection, 2);
  rb_define_method(rb_clibssh_connection, "connected?", rb_clibssh_connection_connected_q, 0);
  rb_define_method(rb_clibssh_connection, "hostname", rb_clibssh_connection_hostname, 0);
  rb_define_method(rb_clibssh_connection, "connect", rb_clibssh_connection_connect, 0);
}

VALUE rb_clibssh_connection_connected_q(VALUE self)
{
  RB_SSH_CONNECTION *rb_ssh_connection;
  Data_Get_Struct(self, RB_SSH_CONNECTION, rb_ssh_connection);
  if(rb_ssh_connection->connected == 1){
    return Qtrue;
  }else{
    return Qfalse;
  }
}

VALUE rb_clibssh_connection_hostname(VALUE self)
{
  RB_SSH_CONNECTION *rb_ssh_connection;
  Data_Get_Struct(self, RB_SSH_CONNECTION, rb_ssh_connection);
  VALUE rb_hostname_string = ENCODED_STR_NEW2(rb_ssh_connection->hostname, "UTF-8");
  return rb_hostname_string;
}

VALUE initialize_rb_clibssh_connection(VALUE self, VALUE hostname, VALUE options)
{
  char* str;
  RB_SSH_CONNECTION *rb_ssh_connection;
  Data_Get_Struct(self, RB_SSH_CONNECTION, rb_ssh_connection);
  str = StringValuePtr(hostname);
  rb_ssh_connection->hostname = str;
  /* iterate through the hash and set the ssh options */
  ssh_options_set(rb_ssh_connection->libssh_session, SSH_OPTIONS_HOST, rb_ssh_connection->hostname);
  ssh_options_set(rb_ssh_connection->libssh_session, SSH_OPTIONS_USER, "root");
  rb_hash_foreach(options, set_ssh_options_iterator, self);
  return rb_clibssh_connection;
}

VALUE alloc_rb_libssh_connection(VALUE class)
{
  RB_SSH_CONNECTION *rb_ssh_connection = malloc(sizeof(RB_SSH_CONNECTION));
  rb_ssh_connection->libssh_session = ssh_new();
  if(rb_ssh_connection->libssh_session == NULL)
    rb_fatal("Unable to alloc at c function ssh_new()");
  VALUE obj = Data_Wrap_Struct(class, 0, free_rb_libssh_connection, rb_ssh_connection);
  return obj;
}

void free_rb_libssh_connection(RB_SSH_CONNECTION *rb_ssh_connection)
{
  ssh_free(rb_ssh_connection->libssh_session);
  free(rb_ssh_connection);
}

VALUE rb_clibssh_connection_connect(VALUE self)
{
  int rc;
  RB_SSH_CONNECTION *rb_ssh_connection;
  Data_Get_Struct(self, RB_SSH_CONNECTION, rb_ssh_connection);
  ssh_session session = rb_ssh_connection->libssh_session;

  if( ssh_connect(session) == SSH_ERROR )
    rb_raise(rb_elibssh_connection_error, "Error establishing Connection");


  fprintf(stderr, "Success would be: %i\n", SSH_AUTH_SUCCESS);
  rc = ssh_userauth_autopubkey(session, NULL);
  if( rc == SSH_AUTH_SUCCESS )
  {

  } else {
    rb_raise(rb_elibssh_connection_error, "Error authenticating");
  }

  fprintf(stderr, "Actual return code is: %i\n", rc);
  fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
  return Qtrue;
}

int set_ssh_options_iterator(VALUE opt_key, VALUE opt_value, VALUE self)
{
  int success;
  char *key = rb_id2name(SYM2ID(opt_key));
  RB_SSH_CONNECTION *rb_ssh_connection;
  ssh_session session;

  Data_Get_Struct(self, RB_SSH_CONNECTION, rb_ssh_connection);
  session = rb_ssh_connection->libssh_session;

  if ( strcmp(key, "port") == 0)
  {
    printf("will set port to: %d\n", NUM2INT(opt_value));
    int port = NUM2INT(opt_value);
    success = ssh_options_set(session, SSH_OPTIONS_PORT, &port);
  } else {
    rb_raise(rb_eStandardError, "unrecognized option: %s\n", key);
  }
  return success;
}

