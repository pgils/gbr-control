#ifndef GBRCONTROL_H
#define GBRCONTROL_H

#include "gbrxml.h"
#include "gbrdatabasehandler.h"
#include "gbrsocketlistener.h"

#define		POLL_INTERVAL	30	// network poll interval in seconds

class gbrControl
{
public:
    gbrControl(const char* database, unsigned short port);
    virtual ~gbrControl();

private:
    gbrControl( const gbrControl& );
    gbrControl& operator=( const gbrControl& );

public:
    int	Run();
    void Stop();

private:
    bool				running;
    gbrDatabaseHandler	*db;
    gbrSocketListener	*listener;

    int HandleNewMessage();
    void PollNetwork();
};

extern gbrControl *gbrcontrol;

#endif // GBRCONTROL_H
