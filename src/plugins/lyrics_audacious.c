#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <audacious/audctrl.h>  /* provided by audacious */
#include <audacious/dbus.h>  /* provided by audacious */

DBusGProxy *dbus_proxy = NULL;
static DBusGConnection *connection = NULL;

static void audtool_connect(void)
{
        GError *error = NULL;

	g_type_init();
        connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);

        if (connection == NULL)
                fprintf(stderr, "D-Bus Error: %s", error->message);

        dbus_proxy = dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE,
                                           AUDACIOUS_DBUS_PATH,
                                           AUDACIOUS_DBUS_INTERFACE);
}

void set_time(DBusGProxy *session, int time)
{
  gint ptime = time * 1000;
  gint pos = audacious_remote_get_playlist_pos( session );
  gint length = audacious_remote_get_playlist_time( session, pos );

  /* check bounds */
  if ( ptime < 0 ) {
    ptime = 0;
  } else if ( ptime > length ) {
	ptime = length;
  }
  
  audacious_remote_jump_to_time( session, ptime );
}

/*
 * launch a new audacious and return the session number
 * exit if error
 */
DBusGProxy *launch(void)
{
	unsigned int tries;

	switch( fork() ) {
		case -1:
			exit(-1);

		case 0:
			execlp("audacious", "audacious", NULL);
			exit(1);

		default:
			for( tries = 0 ; tries < 10 ; tries++ ) {
				usleep( 500000 ); /* in usec */
				dbus_proxy = dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE,
						AUDACIOUS_DBUS_PATH,
						AUDACIOUS_DBUS_INTERFACE);
				if ( audacious_remote_is_running( dbus_proxy ) )
					return dbus_proxy;
			}
			exit(-2); /* if no session found, abort */
	}
}

DBusGProxy *is_on()
{
	if(dbus_proxy == NULL)
		audtool_connect();
	if(audacious_remote_is_running(dbus_proxy ))
		return dbus_proxy;
	return NULL;
}

int get_time(DBusGProxy *session)
{
	return audacious_remote_get_output_time(session)/1000 ;
}

char* get_song(DBusGProxy *session)
{
	  char *song;
	  int posi;

	  posi = audacious_remote_get_playlist_pos( session );
	  song  = audacious_remote_get_playlist_file(session,posi);
	  return song;
}
