#ifndef FLEETWAVE_H
#define FLEETWAVE_H

#include <QString>

namespace fleetwave 
{
    typedef enum { FLEETWAVE_OK,        // Returned when everything is OK
                   FLEETWAVE_ERROR,     // Returned when Fleetwave returns a value other than 1
                                        // Meaning successful communication with the Fleetwave
                                        // interface but incorrect data
                   FLEETWAVE_FAILED     // Returned when there is a failure in the communication channel
                                        // Could be a failure in the Python script, a failure in the network,
                                        // etc.
                   } FLEETWAVE_RETURN_TYPE;

    FLEETWAVE_RETURN_TYPE SendTakeRequest(QString code, QString& lockNum);
    FLEETWAVE_RETURN_TYPE SendTakeComplete(QString code, QString lockNum);
    FLEETWAVE_RETURN_TYPE SendReturnRequest(QString code, QString& lockNum, QString& question1, QString& question2, QString& question3);
    FLEETWAVE_RETURN_TYPE SendReturnComplete(QString lockNum, QString answer1, QString answer2, QString answer3);
}

#endif
