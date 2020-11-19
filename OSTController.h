#include "OSTClientSEQ.h"
#include "OSTClientFOC.h"


class OSTController : public QObject
{
    Q_OBJECT
public:
    OSTController();
    ~OSTController();
private:
    std::unique_ptr<OSTClientFOC> MyOSTClientFOC;
};
