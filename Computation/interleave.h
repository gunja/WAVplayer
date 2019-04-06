/**
    \file interleave.h
*/

#include <QObject>

#include <cstddef>
#include <list>

struct __sourceDefine {
    enum __sourceType {
        LOCAL = 0,
        REMOTE } srcType;
    std::string path;
    int port;
    int remoteId;
};

class multiPointReceiver {
    Q_OBJECT
    std::list<__sourceDefine> sources;

  public:
    multiPointReceiver( QObject * par);
    ~multiPointReceiver();

    bool attachToBuffer( bufferedStream *);
    bool detachFromBuffer();

    void setSources( const std::list<__sourceDefine>&);
};

