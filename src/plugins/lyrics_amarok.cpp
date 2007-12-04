
#include <kapp.h>
#include <dcopclient.h>
#include <unistd.h>
#include <iostream>

using namespace std;

static DCOPClient *dcop = 0L;
static char song[1024];

/*
 * Launch the application specialed by the name: app.
 * This code is fetched from the kdelibs.
 */
static bool launchApp(DCOPClient *dcop, QString app)
{
	// parse the arguments
	int l = app.length();
	if (l && (app[l-1] == '*'))
		l--;
	if (l && (app[l-1] == '-'))
		l--;
	if (!l) return false;
	app.truncate(l);

	// try to launch the application
	QStringList URLs;
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);
	arg << app << URLs;
	if ( !dcop->call( "klauncher", "klauncher",
				"start_service_by_desktop_name(QString,QStringList)",
				data, replyType, replyData) ) {
		qWarning( "call to klauncher failed.");
		return false;
	}

	// parse the return values
	QDataStream reply(replyData, IO_ReadOnly);
	if ( replyType != "serviceResult" )
	{
		qWarning( "unexpected result '%s' from klauncher.", replyType.data());
		return false;
	}
	int result;
	QCString dcopName;
	QString error;
	reply >> result >> dcopName >> error;
	if (result != 0)
	{
		qWarning("Error starting '%s': %s", app.local8Bit().data(),
				error.local8Bit().data());
		return false;
	}
	return true;
}

#ifdef __cplusplus
extern "C"
{
#endif

int is_on(void)
{
	if(!dcop){
		dcop = new DCOPClient;
		if(!dcop){
			cerr<<"Cann't create a DCOPClient."<<endl;
			return -1;
		}
		dcop->attach();
	}
	if(!dcop->isApplicationRegistered("amarok")){
		return -1;
	}

	return 0;
}

int launch(void)
{
	if(!launchApp(dcop,"amarok")){
		return -1;
	}
	/* wait for the amarok running */
	for(int i = 0; i < 10; i ++){
		usleep(500000);
		if(is_on() == 0) return 0;
	}

	return -1;
}

char* get_song(int session)
{
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);

	if (!dcop->call("amarok", "player", "path()", 
				data, replyType, replyData)){
		cerr<<"call to path() failed"<<endl;
		return 0L;
	}
	QDataStream reply(replyData, IO_ReadOnly);
	if (replyType != "QString"){
		return 0L;
	}
	QString replyString;
	reply >> replyString;
	strncpy(song, replyString.local8Bit().data(), sizeof(song));
	song[sizeof(song) - 1] = '\0';

	return song;
}

int get_time(int session)
{
	int seconds;
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);

	if(!dcop->call("amarok", "player", "trackCurrentTime()",
				data, replyType, replyData)){
		cerr<<"call to trackCurrentTime() failed"<<endl;
		return 0;
	}
	QDataStream reply(replyData, IO_ReadOnly);
	if(replyType != "int"){
		return 0;
	}
	reply >> seconds;

	return seconds;
}

void set_time(int session, int sec)
{
	int total;
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);

	if(!dcop->call("amarok", "player", "trackTotalTime()",
				data, replyType, replyData)){
		cerr<<"call to trackTotalTime() failed"<<endl;
		return;
	}
	QDataStream reply(replyData, IO_ReadOnly);
	if(replyType != "int"){
		return;
	}
	reply >> total;
	if(sec <= 0){
		return;
	}else if(sec > total){
		sec = total;
	}
	arg << sec;
	if(!dcop->call("amarok", "player", "seek(int)",
				data, replyType, replyData)){
		cerr<<"call to seek(int s) failed"<<endl;
		return;
	}
	return;
}

#ifdef __cplusplus
} // end of extern "C"
#endif

#ifdef DEBUG
int main(int argc, char **argv)
{
	char *song;

	if(is_on() < 0){
		if(launch() < 0){
			cerr<<"launch failed"<<endl;
		}
	}
	song = get_song(0);
	if(song){
		cout<<song<<endl;
	}
	cout<<get_time(0)<<endl;
	set_time(0, 10);

	return 0;
}
#endif
