#include "global.h"


static bool m_verbose = false;


void libmodbus_cpp::setVerbose(bool verbose)
{
    m_verbose = verbose;
}


bool libmodbus_cpp::isVerbose()
{

    return m_verbose;
}
